#include "PeterCore/CreatorContentStore.h"

#include "PeterCore/StructuredStore.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <utility>

namespace Peter::Core
{
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

  CreatorArtifactReadResult CreatorContentStore::ReadArtifactChecked(
    const CreatorContentKind kind,
    const std::string& contentId,
    const int revision) const
  {
    CreatorArtifactReadResult result;
    result.revision = revision < 0 ? ResolveLatestRevision(kind, contentId) : revision;
    if (result.revision <= 0)
    {
      result.valid = true;
      result.message = "Artifact does not exist yet.";
      return result;
    }

    result.sourcePath = ArtifactPath(kind, contentId, result.revision);
    const auto parseResult = ParseStructuredFieldsFile(result.sourcePath);
    result.valid = parseResult.valid;
    result.fields = parseResult.fields;
    result.message = parseResult.valid ? "Artifact loaded successfully." : parseResult.error;

    if (!result.fields.empty())
    {
      m_eventBus.Emit(Event{
        EventCategory::CreatorTools,
        "creator_tools.content.read",
        {
          {"content_id", contentId},
          {"kind", std::string(ToString(kind))},
          {"path", result.sourcePath.string()},
          {"revision", std::to_string(result.revision)},
          {"valid", result.valid ? "true" : "false"}
        }});
    }

    return result;
  }

  StructuredFields CreatorContentStore::ReadArtifact(
    const CreatorContentKind kind,
    const std::string& contentId,
    const int revision) const
  {
    return ReadArtifactChecked(kind, contentId, revision).fields;
  }

  CreatorArtifactWriteResult CreatorContentStore::WriteArtifactWithResult(
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
    const auto writeResult = WriteStructuredFieldsFileAtomic(path, fields, {}, {});
    m_eventBus.Emit(Event{
      EventCategory::CreatorTools,
      "creator_tools.content.write",
      {
        {"content_id", contentId},
        {"kind", std::string(ToString(kind))},
        {"path", path.string()},
        {"revision", std::to_string(resolvedRevision)},
        {"success", writeResult.success ? "true" : "false"}
      }});

    return CreatorArtifactWriteResult{
      writeResult.success,
      resolvedRevision,
      writeResult.durationMs,
      writeResult.bytesWritten,
      writeResult.message,
      path};
  }

  int CreatorContentStore::WriteArtifact(
    const CreatorContentKind kind,
    const std::string& contentId,
    const StructuredFields& fields,
    const int revision) const
  {
    return WriteArtifactWithResult(kind, contentId, fields, revision).revision;
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

  CreatorArtifactRestoreResult CreatorContentStore::RestoreArtifactRevision(
    const CreatorContentKind kind,
    const std::string_view contentId,
    const int revision) const
  {
    CreatorArtifactRestoreResult result;
    result.restoredRevision = revision;
    if (revision <= 0 || !ArtifactExists(kind, contentId, revision))
    {
      result.message = "Revision does not exist.";
      return result;
    }

    const auto fields = ReadArtifact(kind, std::string(contentId), revision);
    const auto writeResult = WriteArtifactWithResult(kind, std::string(contentId), fields);
    result.success = writeResult.success;
    result.newRevision = writeResult.revision;
    result.message = writeResult.success ? "Artifact restored into a new revision." : writeResult.message;
    result.sourcePath = ArtifactPath(kind, contentId, revision);
    result.restoredPath = writeResult.path;
    return result;
  }

  CreatorContentHealthReport CreatorContentStore::InspectHealth() const
  {
    CreatorContentHealthReport report;
    for (const auto kind : {
           CreatorContentKind::TinkerPreset,
           CreatorContentKind::LogicRules,
           CreatorContentKind::TinyScript,
           CreatorContentKind::MiniMission})
    {
      const auto root = KindRoot(kind);
      if (!std::filesystem::exists(root))
      {
        continue;
      }

      for (const auto& entry : std::filesystem::directory_iterator(root))
      {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
        {
          continue;
        }

        ++report.checkedArtifacts;
        const auto parseResult = ParseStructuredFieldsFile(entry.path());
        if (!parseResult.valid)
        {
          report.healthy = false;
          report.invalidArtifactIds.push_back(entry.path().stem().string());
        }
      }
    }

    report.summary = report.healthy
      ? "All creator artifacts parsed successfully."
      : "One or more creator artifacts are invalid.";
    return report;
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
