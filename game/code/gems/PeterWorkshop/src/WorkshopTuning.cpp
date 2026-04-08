#include "PeterWorkshop/WorkshopTuning.h"

#include "PeterValidation/ValidationModule.h"

#include <sstream>

namespace Peter::Workshop
{
  RuleEditPreview BuildFollowDistancePreview(const double currentValue, const double previewValue)
  {
    const auto validation = Peter::Validation::ValidateFollowDistance(previewValue);

    std::ostringstream summary;
    summary << "Follow distance preview: " << previewValue << "m";

    std::ostringstream delta;
    delta << (previewValue > currentValue ? "Companion will give the player more space."
                                          : "Companion will stay closer to the player.");

    if (!validation.valid)
    {
      delta << " " << validation.message;
    }

    return RuleEditPreview{
      "companion_rule.follow_distance.default",
      currentValue,
      previewValue,
      validation.valid,
      summary.str(),
      delta.str()};
  }
} // namespace Peter::Workshop
