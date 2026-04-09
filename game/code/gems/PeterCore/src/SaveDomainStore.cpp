#include "PeterCore/SaveDomainStore.h"

#include "PeterCore/StructuredStore.h"

#include <utility>

namespace Peter::Core
{
  SaveDomainStore::SaveDomainStore(ProfileInfo profile, EventBus& eventBus)
    : m_profile(std::move(profile))
    , m_eventBus(eventBus)
  {
  }

  bool SaveDomainStore::DomainExists(const std::string_view domainId) const
  {
    return std::filesystem::exists(DomainPath(domainId));
  }

  SaveReadResult SaveDomainStore::ReadDomainChecked(const std::string& domainId) const
  {
    SaveReadResult result;
    result.sourcePath = DomainPath(domainId);
    const auto parseResult = ParseStructuredFieldsFile(result.sourcePath);
    result.fields = parseResult.fields;
    result.valid = parseResult.valid;
    result.message = parseResult.valid ? "Domain loaded successfully." : parseResult.error;

    if (!parseResult.valid)
    {
      const auto restoreResult = RestoreStructuredFieldsFile(
        result.sourcePath,
        LatestBackupPath(domainId),
        PreviousBackupPath(domainId));
      if (restoreResult.success)
      {
        const auto repaired = ParseStructuredFieldsFile(result.sourcePath);
        result.fields = repaired.fields;
        result.valid = repaired.valid;
        result.restoredFromBackup = repaired.valid;
        result.message = repaired.valid ? "Domain restored from backup." : repaired.error;
      }
    }

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.domain.read",
      {
        {"domain_id", domainId},
        {"field_count", std::to_string(result.fields.size())},
        {"path", result.sourcePath.string()},
        {"restored_from_backup", result.restoredFromBackup ? "true" : "false"},
        {"valid", result.valid ? "true" : "false"}
      }});

    return result;
  }

  StructuredFields SaveDomainStore::ReadDomain(const std::string& domainId) const
  {
    return ReadDomainChecked(domainId).fields;
  }

  SaveWriteResult SaveDomainStore::WriteDomain(const std::string& domainId, const StructuredFields& fields) const
  {
    const auto path = DomainPath(domainId);
    const auto latestBackupPath = LatestBackupPath(domainId);
    const auto previousBackupPath = PreviousBackupPath(domainId);
    const auto writeResult = WriteStructuredFieldsFileAtomic(path, fields, latestBackupPath, previousBackupPath);

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.domain.write",
      {
        {"atomic_replace", writeResult.atomicReplace ? "true" : "false"},
        {"domain_id", domainId},
        {"duration_ms", std::to_string(writeResult.durationMs)},
        {"field_count", std::to_string(fields.size())},
        {"path", path.string()},
        {"success", writeResult.success ? "true" : "false"}
      }});

    return SaveWriteResult{
      writeResult.success,
      writeResult.durationMs,
      writeResult.bytesWritten,
      writeResult.message,
      path,
      latestBackupPath,
      previousBackupPath};
  }

  SaveRestoreResult SaveDomainStore::RestoreDomain(const std::string_view domainId) const
  {
    const auto restoreResult = RestoreStructuredFieldsFile(
      DomainPath(domainId),
      LatestBackupPath(domainId),
      PreviousBackupPath(domainId));

    m_eventBus.Emit(Event{
      EventCategory::SaveLoad,
      "save_load.domain.restore",
      {
        {"domain_id", std::string(domainId)},
        {"restored_from", restoreResult.restoredFromPath.string()},
        {"restored_to", restoreResult.restoredToPath.string()},
        {"success", restoreResult.success ? "true" : "false"}
      }});

    return SaveRestoreResult{
      restoreResult.success,
      restoreResult.message,
      restoreResult.restoredFromPath,
      restoreResult.restoredToPath};
  }

  SaveHealthReport SaveDomainStore::InspectHealth() const
  {
    SaveHealthReport report;
    if (!std::filesystem::exists(m_profile.saveDataRoot))
    {
      report.summary = "No save domains have been created yet.";
      return report;
    }

    for (const auto& entry : std::filesystem::directory_iterator(m_profile.saveDataRoot))
    {
      if (!entry.is_regular_file() || entry.path().extension() != ".json")
      {
        continue;
      }

      ++report.checkedDomains;
      const auto domainId = entry.path().stem().string();
      const auto readResult = ReadDomainChecked(domainId);
      if (readResult.restoredFromBackup)
      {
        ++report.restoredDomains;
      }
      if (!readResult.valid)
      {
        report.healthy = false;
        report.invalidDomainIds.push_back(domainId);
      }
    }

    report.summary = report.healthy
      ? "All save domains are healthy."
      : "One or more save domains are invalid.";
    return report;
  }

  std::filesystem::path SaveDomainStore::DomainPath(const std::string_view domainId) const
  {
    return m_profile.saveDataRoot / (std::string(domainId) + ".json");
  }

  std::filesystem::path SaveDomainStore::LatestBackupPath(const std::string_view domainId) const
  {
    return m_profile.backupRoot / (std::string(domainId) + ".latest.bak.json");
  }

  std::filesystem::path SaveDomainStore::PreviousBackupPath(const std::string_view domainId) const
  {
    return m_profile.backupRoot / (std::string(domainId) + ".previous.bak.json");
  }
} // namespace Peter::Core
