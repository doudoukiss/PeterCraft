#include "PeterTelemetry/JsonlTelemetrySink.h"

#include <fstream>
#include <sstream>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

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

#ifdef _WIN32
    class ScopedTelemetryMutex final
    {
    public:
      ScopedTelemetryMutex()
      {
        m_handle = ::CreateMutexW(nullptr, FALSE, L"Local\\PeterCraftJsonlTelemetrySink");
        if (m_handle != nullptr)
        {
          m_locked = ::WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0;
        }
      }

      ~ScopedTelemetryMutex()
      {
        if (m_locked && m_handle != nullptr)
        {
          ::ReleaseMutex(m_handle);
        }
        if (m_handle != nullptr)
        {
          ::CloseHandle(m_handle);
        }
      }

      ScopedTelemetryMutex(const ScopedTelemetryMutex&) = delete;
      ScopedTelemetryMutex& operator=(const ScopedTelemetryMutex&) = delete;

    private:
      HANDLE m_handle = nullptr;
      bool m_locked = false;
    };
#endif
  } // namespace

  JsonlTelemetrySink::JsonlTelemetrySink(std::filesystem::path outputPath)
    : m_outputPath(std::move(outputPath))
  {
  }

  void JsonlTelemetrySink::Consume(const Peter::Core::Event& event)
  {
    std::filesystem::create_directories(m_outputPath.parent_path());

#ifdef _WIN32
    const ScopedTelemetryMutex lock;
#endif
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
