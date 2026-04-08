#pragma once

#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Core
{
  enum class CreatorContentKind
  {
    TinkerPreset,
    LogicRules,
    TinyScript,
    MiniMission,
    MentorReport
  };

  [[nodiscard]] std::string_view ToString(CreatorContentKind kind);

  class CreatorContentStore
  {
  public:
    CreatorContentStore(ProfileInfo profile, EventBus& eventBus);

    void EnsureLayout() const;
    [[nodiscard]] std::filesystem::path Root() const;
    [[nodiscard]] std::filesystem::path KindRoot(CreatorContentKind kind) const;
    [[nodiscard]] bool ArtifactExists(
      CreatorContentKind kind,
      std::string_view contentId,
      int revision = -1) const;
    [[nodiscard]] StructuredFields ReadArtifact(
      CreatorContentKind kind,
      const std::string& contentId,
      int revision = -1) const;
    [[nodiscard]] int WriteArtifact(
      CreatorContentKind kind,
      const std::string& contentId,
      const StructuredFields& fields,
      int revision = -1) const;
    [[nodiscard]] std::vector<std::string> ListArtifactIds(CreatorContentKind kind) const;
    [[nodiscard]] std::filesystem::path WriteMentorSummary(
      std::string_view reportId,
      std::string_view body) const;

  private:
    [[nodiscard]] int ResolveLatestRevision(CreatorContentKind kind, std::string_view contentId) const;
    [[nodiscard]] std::filesystem::path ArtifactPath(
      CreatorContentKind kind,
      std::string_view contentId,
      int revision) const;

    ProfileInfo m_profile;
    EventBus& m_eventBus;
  };
} // namespace Peter::Core
