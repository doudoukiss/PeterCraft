#pragma once

#include "PeterCore/EventBus.h"

#include <filesystem>

namespace Peter::Telemetry
{
  class JsonlTelemetrySink final : public Peter::Core::IEventSink
  {
  public:
    explicit JsonlTelemetrySink(std::filesystem::path outputPath);
    void Consume(const Peter::Core::Event& event) override;

  private:
    std::filesystem::path m_outputPath;
  };
} // namespace Peter::Telemetry
