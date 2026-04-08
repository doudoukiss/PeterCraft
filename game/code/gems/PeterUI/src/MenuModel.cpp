#include "PeterUI/MenuModel.h"

namespace Peter::UI
{
  void MenuModel::EnterMainMenu()
  {
    m_activeState = "ui.main_menu";
  }

  void MenuModel::EnterSettings()
  {
    m_activeState = "ui.settings";
  }

  const std::string& MenuModel::ActiveState() const
  {
    return m_activeState;
  }
} // namespace Peter::UI
