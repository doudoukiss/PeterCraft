#include "PeterAdapters/PlatformServices.h"

#define NOMINMAX
#include <windows.h>

#include <cstdlib>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Peter::Adapters
{
  namespace
  {
    std::string EscapeJson(std::string value)
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
          default:
            escaped += character;
            break;
        }
      }
      return escaped;
    }

    std::wstring ToWide(const std::string& value)
    {
      if (value.empty())
      {
        return {};
      }

      const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
      std::wstring wide(static_cast<std::size_t>(size), L'\0');
      MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wide.data(), size);
      if (!wide.empty() && wide.back() == L'\0')
      {
        wide.pop_back();
      }
      return wide;
    }

    std::wstring ToWide(const std::filesystem::path& value)
    {
      return value.wstring();
    }

    std::string ToUtf8(const std::wstring& value)
    {
      if (value.empty())
      {
        return {};
      }

      const int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
      std::string utf8(static_cast<std::size_t>(size), '\0');
      WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, utf8.data(), size, nullptr, nullptr);
      if (!utf8.empty() && utf8.back() == '\0')
      {
        utf8.pop_back();
      }
      return utf8;
    }

    std::string QuoteForCommand(const std::filesystem::path& value)
    {
      auto preferred = value;
      preferred.make_preferred();
      return std::string("\"") + preferred.string() + "\"";
    }

    std::wstring QuoteForCommandWide(const std::filesystem::path& value)
    {
      auto preferred = value;
      preferred.make_preferred();
      return std::wstring(L"\"") + preferred.wstring() + L"\"";
    }

    void AppendLogLine(const std::filesystem::path& logPath, const std::string& line)
    {
      std::filesystem::create_directories(logPath.parent_path());
      std::ofstream output(logPath, std::ios::app);
      output << line << '\n';
    }

    int RunRegisterCommand(
      const std::filesystem::path& scriptPath,
      const std::wstring& arguments,
      const std::filesystem::path& logPath)
    {
      std::wstring commandLine = L"cmd.exe /c \"";
      commandLine += QuoteForCommandWide(scriptPath);
      commandLine += L" ";
      commandLine += arguments;
      commandLine += L"\"";

      AppendLogLine(logPath, "[register] " + ToUtf8(commandLine));

      STARTUPINFOW startupInfo{};
      startupInfo.cb = sizeof(startupInfo);
      PROCESS_INFORMATION processInfo{};
      const BOOL launched = CreateProcessW(
        nullptr,
        commandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);
      if (!launched)
      {
        return static_cast<int>(GetLastError());
      }

      WaitForSingleObject(processInfo.hProcess, INFINITE);
      DWORD exitCode = 0;
      GetExitCodeProcess(processInfo.hProcess, &exitCode);
      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);
      return static_cast<int>(exitCode);
    }

    bool WriteTextFile(
      const std::filesystem::path& path,
      const std::string& content,
      std::string& errorMessage)
    {
      std::error_code error;
      std::filesystem::create_directories(path.parent_path(), error);
      if (error)
      {
        errorMessage = "Failed to create directory: " + error.message();
        return false;
      }

      std::ofstream output(path, std::ios::trunc);
      if (!output.is_open())
      {
        errorMessage = "Failed to open file for writing: " + path.string();
        return false;
      }

      output << content;
      return true;
    }

    bool WriteUserProjectOverride(
      const std::filesystem::path& projectRoot,
      const std::filesystem::path& engineRoot,
      const std::filesystem::path& logPath,
      std::string& errorMessage)
    {
      const auto userProjectPath = projectRoot / "user" / "project.json";
      const std::string content =
        "{\n"
        "  \"engine_path\": \"" + EscapeJson(engineRoot.generic_string()) + "\"\n"
        "}\n";
      const bool written = WriteTextFile(userProjectPath, content, errorMessage);
      if (written)
      {
        AppendLogLine(logPath, "[bootstrap] wrote user/project.json override at " + userProjectPath.string());
      }
      return written;
    }

    bool WriteRuntimeLevelRegistry(
      const std::filesystem::path& runtimeRegistryPath,
      const std::string_view levelName,
      std::string& errorMessage)
    {
      const std::string content =
        "{\n"
        "  \"O3DE\": {\n"
        "    \"Autoexec\": {\n"
        "      \"ConsoleCommands\": {\n"
        "        \"LoadLevel\": \"" + EscapeJson(std::string(levelName)) + "\"\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";
      return WriteTextFile(runtimeRegistryPath, content, errorMessage);
    }

    struct O3DESessionState
    {
      O3DEBootstrapResult bootstrap;
      std::filesystem::path projectUserPath;
      std::filesystem::path projectLogPath;
      CameraRigState cameraRig;
      PresentationSettings presentationSettings;
      mutable std::mutex mutex;
      PROCESS_INFORMATION launcherProcess{};
      bool launcherActive = false;
    };

    void CloseProcessHandle(PROCESS_INFORMATION& processInfo)
    {
      if (processInfo.hProcess != nullptr)
      {
        CloseHandle(processInfo.hProcess);
        processInfo.hProcess = nullptr;
      }
      if (processInfo.hThread != nullptr)
      {
        CloseHandle(processInfo.hThread);
        processInfo.hThread = nullptr;
      }
      processInfo.dwProcessId = 0;
      processInfo.dwThreadId = 0;
    }

    bool StopPreviousLauncher(O3DESessionState& session)
    {
      if (!session.launcherActive || session.launcherProcess.hProcess == nullptr)
      {
        return true;
      }

      const DWORD waitResult = WaitForSingleObject(session.launcherProcess.hProcess, 0);
      if (waitResult == WAIT_TIMEOUT)
      {
        TerminateProcess(session.launcherProcess.hProcess, 0);
        WaitForSingleObject(session.launcherProcess.hProcess, 2000);
      }

      CloseProcessHandle(session.launcherProcess);
      session.launcherActive = false;
      return true;
    }

    bool LaunchO3DELevel(
      O3DESessionState& session,
      const std::string_view levelName,
      std::string& errorMessage)
    {
      std::lock_guard<std::mutex> lock(session.mutex);
      StopPreviousLauncher(session);

      std::ostringstream command;
      command << QuoteForCommand(session.bootstrap.launcherPath)
              << " --project-path=" << QuoteForCommand(session.bootstrap.projectRoot)
              << " --project-user-path=" << QuoteForCommand(session.projectUserPath)
              << " --project-log-path=" << QuoteForCommand(session.projectLogPath)
              << " --regset=\"/O3DE/Autoexec/ConsoleCommands/LoadLevel="
              << std::string(levelName) << "\"";

      std::wstring commandLine = ToWide(command.str());
      std::wstring workingDirectory = ToWide(session.bootstrap.projectRoot);

      STARTUPINFOW startupInfo{};
      startupInfo.cb = sizeof(startupInfo);
      PROCESS_INFORMATION processInfo{};

      const BOOL launched = CreateProcessW(
        nullptr,
        commandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_PROCESS_GROUP,
        nullptr,
        workingDirectory.c_str(),
        &startupInfo,
        &processInfo);

      if (!launched)
      {
        errorMessage = "Failed to launch O3DE.GameLauncher.exe for level '" + std::string(levelName)
          + "' (Win32 error " + std::to_string(GetLastError()) + ").";
        return false;
      }

      session.launcherProcess = processInfo;
      session.launcherActive = true;
      AppendLogLine(
        session.bootstrap.logPath,
        "[scene] launched level '" + std::string(levelName) + "' via " + session.bootstrap.launcherPath.string());
      return true;
    }

    class O3DEInputAdapter final : public IInputAdapter
    {
    public:
      explicit O3DEInputAdapter(std::shared_ptr<O3DESessionState> /*session*/)
      {
      }

      std::string ActiveScheme() const override
      {
        return "mouse_keyboard";
      }

      InputState SampleInput() const override
      {
        return {};
      }

      std::vector<ActionBinding> DefaultBindings() const override
      {
        return {
          {"action.move", "WASD", "Left Stick", "input.move", "movement", false, false},
          {"action.look", "Mouse", "Right Stick", "input.look", "camera", false, false},
          {"action.interact", "E", "X", "input.interact", "interaction", true, true},
          {"action.primary_action", "Left Mouse", "Right Trigger", "input.primary_action", "combat", true, true},
          {"action.secondary_action", "Right Mouse", "Left Trigger", "input.secondary_action", "combat", true, true},
          {"action.jump", "Space", "A", "input.jump", "movement", true, false},
          {"action.sprint", "Left Shift", "Left Stick Click", "input.sprint", "movement", true, true},
          {"action.crouch", "C", "B", "input.crouch", "movement", true, true},
          {"action.call_companion", "Q", "Y", "input.call_companion", "companion", true, false},
          {"action.open_inventory", "Tab", "View", "input.open_inventory", "menu", true, false},
          {"action.open_explain", "F", "DPad Up", "input.open_explain", "menu", true, false},
          {"action.pause", "Escape", "Menu", "input.pause", "menu", true, false}
        };
      }
    };

    class O3DECameraAdapter final : public ICameraAdapter
    {
    public:
      explicit O3DECameraAdapter(std::shared_ptr<O3DESessionState> session)
        : m_session(std::move(session))
      {
      }

      CameraRigState CurrentRig() const override
      {
        std::lock_guard<std::mutex> lock(m_session->mutex);
        return m_session->cameraRig;
      }

      void ApplyRig(const CameraRigState& state) override
      {
        {
          std::lock_guard<std::mutex> lock(m_session->mutex);
          m_session->cameraRig = state;
        }

        AppendLogLine(
          m_session->bootstrap.logPath,
          "[camera] mode=" + state.mode
            + " follow_distance=" + std::to_string(state.followDistanceMeters)
            + " shoulder_offset=" + std::to_string(state.shoulderOffsetMeters));
      }

    private:
      std::shared_ptr<O3DESessionState> m_session;
    };

    class O3DESaveAdapter final : public ISaveAdapter
    {
    public:
      explicit O3DESaveAdapter(std::filesystem::path userRoot)
        : m_userRoot(std::move(userRoot))
      {
      }

      std::filesystem::path ResolveProfileRoot() const override
      {
        return m_userRoot / "Profiles";
      }

      void EnsureDirectory(const std::filesystem::path& directory) const override
      {
        std::filesystem::create_directories(directory);
      }

    private:
      std::filesystem::path m_userRoot;
    };

    class O3DENavigationAdapter final : public INavigationAdapter
    {
    public:
      explicit O3DENavigationAdapter(std::shared_ptr<O3DESessionState> session)
        : m_session(std::move(session))
      {
      }

      std::string BackendName() const override
      {
        return "o3de_navmesh_stub";
      }

      std::vector<std::string> ResolvePath(
        const std::string_view fromNodeId,
        const std::string_view toNodeId) const override
      {
        if (fromNodeId.empty() || toNodeId.empty())
        {
          AppendLogLine(
            m_session->bootstrap.logPath,
            "[nav] failed route request because at least one node id was empty.");
          return {};
        }

        AppendLogLine(
          m_session->bootstrap.logPath,
          "[nav] resolved route from '" + std::string(fromNodeId) + "' to '" + std::string(toNodeId) + "'.");
        return {std::string(fromNodeId), std::string(toNodeId)};
      }

    private:
      std::shared_ptr<O3DESessionState> m_session;
    };

    class O3DEAudioAdapter final : public IAudioAdapter
    {
    public:
      explicit O3DEAudioAdapter(std::shared_ptr<O3DESessionState> session)
        : m_session(std::move(session))
      {
      }

      void PostUiCue(std::string_view cueId) override
      {
        AppendCue("ui", cueId, "", 0, false);
      }

      void PostWorldCue(std::string_view cueId) override
      {
        AppendCue("world", cueId, "", 0, false);
      }

      void PostFeedbackCue(std::string_view cueFamily, std::string_view variantId) override
      {
        AppendCue("feedback", cueFamily, variantId, 0, false);
      }

      void PostPrioritizedFeedbackCue(
        std::string_view cueFamily,
        std::string_view variantId,
        int priority,
        bool critical) override
      {
        AppendCue("feedback", cueFamily, variantId, priority, critical);
      }

    private:
      void AppendCue(
        const std::string_view channel,
        const std::string_view cueFamily,
        const std::string_view variantId,
        const int priority,
        const bool critical) const
      {
        AppendLogLine(
          m_session->bootstrap.logPath,
          "[audio] channel=" + std::string(channel)
            + " cue=" + std::string(cueFamily)
            + (variantId.empty() ? "" : ":" + std::string(variantId))
            + " priority=" + std::to_string(priority)
            + " critical=" + (critical ? "true" : "false"));
      }

      std::shared_ptr<O3DESessionState> m_session;
    };

    class O3DEUiAdapter final : public IUiAdapter
    {
    public:
      explicit O3DEUiAdapter(std::shared_ptr<O3DESessionState> session)
        : m_session(std::move(session))
      {
      }

      void PresentState(std::string_view stateId) override
      {
        Append("state", stateId, "");
      }

      void PresentPrompt(std::string_view prompt) override
      {
        Append("prompt", "prompt.default", prompt);
      }

      void PresentPanel(std::string_view panelId, std::string_view body) override
      {
        Append("panel", panelId, body);
      }

      void PresentCreatorPanel(std::string_view panelId, std::string_view body) override
      {
        Append("creator_panel", panelId, body);
      }

      void PresentTextEditor(std::string_view panelId, std::string_view body) override
      {
        Append("text_editor", panelId, body);
      }

      void PresentReplayTimeline(std::string_view panelId, std::string_view body) override
      {
        Append("replay", panelId, body);
      }

      void PresentMentorSummaryPrompt(std::string_view exportPath, std::string_view body) override
      {
        Append("mentor_prompt", exportPath, body);
      }

      void ApplyPresentationSettings(const PresentationSettings& settings) override
      {
        {
          std::lock_guard<std::mutex> lock(m_session->mutex);
          m_session->presentationSettings = settings;
        }
        AppendLogLine(
          m_session->bootstrap.logPath,
          "[ui] applied presentation settings: subtitles="
            + std::string(settings.subtitlesEnabled ? "true" : "false")
            + " text_scale=" + std::to_string(settings.textScalePercent)
            + " high_contrast=" + std::string(settings.highContrastEnabled ? "true" : "false")
            + " motion_comfort=" + std::string(settings.motionComfortEnabled ? "true" : "false"));
      }

      void PresentDebugMarkers(const std::vector<std::string>& markerIds) override
      {
        std::ostringstream output;
        output << "[ui] debug markers";
        for (const auto& markerId : markerIds)
        {
          output << ' ' << markerId;
        }
        AppendLogLine(m_session->bootstrap.logPath, output.str());
      }

      void PresentCompanionFeedback(
        std::string_view calloutToken,
        std::string_view gestureToken) override
      {
        Append("companion_feedback", calloutToken, gestureToken);
      }

    private:
      void Append(
        const std::string_view kind,
        const std::string_view panelId,
        const std::string_view body) const
      {
        AppendLogLine(
          m_session->bootstrap.logPath,
          "[ui] kind=" + std::string(kind)
            + " id=" + std::string(panelId)
            + (body.empty() ? "" : " body=" + std::string(body)));
      }

      std::shared_ptr<O3DESessionState> m_session;
    };

    class O3DESceneAdapter final : public ISceneAdapter
    {
    public:
      explicit O3DESceneAdapter(std::shared_ptr<O3DESessionState> session)
        : m_session(std::move(session))
      {
      }

      std::string BackendName() const override
      {
        return "o3de_launcher";
      }

      SceneLoadResult LoadScene(const SceneLoadRequest& request) override
      {
        std::string errorMessage;
        if (!WriteRuntimeLevelRegistry(m_session->bootstrap.runtimeRegistryPath, request.levelName, errorMessage))
        {
          AppendLogLine(m_session->bootstrap.logPath, "[scene] failed to write runtime level registry: " + errorMessage);
          return {false, BackendName(), "registry_write_failed", errorMessage};
        }

        if (!LaunchO3DELevel(*m_session, request.levelName, errorMessage))
        {
          AppendLogLine(m_session->bootstrap.logPath, "[scene] launch failure: " + errorMessage);
          return {false, BackendName(), "launcher_start_failed", errorMessage};
        }

        return {
          true,
          BackendName(),
          "launched",
          "Launched O3DE level '" + request.levelName + "' for logical scene '" + request.logicalSceneId + "'."};
      }

    private:
      std::shared_ptr<O3DESessionState> m_session;
    };
  } // namespace

  O3DEBootstrapResult BootstrapO3DEProjectImpl(const O3DEBootstrapConfig& config)
  {
    O3DEBootstrapResult result;
    result.engineRoot = config.engineRoot.empty() ? ResolveDefaultO3DERoot() : config.engineRoot;
    result.projectRoot = config.projectRoot.empty() ? (config.repoRoot / "game" / "o3de") : config.projectRoot;
    result.logPath = config.userRoot / "Logs" / "petercraft-o3de-adapter.log";
    result.runtimeRegistryPath = result.projectRoot / "user" / "Registry" / "load_level.setreg";

    const auto o3deScript = result.engineRoot / "scripts" / "o3de.bat";
    const auto projectLauncherPath =
      config.repoRoot / "out" / "o3de" / "windows-vs2022-playable-runtime" / "bin" / "profile"
      / "PeterCraftRuntime.GameLauncher.exe";
    result.launcherPath = std::filesystem::exists(projectLauncherPath)
      ? projectLauncherPath
      : result.engineRoot / "bin" / "Windows" / "profile" / "Default" / "O3DE.GameLauncher.exe";
    result.assetProcessorPath = result.engineRoot / "bin" / "Windows" / "profile" / "Default" / "AssetProcessorBatch.exe";

    if (!std::filesystem::exists(result.engineRoot))
    {
      result.statusCode = "engine_root_missing";
      result.message = "O3DE engine root not found: " + result.engineRoot.string();
      return result;
    }

    if (!std::filesystem::exists(result.projectRoot / "project.json"))
    {
      result.statusCode = "project_root_missing";
      result.message = "Playable O3DE project not found: " + result.projectRoot.string();
      return result;
    }

    if (!std::filesystem::exists(o3deScript))
    {
      result.statusCode = "o3de_cli_missing";
      result.message = "O3DE CLI not found: " + o3deScript.string();
      return result;
    }

    if (!std::filesystem::exists(result.launcherPath))
    {
      result.statusCode = "game_launcher_missing";
      result.message = "O3DE.GameLauncher.exe not found: " + result.launcherPath.string();
      return result;
    }

    std::filesystem::create_directories(config.userRoot / "Logs");
    std::string errorMessage;
    if (!WriteUserProjectOverride(result.projectRoot, result.engineRoot, result.logPath, errorMessage))
    {
      result.statusCode = "project_override_write_failed";
      result.message = errorMessage;
      return result;
    }

    if (RunRegisterCommand(
          o3deScript,
          L"register --engine-path " + QuoteForCommandWide(result.engineRoot) + L" --force",
          result.logPath)
      != 0)
    {
      result.statusCode = "engine_register_failed";
      result.message = "Failed to register engine with O3DE CLI.";
      return result;
    }
    result.engineRegistered = true;

    if (RunRegisterCommand(
          o3deScript,
          L"register --project-path " + QuoteForCommandWide(result.projectRoot) + L" --force",
          result.logPath)
      != 0)
    {
      result.statusCode = "project_register_failed";
      result.message = "Failed to register PeterCraft playable project with O3DE CLI.";
      return result;
    }
    result.projectRegistered = true;

    std::filesystem::create_directories(result.projectRoot / "user" / "Registry");
    result.success = true;
    result.statusCode = "ok";
    result.message = "Registered O3DE engine/project and prepared the PeterCraft playable runtime.";
    AppendLogLine(result.logPath, "[bootstrap] " + result.message);
    return result;
  }

  PlatformFactoryResult CreateO3DEPlatformServices(
    const BootConfig& bootConfig,
    const RuntimeDescriptor& runtimeDescriptor)
  {
    PlatformFactoryResult result;
    result.descriptor = runtimeDescriptor;

    const auto bootstrap = BootstrapO3DEProject(O3DEBootstrapConfig{
      bootConfig.repoRoot,
      bootConfig.userRoot,
      ResolveDefaultO3DERoot(),
      bootConfig.repoRoot / "game" / "o3de",
      bootConfig.developmentMode});

    result.statusCode = bootstrap.statusCode;
    result.message = bootstrap.message;
    if (!bootstrap.success)
    {
      result.available = false;
      return result;
    }

    auto session = std::make_shared<O3DESessionState>();
    session->bootstrap = bootstrap;
    session->projectUserPath = bootConfig.userRoot / "O3DEUser";
    session->projectLogPath = bootConfig.userRoot / "O3DELogs";
    std::filesystem::create_directories(session->projectUserPath);
    std::filesystem::create_directories(session->projectLogPath);

    result.available = true;
    result.services.input = std::make_unique<O3DEInputAdapter>(session);
    result.services.camera = std::make_unique<O3DECameraAdapter>(session);
    result.services.save = std::make_unique<O3DESaveAdapter>(bootConfig.userRoot);
    result.services.navigation = std::make_unique<O3DENavigationAdapter>(session);
    result.services.audio = std::make_unique<O3DEAudioAdapter>(session);
    result.services.ui = std::make_unique<O3DEUiAdapter>(session);
    result.services.scene = std::make_unique<O3DESceneAdapter>(session);
    return result;
  }
} // namespace Peter::Adapters
