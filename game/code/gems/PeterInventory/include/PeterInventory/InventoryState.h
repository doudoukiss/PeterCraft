#pragma once

#include "PeterCore/EventBus.h"

#include <map>
#include <string>
#include <string_view>

namespace Peter::Inventory
{
  using ItemLedger = std::map<std::string, int, std::less<>>;

  struct InventoryState
  {
    ItemLedger carried;
    ItemLedger stash;
    int carrySlotCapacity = 2;
  };

  struct LoadoutState
  {
    int baseCarrySlotCapacity = 2;
    int carrySlotBonus = 0;
    bool salvagePouchEquipped = false;
  };

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
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(
    const InventoryState& inventory,
    const LoadoutState& loadout);
  void LoadFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    InventoryState& inventory,
    LoadoutState& loadout);
} // namespace Peter::Inventory
