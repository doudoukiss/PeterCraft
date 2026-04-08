#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/StableId.h"
#include "PeterTest/TestMacros.h"

namespace
{
  class CountingSink final : public Peter::Core::IEventSink
  {
  public:
    void Consume(const Peter::Core::Event&) override
    {
      ++count;
    }

    int count = 0;
  };
} // namespace

PETER_TEST_MAIN({
  PETER_ASSERT_TRUE(Peter::Core::StableId::IsValid("item.salvage.scrap_metal"));
  PETER_ASSERT_TRUE(!Peter::Core::StableId::IsValid("Item.Salvage.Bad"));

  Peter::Core::FeatureRegistry registry({"0.1.0", "test"});
  registry.SetFlag("feature.test", true);
  PETER_ASSERT_TRUE(registry.IsEnabled("feature.test"));
  PETER_ASSERT_EQ(std::string("test"), registry.Version().track);

  Peter::Core::EventBus eventBus;
  CountingSink sink;
  eventBus.RegisterSink(&sink);
  eventBus.Emit({Peter::Core::EventCategory::Gameplay, "gameplay.test", {}});
  PETER_ASSERT_EQ(1, sink.count);
})
