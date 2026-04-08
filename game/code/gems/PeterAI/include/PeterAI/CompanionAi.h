#pragma once

#include "PeterCore/EventBus.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::AI
{
  enum class StimulusKind
  {
    LineOfSight,
    HeardEvent,
    InterestMarker,
    ThreatMemory
  };

  enum class AlertLevel
  {
    Calm,
    Curious,
    Alert,
    Critical
  };

  enum class ActionExecutionStage
  {
    Started,
    Succeeded,
    Failed,
    Interrupted
  };

  enum class EnemyVariant
  {
    MeleeChaser,
    AlarmSupport
  };

  struct CompanionConfig
  {
    double followDistanceMeters = 6.0;
    bool holdPosition = false;
    std::string stanceId = "stance.balanced";
    std::vector<std::string> activeChipIds{"chip.stay_near_me"};
    std::map<std::string, double, std::less<>> chipValues{{"chip.stay_near_me", 6.0}};
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
    bool timedMissionPressure = false;
    bool guardProtocolUnlocked = false;
    bool repairPulseUnlocked = false;
    bool lootPingUnlocked = false;
    bool playerInCover = false;
    bool interestMarkerActive = false;
    int distanceToPlayerMeters = 0;
    int distanceToThreatMeters = 6;
    int companionHealthPercent = 100;
    int extractionUrgency = 0;
    int urgencyLevel = 0;
    std::string roomNodeId = "room.unknown";
    std::string routeNodeId = "route.none";
    std::string currentGoal = "goal.follow_player";
    std::string currentTargetId = "player";
    std::string visibleThreatId = "enemy.none";
    std::string lastKnownThreatPositionToken;
    std::string heardEventToken;
    std::string interestMarkerId;
    std::string lastFailedAction;
  };

  struct PerceptionStimulus
  {
    std::string stimulusId;
    StimulusKind kind = StimulusKind::LineOfSight;
    std::string sourceId;
    std::string positionToken;
    int urgencyLevel = 0;
    double confidence = 0.0;
    std::string reasonText;
  };

  struct ThreatMemoryEntry
  {
    std::string threatId;
    std::string lastKnownPositionToken;
    int ageTurns = 0;
    int urgencyLevel = 0;
    double confidence = 0.0;
  };

  struct PerceptionSnapshot
  {
    std::vector<PerceptionStimulus> stimuli;
    std::vector<ThreatMemoryEntry> rememberedThreats;
    AlertLevel alertLevel = AlertLevel::Calm;
    std::string noticedReason;
    std::string roomNodeId;
    std::string routeNodeId;
  };

  struct AgentBlackboardState
  {
    std::string currentTargetId;
    std::string lastKnownThreatPositionToken;
    std::string currentGoalId;
    double healthConfidence = 1.0;
    std::string playerState;
    std::string activeStanceId;
    std::vector<std::string> activeChipIds;
    int extractionUrgency = 0;
    std::string lastFailedActionId;
    std::string routeNodeId;
    std::string roomNodeId;
    std::string alertLevel;
  };

  struct DecisionBreakdown
  {
    std::string label;
    double delta = 0.0;
    std::string explanation;
  };

  struct DecisionCandidate
  {
    std::string actionId;
    std::string goalId;
    double score = 0.0;
    int priority = 100;
    std::vector<DecisionBreakdown> breakdowns;
  };

  struct CandidateOverride
  {
    std::string sourceId;
    std::string actionId;
    double scoreDelta = 0.0;
    bool gateAction = false;
    std::string explanation;
  };

  struct BehaviorOverrideSet
  {
    std::vector<CandidateOverride> overrides;
    std::string summary;
  };

  struct ActionPlan
  {
    std::string actionId;
    std::string goalId;
    std::string targetId;
    std::string routeNodeId;
    std::vector<std::string> telemetryTags;
  };

  struct ActionResult
  {
    std::string actionId;
    ActionExecutionStage stage = ActionExecutionStage::Succeeded;
    bool recoverable = true;
    bool success = true;
    std::string failureReason;
    std::string summary;
  };

  struct BehaviorStanceDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    double attackBias = 0.0;
    double defendBias = 0.0;
    double lootBias = 0.0;
    double reviveBias = 0.0;
    double extractionBias = 0.0;
    double cautionBias = 0.0;
  };

  struct BehaviorChipDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::string promiseText;
    std::string cautionText;
    std::string mappedVariableId;
    double defaultValue = 0.0;
    double minValue = 0.0;
    double maxValue = 1.0;
  };

  struct EnemyUnit
  {
    std::string enemyId;
    EnemyVariant variant = EnemyVariant::MeleeChaser;
    std::string roomId;
    std::string archetypeId;
    std::string patrolRouteId;
    bool alerted = false;
    int health = 20;
    bool active = true;
  };

  struct EnemyArchetypeDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    EnemyVariant variant = EnemyVariant::MeleeChaser;
    double patrolBias = 0.0;
    double investigateBias = 0.0;
    double chaseBias = 0.0;
    double ambushBias = 0.0;
    double returnBias = 0.0;
    int preferredRangeMeters = 4;
  };

  struct PatrolRouteDefinition
  {
    std::string id;
    std::string displayName;
    std::vector<std::string> nodeIds;
    bool looped = true;
  };

  struct InterestMarkerDefinition
  {
    std::string id;
    std::string displayName;
    std::string roomNodeId;
    std::string markerType;
    int urgencyLevel = 1;
  };

  struct AiScenarioStepDefinition
  {
    std::string stepId;
    CompanionWorldContext worldContext;
    bool useEnemy = false;
    EnemyUnit enemy;
    CompanionConfig config;
    std::string expectedActionId;
    std::string expectedState;
  };

  struct AiScenarioDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    bool deterministic = true;
    std::vector<AiScenarioStepDefinition> steps;
  };

  struct AgentExplainSnapshot
  {
    std::string agentId;
    std::string currentGoal;
    std::string lastCompletedAction;
    std::string topReason;
    std::string secondaryReason;
    std::string confidenceLabel;
    std::string riskIndicator;
    std::string editDelta;
    std::string routeNodeId;
    std::string lastActionFailureReason;
    PerceptionSnapshot perception;
    AgentBlackboardState blackboard;
    std::vector<DecisionCandidate> topCandidates;
    std::vector<std::string> stanceModifiers;
    std::vector<std::string> chipModifiers;
  };

  struct CompanionDecisionSnapshot
  {
    std::string currentState;
    std::string currentGoal;
    std::string lastAction;
    std::string lastCompletedAction;
    std::string topReason;
    std::string secondaryReason;
    std::string influentialVariableA;
    std::string influentialVariableB;
    std::string confidenceLabel;
    std::string riskIndicator;
    std::string editDelta;
    std::string calloutToken;
    std::string gestureToken;
    std::string debugSummary;
    PerceptionSnapshot perception;
    AgentBlackboardState blackboard;
    std::vector<DecisionCandidate> topCandidates;
    std::vector<std::string> stanceModifiers;
    std::vector<std::string> chipModifiers;
    ActionPlan plan;
    ActionResult lastResult;
  };

  struct EnemyDecisionSnapshot
  {
    std::string currentState;
    std::string lastAction;
    std::string topReason;
    std::string secondaryReason;
    std::string confidenceLabel;
    PerceptionSnapshot perception;
    AgentBlackboardState blackboard;
    std::vector<DecisionCandidate> topCandidates;
    ActionPlan plan;
    ActionResult lastResult;
  };

  [[nodiscard]] bool HasLineOfSight(int distanceToTargetMeters, int maxSightMeters, bool occluded);
  [[nodiscard]] bool CanHearEvent(int loudness, int distanceToEventMeters, int hearingThreshold);
  [[nodiscard]] std::vector<ThreatMemoryEntry> DecayThreatMemory(
    std::vector<ThreatMemoryEntry> memory,
    int turns);
  [[nodiscard]] double ResolveFollowDistance(const CompanionConfig& config);
  [[nodiscard]] CompanionConfig DefaultCompanionConfig();
  [[nodiscard]] PerceptionSnapshot BuildPerceptionSnapshot(
    const CompanionWorldContext& context,
    std::string_view agentId);
  [[nodiscard]] AgentExplainSnapshot BuildExplainSnapshot(const CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] AgentExplainSnapshot BuildExplainSnapshot(const EnemyDecisionSnapshot& snapshot);
  [[nodiscard]] CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& config,
    const CompanionWorldContext& context);
  [[nodiscard]] CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& config,
    const CompanionWorldContext& context,
    const BehaviorOverrideSet& overrides);
  [[nodiscard]] EnemyDecisionSnapshot EvaluateEnemy(
    const EnemyUnit& enemy,
    const CompanionWorldContext& context);
  [[nodiscard]] std::string RenderExplainText(const CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const CompanionConfig& config);
  [[nodiscard]] CompanionConfig CompanionConfigFromSaveFields(const Peter::Core::StructuredFields& fields);
  [[nodiscard]] const std::vector<BehaviorStanceDefinition>& BuildPhase3Stances();
  [[nodiscard]] const BehaviorStanceDefinition* FindBehaviorStance(std::string_view stanceId);
  [[nodiscard]] const std::vector<BehaviorChipDefinition>& BuildPhase3BehaviorChips();
  [[nodiscard]] const BehaviorChipDefinition* FindBehaviorChip(std::string_view chipId);
  [[nodiscard]] const std::vector<EnemyArchetypeDefinition>& BuildPhase3EnemyArchetypes();
  [[nodiscard]] const EnemyArchetypeDefinition* FindEnemyArchetype(std::string_view archetypeId);
  [[nodiscard]] const std::vector<PatrolRouteDefinition>& BuildPhase3PatrolRoutes();
  [[nodiscard]] const PatrolRouteDefinition* FindPatrolRoute(std::string_view routeId);
  [[nodiscard]] const std::vector<InterestMarkerDefinition>& BuildPhase3InterestMarkers();
  [[nodiscard]] const std::vector<AiScenarioDefinition>& BuildPhase3AiScenarios();
  [[nodiscard]] const AiScenarioDefinition* FindAiScenario(std::string_view scenarioId);
  [[nodiscard]] std::string_view ToString(StimulusKind kind);
  [[nodiscard]] std::string_view ToString(AlertLevel level);
  [[nodiscard]] std::string_view ToString(ActionExecutionStage stage);
  [[nodiscard]] std::string_view ToString(EnemyVariant variant);
  [[nodiscard]] EnemyUnit BuildEnemyUnit(
    std::string_view enemyId,
    EnemyVariant variant,
    std::string_view roomId);
} // namespace Peter::AI
