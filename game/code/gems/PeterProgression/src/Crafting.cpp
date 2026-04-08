#include "PeterProgression/Crafting.h"

#include <sstream>

namespace Peter::Progression
{
  namespace
  {
    std::string SerializeUnlockedNodes(const WorkshopState& workshopState)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [nodeId, unlocked] : workshopState.unlockedNodes)
      {
        if (!unlocked)
        {
          continue;
        }

        if (!first)
        {
          output << ",";
        }
        output << nodeId;
        first = false;
      }
      return output.str();
    }

    void ParseUnlockedNodes(std::string_view serialized, WorkshopState& workshopState)
    {
      std::size_t cursor = 0;
      while (cursor < serialized.size())
      {
        const auto separator = serialized.find(',', cursor);
        const auto token = serialized.substr(
          cursor,
          separator == std::string_view::npos ? serialized.size() - cursor : separator - cursor);
        if (!token.empty())
        {
          workshopState.unlockedNodes[std::string(token)] = true;
        }

        if (separator == std::string_view::npos)
        {
          break;
        }
        cursor = separator + 1;
      }
    }

    const UpgradeTrackDefinition* TrackForNode(const std::string_view nodeId)
    {
      for (const auto& track : BuildPhase2UpgradeTracks())
      {
        for (const auto& node : track.nodes)
        {
          if (node.nodeId == nodeId)
          {
            return &track;
          }
        }
      }
      return nullptr;
    }
  } // namespace

  CraftingRecipe BuildPhase1SalvagePouchRecipe()
  {
    return CraftingRecipe{
      "recipe.upgrade.salvage_pouch",
      "Salvage Pouch",
      "item.salvage.scrap_metal",
      "track.inventory_capacity.salvage_pouch",
      "Increase carry capacity by one slot for the next raid.",
      3,
      1};
  }

  const std::vector<UpgradeTrackDefinition>& BuildPhase2UpgradeTracks()
  {
    static const std::vector<UpgradeTrackDefinition> tracks = {
      UpgradeTrackDefinition{
        "track.player_tools",
        "Player Tools",
        "Unlock better field utility without hidden stats.",
        {
          {"track.player_tools.quick_patch", "Quick Patch", "Repairs cost less Scrap Metal.", "item.salvage.scrap_metal", "", "repair_discount", 2, 1},
          {"track.player_tools.machine_key", "Machine Key", "Activate more machine objectives safely.", "item.material.nanofiber", "track.player_tools.quick_patch", "mission_access", 2, 1},
          {"track.player_tools.steady_grip", "Steady Grip", "Improve interaction-heavy moments.", "item.salvage.power_cell", "track.player_tools.machine_key", "interaction_assist", 1, 1}
        }},
      UpgradeTrackDefinition{
        "track.inventory_capacity",
        "Inventory Capacity",
        "Carry a little more each run without bloating the system.",
        {
          {"track.inventory_capacity.salvage_pouch", "Salvage Pouch", "Gain one extra carry slot.", "item.salvage.scrap_metal", "", "carry_bonus", 3, 1},
          {"track.inventory_capacity.side_holster", "Side Holster", "Gain one more carry slot for risky runs.", "item.material.nanofiber", "track.inventory_capacity.salvage_pouch", "carry_bonus", 2, 1},
          {"track.inventory_capacity.recovery_satchel", "Recovery Satchel", "Gain one final carry slot and safer recovery handling.", "item.salvage.power_cell", "track.inventory_capacity.side_holster", "carry_bonus", 1, 1}
        }},
      UpgradeTrackDefinition{
        "track.companion_capabilities",
        "Companion Capabilities",
        "Expand what the companion can explain and do.",
        {
          {"track.companion_capabilities.guard_protocol", "Guard Protocol", "Unlock guarded support actions.", "item.salvage.scrap_metal", "", "companion_guard", 2, 1},
          {"track.companion_capabilities.repair_pulse", "Repair Pulse", "Unlock repair support actions.", "item.material.nanofiber", "track.companion_capabilities.guard_protocol", "companion_repair", 2, 1},
          {"track.companion_capabilities.loot_ping", "Loot Ping", "Unlock clearer rare-loot callouts.", "item.salvage.power_cell", "track.companion_capabilities.repair_pulse", "companion_loot_ping", 1, 1}
        }},
      UpgradeTrackDefinition{
        "track.creator_unlocks",
        "Creator Unlocks",
        "Earn safer workshop depth through play.",
        {
          {"track.creator_unlocks.lesson_archive", "Lesson Archive", "Replay guided workshop lessons from home.", "item.salvage.scrap_metal", "", "lesson_replay", 2, 1},
          {"track.creator_unlocks.safe_simulation", "Safe Simulation", "Unlock a no-penalty workshop simulation option.", "item.material.nanofiber", "track.creator_unlocks.lesson_archive", "safe_mode", 2, 1},
          {"track.creator_unlocks.mission_choice_console", "Mission Choice Console", "Unlock wider mission selection in the workshop yard.", "item.salvage.power_cell", "track.creator_unlocks.safe_simulation", "mission_choice", 1, 1}
        }}
    };

    return tracks;
  }

  const UpgradeTrackDefinition* FindUpgradeTrack(const std::string_view trackId)
  {
    for (const auto& track : BuildPhase2UpgradeTracks())
    {
      if (track.trackId == trackId)
      {
        return &track;
      }
    }
    return nullptr;
  }

  const UpgradeTrackNodeDefinition* FindUpgradeNode(const std::string_view nodeId)
  {
    for (const auto& track : BuildPhase2UpgradeTracks())
    {
      for (const auto& node : track.nodes)
      {
        if (node.nodeId == nodeId)
        {
          return &node;
        }
      }
    }
    return nullptr;
  }

  bool HasUnlockedNode(const WorkshopState& workshopState, const std::string_view nodeId)
  {
    const auto iterator = workshopState.unlockedNodes.find(std::string(nodeId));
    return iterator != workshopState.unlockedNodes.end() && iterator->second;
  }

  ProgressionResult UnlockUpgradeNode(
    const std::string_view nodeId,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState)
  {
    const auto* node = FindUpgradeNode(nodeId);
    if (node == nullptr)
    {
      return ProgressionResult{false, "Unknown upgrade node.", ""};
    }

    if (HasUnlockedNode(workshopState, nodeId))
    {
      return ProgressionResult{false, "Upgrade already unlocked.", ""};
    }

    if (!node->prerequisiteNodeId.empty() && !HasUnlockedNode(workshopState, node->prerequisiteNodeId))
    {
      return ProgressionResult{false, "Unlock the previous node in this track first.", ""};
    }

    if (!Peter::Inventory::ConsumeFromLedger(inventory.stash, node->costItemId, node->cost))
    {
      return ProgressionResult{false, "Not enough extracted resources for this unlock.", ""};
    }

    workshopState.unlockedNodes[node->nodeId] = true;

    if (node->nodeId == "track.inventory_capacity.salvage_pouch")
    {
      workshopState.salvagePouchCrafted = true;
      loadout.salvagePouchEquipped = true;
    }

    if (node->effectType == "carry_bonus")
    {
      loadout.carrySlotBonus += node->effectValue;
      inventory.carrySlotCapacity = Peter::Inventory::MaxCarrySlots(loadout);
    }

    std::string nextReward = "track complete";
    if (const auto* track = TrackForNode(nodeId))
    {
      for (const auto& candidate : track->nodes)
      {
        if (!HasUnlockedNode(workshopState, candidate.nodeId))
        {
          nextReward = candidate.displayName;
          break;
        }
      }
    }

    return ProgressionResult{
      true,
      "Unlocked " + node->displayName + ".",
      nextReward};
  }

  std::string RenderTrackSummary(
    const std::vector<UpgradeTrackDefinition>& tracks,
    const WorkshopState& workshopState)
  {
    std::ostringstream output;
    bool firstTrack = true;
    for (const auto& track : tracks)
    {
      if (!firstTrack)
      {
        output << "\n";
      }

      int unlockedCount = 0;
      std::string nextReward = "track complete";
      for (const auto& node : track.nodes)
      {
        if (HasUnlockedNode(workshopState, node.nodeId))
        {
          ++unlockedCount;
        }
        else if (nextReward == "track complete")
        {
          nextReward = node.displayName;
        }
      }

      output << track.displayName << ": " << unlockedCount << "/" << track.nodes.size()
             << " unlocked | next=" << nextReward;
      firstTrack = false;
    }
    return output.str();
  }

  CraftingResult CraftRecipe(
    const CraftingRecipe& recipe,
    Peter::Inventory::InventoryState& inventory,
    Peter::Inventory::LoadoutState& loadout,
    WorkshopState& workshopState)
  {
    const auto result = UnlockUpgradeNode(recipe.upgradeId, inventory, loadout, workshopState);
    return CraftingResult{result.unlocked, result.summary};
  }

  Peter::Core::StructuredFields ToSaveFields(const WorkshopState& workshopState)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"salvage_pouch_crafted", workshopState.salvagePouchCrafted ? "true" : "false"},
      {"unlocked_nodes", SerializeUnlockedNodes(workshopState)}
    };
  }

  WorkshopState WorkshopStateFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    WorkshopState state;
    const auto schemaVersion = fields.find("schema_version");
    const auto oldSalvagePouch = fields.find("salvage_pouch_crafted");
    const auto unlockedNodes = fields.find("unlocked_nodes");

    if (schemaVersion == fields.end())
    {
      state.salvagePouchCrafted = oldSalvagePouch != fields.end() && oldSalvagePouch->second == "true";
      if (state.salvagePouchCrafted)
      {
        state.unlockedNodes["track.inventory_capacity.salvage_pouch"] = true;
      }
      return state;
    }

    state.salvagePouchCrafted = oldSalvagePouch != fields.end() && oldSalvagePouch->second == "true";
    if (unlockedNodes != fields.end())
    {
      ParseUnlockedNodes(unlockedNodes->second, state);
    }
    if (state.salvagePouchCrafted)
    {
      state.unlockedNodes["track.inventory_capacity.salvage_pouch"] = true;
    }
    return state;
  }
} // namespace Peter::Progression
