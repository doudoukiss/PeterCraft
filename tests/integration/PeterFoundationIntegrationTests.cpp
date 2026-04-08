#include "PeterAdapters/PlatformServices.h"
#include "PeterAI/CompanionAi.h"
#include "PeterCore/CreatorContentStore.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterInventory/InventoryState.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTest/TestMacros.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/CreatorWorkshop.h"
#include "PeterWorkshop/WorkshopTuning.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

PETER_TEST_MAIN({
  const auto root = std::filesystem::temp_directory_path() / "PeterCraftPhase3Integration";
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
    "save_domain.companion_config",
    Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"follow_distance_meters", "9.0"},
      {"hold_position", "true"}
    });

  const auto migratedConfig = Peter::AI::CompanionConfigFromSaveFields(
    saveDomainStore.ReadDomain("save_domain.companion_config"));
  PETER_ASSERT_EQ(std::string("stance.balanced"), migratedConfig.stanceId);
  PETER_ASSERT_EQ(9.0, Peter::AI::ResolveFollowDistance(migratedConfig));
  PETER_ASSERT_TRUE(!migratedConfig.activeChipIds.empty());

  const auto previewConfig = Peter::Workshop::BuildBehaviorPreviewConfig(
    migratedConfig,
    "stance.guardian",
    {"chip.stay_near_me", "chip.protect_me_first", "chip.help_at_extraction"},
    {{"chip.stay_near_me", 8.5}});
  const auto preview = Peter::Workshop::BuildCompanionBehaviorPreview(migratedConfig, previewConfig);
  PETER_ASSERT_TRUE(preview.valid);

  Peter::Core::CreatorContentStore creatorContentStore(profile, eventBus);
  creatorContentStore.EnsureLayout();
  const int presetRevision = creatorContentStore.WriteArtifact(
    Peter::Core::CreatorContentKind::TinkerPreset,
    "preset.tinker.companion_close_guard",
    Peter::Core::StructuredFields{
      {"display_name", "Close Guard"},
      {"companion.follow_distance_meters", "4.0"},
      {"companion.rare_loot_bias", "0.1"}});
  PETER_ASSERT_EQ(1, presetRevision);

  Peter::Workshop::CreatorManifest creatorManifest;
  const auto activation = Peter::Workshop::ActivateCreatorArtifact(
    creatorManifest,
    "tinker",
    "preset.tinker.companion_close_guard",
    presetRevision,
    true);
  PETER_ASSERT_TRUE(activation.success);
  saveDomainStore.WriteDomain(
    "save_domain.creator_manifest",
    Peter::Workshop::ToSaveFields(creatorManifest));
  saveDomainStore.WriteDomain(
    "save_domain.creator_progress",
    Peter::Workshop::ToSaveFields(Peter::Workshop::CreatorProgressState{{"lesson.phase4.change_value"}, true, 1}));
  saveDomainStore.WriteDomain(
    "save_domain.creator_settings",
    Peter::Workshop::ToSaveFields(Peter::Workshop::CreatorSettings{true, true, true}));

  const auto reloadedManifest = Peter::Workshop::CreatorManifestFromSaveFields(
    saveDomainStore.ReadDomain("save_domain.creator_manifest"));
  PETER_ASSERT_EQ(
    std::string("preset.tinker.companion_close_guard"),
    reloadedManifest.activeDraftIds.at("tinker"));

  const auto storedPreset = creatorContentStore.ReadArtifact(
    Peter::Core::CreatorContentKind::TinkerPreset,
    "preset.tinker.companion_close_guard");
  PETER_ASSERT_EQ(std::string("4.0"), storedPreset.at("companion.follow_distance_meters"));

  saveDomainStore.WriteDomain(
    "save_domain.companion_config",
    Peter::AI::ToSaveFields(previewConfig));
  const auto reloadedConfig = Peter::AI::CompanionConfigFromSaveFields(
    saveDomainStore.ReadDomain("save_domain.companion_config"));
  PETER_ASSERT_EQ(std::string("stance.guardian"), reloadedConfig.stanceId);
  PETER_ASSERT_EQ(8.5, Peter::AI::ResolveFollowDistance(reloadedConfig));

  Peter::AI::CompanionWorldContext extractionContext;
  extractionContext.extractionActive = true;
  extractionContext.timedMissionPressure = true;
  extractionContext.playerLowHealth = true;
  extractionContext.distanceToPlayerMeters = 2;
  extractionContext.distanceToThreatMeters = 7;
  extractionContext.companionHealthPercent = 80;
  extractionContext.extractionUrgency = 3;
  extractionContext.urgencyLevel = 3;
  extractionContext.roomNodeId = "room.raid.extraction_pad";
  extractionContext.routeNodeId = "route.machine_silo.vault_watch";
  extractionContext.currentGoal = "goal.reach_extraction";
  extractionContext.currentTargetId = "player";
  extractionContext.heardEventToken = "sound.extraction_alarm";
  extractionContext.interestMarkerActive = true;
  extractionContext.interestMarkerId = "marker.extraction.pad";

  const auto beforeExplain = Peter::AI::EvaluateCompanion(migratedConfig, extractionContext);
  const auto afterExplain = Peter::AI::EvaluateCompanion(reloadedConfig, extractionContext);
  const auto compareView = Peter::UI::RenderCompanionCompareView(beforeExplain, afterExplain, preview.deltaSummary);
  PETER_ASSERT_TRUE(compareView.find("What changed after your edit?") != std::string::npos);

  const auto explainPanel = Peter::UI::RenderCompanionExplainPanel(afterExplain);
  const auto debugPanel = Peter::UI::RenderAiDebugPanel(Peter::AI::BuildExplainSnapshot(afterExplain));
  PETER_ASSERT_TRUE(explainPanel.find("Goal:") != std::string::npos);
  PETER_ASSERT_TRUE(debugPanel.find("Top actions:") != std::string::npos);

  const auto creatorPanel = Peter::UI::RenderCreatorPanel(
    Peter::Workshop::DefaultTinkerValues(),
    Peter::Workshop::FindLogicTemplate("logic.template.protect_player"),
    Peter::Workshop::FindTinyScriptTemplate("script.template.priority_hint"));
  PETER_ASSERT_TRUE(creatorPanel.find("Creator Workshop") != std::string::npos);

  const auto contentRoot = Peter::World::ResolveContentRoot();
  PETER_ASSERT_TRUE(std::filesystem::exists(contentRoot / "mission-blueprints"));
  const auto* missionBlueprint = Peter::World::FindMissionBlueprint("mission.recover_artifact.sky_docks");
  PETER_ASSERT_TRUE(missionBlueprint != nullptr);
  const auto raidZone = Peter::World::BuildRaidZoneForMission("mission.recover_artifact.sky_docks");
  const auto missionPreview = Peter::UI::RenderMissionBlueprintPreview(*missionBlueprint, raidZone);
  PETER_ASSERT_TRUE(missionPreview.find("Sky Docks") != std::string::npos);

  platform.ui->PresentCompanionFeedback(afterExplain.calloutToken, afterExplain.gestureToken);
  platform.ui->PresentDebugMarkers({"room.raid.extraction_pad", afterExplain.blackboard.routeNodeId});
  eventBus.Emit({Peter::Core::EventCategory::AI, "ai.decision.selected", {{"action", afterExplain.lastAction}}});

  PETER_ASSERT_TRUE(Peter::Validation::ValidateCompanionConfig(reloadedConfig).valid);

  const auto logPath = root / "Logs" / "integration-events.jsonl";
  PETER_ASSERT_TRUE(std::filesystem::exists(logPath));

  const std::string logContents = [&logPath]() {
    std::ifstream input(logPath);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  }();

  PETER_ASSERT_TRUE(logContents.find("save_load.profile.ready") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.write") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("save_load.domain.read") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("ai.decision.selected") != std::string::npos);
})
