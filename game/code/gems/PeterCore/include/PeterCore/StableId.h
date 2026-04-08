#pragma once

#include <string_view>

namespace Peter::Core
{
  class StableId
  {
  public:
    [[nodiscard]] static bool IsValid(std::string_view value);
  };
} // namespace Peter::Core
