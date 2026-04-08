#include "PeterProgression/Crafting.h"

namespace Peter::Progression
{
  CraftingRecipe BuildPhase1SalvagePouchRecipe()
  {
    return CraftingRecipe{
      "recipe.upgrade.salvage_pouch",
      "Salvage Pouch",
      "item.salvage.scrap_metal",
      "upgrade.salvage_pouch",
      "Increase carry capacity by one slot for the next raid.",
      3,
      1};
  }

  CraftingResult CraftRecipe(
    const CraftingRecipe& recipe,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState)
  {
    if (workshopState.salvagePouchCrafted)
    {
      return CraftingResult{false, "Salvage Pouch already crafted."};
    }

    if (!Peter::Inventory::ConsumeFromLedger(inventory.stash, recipe.inputItemId, recipe.inputCount))
    {
      return CraftingResult{false, "Not enough extracted salvage to craft the upgrade."};
    }

    loadout.carrySlotBonus += recipe.capacityBonus;
    loadout.salvagePouchEquipped = true;
    inventory.carrySlotCapacity = Peter::Inventory::MaxCarrySlots(loadout);
    workshopState.salvagePouchCrafted = true;
    return CraftingResult{true, "Crafted Salvage Pouch. Carry capacity increased by one slot."};
  }

  Peter::Core::StructuredFields ToSaveFields(const WorkshopState& workshopState)
  {
    return Peter::Core::StructuredFields{
      {"salvage_pouch_crafted", workshopState.salvagePouchCrafted ? "true" : "false"}
    };
  }

  WorkshopState WorkshopStateFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    WorkshopState state;
    const auto iterator = fields.find("salvage_pouch_crafted");
    state.salvagePouchCrafted = iterator != fields.end() && iterator->second == "true";
    return state;
  }
} // namespace Peter::Progression
