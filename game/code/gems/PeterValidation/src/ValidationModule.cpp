#include "PeterValidation/ValidationModule.h"

#include <set>

namespace Peter::Validation
{
  namespace
  {
    RuleValidationResult ValidateAccessibilitySettingsAgainstProfile(
      const Peter::UI::AccessibilitySettings& settings,
      const Peter::Core::QualityProfileBase& qualityProfile)
    {
      if (settings.textScalePercent < 90 || settings.textScalePercent > 200)
      {
        return RuleValidationResult{false, "Text scale must stay inside the supported 90% to 200% range."};
      }
      if (settings.subtitleScalePercent < 90 || settings.subtitleScalePercent > 180)
      {
        return RuleValidationResult{false, "Subtitle scale must stay inside the supported 90% to 180% range."};
      }
      if (qualityProfile.subtitleBackgroundRequired && !settings.subtitleBackgroundEnabled)
      {
        return RuleValidationResult{false, "Subtitle background must remain available in the runtime."};
      }
      if (qualityProfile.iconRedundancyRequired && !settings.iconRedundancyEnabled)
      {
        return RuleValidationResult{false, "Critical prompts cannot disable icon redundancy completely."};
      }
      return RuleValidationResult{true, "Accessibility settings are valid."};
    }

    RuleValidationResult ValidateQualityProfileBase(
      const Peter::Core::QualityProfileBase& profile,
      const std::string_view profileLabel)
    {
      if (profile.id.empty() || profile.displayName.empty() || profile.targetHardware.id.empty())
      {
        return RuleValidationResult{
          false,
          std::string(profileLabel) + " quality profile needs id, display name, and target hardware."
        };
      }
      if (profile.budgets.fpsTarget < 30 || profile.budgets.frameTimeP95Ms <= 0.0)
      {
        return RuleValidationResult{false, "Performance budgets must be positive and realistic."};
      }
      if (profile.feedback.requiredCueFamilies.empty())
      {
        return RuleValidationResult{false, "Feedback profile must list required cue families."};
      }
      for (const auto& family : profile.feedback.requiredCueFamilies)
      {
        if (!profile.feedback.cuePriorityByFamily.contains(family))
        {
          return RuleValidationResult{false, "Every required cue family needs a priority mapping."};
        }
      }
      return RuleValidationResult{true, std::string(profileLabel) + " quality profile is valid."};
    }
  } // namespace

  ValidationStatus ValidationStatus::PlaceholderHealthy()
  {
    return ValidationStatus{"ok", "phase 7.0 validation, quality profiles, runtime-mode separation checks, content catalogs, save hardening checks, authoring checks, migrations, and deterministic scenario guards are active"};
  }

  RuleValidationResult ValidateFollowDistance(const double followDistanceMeters)
  {
    if (followDistanceMeters < 4.0)
    {
      return RuleValidationResult{false, "Follow distance cannot go below 4 meters."};
    }

    if (followDistanceMeters > 10.0)
    {
      return RuleValidationResult{false, "Follow distance cannot exceed 10 meters."};
    }

    return RuleValidationResult{true, "Follow distance edit is safe to apply."};
  }

  RuleValidationResult ValidateExtractionCountdown(const int countdownSeconds)
  {
    if (countdownSeconds < 3 || countdownSeconds > 12)
    {
      return RuleValidationResult{false, "Extraction countdown must stay between 3 and 12 seconds."};
    }

    return RuleValidationResult{true, "Extraction countdown is valid."};
  }

  RuleValidationResult ValidateRoomKitDefinition(const Peter::World::RoomKitDefinition& roomKit)
  {
    if (roomKit.id.empty() || roomKit.displayName.empty() || roomKit.archetype.empty() ||
      roomKit.connectorClass.empty())
    {
      return RuleValidationResult{false, "Room kits need id, display name, archetype, and connector class."};
    }

    if (roomKit.widthMeters < 4 || roomKit.depthMeters < 4 || roomKit.heightMeters < 3)
    {
      return RuleValidationResult{false, "Room kit metrics must stay above the minimum shell size."};
    }

    return RuleValidationResult{true, "Room kit definition is valid."};
  }

