#include "PeterInventory/InventoryState.h"

#include <sstream>

namespace Peter::Inventory
{
  int MaxCarrySlots(const LoadoutState& loadout)
  {
    return loadout.baseCarrySlotCapacity + loadout.carrySlotBonus;
  }

  int UsedCarrySlots(const InventoryState& inventory)
  {
    int count = 0;
    for (const auto& [itemId, quantity] : inventory.carried)
    {
      if (!itemId.empty() && quantity > 0)
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

    const auto existing = inventory.carried.find(itemId);
    if (existing == inventory.carried.end() && UsedCarrySlots(inventory) >= MaxCarrySlots(loadout))
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
    ItemLedger lost = inventory.carried;
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

  Peter::Core::StructuredFields ToSaveFields(const InventoryState& inventory, const LoadoutState& loadout)
  {
    return Peter::Core::StructuredFields{
      {"carried", SerializeLedger(inventory.carried)},
      {"stash", SerializeLedger(inventory.stash)},
      {"base_carry_slot_capacity", std::to_string(loadout.baseCarrySlotCapacity)},
      {"carry_slot_bonus", std::to_string(loadout.carrySlotBonus)},
      {"salvage_pouch_equipped", loadout.salvagePouchEquipped ? "true" : "false"}
    };
  }

  void LoadFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    InventoryState& inventory,
    LoadoutState& loadout)
  {
    const auto carried = fields.find("carried");
    const auto stash = fields.find("stash");
    const auto baseCapacity = fields.find("base_carry_slot_capacity");
    const auto slotBonus = fields.find("carry_slot_bonus");
    const auto pouch = fields.find("salvage_pouch_equipped");

    inventory.carried = carried == fields.end() ? ItemLedger{} : ParseLedger(carried->second);
    inventory.stash = stash == fields.end() ? ItemLedger{} : ParseLedger(stash->second);
    loadout.baseCarrySlotCapacity = baseCapacity == fields.end() ? 2 : std::stoi(baseCapacity->second);
    loadout.carrySlotBonus = slotBonus == fields.end() ? 0 : std::stoi(slotBonus->second);
    loadout.salvagePouchEquipped = pouch != fields.end() && pouch->second == "true";
    inventory.carrySlotCapacity = MaxCarrySlots(loadout);
  }
} // namespace Peter::Inventory
