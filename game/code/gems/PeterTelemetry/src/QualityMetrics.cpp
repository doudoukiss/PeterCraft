#include "PeterTelemetry/QualityMetrics.h"

#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace Peter::Telemetry
{
  std::size_t CurrentWorkingSetMegabytes()
  {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(
          GetCurrentProcess(),
          reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
          sizeof(counters)) != 0)
    {
      return static_cast<std::size_t>(counters.WorkingSetSize / (1024 * 1024));
    }
#endif
    return 0;
  }

  QualityReport EvaluateQualityReport(
    const Peter::Core::Phase6QualityProfile& profile,
    const std::vector<QualityMetricSample>& samples)
  {
    QualityReport report;
    report.samples = samples;
    for (auto& sample : report.samples)
    {
      if (sample.metricId == "fps_average")
      {
        sample.budget = static_cast<double>(profile.budgets.fpsTarget);
        sample.passed = sample.value >= sample.budget;
      }
      else if (sample.metricId == "frame_time_p95_ms")
      {
        sample.budget = profile.budgets.frameTimeP95Ms;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "cold_boot_ms")
      {
        sample.budget = profile.budgets.coldBootBudgetMs;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "transition_ms")
      {
        sample.budget = profile.budgets.transitionBudgetMs;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "working_set_mb")
      {
        sample.budget = static_cast<double>(profile.budgets.peakWorkingSetBudgetMb);
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "ai_decision_p95_ms")
      {
        sample.budget = profile.budgets.aiDecisionP95Ms;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "ui_render_p95_ms")
      {
        sample.budget = profile.budgets.uiPanelRenderP95Ms;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "single_save_ms")
      {
        sample.budget = profile.budgets.singleSaveBudgetMs;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "full_load_ms")
      {
        sample.budget = profile.budgets.fullLoadBudgetMs;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "full_save_ms")
      {
        sample.budget = profile.budgets.fullSaveBudgetMs;
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "concurrent_world_feedback")
      {
        sample.budget = static_cast<double>(profile.feedback.maxConcurrentWorldCues);
        sample.passed = sample.value <= sample.budget;
      }
      else if (sample.metricId == "critical_feedback_per_beat")
      {
        sample.budget = static_cast<double>(profile.feedback.maxCriticalCuesPerBeat);
        sample.passed = sample.value <= sample.budget;
      }

      if (!sample.passed)
      {
        report.passed = false;
      }
    }

    report.summary = report.passed
      ? "All Phase 6 quality budgets passed."
      : "One or more Phase 6 quality budgets failed.";
    return report;
  }

  std::string RenderQualityReport(const QualityReport& report)
  {
    std::ostringstream output;
    output << "Quality Report\n"
           << report.summary;
    for (const auto& sample : report.samples)
    {
      output << "\n- " << sample.metricId
             << ": value=" << sample.value << sample.unit
             << " budget=" << sample.budget << sample.unit
             << " source=" << sample.source
             << " pass=" << (sample.passed ? "true" : "false");
    }
    return output.str();
  }
} // namespace Peter::Telemetry
