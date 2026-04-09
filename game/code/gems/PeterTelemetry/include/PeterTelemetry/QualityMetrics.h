#pragma once

#include "PeterCore/QualityProfile.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Telemetry
{
  struct QualityMetricSample
  {
    std::string metricId;
    std::string unit;
    double value = 0.0;
    double budget = 0.0;
    bool passed = true;
    std::string source;
  };

  struct QualityReport
  {
    bool passed = true;
    std::string summary;
    std::vector<QualityMetricSample> samples;
  };

  [[nodiscard]] std::size_t CurrentWorkingSetMegabytes();
  [[nodiscard]] QualityReport EvaluateQualityReport(
    const Peter::Core::Phase6QualityProfile& profile,
    const std::vector<QualityMetricSample>& samples);
  [[nodiscard]] std::string RenderQualityReport(const QualityReport& report);
} // namespace Peter::Telemetry
