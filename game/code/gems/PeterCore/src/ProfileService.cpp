#include "PeterCore/ProfileService.h"

#include "PeterCore/StableId.h"

#include <stdexcept>

namespace Peter::Core
{
  ProfileService::ProfileService(const Peter::Adapters::ISaveAdapter& saveAdapter, EventBus& eventBus)
    : m_saveAdapter(saveAdapter)
    , m_eventBus(eventBus)
  {
  }

  ProfileInfo ProfileService::EnsureProfile(const std::string& profileId) const
  {
    if (!StableId::IsValid(profileId))
    {
      throw std::invalid_argument("Profile ID must use dotted lowercase stable ID form.");
    }

    const auto root = m_saveAdapter.ResolveProfileRoot() / profileId;
    const auto saveDataRoot = root / "SaveData";
    const auto userContentRoot = root / "UserContent";

    m_saveAdapter.EnsureDirectory(root);
    m_saveAdapter.EnsureDirectory(saveDataRoot);
    m_saveAdapter.EnsureDirectory(userContentRoot);

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.profile.ready",
      {
        {"profile_id", profileId},
        {"root", root.string()}
      }});

    return ProfileInfo{profileId, root, saveDataRoot, userContentRoot};
  }
} // namespace Peter::Core