  RuleValidationResult ValidateRoomVariantDefinition(const Peter::World::RoomVariantDefinition& roomVariant)
  {
    if (roomVariant.id.empty() || roomVariant.roomId.empty() || roomVariant.displayName.empty() ||
      roomVariant.kitId.empty() || roomVariant.styleProfileId.empty())
    {
      return RuleValidationResult{false, "Room variants need id, room id, display name, kit id, and style profile id."};
    }

    if (Peter::World::FindRoomKit(roomVariant.kitId) == nullptr)
    {
      return RuleValidationResult{false, "Room variants must reference a known room kit."};
    }

    if (Peter::World::FindWorldStyleProfile(roomVariant.styleProfileId) == nullptr)
    {
      return RuleValidationResult{false, "Room variants must reference a known style profile."};
    }

    return RuleValidationResult{true, "Room variant definition is valid."};
  }

  RuleValidationResult ValidateEncounterPatternDefinition(
    const Peter::World::EncounterPatternDefinition& encounterPattern)
  {
    if (encounterPattern.id.empty() || encounterPattern.displayName.empty() || encounterPattern.roomId.empty())
    {
      return RuleValidationResult{false, "Encounter patterns need id, display name, and room id."};
    }

    if (encounterPattern.enemies.empty())
    {
      return RuleValidationResult{false, "Encounter patterns need at least one enemy unit."};
    }

    for (const auto& feedbackTagId : encounterPattern.feedbackTagIds)
    {
      if (Peter::World::FindFeedbackTag(feedbackTagId) == nullptr)
      {
        return RuleValidationResult{false, "Encounter patterns must reference known feedback tags."};
      }
    }

    return RuleValidationResult{true, "Encounter pattern definition is valid."};
  }

  RuleValidationResult ValidateItemDefinition(const Peter::Inventory::ItemDefinition& definition)
  {
    if (definition.id.empty() || definition.displayName.empty() || definition.description.empty())
    {
      return RuleValidationResult{false, "Item definitions must include id, display name, and description."};
    }

    if (definition.stackLimit < 1)
    {
      return RuleValidationResult{false, "Item stack limits must be positive."};
    }

    if (!Peter::Inventory::IsRarityAllowed(definition.category, definition.rarity))
    {
      return RuleValidationResult{false, "That rarity is not allowed for the chosen category."};
    }

    if (Peter::Inventory::IsDurableCategory(definition.category) && !definition.repairable)
    {
      return RuleValidationResult{false, "Durable categories must be marked repairable."};
    }

    return RuleValidationResult{true, "Item definition is valid."};
  }

  RuleValidationResult ValidateMissionTemplate(const Peter::World::MissionTemplateDefinition& mission)
  {
    if (mission.roomIds.size() < 2)
    {
      return RuleValidationResult{false, "Mission templates need at least two rooms."};
    }

    if (mission.objectives.empty())
    {
      return RuleValidationResult{false, "Mission templates need at least one required objective."};
    }

    std::set<std::string, std::less<>> objectiveIds;
    for (const auto& objective : mission.objectives)
    {
      if (!objectiveIds.insert(objective.id).second)
      {
        return RuleValidationResult{false, "Mission objective IDs must be unique."};
      }
      if (objective.description.empty() || objective.kind.empty())
      {
        return RuleValidationResult{false, "Mission objectives need descriptions and kinds."};
      }
    }

    for (const auto& sideObjective : mission.sideObjectives)
    {
      if (!objectiveIds.insert(sideObjective.id).second)
      {
        return RuleValidationResult{false, "Side-objective IDs must not collide with required objectives."};
      }
    }

    return RuleValidationResult{true, "Mission template is valid."};
  }

