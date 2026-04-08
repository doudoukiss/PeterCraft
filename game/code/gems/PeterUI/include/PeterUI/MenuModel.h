#pragma once

#include <string>

namespace Peter::UI
{
  class MenuModel
  {
  public:
    void EnterMainMenu();
    void EnterSettings();
    [[nodiscard]] const std::string& ActiveState() const;

  private:
    std::string m_activeState = "ui.uninitialized";
  };
} // namespace Peter::UI
