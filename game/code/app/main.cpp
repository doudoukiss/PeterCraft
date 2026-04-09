#include "PeterCraftApp.h"

#include <iostream>
#include <string_view>

namespace
{
  struct ParseResult
  {
    Peter::App::AppOptions options;
    bool valid = true;
    std::string error;
  };

  ParseResult ParseArguments(int argc, char** argv)
  {
    ParseResult result;

    for (int index = 1; index < argc; ++index)
    {
      const std::string_view argument(argv[index]);

      if (argument == "--smoke-test")
      {
        result.options.smokeTest = true;
        result.options.scenario = "smoke";
      }
      else if (argument == "--development")
      {
        result.options.developmentMode = true;
      }
      else if (argument == "--no-settings")
      {
        result.options.visitSettings = false;
      }
      else if (argument == "--release-mode")
      {
        result.options.developmentMode = false;
      }
      else if (argument == "--runtime" && index + 1 < argc)
      {
        Peter::Adapters::RuntimeMode runtimeMode = Peter::Adapters::RuntimeMode::Headless;
        if (!Peter::Adapters::TryParseRuntimeMode(argv[++index], runtimeMode))
        {
          result.valid = false;
          result.error = "Invalid runtime mode. Use --runtime headless|playable.";
          break;
        }
        result.options.runtimeMode = runtimeMode;
      }
      else if (argument == "--scenario" && index + 1 < argc)
      {
        result.options.scenario = argv[++index];
      }
      else if (argument == "--profile-id" && index + 1 < argc)
      {
        result.options.profileId = argv[++index];
      }
    }

    return result;
  }
} // namespace

int main(int argc, char** argv)
{
  const auto parseResult = ParseArguments(argc, argv);
  if (!parseResult.valid)
  {
    std::cerr << parseResult.error << '\n';
    return 2;
  }

  Peter::App::PeterCraftApp app(parseResult.options);
  return app.Run();
}