  RuleValidationResult ValidateMissionBlueprintDefinition(
    const Peter::World::MissionBlueprintDefinition& missionBlueprint)
  {
    if (missionBlueprint.id.empty() || missionBlueprint.displayName.empty() ||
      missionBlueprint.templateFamilyId.empty() || missionBlueprint.sceneId.empty())
    {
      return RuleValidationResult{false, "Mission blueprints need id, display name, template family, and scene id."};
    }

    if (missionBlueprint.roomVariantIds.size() < 2)
    {
      return RuleValidationResult{false, "Mission blueprints need at least two room variants."};
    }

    for (const auto& roomVariantId : missionBlueprint.roomVariantIds)
    {
      if (Peter::World::FindRoomVariant(roomVariantId) == nullptr)
      {
        return RuleValidationResult{false, "Mission blueprints must reference known room variants."};
      }
    }

    for (const auto& encounterPatternId : missionBlueprint.encounterPatternIds)
    {
      if (Peter::World::FindEncounterPattern(encounterPatternId) == nullptr)
      {
        return RuleValidationResult{false, "Mission blueprints must reference known encounter patterns."};
      }
    }

    if (missionBlueprint.feedbackTagIds.empty())
    {
      return RuleValidationResult{false, "Mission blueprints need at least one feedback tag."};
    }

    return RuleValidationResult{true, "Mission blueprint definition is valid."};
  }

  RuleValidationResult ValidateFeedbackTagDefinition(const Peter::World::FeedbackTagDefinition& feedbackTag)
  {
    if (feedbackTag.id.empty() || feedbackTag.displayName.empty() || feedbackTag.category.empty() ||
      feedbackTag.cueFamily.empty())
    {
      return RuleValidationResult{false, "Feedback tags need id, display name, category, and cue family."};
    }

    return RuleValidationResult{true, "Feedback tag definition is valid."};
  }

  RuleValidationResult ValidateWorldStyleProfileDefinition(
    const Peter::World::WorldStyleProfileDefinition& styleProfile)
  {
    if (styleProfile.id.empty() || styleProfile.displayName.empty() || styleProfile.visualMotif.empty() ||
      styleProfile.signageGrammar.empty())
    {
      return RuleValidationResult{false, "Style profiles need id, display name, visual motif, and signage grammar."};
    }

    return RuleValidationResult{true, "World style profile definition is valid."};
  }

  RuleValidationResult ValidateShippableContentManifest(const Peter::World::ShippableContentManifest& manifest)
  {
    if (manifest.id.empty() || manifest.roomVariantIds.empty() || manifest.missionBlueprintIds.empty())
    {
      return RuleValidationResult{false, "Shippable content manifests need id, room variants, and mission blueprints."};
    }

    return RuleValidationResult{true, "Shippable content manifest is valid."};
  }

  RuleValidationResult ValidateTutorialLesson(const Peter::World::TutorialLessonDefinition& lesson)
  {
    if (lesson.steps.empty())
    {
      return RuleValidationResult{false, "Lessons need at least one step."};
    }

    bool hasCompletion = false;
    for (const auto& step : lesson.steps)
    {
      if (step.prompt.empty() || step.action.empty())
      {
        return RuleValidationResult{false, "Every lesson step needs an action and prompt."};
      }
      if (step.action == "complete_lesson")
      {
        hasCompletion = true;
      }
    }

    if (!hasCompletion)
    {
      return RuleValidationResult{false, "Lessons must include a complete_lesson step."};
    }

    return RuleValidationResult{true, "Tutorial lesson is valid."};
  }

  RuleValidationResult ValidateCompanionConfig(const Peter::AI::CompanionConfig& config)
  {
    if (Peter::AI::FindBehaviorStance(config.stanceId) == nullptr)
    {
      return RuleValidationResult{false, "Companion stance must reference a known Phase 3 stance."};
    }

    std::set<std::string, std::less<>> uniqueChips;
    if (config.activeChipIds.size() > 3)
    {
      return RuleValidationResult{false, "Only three active behavior chips are allowed at once."};
    }

    for (const auto& chipId : config.activeChipIds)
    {
      if (!uniqueChips.insert(chipId).second)
      {
        return RuleValidationResult{false, "Behavior chips cannot be duplicated."};
      }
      if (Peter::AI::FindBehaviorChip(chipId) == nullptr)
      {
        return RuleValidationResult{false, "Behavior chips must reference the Phase 3 chip catalog."};
      }
    }

    return ValidateFollowDistance(Peter::AI::ResolveFollowDistance(config));
  }

