#include "PeterCore/StableId.h"

namespace Peter::Core
{
  bool StableId::IsValid(const std::string_view value)
  {
    if (value.size() < 3 || value.front() == '.' || value.back() == '.')
    {
      return false;
    }

    bool sawSeparator = false;
    bool previousWasSeparator = false;

    for (const char character : value)
    {
      const bool isAlpha = character >= 'a' && character <= 'z';
      const bool isDigit = character >= '0' && character <= '9';
      const bool isUnderscore = character == '_';
      const bool isSeparator = character == '.';

      if (!(isAlpha || isDigit || isUnderscore || isSeparator))
      {
        return false;
      }

      if (isSeparator)
      {
        if (previousWasSeparator)
        {
          return false;
        }

        sawSeparator = true;
      }

      previousWasSeparator = isSeparator;
    }

    return sawSeparator;
  }
} // namespace Peter::Core
