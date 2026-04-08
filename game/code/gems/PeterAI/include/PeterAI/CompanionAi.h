#pragma once

#include "PeterCore/EventBus.h"

#include <string>
#include <string_view>

namespace Peter::AI
{
  struct CompanionConfig
  {
    double followDistanceMeters = 6.0;
    bool holdPosition = false;
  };

  struct CompanionWorldContext
  {
    bool threatVisible = false;
    bool sameTargetMarked = false;
    bool extractionActive = false;
    bool playerLowHealth = false;
    bool playerNeedsRevive = false;
    bool unsafeToAdvance = false;
    bool rareLootVisible = false;
    int distanceToPlayerMeters = 0;
  };

  struct CompanionDecisionSnapshot
  {
    std::string currentState;
    std::string lastAction;
    std::string topReason;
    std::string influentialVariableA;
    std::string influentialVariableB;
    std::string debugSummary;
  };

  enum class EnemyVariant
  {
    MeleeChaser,
    AlarmSupport
  };

  struct EnemyUnit
  {
    std::string enemyId;
    EnemyVariant variant = EnemyVariant::MeleeChaser;
    std::string roomId;
    bool alerted = false;
    int health = 20;
    bool active = true;
  };

  [[nodiscard]] CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& config,
    const CompanionWorldContext& context);
  [[nodiscard]] std::string RenderExplainText(const CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const CompanionConfig& config);
  [[nodiscard]] CompanionConfig CompanionConfigFromSaveFields(const Peter::Core::StructuredFields& fields);
  [[nodiscard]] std::string_view ToString(EnemyVariant variant);
  [[nodiscard]] EnemyUnit BuildEnemyUnit(
    std::string_view enemyId,
    EnemyVariant variant,
    std::string_view roomId);
} // namespace Peter::AI
