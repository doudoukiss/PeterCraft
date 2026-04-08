#include "PeterCore/SaveDomainStore.h"

#include <fstream>
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
  } // namespace

  SaveDomainStore::SaveDomainStore(ProfileInfo profile, EventBus& eventBus)
    : m_profile(std::move(profile))
    , m_eventBus(eventBus)
  {
  }

  bool SaveDomainStore::DomainExists(const std::string_view domainId) const
  {
    return std::filesystem::exists(DomainPath(domainId));
  }

  StructuredFields SaveDomainStore::ReadDomain(const std::string& domainId) const
  {
    StructuredFields fields;
    const auto path = DomainPath(domainId);
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

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.domain.read",
      {
        {"domain_id", domainId},
        {"field_count", std::to_string(fields.size())},
        {"path", path.string()}
      }});

    return fields;
  }

  void SaveDomainStore::WriteDomain(const std::string& domainId, const StructuredFields& fields) const
  {
    const auto path = DomainPath(domainId);
    std::filesystem::create_directories(path.parent_path());

    {
      std::ofstream backup(m_profile.backupRoot / (domainId + ".bak.json"), std::ios::trunc);
      backup << "{";
      bool first = true;
      for (const auto& [key, value] : fields)
      {
        if (!first)
        {
          backup << ",";
        }
        backup << '"' << EscapeJson(key) << "\":\"" << EscapeJson(value) << '"';
        first = false;
      }
      backup << "}\n";
    }

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

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.domain.write",
      {
        {"domain_id", domainId},
        {"field_count", std::to_string(fields.size())},
        {"path", path.string()}
      }});
  }

  std::filesystem::path SaveDomainStore::DomainPath(const std::string_view domainId) const
  {
    return m_profile.saveDataRoot / (std::string(domainId) + ".json");
  }
} // namespace Peter::Core
