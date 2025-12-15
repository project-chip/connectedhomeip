#include "level-control-internals.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

namespace {

// StartUpCurrentLevel is only applied when the backing attributes are non-volatile.
bool AreStartUpLevelControlServerAttributesNonVolatile(EndpointId endpoint)
{
	return !emberAfIsKnownVolatileAttribute(endpoint, LevelControl::Id, Attributes::CurrentLevel::Id) &&
		!emberAfIsKnownVolatileAttribute(endpoint, LevelControl::Id, Attributes::StartUpCurrentLevel::Id);
}

} // namespace

void HandleStartUpCurrentLevel(EndpointId endpoint, EmberAfLevelControlState * state,
							   DataModel::Nullable<uint8_t> & currentLevel)
{
	if (!AreStartUpLevelControlServerAttributesNonVolatile(endpoint))
	{
		return;
	}

	DataModel::Nullable<uint8_t> startUpCurrentLevel;
	Status status = Attributes::StartUpCurrentLevel::Get(endpoint, startUpCurrentLevel);
	if (status != Status::Success)
	{
		ChipLogProgress(Zcl, "ERR: reading start up level %x", to_underlying(status));
		return;
	}

	if (startUpCurrentLevel.IsNull())
	{
		// Use previous CurrentLevel value read by caller.
		return;
	}

	uint8_t targetLevel = startUpCurrentLevel.Value();
	if (targetLevel == kStartUpCurrentLevelUseDeviceMinimum)
	{
		targetLevel = state->minLevel;
	}
	else
	{
		if (targetLevel < state->minLevel)
		{
			targetLevel = state->minLevel;
		}
		else if (targetLevel > state->maxLevel)
		{
			targetLevel = state->maxLevel;
		}
	}

	currentLevel.SetNonNull(targetLevel);
	SetCurrentLevelQuietReport(endpoint, state, currentLevel, false /* isEndOfTransition */);
}
