#include "PeterValidation/ValidationModule.h"

namespace Peter::Validation
{
  ValidationStatus ValidationStatus::PlaceholderHealthy()
  {
    return ValidationStatus{"ok", "schema validation is owned by tools during Phase 0"};
  }

  std::string_view GetModuleSummary()
  {
    return "Runtime validation hooks and future creator safety boundaries.";
  }
} // namespace Peter::Validation
