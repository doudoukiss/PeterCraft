#pragma once

#include "PeterCore/EventBus.h"
#include "PeterInventory/InventoryState.h"

#include <string>

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

  struct WorkshopState
  {
    bool salvagePouchCrafted = false;
  };

  struct CraftingResult
  {
    bool crafted = false;
    std::string summary;
  };

  [[nodiscard]] CraftingRecipe BuildPhase1SalvagePouchRecipe();
  [[nodiscard]] CraftingResult CraftRecipe(
    const CraftingRecipe& recipe,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const WorkshopState& workshopState);
  [[nodiscard]] WorkshopState WorkshopStateFromSaveFields(const Peter::Core::StructuredFields& fields);
} // namespace Peter::Progression
