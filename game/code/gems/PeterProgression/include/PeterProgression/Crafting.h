#pragma once

#include "PeterCore/EventBus.h"
#include "PeterInventory/InventoryState.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Progression
{
  struct CraftingRecipe
  {
    std::string recipeId;
    std::string displayName;
    std::string inputItemId;
    std::string upgradeId;
    std::string description;
    int inputCount = 0;
    int capacityBonus = 0;
  };

  struct UpgradeTrackNodeDefinition
  {
    std::string nodeId;
    std::string displayName;
    std::string description;
    std::string costItemId;
    std::string prerequisiteNodeId;
    std::string effectType;
    int cost = 0;
    int effectValue = 0;
  };

  struct UpgradeTrackDefinition
  {
    std::string trackId;
    std::string displayName;
    std::string summary;
    std::vector<UpgradeTrackNodeDefinition> nodes;
  };

  struct WorkshopState
  {
    bool salvagePouchCrafted = false;
    std::map<std::string, bool, std::less<>> unlockedNodes;
  };

  struct CraftingResult
  {
    bool crafted = false;
    std::string summary;
  };

  struct ProgressionResult
  {
    bool unlocked = false;
    std::string summary;
    std::string nextReward;
  };

  [[nodiscard]] CraftingRecipe BuildPhase1SalvagePouchRecipe();
  [[nodiscard]] const std::vector<UpgradeTrackDefinition>& BuildPhase2UpgradeTracks();
  [[nodiscard]] const UpgradeTrackDefinition* FindUpgradeTrack(std::string_view trackId);
  [[nodiscard]] const UpgradeTrackNodeDefinition* FindUpgradeNode(std::string_view nodeId);
  [[nodiscard]] bool HasUnlockedNode(const WorkshopState& workshopState, std::string_view nodeId);
  [[nodiscard]] ProgressionResult UnlockUpgradeNode(
    std::string_view nodeId,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState);
  [[nodiscard]] std::string RenderTrackSummary(
    const std::vector<UpgradeTrackDefinition>& tracks,
    const WorkshopState& workshopState);
  [[nodiscard]] CraftingResult CraftRecipe(
    const CraftingRecipe& recipe,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const WorkshopState& workshopState);
  [[nodiscard]] WorkshopState WorkshopStateFromSaveFields(const Peter::Core::StructuredFields& fields);
} // namespace Peter::Progression
