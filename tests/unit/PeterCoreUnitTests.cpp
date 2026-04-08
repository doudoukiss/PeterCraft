#include "PeterAI/CompanionAi.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/StableId.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTest/TestMacros.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/WorkshopTuning.h"

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

  Peter::Inventory::InventoryState inventory;
  Peter::Inventory::LoadoutState loadout;
  std::string denialReason;
  PETER_ASSERT_TRUE(Peter::Inventory::TryAddCarriedItem(
    inventory,
    loadout,
    "item.salvage.scrap_metal",
    3,
    &denialReason));
  const auto moved = Peter::Inventory::MoveCarriedToStash(inventory);
  PETER_ASSERT_EQ(3, Peter::Inventory::CountItem(moved, "item.salvage.scrap_metal"));

  Peter::Progression::WorkshopState workshopState;
  const auto recipe = Peter::Progression::BuildPhase1SalvagePouchRecipe();
  const auto craftingResult =
    Peter::Progression::CraftRecipe(recipe, inventory, loadout, workshopState);
  PETER_ASSERT_TRUE(craftingResult.crafted);
  PETER_ASSERT_EQ(1, loadout.carrySlotBonus);

  const auto preview = Peter::Workshop::BuildFollowDistancePreview(6.0, 9.0);
  PETER_ASSERT_TRUE(preview.valid);

  const auto invalid = Peter::Validation::ValidateFollowDistance(12.0);
  PETER_ASSERT_TRUE(!invalid.valid);

  const auto beforeEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{6.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
  const auto afterEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{9.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
  PETER_ASSERT_EQ(std::string("regroup"), beforeEdit.currentState);
  PETER_ASSERT_EQ(std::string("support_loot"), afterEdit.currentState);
})
