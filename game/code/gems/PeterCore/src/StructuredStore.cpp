#include "PeterCore/StructuredStore.h"

#include <chrono>
#include <fstream>
#include <iterator>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Peter::Core
{
  namespace
  {
    std::string EscapeJson(const std::string_view value)
    {
      std::string escaped;
      escaped.reserve(value.size());

      for (const char character : value)
      {
        switch (character)
        {
          case '\\':
            escaped += "\\\\";
            break;
          case '"':
            escaped += "\\\"";
            break;
          case '\n':
            escaped += "\\n";
            break;
          case '\r':
            escaped += "\\r";
            break;
          case '\t':
            escaped += "\\t";
            break;
          default:
            escaped += character;
            break;
        }
      }

      return escaped;
    }

    bool ConsumeWhitespace(const std::string_view content, std::size_t& cursor)
    {
      while (cursor < content.size() &&
        (content[cursor] == ' ' || content[cursor] == '\n' || content[cursor] == '\r' || content[cursor] == '\t'))
      {
        ++cursor;
      }
      return cursor < content.size();
    }

    bool ParseQuotedString(const std::string_view content, std::size_t& cursor, std::string& value, std::string& error)
    {
      if (cursor >= content.size() || content[cursor] != '"')
      {
        error = "Expected opening quote.";
        return false;
      }

      ++cursor;
      bool escaping = false;
      value.clear();
      while (cursor < content.size())
      {
        const char character = content[cursor++];
        if (escaping)
        {
          switch (character)
          {
            case 'n':
              value += '\n';
              break;
            case 'r':
              value += '\r';
              break;
            case 't':
              value += '\t';
              break;
            case '\\':
              value += '\\';
              break;
            case '"':
              value += '"';
              break;
            default:
              value += character;
              break;
          }
          escaping = false;
          continue;
        }

        if (character == '\\')
        {
          escaping = true;
          continue;
        }

        if (character == '"')
        {
          return true;
        }

        value += character;
      }

      error = "Unterminated quoted string.";
      return false;
    }

    bool ReplaceFileAtomic(const std::filesystem::path& source, const std::filesystem::path& destination)
    {
#ifdef _WIN32
      return MoveFileExW(
               source.c_str(),
               destination.c_str(),
               MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
      std::error_code error;
      std::filesystem::rename(source, destination, error);
      return !error;
#endif
    }

    void RotateBackup(
      const std::filesystem::path& sourcePath,
      const std::filesystem::path& latestBackupPath,
      const std::filesystem::path& previousBackupPath)
    {
      if (!std::filesystem::exists(sourcePath))
      {
        return;
      }

      std::error_code error;
      if (!previousBackupPath.empty() && std::filesystem::exists(latestBackupPath))
      {
        std::filesystem::remove(previousBackupPath, error);
        error.clear();
        std::filesystem::copy_file(
          latestBackupPath,
          previousBackupPath,
          std::filesystem::copy_options::overwrite_existing,
          error);
        error.clear();
      }

      if (!latestBackupPath.empty())
      {
        std::filesystem::create_directories(latestBackupPath.parent_path());
        std::filesystem::copy_file(
          sourcePath,
          latestBackupPath,
          std::filesystem::copy_options::overwrite_existing,
          error);
      }
    }
  } // namespace

  std::string SerializeStructuredFields(const StructuredFields& fields)
  {
    std::ostringstream output;
    output << "{";
    bool first = true;
    for (const auto& [key, value] : fields)
    {
      if (!first)
      {
        output << ",";
      }
      output << '"' << EscapeJson(key) << "\":\"" << EscapeJson(value) << '"';
      first = false;
    }
    output << "}\n";
    return output.str();
  }

  StructuredStoreParseResult ParseStructuredFieldsFile(const std::filesystem::path& path)
  {
    StructuredStoreParseResult result;
    if (!std::filesystem::exists(path))
    {
      result.valid = true;
      return result;
    }

    std::ifstream input(path, std::ios::binary);
    const std::string content(
      (std::istreambuf_iterator<char>(input)),
      std::istreambuf_iterator<char>());
    result.bytesRead = content.size();

    std::size_t cursor = 0;
    ConsumeWhitespace(content, cursor);
    if (cursor >= content.size() || content[cursor] != '{')
    {
      result.error = "Expected object start.";
      return result;
    }

    ++cursor;
    ConsumeWhitespace(content, cursor);
    if (cursor < content.size() && content[cursor] == '}')
    {
      result.valid = true;
      return result;
    }

    while (cursor < content.size())
    {
      std::string key;
      std::string value;
      if (!ParseQuotedString(content, cursor, key, result.error))
      {
        return result;
      }

      ConsumeWhitespace(content, cursor);
      if (cursor >= content.size() || content[cursor] != ':')
      {
        result.error = "Expected key/value separator.";
        return result;
      }
      ++cursor;
      ConsumeWhitespace(content, cursor);

      if (!ParseQuotedString(content, cursor, value, result.error))
      {
        return result;
      }

      result.fields[key] = value;
      ConsumeWhitespace(content, cursor);

      if (cursor >= content.size())
      {
        break;
      }

      if (content[cursor] == '}')
      {
        ++cursor;
        result.valid = true;
        return result;
      }

      if (content[cursor] != ',')
      {
        result.error = "Expected item separator.";
        return result;
      }

      ++cursor;
      ConsumeWhitespace(content, cursor);
    }

    ConsumeWhitespace(content, cursor);
    if (cursor == content.size())
    {
      result.valid = true;
      return result;
    }

    result.error = "Unexpected trailing content.";
    return result;
  }

  StructuredStoreWriteResult WriteStructuredFieldsFileAtomic(
    const std::filesystem::path& path,
    const StructuredFields& fields,
    const std::filesystem::path& latestBackupPath,
    const std::filesystem::path& previousBackupPath)
  {
    const auto started = std::chrono::steady_clock::now();
    StructuredStoreWriteResult result;
    result.path = path;
    result.latestBackupPath = latestBackupPath;
    result.previousBackupPath = previousBackupPath;
    result.tempPath = path;
    result.tempPath += ".tmp";

    try
    {
      const auto serialized = SerializeStructuredFields(fields);
      result.bytesWritten = serialized.size();

      std::filesystem::create_directories(path.parent_path());
      if (!latestBackupPath.empty())
      {
        std::filesystem::create_directories(latestBackupPath.parent_path());
      }
      if (!previousBackupPath.empty())
      {
        std::filesystem::create_directories(previousBackupPath.parent_path());
      }

      RotateBackup(path, latestBackupPath, previousBackupPath);

      {
        std::ofstream output(result.tempPath, std::ios::binary | std::ios::trunc);
        output << serialized;
      }

      if (!ReplaceFileAtomic(result.tempPath, path))
      {
        std::error_code removeError;
        std::filesystem::remove(path, removeError);
        if (!ReplaceFileAtomic(result.tempPath, path))
        {
          throw std::runtime_error("Atomic replace failed.");
        }
      }

      result.success = true;
      result.atomicReplace = true;
      result.message = "Structured fields written atomically.";
    }
    catch (const std::exception& exception)
    {
      result.message = exception.what();
      std::error_code removeError;
      std::filesystem::remove(result.tempPath, removeError);
    }

    const auto finished = std::chrono::steady_clock::now();
    result.durationMs =
      std::chrono::duration<double, std::milli>(finished - started).count();
    return result;
  }

  StructuredStoreRestoreResult RestoreStructuredFieldsFile(
    const std::filesystem::path& targetPath,
    const std::filesystem::path& latestBackupPath,
    const std::filesystem::path& previousBackupPath)
  {
    StructuredStoreRestoreResult result;
    result.restoredToPath = targetPath;

    const auto tryRestore = [&](const std::filesystem::path& sourcePath) -> bool {
      if (sourcePath.empty() || !std::filesystem::exists(sourcePath))
      {
        return false;
      }

      std::filesystem::create_directories(targetPath.parent_path());
      const auto tempPath = targetPath.string() + ".restore.tmp";
      std::error_code error;
      std::filesystem::copy_file(
        sourcePath,
        tempPath,
        std::filesystem::copy_options::overwrite_existing,
        error);
      if (error)
      {
        return false;
      }

      if (!ReplaceFileAtomic(tempPath, targetPath))
      {
        error.clear();
        std::filesystem::remove(targetPath, error);
        if (!ReplaceFileAtomic(tempPath, targetPath))
        {
          error.clear();
          std::filesystem::remove(tempPath, error);
          return false;
        }
      }

      result.success = true;
      result.restoredFromPath = sourcePath;
      result.message = "Restored from backup.";
      return true;
    };

    if (tryRestore(latestBackupPath) || tryRestore(previousBackupPath))
    {
      return result;
    }

    result.message = "No restorable backup was found.";
    return result;
  }
} // namespace Peter::Core
