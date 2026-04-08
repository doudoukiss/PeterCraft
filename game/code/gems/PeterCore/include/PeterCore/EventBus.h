#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Core
{
  enum class EventCategory
  {
    Gameplay,
    AI,
    SaveLoad,
    CreatorTools,
    Validation,
    Performance
  };

  using StructuredFields = std::map<std::string, std::string>;

  struct Event
  {
    EventCategory category;
    std::string name;
    StructuredFields fields;
  };

  std::string_view ToString(EventCategory category);

  class IEventSink
  {
  public:
    virtual ~IEventSink() = default;
    virtual void Consume(const Event& event) = 0;
  };

  class EventBus
  {
  public:
    void RegisterSink(IEventSink* sink);
    void Emit(const Event& event) const;
    [[nodiscard]] std::size_t SinkCount() const;

  private:
    std::vector<IEventSink*> m_sinks;
  };
} // namespace Peter::Core
