#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTest/TestMacros.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

PETER_TEST_MAIN({
  const auto root = std::filesystem::temp_directory_path() / "PeterCraftPhase0Integration";
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

  Peter::Inventory::InventoryState inventory;
  Peter::Inventory::LoadoutState loadout;
  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 3);
  Peter::Progression::WorkshopState workshop;
  const auto recipe = Peter::Progression::BuildPhase1SalvagePouchRecipe();
  const auto craftResult = Peter::Progression::CraftRecipe(recipe, inventory, loadout, workshop);
  PETER_ASSERT_TRUE(craftResult.crafted);

  const Peter::AI::CompanionConfig companionConfig{9.0, false};
  saveDomainStore.WriteDomain(
    "save_domain.inventory",
    Peter::Inventory::ToSaveFields(inventory, loadout));
  saveDomainStore.WriteDomain(
    "save_domain.workshop_upgrades",
    Peter::Progression::ToSaveFields(workshop));
  saveDomainStore.WriteDomain(
    "save_domain.companion_config",
    Peter::AI::ToSaveFields(companionConfig));
  const auto inventoryFields = saveDomainStore.ReadDomain("save_domain.inventory");
  const auto companionFields = saveDomainStore.ReadDomain("save_domain.companion_config");
  eventBus.Emit({Peter::Core::EventCategory::Gameplay, "gameplay.integration.complete", {}});

  PETER_ASSERT_TRUE(std::filesystem::exists(profile.root));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.backupRoot));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.userContentRoot));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot / "save_domain.inventory.json"));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.backupRoot / "save_domain.inventory.bak.json"));

  const auto logPath = root / "Logs" / "integration-events.jsonl";
  PETER_ASSERT_TRUE(std::filesystem::exists(logPath));
  PETER_ASSERT_TRUE(inventoryFields.find("carry_slot_bonus") != inventoryFields.end());
  PETER_ASSERT_TRUE(companionFields.find("follow_distance_meters") != companionFields.end());

  const std::string logContents = [&logPath]() {
    std::ifstream input(logPath);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  }();

  PETER_ASSERT_TRUE(logContents.find("save_load.profile.ready") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.write") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.read") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("gameplay.integration.complete") != std::string::npos);
})
