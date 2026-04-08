#pragma once

#include "PeterCore/EventBus.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Inventory
{
  using ItemLedger = std::map<std::string, int, std::less<>>;

  enum class ItemCategory
  {
    Salvage,
    Consumable,
    Tool,
    Gadget,
    CompanionModule,
    CraftingMaterial,
    QuestItem
  };

  enum class ItemRarity
  {
    Common,
    Uncommon,
    Rare,
    Epic
  };

  struct ItemDefinition
  {
    std::string id;
    std::string displayName;
    std::string description;
    ItemCategory category = ItemCategory::Salvage;
    ItemRarity rarity = ItemRarity::Common;
    std::string colorToken = "steel";
    std::string audioCueFamily = "loot.common";
    int stackLimit = 1;
    bool countsTowardCarrySlots = true;
    bool lostOnFailure = true;
    bool repairable = false;
    int maxDurability = 100;
  };

  using ItemCatalog = std::map<std::string, ItemDefinition, std::less<>>;

  struct InventoryState
  {
    ItemLedger carried;
    ItemLedger stash;
    int carrySlotCapacity = 2;
    ItemLedger equippedDurability;
  };

  struct LoadoutState
  {
    int baseCarrySlotCapacity = 2;
    int carrySlotBonus = 0;
    bool salvagePouchEquipped = false;
    std::string favoriteItemId;
    std::string equippedToolId = "item.tool.field_wrench";
    std::string equippedGadgetId = "item.gadget.burst_beacon";
    std::string equippedCompanionModuleId = "item.companion_module.guard_chip";
  };

  struct RecoveryState
  {
    std::string favoriteItemInRecovery;
    ItemLedger brokenItems;
    int favoriteReclaimCost = 1;
  };

  struct RecoveryResolution
  {
    ItemLedger brokenItems;
    std::string favoriteItemMovedToRecovery;
    int durabilityLossPercent = 25;
    std::string summary;
  };

  [[nodiscard]] const ItemCatalog& Phase2ItemCatalog();
  [[nodiscard]] const ItemDefinition* FindItemDefinition(std::string_view itemId);
  [[nodiscard]] std::string_view ToString(ItemCategory category);
  [[nodiscard]] std::string_view ToString(ItemRarity rarity);
  [[nodiscard]] bool IsDurableCategory(ItemCategory category);
  [[nodiscard]] bool IsRarityAllowed(ItemCategory category, ItemRarity rarity);
  [[nodiscard]] std::string RenderRarityFeedback(const ItemDefinition& definition);
  [[nodiscard]] std::vector<std::string> EquippedItemIds(const LoadoutState& loadout);

  [[nodiscard]] int MaxCarrySlots(const LoadoutState& loadout);
  [[nodiscard]] int UsedCarrySlots(const InventoryState& inventory);
  [[nodiscard]] bool TryAddCarriedItem(
    InventoryState& inventory,
    const LoadoutState& loadout,
    const std::string& itemId,
    int quantity,
    std::string* denialReason);
  [[nodiscard]] ItemLedger MoveCarriedToStash(InventoryState& inventory);
  [[nodiscard]] ItemLedger LoseCarried(InventoryState& inventory);
  void AddToLedger(ItemLedger& ledger, std::string_view itemId, int quantity);
  [[nodiscard]] bool ConsumeFromLedger(ItemLedger& ledger, std::string_view itemId, int quantity);
  [[nodiscard]] int CountItem(const ItemLedger& ledger, std::string_view itemId);
  [[nodiscard]] std::string SerializeLedger(const ItemLedger& ledger);
  [[nodiscard]] ItemLedger ParseLedger(std::string_view serializedLedger);
  [[nodiscard]] std::string SummarizeLedger(const ItemLedger& ledger);
  [[nodiscard]] RecoveryResolution ApplyRaidFailureRecovery(
    InventoryState& inventory,
    const LoadoutState& loadout,
    RecoveryState& recoveryState,
    bool reducedTimePressure);
  [[nodiscard]] bool RepairBrokenItem(
    RecoveryState& recoveryState,
    InventoryState& inventory,
    std::string_view itemId,
    std::string* summary);
  [[nodiscard]] bool ReclaimFavoriteItem(
    RecoveryState& recoveryState,
    InventoryState& inventory,
    LoadoutState& loadout,
    std::string* summary);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(
    const InventoryState& inventory,
    const LoadoutState& loadout);
  [[nodiscard]] Peter::Core::StructuredFields RecoveryStateToSaveFields(const RecoveryState& recoveryState);
  void LoadFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    InventoryState& inventory,
    LoadoutState& loadout);
  void LoadRecoveryStateFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    RecoveryState& recoveryState);
} // namespace Peter::Inventory
