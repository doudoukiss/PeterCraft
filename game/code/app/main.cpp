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
    }

    return options;
  }
} // namespace

int main(int argc, char** argv)
{
  Peter::App::PeterCraftApp app(ParseArguments(argc, argv));
  return app.Run();
}