  RuleValidationResult ValidateBehaviorChipDefinition(const Peter::AI::BehaviorChipDefinition& chip)
  {
    if (chip.id.empty() || chip.displayName.empty() || chip.promiseText.empty() || chip.mappedVariableId.empty())
    {
      return RuleValidationResult{false, "Behavior chips need id, display name, promise text, and a mapped variable."};
    }

    if (chip.minValue > chip.maxValue || chip.defaultValue < chip.minValue || chip.defaultValue > chip.maxValue)
    {
      return RuleValidationResult{false, "Behavior chip ranges must be safe and internally consistent."};
    }

    if (chip.id == "chip.stay_near_me" && (chip.minValue < 4.0 || chip.maxValue > 10.0))
    {
      return RuleValidationResult{false, "Stay Near Me must stay within the safe 4m to 10m range."};
    }

    return RuleValidationResult{true, "Behavior chip definition is valid."};
  }

  RuleValidationResult ValidateBehaviorStanceDefinition(const Peter::AI::BehaviorStanceDefinition& stance)
  {
    if (stance.id.empty() || stance.displayName.empty() || stance.summary.empty())
    {
      return RuleValidationResult{false, "Behavior stances need id, display name, and summary text."};
    }

    return RuleValidationResult{true, "Behavior stance definition is valid."};
  }

  RuleValidationResult ValidateEnemyArchetypeDefinition(
    const Peter::AI::EnemyArchetypeDefinition& archetype)
  {
    if (archetype.id.empty() || archetype.displayName.empty() || archetype.summary.empty())
    {
      return RuleValidationResult{false, "Enemy archetypes need id, display name, and summary text."};
    }

    if (archetype.preferredRangeMeters < 1)
    {
      return RuleValidationResult{false, "Enemy archetypes need a positive preferred range."};
    }

    return RuleValidationResult{true, "Enemy archetype definition is valid."};
  }

  RuleValidationResult ValidatePatrolRouteDefinition(const Peter::AI::PatrolRouteDefinition& route)
  {
    if (route.id.empty() || route.nodeIds.size() < 2)
    {
      return RuleValidationResult{false, "Patrol routes need an id and at least two linked nodes."};
    }

    std::set<std::string, std::less<>> uniqueNodes;
    for (const auto& nodeId : route.nodeIds)
    {
      if (!uniqueNodes.insert(nodeId).second)
      {
        return RuleValidationResult{false, "Patrol routes cannot repeat node ids in this shell."};
      }
    }

    return RuleValidationResult{true, "Patrol route definition is valid."};
  }

  RuleValidationResult ValidateAiScenarioDefinition(const Peter::AI::AiScenarioDefinition& scenario)
  {
    if (scenario.id.empty() || scenario.steps.empty())
    {
      return RuleValidationResult{false, "AI scenarios need a stable id and at least one step."};
    }

    if (!scenario.deterministic)
    {
      return RuleValidationResult{false, "Phase 3 AI scenarios must be deterministic."};
    }

    for (const auto& step : scenario.steps)
    {
      if (step.expectedActionId.empty() || step.expectedState.empty())
      {
        return RuleValidationResult{false, "Every AI scenario step needs an expected action and state."};
      }
      if (step.useEnemy && step.enemy.enemyId.empty())
      {
        return RuleValidationResult{false, "Enemy AI scenario steps need an enemy unit."};
      }
      if (!step.useEnemy)
      {
        const auto configValidation = ValidateCompanionConfig(step.config);
        if (!configValidation.valid)
        {
          return configValidation;
        }
      }
    }

    return RuleValidationResult{true, "AI scenario definition is valid."};
  }

  RuleValidationResult ValidateTinkerVariableDefinition(
    const Peter::Workshop::TinkerVariableDefinition& variable)
  {
    if (variable.id.empty() || variable.displayName.empty() || variable.groupId.empty())
    {
      return RuleValidationResult{false, "Tinker variables need id, display name, and group id."};
    }
    if (variable.minValue > variable.maxValue ||
      variable.defaultValue < variable.minValue ||
      variable.defaultValue > variable.maxValue)
    {
      return RuleValidationResult{false, "Tinker variable ranges must be safe and internally consistent."};
    }
    return RuleValidationResult{true, "Tinker variable definition is valid."};
  }

