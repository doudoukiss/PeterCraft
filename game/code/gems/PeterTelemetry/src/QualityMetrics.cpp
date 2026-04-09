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
#elif defined(__linux__)
#include <sys/resource.h>
#endif

namespace Peter::Telemetry
{
  namespace
  {
    bool TryAssignBudget(
      const Peter::Core::QualityProfileBase& profile,
      QualityMetricSample& sample)
    {
      if (sample.metricId == "fps_average")
      {
        sample.budget = static_cast<double>(profile.budgets.fpsTarget);
        return true;
      }
      if (sample.metricId == "frame_time_p95_ms")
      {
        sample.budget = profile.budgets.frameTimeP95Ms;
        return true;
      }
      if (sample.metricId == "cold_boot_ms")
      {
        sample.budget = profile.budgets.coldBootBudgetMs;
        return true;
      }
      if (sample.metricId == "transition_ms")
      {
        sample.budget = profile.budgets.transitionBudgetMs;
        return true;
      }
      if (sample.metricId == "working_set_mb")
      {
        sample.budget = static_cast<double>(profile.budgets.peakWorkingSetBudgetMb);
        return true;
      }
      if (sample.metricId == "input_to_motion_latency_ms")
      {
        sample.budget = profile.budgets.inputToMotionLatencyBudgetMs;
        return true;
      }
      if (sample.metricId == "ai_decision_p95_ms")
      {
        sample.budget = profile.budgets.aiDecisionP95Ms;
        return true;
      }
      if (sample.metricId == "ui_render_p95_ms")
      {
        sample.budget = profile.budgets.uiPanelRenderP95Ms;
        return true;
      }
      if (sample.metricId == "interaction_hitch_p95_ms")
      {
        sample.budget = profile.budgets.interactionHitchBudgetMs;
        return true;
      }
      if (sample.metricId == "single_save_ms")
      {
        sample.budget = profile.budgets.singleSaveBudgetMs;
        return true;
      }
      if (sample.metricId == "full_load_ms")
      {
        sample.budget = profile.budgets.fullLoadBudgetMs;
        return true;
      }
      if (sample.metricId == "full_save_ms")
      {
        sample.budget = profile.budgets.fullSaveBudgetMs;
        return true;
      }
      if (sample.metricId == "audio_voice_concurrency")
      {
        sample.budget = static_cast<double>(profile.budgets.audioVoiceConcurrencyBudget);
        return true;
      }
      if (sample.metricId == "concurrent_world_feedback")
      {
        sample.budget = static_cast<double>(profile.feedback.maxConcurrentWorldCues);
        return true;
      }
      if (sample.metricId == "critical_feedback_per_beat")
      {
        sample.budget = static_cast<double>(profile.feedback.maxCriticalCuesPerBeat);
        return true;
      }

      return false;
    }

    bool MetricPasses(const std::string_view metricId, const double value, const double threshold)
    {
      if (metricId == "fps_average")
      {
        return value >= threshold;
      }

      return value <= threshold;
    }
  } // namespace

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
#elif defined(__linux__)
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
      return static_cast<std::size_t>(usage.ru_maxrss / 1024);
    }
#endif

    return 0;
  }

  QualityReport EvaluateQualityReport(
    const Peter::Core::QualityProfileBase& profile,
    const std::vector<QualityMetricSample>& samples)
  {
    QualityReport report;
    report.profileId = profile.id;
    report.profileDisplayName = profile.displayName;
    report.samples = samples;

    for (auto& sample : report.samples)
    {
      const auto hasBudget = TryAssignBudget(profile, sample);
      if (!hasBudget)
      {
        continue;
      }

      if (!sample.measured)
      {
        ++report.unmeasuredSamples;
        sample.passed = true;
        continue;
      }

      sample.passed = MetricPasses(sample.metricId, sample.value, sample.budget);
      if (!sample.passed)
      {
        report.passed = false;
      }
    }

    if (!report.passed)
    {
      report.summary = "One or more quality budgets failed for " + profile.displayName + '.';
    }
    else if (report.unmeasuredSamples > 0)
    {
      report.summary = "All measured quality budgets passed for " + profile.displayName +
        "; " + std::to_string(report.unmeasuredSamples) + " metrics remain unmeasured.";
    }
    else
    {
      report.summary = "All quality budgets passed for " + profile.displayName + '.';
    }

    return report;
  }

  std::string RenderQualityReport(const QualityReport& report)
  {
    std::ostringstream output;
    output << "Quality Report\n"
           << "Profile: " << report.profileDisplayName << " (" << report.profileId << ")\n"
           << report.summary;
    for (const auto& sample : report.samples)
    {
      output << "\n- " << sample.metricId
             << ": ";
      if (!sample.measured)
      {
        output << "value=unmeasured budget=" << sample.budget << sample.unit;
      }
      else
      {
        output << "value=" << sample.value << sample.unit
               << " budget=" << sample.budget << sample.unit;
      }
      output << " source=" << sample.source
             << " pass=" << (sample.passed ? "true" : "false");
    }
    return output.str();
  }
} // namespace Peter::Telemetry
