#include "PeterCore/CreatorContentStore.h"

#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <utility>

namespace Peter::Core
{
  namespace
  {
    std::string EscapeJson(const std::string_view value)
    {
      std::string escaped;
      escaped.reserve(value.size());

      for (const char character : value)
      {
        switch (character)
        {
          case '\\':
            escaped += "\\\\";
            break;
          case '"':
            escaped += "\\\"";
            break;
          case '\n':
            escaped += "\\n";
            break;
          default:
            escaped += character;
            break;
        }
      }

      return escaped;
    }

    std::string UnescapeJson(const std::string_view value)
    {
      std::string unescaped;
      unescaped.reserve(value.size());

      bool escaping = false;
      for (const char character : value)
      {
        if (!escaping)
        {
          if (character == '\\')
          {
            escaping = true;
          }
          else
          {
            unescaped += character;
          }
          continue;
        }

        switch (character)
        {
          case 'n':
            unescaped += '\n';
            break;
          case '\\':
            unescaped += '\\';
            break;
          case '"':
            unescaped += '"';
            break;
          default:
            unescaped += character;
            break;
        }
        escaping = false;
      }

      return unescaped;
    }

    StructuredFields ParseFields(const std::filesystem::path& path)
    {
      StructuredFields fields;
      if (!std::filesystem::exists(path))
      {
        return fields;
      }

      std::ifstream input(path);
      const std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

      std::size_t cursor = 0;
      while (true)
      {
        const auto keyStart = content.find('"', cursor);
        if (keyStart == std::string::npos)
        {
          break;
        }

        const auto keyEnd = content.find('"', keyStart + 1);
        const auto valueStart = content.find('"', keyEnd + 1);
        const auto valueEnd = content.find('"', valueStart + 1);
        if (keyEnd == std::string::npos || valueStart == std::string::npos || valueEnd == std::string::npos)
        {
          break;
        }

        const auto key = UnescapeJson(std::string_view(content).substr(keyStart + 1, keyEnd - keyStart - 1));
        const auto value =
          UnescapeJson(std::string_view(content).substr(valueStart + 1, valueEnd - valueStart - 1));
        fields[key] = value;
        cursor = valueEnd + 1;
      }

      return fields;
    }

    void WriteFields(const std::filesystem::path& path, const StructuredFields& fields)
    {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream output(path, std::ios::trunc);
      output << "{";
      bool first = true;
      for (const auto& [key, value] : fields)
      {
        if (!first)
        {
          output << ",";
        }
        output << '"' << EscapeJson(key) << "\":\"" << EscapeJson(value) << '"';
        first = false;
      }
      output << "}\n";
    }
  } // namespace

  std::string_view ToString(const CreatorContentKind kind)
  {
    switch (kind)
    {
      case CreatorContentKind::TinkerPreset:
        return "tinker-presets";
      case CreatorContentKind::LogicRules:
        return "logic-rules";
      case CreatorContentKind::TinyScript:
        return "tiny-scripts";
      case CreatorContentKind::MiniMission:
        return "mini-missions";
      case CreatorContentKind::MentorReport:
        return "mentor-reports";
    }

    return "creator-content";
  }

  CreatorContentStore::CreatorContentStore(ProfileInfo profile, EventBus& eventBus)
    : m_profile(std::move(profile))
    , m_eventBus(eventBus)
  {
  }

  void CreatorContentStore::EnsureLayout() const
  {
    std::filesystem::create_directories(Root());
    std::filesystem::create_directories(KindRoot(CreatorContentKind::TinkerPreset));
    std::filesystem::create_directories(KindRoot(CreatorContentKind::LogicRules));
    std::filesystem::create_directories(KindRoot(CreatorContentKind::TinyScript));
    std::filesystem::create_directories(KindRoot(CreatorContentKind::MiniMission));
    std::filesystem::create_directories(KindRoot(CreatorContentKind::MentorReport));
  }

  std::filesystem::path CreatorContentStore::Root() const
  {
    return m_profile.root / "CreatorContent";
  }

  std::filesystem::path CreatorContentStore::KindRoot(const CreatorContentKind kind) const
  {
    return Root() / std::string(ToString(kind));
  }