  RuleValidationResult ValidateTinkerPresetDefinition(
    const Peter::Workshop::TinkerPresetDefinition& preset)
  {
    if (preset.id.empty() || preset.displayName.empty() || preset.values.empty())
    {
      return RuleValidationResult{false, "Tinker presets need id, display name, and at least one value."};
    }
    for (const auto& [variableId, value] : preset.values)
    {
      const auto* variable = Peter::Workshop::FindTinkerVariable(variableId);
      if (variable == nullptr)
      {
        return RuleValidationResult{false, "Tinker presets must reference known variables."};
      }
      if (value < variable->minValue || value > variable->maxValue)
      {
        return RuleValidationResult{false, "Tinker preset values must stay inside the variable's safe range."};
      }
    }
    return RuleValidationResult{true, "Tinker preset definition is valid."};
  }

  RuleValidationResult ValidateLogicRulesetDefinition(
    const Peter::Workshop::LogicRulesetDefinition& ruleset)
  {
    if (ruleset.id.empty() || ruleset.displayName.empty())
    {
      return RuleValidationResult{false, "Logic rulesets need id and display name."};
    }
    if (ruleset.cards.empty() || ruleset.cards.size() > 5)
    {
      return RuleValidationResult{false, "Logic rulesets need between one and five ordered cards."};
    }

    const std::set<std::string, std::less<>> validConditions = {
      "player_health_low",
      "player_needs_revive",
      "rare_loot_visible",
      "extraction_active",
      "companion_health_low",
      "timed_pressure_high"
    };
    const std::set<std::string, std::less<>> validActions = {
      "prioritize_help",
      "prioritize_cover",
      "mark_rare_loot",
      "retreat_regroup",
      "hold_position",
      "favor_extraction"
    };

    std::set<std::string, std::less<>> uniqueCardIds;
    for (const auto& card : ruleset.cards)
    {
      if (card.id.empty() || card.displayName.empty() || card.summary.empty())
      {
        return RuleValidationResult{false, "Logic cards need id, display name, and summary text."};
      }
      if (!uniqueCardIds.insert(card.id).second)
      {
        return RuleValidationResult{false, "Logic card IDs must be unique."};
      }
      if (!validConditions.contains(card.conditionId))
      {
        return RuleValidationResult{false, "Logic cards must use the locked Phase 4 condition vocabulary."};
      }
      if (!validActions.contains(card.actionId))
      {
        return RuleValidationResult{false, "Logic cards must use the locked Phase 4 action vocabulary."};
      }
      if (card.scoreDelta < -20.0 || card.scoreDelta > 20.0)
      {
        return RuleValidationResult{false, "Logic cards must stay inside the bounded score range."};
      }
    }
    return RuleValidationResult{true, "Logic ruleset definition is valid."};
  }

  RuleValidationResult ValidateTinyScriptDefinition(
    const Peter::Workshop::TinyScriptDefinition& script)
  {
    if (script.id.empty() || script.displayName.empty() || script.body.empty())
    {
      return RuleValidationResult{false, "Tiny scripts need id, display name, and body text."};
    }

    const auto validation = Peter::Workshop::ValidateTinyScript(script);
    if (!validation.valid)
    {
      return RuleValidationResult{false, validation.error};
    }

    if (script.hookKind == Peter::Workshop::TinyScriptHookKind::CompanionPriorityHint &&
      script.targetActionId.empty())
    {
      return RuleValidationResult{false, "Companion priority hint scripts need a target action id."};
    }

    return RuleValidationResult{true, "Tiny script definition is valid."};
  }

  RuleValidationResult ValidateMiniMissionDraftDefinition(
    const Peter::Workshop::MiniMissionDraftDefinition& draft)
  {
    if (draft.id.empty() || draft.displayName.empty())
    {
      return RuleValidationResult{false, "Mini mission drafts need id and display name."};
    }
    if (Peter::World::FindMiniMissionRoomBundle(draft.roomBundleId) == nullptr)
    {
      return RuleValidationResult{false, "Mini missions must reference a known room bundle."};
    }
    if (Peter::World::FindMiniMissionEnemyGroup(draft.enemyGroupId) == nullptr)
    {
      return RuleValidationResult{false, "Mini missions must reference a known enemy group."};
    }
    if (Peter::World::FindMiniMissionReward(draft.rewardBundleId) == nullptr)
    {
      return RuleValidationResult{false, "Mini missions must reference a known reward bundle."};
    }
    if (draft.revision < 1)
    {
      return RuleValidationResult{false, "Mini mission revisions must be positive."};
    }
    return RuleValidationResult{true, "Mini mission draft definition is valid."};
  }

