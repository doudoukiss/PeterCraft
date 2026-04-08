#include "PeterCore/EventBus.h"

namespace Peter::Core
{
  std::string_view ToString(const EventCategory category)
  {
    switch (category)
    {
      case EventCategory::Gameplay:
        return "gameplay";
      case EventCategory::AI:
        return "ai";
      case EventCategory::SaveLoad:
        return "save_load";
      case EventCategory::CreatorTools:
        return "creator_tools";
      case EventCategory::Validation:
        return "validation";
      case EventCategory::Performance:
        return "performance";
    }

    return "unknown";
  }

  void EventBus::RegisterSink(IEventSink* sink)
  {
    if (sink != nullptr)
    {
      m_sinks.push_back(sink);
    }
  }

  void EventBus::Emit(const Event& event) const
  {
    for (auto* sink : m_sinks)
    {
      sink->Consume(event);
    }
  }

  std::size_t EventBus::SinkCount() const
  {
    return m_sinks.size();
  }
} // namespace Peter::Core
