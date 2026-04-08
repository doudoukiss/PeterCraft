#pragma once

#include <map>
#include <string>

namespace Peter::Debug
{
  class DebugOverlay
  {
  public:
    void SetValue(const std::string& key, const std::string& value);
    [[nodiscard]] std::string Render() const;

  private:
    std::map<std::string, std::string, std::less<>> m_values;
  };
} // namespace Peter::Debug
