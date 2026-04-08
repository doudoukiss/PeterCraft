#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCombat/EncounterSimulator.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/StableId.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTest/TestMacros.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/WorkshopTuning.h"
#include "PeterWorld/SliceContent.h"

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

  Peter::Core::FeatureRegistry registry({"0.3.0", "test"});
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
  loadout.favoriteItemId = loadout.equippedToolId;
  for (const auto& itemId : Peter::Inventory::EquippedItemIds(loadout))
  {
    const auto* definition = Peter::Inventory::FindItemDefinition(itemId);
    if (definition != nullptr && definition->repairable)
    {
      inventory.equippedDurability[itemId] = definition->maxDurability;
    }
  }

  std::string denialReason;
  PETER_ASSERT_TRUE(Peter::Inventory::TryAddCarriedItem(
    inventory,
    loadout,
    "item.salvage.scrap_metal",
    3,
    &denialReason));
  PETER_ASSERT_EQ(1, Peter::Inventory::UsedCarrySlots(inventory));

  const auto* questDefinition = Peter::Inventory::FindItemDefinition("item.quest.artifact_seed");
  PETER_ASSERT_TRUE(questDefinition != nullptr);
  PETER_ASSERT_EQ(std::string("epic"), std::string(Peter::Inventory::ToString(questDefinition->rarity)));
  PETER_ASSERT_TRUE(Peter::Validation::ValidateItemDefinition(*questDefinition).valid);

  Peter::Combat::CombatantState combatant{"player", 40, 100};
  const auto directHit = Peter::Combat::ResolveDamageAction(
    combatant,
    Peter::Combat::DamageSpec{
      "test.direct",
      "enemy.test",
      "player",
      Peter::Combat::CombatActionKind::DirectHit,
      10,
      {"machine"},
      {Peter::Combat::StatusEffectSpec{"overheat", 1, 3}}});
  PETER_ASSERT_EQ(10, directHit.damageApplied);
  PETER_ASSERT_EQ(30, combatant.health);

  const auto statusTicks = Peter::Combat::TickStatuses(combatant, "system.status");
  PETER_ASSERT_EQ(1, static_cast<int>(statusTicks.size()));
  PETER_ASSERT_EQ(3, statusTicks.front().damageApplied);

  const auto support = Peter::Combat::ResolveSupportAction(
    combatant,
    Peter::Combat::SupportActionSpec{
      "support.repair_pulse",
      "companion",
      "player",
      8,
      1,
      {"support"},
      {Peter::Combat::StatusEffectSpec{"guarded", 1, 6}}});
  PETER_ASSERT_EQ(8, support.healingApplied);
  PETER_ASSERT_TRUE(!support.statusesApplied.empty());

  Peter::Inventory::RecoveryState recoveryState;
  inventory.equippedDurability[loadout.equippedToolId] = 20;
  const auto failureRecovery =
    Peter::Inventory::ApplyRaidFailureRecovery(inventory, loadout, recoveryState, false);
  PETER_ASSERT_TRUE(!failureRecovery.favoriteItemMovedToRecovery.empty());
  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 2);
  std::string recoverySummary;
  PETER_ASSERT_TRUE(Peter::Inventory::ReclaimFavoriteItem(recoveryState, inventory, loadout, &recoverySummary));

  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 3);
  Peter::Progression::WorkshopState workshopState;
  const auto unlockResult = Peter::Progression::UnlockUpgradeNode(
    "track.inventory_capacity.salvage_pouch",
    inventory,
    loadout,
    workshopState);
  PETER_ASSERT_TRUE(unlockResult.unlocked);

  Peter::UI::AccessibilitySettings settings;
  settings.actionBindings["action.jump"] = "Space";
  const auto serializedSettings = Peter::UI::ToSaveFields(settings);
  const auto loadedSettings = Peter::UI::AccessibilitySettingsFromSaveFields(
    serializedSettings,
    {
      Peter::Adapters::ActionBinding{"action.jump", "Space", "A", true},
      Peter::Adapters::ActionBinding{"action.interact", "E", "X", true}
    });
  PETER_ASSERT_EQ(100, loadedSettings.textScalePercent);

  const auto validMission = Peter::Validation::ValidateMissionTemplate(
    Peter::World::BuildPhase2MissionTemplates().front());
  PETER_ASSERT_TRUE(validMission.valid);

  const auto preview = Peter::Workshop::BuildFollowDistancePreview(6.0, 9.0);
  PETER_ASSERT_TRUE(preview.valid);

  const auto beforeEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{6.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, false, false, false, true, 8});
  const auto afterEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{9.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, false, false, false, true, 8});
  PETER_ASSERT_EQ(std::string("regroup"), beforeEdit.currentState);
  PETER_ASSERT_EQ(std::string("support_loot"), afterEdit.currentState);
})
