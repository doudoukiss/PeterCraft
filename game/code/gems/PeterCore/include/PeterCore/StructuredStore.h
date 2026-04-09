#pragma once

#include "PeterCore/EventBus.h"

#include <cstddef>
#include <filesystem>
#include <string>

namespace Peter::Core
{
  struct StructuredStoreParseResult
  {
    bool valid = false;
    StructuredFields fields;
    std::string error;
    std::size_t bytesRead = 0;
  };

  struct StructuredStoreWriteResult
  {
    bool success = false;
    bool atomicReplace = false;
    double durationMs = 0.0;
    std::size_t bytesWritten = 0;
    std::filesystem::path path;
    std::filesystem::path tempPath;
    std::filesystem::path latestBackupPath;
    std::filesystem::path previousBackupPath;
    std::string message;
  };

  struct StructuredStoreRestoreResult
  {
    bool success = false;
    std::filesystem::path restoredFromPath;
    std::filesystem::path restoredToPath;
    std::string message;
  };

  [[nodiscard]] StructuredStoreParseResult ParseStructuredFieldsFile(const std::filesystem::path& path);
  [[nodiscard]] StructuredStoreWriteResult WriteStructuredFieldsFileAtomic(
    const std::filesystem::path& path,
    const StructuredFields& fields,
    const std::filesystem::path& latestBackupPath,
    const std::filesystem::path& previousBackupPath);
  [[nodiscard]] StructuredStoreRestoreResult RestoreStructuredFieldsFile(
    const std::filesystem::path& targetPath,
    const std::filesystem::path& latestBackupPath,
    const std::filesystem::path& previousBackupPath);
  [[nodiscard]] std::string SerializeStructuredFields(const StructuredFields& fields);
} // namespace Peter::Core