  bool CreatorContentStore::ArtifactExists(
    const CreatorContentKind kind,
    const std::string_view contentId,
    const int revision) const
  {
    const int resolvedRevision = revision < 0 ? ResolveLatestRevision(kind, contentId) : revision;
    if (resolvedRevision <= 0)
    {
      return false;
    }
    return std::filesystem::exists(ArtifactPath(kind, contentId, resolvedRevision));
  }

  StructuredFields CreatorContentStore::ReadArtifact(
    const CreatorContentKind kind,
    const std::string& contentId,
    const int revision) const
  {
    const int resolvedRevision = revision < 0 ? ResolveLatestRevision(kind, contentId) : revision;
    if (resolvedRevision <= 0)
    {
      return {};
    }

    const auto path = ArtifactPath(kind, contentId, resolvedRevision);
    auto fields = ParseFields(path);
    if (!fields.empty())
    {
      m_eventBus.Emit(Event{
        EventCategory::CreatorTools,
        "creator_tools.content.read",
        {
          {"content_id", contentId},
          {"kind", std::string(ToString(kind))},
          {"path", path.string()},
          {"revision", std::to_string(resolvedRevision)}
        }});
    }
    return fields;
  }

  int CreatorContentStore::WriteArtifact(
    const CreatorContentKind kind,
    const std::string& contentId,
    const StructuredFields& inputFields,
    const int revision) const
  {
    EnsureLayout();

    const int resolvedRevision = revision > 0 ? revision : ResolveLatestRevision(kind, contentId) + 1;
    StructuredFields fields = inputFields;
    fields["content_id"] = contentId;
    fields["kind"] = std::string(ToString(kind));
    fields["revision"] = std::to_string(resolvedRevision);

    const auto path = ArtifactPath(kind, contentId, resolvedRevision);
    WriteFields(path, fields);

    m_eventBus.Emit(Event{
      EventCategory::CreatorTools,
      "creator_tools.content.write",
      {
        {"content_id", contentId},
        {"kind", std::string(ToString(kind))},
        {"path", path.string()},
        {"revision", std::to_string(resolvedRevision)}
      }});
    return resolvedRevision;
  }

  std::vector<std::string> CreatorContentStore::ListArtifactIds(const CreatorContentKind kind) const
  {
    std::set<std::string, std::less<>> ids;
    const auto root = KindRoot(kind);
    if (!std::filesystem::exists(root))
    {
      return {};
    }

    for (const auto& entry : std::filesystem::directory_iterator(root))
    {
      if (!entry.is_regular_file())
      {
        continue;
      }

      const auto stem = entry.path().stem().string();
      const auto delimiter = stem.find("__v");
      if (delimiter == std::string::npos)
      {
        continue;
      }
      ids.insert(stem.substr(0, delimiter));
    }

    return {ids.begin(), ids.end()};
  }

  std::filesystem::path CreatorContentStore::WriteMentorSummary(
    const std::string_view reportId,
    const std::string_view body) const
  {
    EnsureLayout();
    const auto path = KindRoot(CreatorContentKind::MentorReport) / (std::string(reportId) + ".txt");
    std::ofstream output(path, std::ios::trunc);
    output << body << '\n';

    m_eventBus.Emit(Event{
      EventCategory::CreatorTools,
      "creator_tools.mentor_summary.exported",
      {
        {"path", path.string()},
        {"report_id", std::string(reportId)}
      }});
    return path;
  }

  int CreatorContentStore::ResolveLatestRevision(
    const CreatorContentKind kind,
    const std::string_view contentId) const
  {
    const auto root = KindRoot(kind);
    if (!std::filesystem::exists(root))
    {
      return 0;
    }

    const std::string prefix = std::string(contentId) + "__v";
    int latest = 0;
    for (const auto& entry : std::filesystem::directory_iterator(root))
    {
      if (!entry.is_regular_file())
      {
        continue;
      }

      const auto stem = entry.path().stem().string();
      if (!stem.starts_with(prefix))
      {
        continue;
      }

      const auto revisionText = stem.substr(prefix.size());
      latest = std::max(latest, std::stoi(revisionText));
    }

    return latest;
  }

  std::filesystem::path CreatorContentStore::ArtifactPath(
    const CreatorContentKind kind,
    const std::string_view contentId,
    const int revision) const
  {
    return KindRoot(kind) / (std::string(contentId) + "__v" + std::to_string(revision) + ".json");
  }
} // namespace Peter::Core