  RuleValidationResult ValidateCreatorManifest(const Peter::Workshop::CreatorManifest& manifest)
  {
    for (const auto& [kindId, contentId] : manifest.activeDraftIds)
    {
      if (kindId.empty() || contentId.empty())
      {
        return RuleValidationResult{false, "Creator manifest active draft entries must be complete."};
      }
    }
    for (const auto& [key, revision] : manifest.lastKnownGoodRevisions)
    {
      if (key.empty() || revision < 0)
      {
        return RuleValidationResult{false, "Creator manifest revisions must be non-negative."};
      }
    }
    for (const auto& [key, revision] : manifest.rollbackTargets)
    {
      if (key.empty() || revision < 0)
      {
        return RuleValidationResult{false, "Creator manifest rollback targets must be non-negative."};
      }
    }
    return RuleValidationResult{true, "Creator manifest is valid."};
  }

  RuleValidationResult ValidateFavoriteItemReference(
    const std::string_view favoriteItemId,
    const Peter::Inventory::InventoryState& inventory)
  {
    if (favoriteItemId.empty())
    {
      return RuleValidationResult{true, "No favorite item selected."};
    }

    if (Peter::Inventory::CountItem(inventory.equippedDurability, favoriteItemId) == 0)
    {
      return RuleValidationResult{false, "Favorite item must reference equipped durable gear."};
    }

    return RuleValidationResult{true, "Favorite item reference is valid."};
  }

  RuleValidationResult ValidateInputBindings(
    const std::vector<Peter::Adapters::ActionBinding>& bindings,
    const Peter::UI::AccessibilitySettings& settings)
  {
    std::set<std::string, std::less<>> activeBindings;
    for (const auto& binding : bindings)
    {
      const auto override = settings.actionBindings.find(binding.actionId);
      const std::string resolved = override == settings.actionBindings.end() ? binding.primaryInput : override->second;
      if (!activeBindings.insert(resolved).second)
      {
        return RuleValidationResult{false, "Two actions cannot share the same primary binding in this shell."};
      }
    }

    return RuleValidationResult{true, "Input bindings are conflict-free."};
  }

  RuleValidationResult ValidateAccessibilitySettings(
    const Peter::UI::AccessibilitySettings& settings,
    const Peter::Core::Phase6QualityProfile& qualityProfile)
  {
    return ValidateAccessibilitySettingsAgainstProfile(settings, qualityProfile);
  }

  RuleValidationResult ValidateAccessibilitySettings(
    const Peter::UI::AccessibilitySettings& settings,
    const Peter::Core::Phase7PlayableQualityProfile& qualityProfile)
  {
    return ValidateAccessibilitySettingsAgainstProfile(settings, qualityProfile);
  }

  RuleValidationResult ValidatePhase6QualityProfile(const Peter::Core::Phase6QualityProfile& profile)
  {
    return ValidateQualityProfileBase(profile, "Phase 6");
  }

  RuleValidationResult ValidatePhase7PlayableQualityProfile(
    const Peter::Core::Phase7PlayableQualityProfile& profile)
  {
    const auto commonValidation = ValidateQualityProfileBase(profile, "Phase 7 playable");
    if (!commonValidation.valid)
    {
      return commonValidation;
    }

    if (profile.budgets.inputToMotionLatencyBudgetMs <= 0.0)
    {
      return RuleValidationResult{false, "Phase 7 playable profile needs an input-to-motion latency budget."};
    }
    if (profile.budgets.interactionHitchBudgetMs <= 0.0)
    {
      return RuleValidationResult{false, "Phase 7 playable profile needs an interaction hitch budget."};
    }
    if (profile.budgets.audioVoiceConcurrencyBudget < 1)
    {
      return RuleValidationResult{false, "Phase 7 playable profile needs an audio concurrency budget."};
    }

    return RuleValidationResult{true, "Phase 7 playable quality profile is valid."};
  }

  std::string_view GetModuleSummary()
  {
    return "Runtime validation hooks, save migrations, content catalog checks, quality profile validation, mission checks, and authored safety boundaries including AI contracts.";
  }
} // namespace Peter::Validation
