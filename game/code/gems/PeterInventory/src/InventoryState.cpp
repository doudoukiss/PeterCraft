#include "PeterInventory/InventoryState.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace Peter::Inventory
{
  namespace
  {
    const ItemDefinition* SafeFind(const std::string_view itemId)
    {
      return FindItemDefinition(itemId);
    }

    void EnsureDurabilityEntry(
      InventoryState& inventory,
      const std::string& itemId)
    {
      if (itemId.empty())
      {
        return;
      }

      const auto* definition = SafeFind(itemId);
      if (definition == nullptr || !definition->repairable)
      {
        return;
      }

      const auto iterator = inventory.equippedDurability.find(itemId);
      if (iterator == inventory.equippedDurability.end())
      {
        inventory.equippedDurability[itemId] = definition->maxDurability;
      }
    }

    int RepairCostForItem(const std::string_view itemId)
    {
      const auto* definition = SafeFind(itemId);
      if (definition == nullptr)
      {
        return 2;
      }

      switch (definition->rarity)
      {
        case ItemRarity::Common:
        case ItemRarity::Uncommon:
          return 1;
        case ItemRarity::Rare:
          return 2;
        case ItemRarity::Epic:
          return 3;
      }

      return 2;
    }

    int DurabilityLossPercent(const bool reducedTimePressure)
    {
      return reducedTimePressure ? 13 : 25;
    }
  } // namespace

  const ItemCatalog& Phase2ItemCatalog()
  {
    static const ItemCatalog catalog = []() {
      ItemCatalog built;
      built.emplace(
        "item.salvage.scrap_metal",
        ItemDefinition{
          "item.salvage.scrap_metal",
          "Scrap Metal",
          "Reliable base salvage used for repairs and starter upgrades.",
          ItemCategory::Salvage,
          ItemRarity::Common,
          "steel",
          "loot.common",
          50,
          true,
          true,
          false,
          0});
      built.emplace(
        "item.salvage.power_cell",
        ItemDefinition{
          "item.salvage.power_cell",
          "Power Cell",
          "A high-output cell recovered from machine vaults.",
          ItemCategory::CraftingMaterial,
          ItemRarity::Rare,
          "amber",
          "loot.rare",
          10,
          true,
          true,
          false,
          0});
      built.emplace(
        "item.material.nanofiber",
        ItemDefinition{
          "item.material.nanofiber",
          "Nanofiber Coil",
          "Flexible crafting material used for advanced upgrades.",
          ItemCategory::CraftingMaterial,
          ItemRarity::Uncommon,
          "teal",
          "loot.uncommon",
          12,
          true,
          true,
          false,
          0});
      built.emplace(
        "item.consumable.field_ration",
        ItemDefinition{
          "item.consumable.field_ration",
          "Field Ration",
          "A small recovery snack that keeps a run moving.",
          ItemCategory::Consumable,
          ItemRarity::Common,
          "olive",
          "loot.common",
          3,
          true,
          true,
          false,
          0});
      built.emplace(
        "item.consumable.stabilizer_gel",
        ItemDefinition{
          "item.consumable.stabilizer_gel",
          "Stabilizer Gel",
          "A rare support consumable that softens timed pressure.",
          ItemCategory::Consumable,
          ItemRarity::Rare,
          "azure",
          "loot.rare",
          2,
          true,
          true,
          false,
          0});
      built.emplace(
        "item.tool.field_wrench",
        ItemDefinition{
          "item.tool.field_wrench",
          "Field Wrench",
          "Starter tool used to repair and activate old machines.",
          ItemCategory::Tool,
          ItemRarity::Uncommon,
          "teal",
          "loot.uncommon",
          1,
          false,
          false,
          true,
          100});
      built.emplace(
        "item.gadget.burst_beacon",
        ItemDefinition{
          "item.gadget.burst_beacon",
          "Burst Beacon",
          "A compact gadget that helps mark threats and loot.",
          ItemCategory::Gadget,
          ItemRarity::Rare,
          "azure",
          "loot.rare",
          1,
          false,
          false,
          true,
          100});
      built.emplace(
        "item.companion_module.guard_chip",
        ItemDefinition{
          "item.companion_module.guard_chip",
          "Guard Chip",
          "Starter companion module for safer support behavior.",
          ItemCategory::CompanionModule,
          ItemRarity::Uncommon,
          "teal",
          "loot.uncommon",
          1,
          false,
          false,
          true,
          100});
      built.emplace(
        "item.quest.artifact_seed",
        ItemDefinition{
          "item.quest.artifact_seed",
          "Artifact Seed",
          "A mission-critical relic that is preserved after failure.",
          ItemCategory::QuestItem,
          ItemRarity::Epic,
          "sunrise",
          "loot.epic",
          1,
          false,
          false,
          false,
          0});
      built.emplace(
        "item.quest.tutorial_schematic",
        ItemDefinition{
          "item.quest.tutorial_schematic",
          "Tutorial Schematic",
          "A workshop lesson item used for guided repairs.",
          ItemCategory::QuestItem,
          ItemRarity::Rare,
          "azure",
          "loot.rare",
          1,
          false,
          false,
          false,
          0});
      return built;
    }();

    return catalog;
  }

  const ItemDefinition* FindItemDefinition(const std::string_view itemId)
  {
    const auto& catalog = Phase2ItemCatalog();
    const auto iterator = catalog.find(std::string(itemId));
    return iterator == catalog.end() ? nullptr : &iterator->second;
  }

  std::string_view ToString(const ItemCategory category)
  {
    switch (category)
    {
      case ItemCategory::Salvage:
        return "salvage";
      case ItemCategory::Consumable:
        return "consumable";
      case ItemCategory::Tool:
        return "tool";
      case ItemCategory::Gadget:
        return "gadget";
      case ItemCategory::CompanionModule:
        return "companion_module";
      case ItemCategory::CraftingMaterial:
        return "crafting_material";
      case ItemCategory::QuestItem:
        return "quest_item";
    }

    return "salvage";
  }

  std::string_view ToString(const ItemRarity rarity)
  {
    switch (rarity)
    {
      case ItemRarity::Common:
        return "common";
      case ItemRarity::Uncommon:
        return "uncommon";
      case ItemRarity::Rare:
        return "rare";
      case ItemRarity::Epic:
        return "epic";
    }

    return "common";
  }

  bool IsDurableCategory(const ItemCategory category)
  {
    return category == ItemCategory::Tool || category == ItemCategory::Gadget ||
      category == ItemCategory::CompanionModule;
  }

  bool IsRarityAllowed(const ItemCategory category, const ItemRarity rarity)
  {
    if (category == ItemCategory::QuestItem)
    {
      return rarity == ItemRarity::Rare || rarity == ItemRarity::Epic;
    }

    return true;
  }

  std::string RenderRarityFeedback(const ItemDefinition& definition)
  {
    std::ostringstream output;
    output << definition.displayName << " [" << ToString(definition.rarity) << "]"
           << " color=" << definition.colorToken
           << " cue=" << definition.audioCueFamily;
    return output.str();
  }

  std::vector<std::string> EquippedItemIds(const LoadoutState& loadout)
  {
    std::vector<std::string> itemIds;
    if (!loadout.equippedToolId.empty())
    {
      itemIds.push_back(loadout.equippedToolId);
    }
    if (!loadout.equippedGadgetId.empty())
    {
      itemIds.push_back(loadout.equippedGadgetId);
    }
    if (!loadout.equippedCompanionModuleId.empty())
    {
      itemIds.push_back(loadout.equippedCompanionModuleId);
    }
    return itemIds;
  }

  int MaxCarrySlots(const LoadoutState& loadout)
  {
    return loadout.baseCarrySlotCapacity + loadout.carrySlotBonus;
  }

  int UsedCarrySlots(const InventoryState& inventory)
  {
    int count = 0;
    for (const auto& [itemId, quantity] : inventory.carried)
    {
      const auto* definition = SafeFind(itemId);
      if (!itemId.empty() && quantity > 0 && (definition == nullptr || definition->countsTowardCarrySlots))
      {
        ++count;
      }
    }
    return count;
  }

  bool TryAddCarriedItem(
    InventoryState& inventory,
    const LoadoutState& loadout,
    const std::string& itemId,
    const int quantity,
    std::string* denialReason)
  {
    if (quantity <= 0)
    {
      if (denialReason != nullptr)
      {
        *denialReason = "quantity must be positive";
      }
      return false;
    }

    const auto* definition = SafeFind(itemId);
    const int stackLimit = definition == nullptr ? 999 : definition->stackLimit;
    const bool countsTowardCarry = definition == nullptr ? true : definition->countsTowardCarrySlots;
    const auto existing = inventory.carried.find(itemId);
    const int existingQuantity = existing == inventory.carried.end() ? 0 : existing->second;

    if (existingQuantity + quantity > stackLimit)
    {
      if (denialReason != nullptr)
      {
        *denialReason = "stack limit reached";
      }
      return false;
    }

    if (existing == inventory.carried.end() && countsTowardCarry && UsedCarrySlots(inventory) >= MaxCarrySlots(loadout))
    {
      if (denialReason != nullptr)
      {
        *denialReason = "carry capacity reached";
      }
      return false;
    }

    inventory.carried[itemId] += quantity;
    inventory.carrySlotCapacity = MaxCarrySlots(loadout);
    return true;
  }

  ItemLedger MoveCarriedToStash(InventoryState& inventory)
  {
    ItemLedger moved = inventory.carried;
    for (const auto& [itemId, quantity] : moved)
    {
      inventory.stash[itemId] += quantity;
    }
    inventory.carried.clear();
    return moved;
  }

  ItemLedger LoseCarried(InventoryState& inventory)
  {
    ItemLedger lost;
    ItemLedger retained;

    for (const auto& [itemId, quantity] : inventory.carried)
    {
      const auto* definition = SafeFind(itemId);
      if (definition != nullptr && !definition->lostOnFailure)
      {
        retained[itemId] += quantity;
      }
      else
      {
        lost[itemId] += quantity;
      }
    }

    for (const auto& [itemId, quantity] : retained)
    {
      inventory.stash[itemId] += quantity;
    }
    inventory.carried.clear();
    return lost;
  }

  void AddToLedger(ItemLedger& ledger, const std::string_view itemId, const int quantity)
  {
    if (quantity > 0)
    {
      ledger[std::string(itemId)] += quantity;
    }
  }

  bool ConsumeFromLedger(ItemLedger& ledger, const std::string_view itemId, const int quantity)
  {
    auto iterator = ledger.find(std::string(itemId));
    if (iterator == ledger.end() || iterator->second < quantity)
    {
      return false;
    }

    iterator->second -= quantity;
    if (iterator->second == 0)
    {
      ledger.erase(iterator);
    }
    return true;
  }

  int CountItem(const ItemLedger& ledger, const std::string_view itemId)
  {
    const auto iterator = ledger.find(std::string(itemId));
    return iterator == ledger.end() ? 0 : iterator->second;
  }

  std::string SerializeLedger(const ItemLedger& ledger)
  {
    std::ostringstream output;
    bool first = true;
    for (const auto& [itemId, quantity] : ledger)
    {
      if (!first)
      {
        output << ",";
      }
      output << itemId << "=" << quantity;
      first = false;
    }
    return output.str();
  }

  ItemLedger ParseLedger(const std::string_view serializedLedger)
  {
    ItemLedger ledger;
    std::size_t cursor = 0;

    while (cursor < serializedLedger.size())
    {
      const auto nextSeparator = serializedLedger.find(',', cursor);
      const auto token = serializedLedger.substr(
        cursor,
        nextSeparator == std::string_view::npos ? serializedLedger.size() - cursor : nextSeparator - cursor);

      if (!token.empty())
      {
        const auto equals = token.find('=');
        if (equals != std::string_view::npos)
        {
          const auto itemId = token.substr(0, equals);
          const auto quantity = std::stoi(std::string(token.substr(equals + 1)));
          ledger[std::string(itemId)] = quantity;
        }
      }

      if (nextSeparator == std::string_view::npos)
      {
        break;
      }
      cursor = nextSeparator + 1;
    }

    return ledger;
  }

  std::string SummarizeLedger(const ItemLedger& ledger)
  {
    if (ledger.empty())
    {
      return "none";
    }

    std::ostringstream output;
    bool first = true;
    for (const auto& [itemId, quantity] : ledger)
    {
      if (!first)
      {
        output << ", ";
      }
      output << itemId << " x" << quantity;
      first = false;
    }
    return output.str();
  }

  RecoveryResolution ApplyRaidFailureRecovery(
    InventoryState& inventory,
    const LoadoutState& loadout,
    RecoveryState& recoveryState,
    const bool reducedTimePressure)
  {
    RecoveryResolution resolution;
    resolution.durabilityLossPercent = DurabilityLossPercent(reducedTimePressure);

    bool breakCapReached = false;
    for (const auto& itemId : EquippedItemIds(loadout))
    {
      const auto* definition = SafeFind(itemId);
      if (definition == nullptr || !definition->repairable)
      {
        continue;
      }

      EnsureDurabilityEntry(inventory, itemId);
      auto& durability = inventory.equippedDurability[itemId];
      const int updatedDurability = durability - resolution.durabilityLossPercent;

      if (!breakCapReached && updatedDurability <= 0)
      {
        durability = 0;
        resolution.brokenItems[itemId] = 1;
        breakCapReached = true;

        if (itemId == loadout.favoriteItemId)
        {
          recoveryState.favoriteItemInRecovery = itemId;
          resolution.favoriteItemMovedToRecovery = itemId;
        }
        else
        {
          recoveryState.brokenItems[itemId] = 1;
        }
      }
      else if (updatedDurability <= 0)
      {
        durability = 1;
      }
      else
      {
        durability = updatedDurability;
      }
    }

    std::ostringstream output;
    output << "Durability reduced by " << resolution.durabilityLossPercent << "%";
    if (!resolution.favoriteItemMovedToRecovery.empty())
    {
      output << "; favorite item moved to recovery";
    }
    else if (!resolution.brokenItems.empty())
    {
      output << "; one durable item needs repair";
    }
    else
    {
      output << "; no item fully broke this run";
    }
    resolution.summary = output.str();
    return resolution;
  }

  bool RepairBrokenItem(
    RecoveryState& recoveryState,
    InventoryState& inventory,
    const std::string_view itemId,
    std::string* summary)
  {
    if (CountItem(recoveryState.brokenItems, itemId) == 0)
    {
      if (summary != nullptr)
      {
        *summary = "That item is not waiting for repair.";
      }
      return false;
    }

    const int repairCost = RepairCostForItem(itemId);
    if (!ConsumeFromLedger(inventory.stash, "item.salvage.scrap_metal", repairCost))
    {
      if (summary != nullptr)
      {
        *summary = "Not enough Scrap Metal to repair the item.";
      }
      return false;
    }

    const bool removedBrokenItem = ConsumeFromLedger(recoveryState.brokenItems, itemId, 1);
    (void)removedBrokenItem;
    const auto* definition = SafeFind(itemId);
    inventory.equippedDurability[std::string(itemId)] = definition == nullptr ? 100 : definition->maxDurability;

    if (summary != nullptr)
    {
      *summary = "Repaired the item and restored full durability.";
    }
    return true;
  }

  bool ReclaimFavoriteItem(
    RecoveryState& recoveryState,
    InventoryState& inventory,
    LoadoutState& loadout,
    std::string* summary)
  {
    if (recoveryState.favoriteItemInRecovery.empty())
    {
      if (summary != nullptr)
      {
        *summary = "No favorite item is waiting in recovery.";
      }
      return false;
    }

    if (!ConsumeFromLedger(inventory.stash, "item.salvage.scrap_metal", recoveryState.favoriteReclaimCost))
    {
      if (summary != nullptr)
      {
        *summary = "Not enough Scrap Metal to reclaim the favorite item.";
      }
      return false;
    }

    const std::string itemId = recoveryState.favoriteItemInRecovery;
    const auto* definition = SafeFind(itemId);
    inventory.equippedDurability[itemId] = definition == nullptr ? 100 : definition->maxDurability;
    loadout.favoriteItemId = itemId;
    recoveryState.favoriteItemInRecovery.clear();

    if (summary != nullptr)
    {
      *summary = "Favorite item reclaimed and ready for the next run.";
    }
    return true;
  }

  Peter::Core::StructuredFields ToSaveFields(const InventoryState& inventory, const LoadoutState& loadout)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"carried", SerializeLedger(inventory.carried)},
      {"stash", SerializeLedger(inventory.stash)},
      {"equipped_durability", SerializeLedger(inventory.equippedDurability)},
      {"base_carry_slot_capacity", std::to_string(loadout.baseCarrySlotCapacity)},
      {"carry_slot_bonus", std::to_string(loadout.carrySlotBonus)},
      {"salvage_pouch_equipped", loadout.salvagePouchEquipped ? "true" : "false"},
      {"favorite_item_id", loadout.favoriteItemId},
      {"equipped_tool_id", loadout.equippedToolId},
      {"equipped_gadget_id", loadout.equippedGadgetId},
      {"equipped_companion_module_id", loadout.equippedCompanionModuleId}
    };
  }

  Peter::Core::StructuredFields RecoveryStateToSaveFields(const RecoveryState& recoveryState)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"favorite_item_in_recovery", recoveryState.favoriteItemInRecovery},
      {"favorite_reclaim_cost", std::to_string(recoveryState.favoriteReclaimCost)},
      {"broken_items", SerializeLedger(recoveryState.brokenItems)}
    };
  }

  void LoadFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    InventoryState& inventory,
    LoadoutState& loadout)
  {
    const auto schemaVersion = fields.find("schema_version");
    const auto carried = fields.find("carried");
    const auto stash = fields.find("stash");
    const auto durability = fields.find("equipped_durability");
    const auto baseCapacity = fields.find("base_carry_slot_capacity");
    const auto slotBonus = fields.find("carry_slot_bonus");
    const auto pouch = fields.find("salvage_pouch_equipped");
    const auto favoriteItemId = fields.find("favorite_item_id");
    const auto equippedToolId = fields.find("equipped_tool_id");
    const auto equippedGadgetId = fields.find("equipped_gadget_id");
    const auto equippedCompanionModuleId = fields.find("equipped_companion_module_id");

    inventory.carried = carried == fields.end() ? ItemLedger{} : ParseLedger(carried->second);
    inventory.stash = stash == fields.end() ? ItemLedger{} : ParseLedger(stash->second);
    loadout.baseCarrySlotCapacity = baseCapacity == fields.end() ? 2 : std::stoi(baseCapacity->second);
    loadout.carrySlotBonus = slotBonus == fields.end() ? 0 : std::stoi(slotBonus->second);
    loadout.salvagePouchEquipped = pouch != fields.end() && pouch->second == "true";

    if (schemaVersion == fields.end())
    {
      loadout.favoriteItemId.clear();
      loadout.equippedToolId = "item.tool.field_wrench";
      loadout.equippedGadgetId = "item.gadget.burst_beacon";
      loadout.equippedCompanionModuleId = "item.companion_module.guard_chip";
      inventory.equippedDurability.clear();
    }
    else
    {
      loadout.favoriteItemId = favoriteItemId == fields.end() ? "" : favoriteItemId->second;
      loadout.equippedToolId =
        equippedToolId == fields.end() ? "item.tool.field_wrench" : equippedToolId->second;
      loadout.equippedGadgetId =
        equippedGadgetId == fields.end() ? "item.gadget.burst_beacon" : equippedGadgetId->second;
      loadout.equippedCompanionModuleId =
        equippedCompanionModuleId == fields.end()
          ? "item.companion_module.guard_chip"
          : equippedCompanionModuleId->second;
      inventory.equippedDurability = durability == fields.end() ? ItemLedger{} : ParseLedger(durability->second);
    }

    EnsureDurabilityEntry(inventory, loadout.equippedToolId);
    EnsureDurabilityEntry(inventory, loadout.equippedGadgetId);
    EnsureDurabilityEntry(inventory, loadout.equippedCompanionModuleId);

    inventory.carrySlotCapacity = MaxCarrySlots(loadout);
  }

  void LoadRecoveryStateFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    RecoveryState& recoveryState)
  {
    const auto favorite = fields.find("favorite_item_in_recovery");
    const auto reclaimCost = fields.find("favorite_reclaim_cost");
    const auto brokenItems = fields.find("broken_items");

    recoveryState.favoriteItemInRecovery = favorite == fields.end() ? "" : favorite->second;
    recoveryState.favoriteReclaimCost = reclaimCost == fields.end() ? 1 : std::stoi(reclaimCost->second);
    recoveryState.brokenItems = brokenItems == fields.end() ? ItemLedger{} : ParseLedger(brokenItems->second);
  }
} // namespace Peter::Inventory
