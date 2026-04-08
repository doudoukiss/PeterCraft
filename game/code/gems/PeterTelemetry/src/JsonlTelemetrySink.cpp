#include "PeterTelemetry/JsonlTelemetrySink.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace Peter::Telemetry
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
  } // namespace

  JsonlTelemetrySink::JsonlTelemetrySink(std::filesystem::path outputPath)
    : m_outputPath(std::move(outputPath))
  {
  }

  void JsonlTelemetrySink::Consume(const Peter::Core::Event& event)
  {
    std::filesystem::create_directories(m_outputPath.parent_path());

    std::ofstream output(m_outputPath, std::ios::app);
    output << "{\"category\":\"" << Peter::Core::ToString(event.category) << "\",";
    output << "\"name\":\"" << EscapeJson(event.name) << "\",";
    output << "\"fields\":{";

    bool first = true;
    for (const auto& [key, value] : event.fields)
    {
      if (!first)
      {
        output << ',';
      }

      output << '"' << EscapeJson(key) << "\":\"" << EscapeJson(value) << '"';
      first = false;
    }

    output << "}}\n";
  }
} // namespace Peter::Telemetry
