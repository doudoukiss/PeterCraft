#include "PeterCraftApp.h"

#include <string_view>

namespace
{
  Peter::App::AppOptions ParseArguments(int argc, char** argv)
  {
    Peter::App::AppOptions options;

    for (int index = 1; index < argc; ++index)
    {
      const std::string_view argument(argv[index]);

      if (argument == "--smoke-test")
      {
        options.smokeTest = true;
        options.scenario = "smoke";
      }
      else if (argument == "--development")
      {
        options.developmentMode = true;
      }
      else if (argument == "--no-settings")
      {
        options.visitSettings = false;
      }
      else if (argument == "--release-mode")
      {
        options.developmentMode = false;
      }
      else if (argument == "--scenario" && index + 1 < argc)
      {
        options.scenario = argv[++index];
      }
      else if (argument == "--profile-id" && index + 1 < argc)
      {
        options.profileId = argv[++index];
      }
    }

    return options;
  }
} // namespace

int main(int argc, char** argv)
{
  Peter::App::PeterCraftApp app(ParseArguments(argc, argv));
  return app.Run();
}
