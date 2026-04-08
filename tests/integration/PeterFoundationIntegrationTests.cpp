#include "PeterAdapters/PlatformServices.h"
#include "PeterAI/CompanionAi.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTest/TestMacros.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SliceContent.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

PETER_TEST_MAIN({
  const auto root = std::filesystem::temp_directory_path() / "PeterCraftPhase2Integration";
  std::filesystem::remove_all(root);

  const Peter::Adapters::BootConfig bootConfig{
    std::filesystem::current_path(),
    root,
    true
  };

  auto platform = Peter::Adapters::CreateNullPlatformServices(bootConfig);
  Peter::Core::EventBus eventBus;
  Peter::Telemetry::JsonlTelemetrySink sink(root / "Logs" / "integration-events.jsonl");
  eventBus.RegisterSink(&sink);

  Peter::Core::ProfileService profileService(*platform.save, eventBus);
  const auto profile = profileService.EnsureProfile("player.integration");
  Peter::Core::SaveDomainStore saveDomainStore(profile, eventBus);

  saveDomainStore.WriteDomain(
    "save_domain.inventory",
    Peter::Core::StructuredFields{
      {"carried", ""},
      {"stash", "item.salvage.scrap_metal=3"},
      {"base_carry_slot_capacity", "2"},
      {"carry_slot_bonus", "1"},
      {"salvage_pouch_equipped", "true"}
    });
  saveDomainStore.WriteDomain(
    "save_domain.workshop_upgrades",
    Peter::Core::StructuredFields{
      {"salvage_pouch_crafted", "true"}
    });

  Peter::Inventory::InventoryState inventory;
  Peter::Inventory::LoadoutState loadout;
  Peter::Inventory::LoadFromSaveFields(saveDomainStore.ReadDomain("save_domain.inventory"), inventory, loadout);
  const auto workshop =
    Peter::Progression::WorkshopStateFromSaveFields(saveDomainStore.ReadDomain("save_domain.workshop_upgrades"));

  PETER_ASSERT_EQ(1, loadout.carrySlotBonus);
  PETER_ASSERT_TRUE(workshop.salvagePouchCrafted);
  PETER_ASSERT_TRUE(Peter::Inventory::CountItem(inventory.equippedDurability, loadout.equippedToolId) > 0);

  Peter::Inventory::RecoveryState recoveryState;
  recoveryState.favoriteItemInRecovery = loadout.equippedToolId;
  saveDomainStore.WriteDomain(
    "save_domain.recovery_state",
    Peter::Inventory::RecoveryStateToSaveFields(recoveryState));
  Peter::Inventory::RecoveryState loadedRecovery;
  Peter::Inventory::LoadRecoveryStateFromSaveFields(
    saveDomainStore.ReadDomain("save_domain.recovery_state"),
    loadedRecovery);
  PETER_ASSERT_EQ(loadout.equippedToolId, loadedRecovery.favoriteItemInRecovery);

  Peter::UI::AccessibilitySettings settings;
  settings.textScalePercent = 120;
  settings.actionBindings["action.interact"] = "F";
  saveDomainStore.WriteDomain(
    "save_domain.settings_accessibility",
    Peter::UI::ToSaveFields(settings));
  const auto loadedSettings = Peter::UI::AccessibilitySettingsFromSaveFields(
    saveDomainStore.ReadDomain("save_domain.settings_accessibility"),
    platform.input->DefaultBindings());
  PETER_ASSERT_EQ(120, loadedSettings.textScalePercent);
  PETER_ASSERT_EQ(std::string("F"), loadedSettings.actionBindings.at("action.interact"));

  const auto lessonValidation =
    Peter::Validation::ValidateTutorialLesson(Peter::World::BuildPhase2TutorialLessons().front());
  PETER_ASSERT_TRUE(lessonValidation.valid);

  eventBus.Emit({Peter::Core::EventCategory::Gameplay, "gameplay.integration.complete", {}});

  PETER_ASSERT_TRUE(std::filesystem::exists(profile.root));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot / "save_domain.inventory.json"));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot / "save_domain.recovery_state.json"));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot / "save_domain.settings_accessibility.json"));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.backupRoot / "save_domain.inventory.bak.json"));

  const auto logPath = root / "Logs" / "integration-events.jsonl";
  PETER_ASSERT_TRUE(std::filesystem::exists(logPath));

  const std::string logContents = [&logPath]() {
    std::ifstream input(logPath);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  }();

  PETER_ASSERT_TRUE(logContents.find("save_load.profile.ready") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.write") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.read") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("gameplay.integration.complete") != std::string::npos);
})
