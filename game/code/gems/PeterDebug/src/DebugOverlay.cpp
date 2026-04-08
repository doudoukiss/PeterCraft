#include "PeterDebug/DebugOverlay.h"

#include <sstream>

namespace Peter::Debug
{
  void DebugOverlay::SetValue(const std::string& key, const std::string& value)
  {
    m_values[key] = value;
  }

  std::string DebugOverlay::Render() const
  {
    std::ostringstream output;
    output << "Debug Overlay\n";

    for (const auto& [key, value] : m_values)
    {
      output << "- " << key << ": " << value << '\n';
    }

    return output.str();
  }
} // namespace Peter::Debug
