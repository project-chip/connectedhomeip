/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "../level-control.h"
#include "level-control-internals.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;

void writeRemainingTime(chip::EndpointId endpoint, uint16_t remainingTimeMs, bool isNewTransition)
{
  bool containsAttribute = emberAfContainsAttribute(endpoint, LevelControl::Id, LevelControl::Attributes::RemainingTime::Id);
	if (!containsAttribute)
	{
		return;
	}
	// Convert milliseconds to tenths of a second, rounding any fractional value
	// up to the nearest whole value.  This means:
	//
	//   0 ms = 0.00 ds = 0 ds
	//   1 ms = 0.01 ds = 1 ds
	//   ...
	//   100 ms = 1.00 ds = 1 ds
	//   101 ms = 1.01 ds = 2 ds
	//   ...
	//   200 ms = 2.00 ds = 2 ds
	//   201 ms = 2.01 ds = 3 ds
	//   ...
	//
	// This is done to ensure that the attribute, in tenths of a second, only
	// goes to zero when the remaining time in milliseconds is actually zero.
	auto markDirty             = MarkAttributeDirty::kNo;
	auto state                 = getState(endpoint);
	auto now                   = System::SystemClock().GetMonotonicTimestamp();
	uint16_t remainingTimeDs   = static_cast<uint16_t>((remainingTimeMs + 99) / 100);
	uint16_t lastRemainingTime = state->quietRemainingTime.value().ValueOr(0);

	// RemainingTime Quiet report conditions:
	// - When it changes to 0, or
	// - When it changes from 0 to any value higher than 10, or
	// - When it changes, with a delta larger than 10, caused by the invoke of a command.
	auto predicate = [isNewTransition, lastRemainingTime](
	                        const decltype(state->quietRemainingTime)::SufficientChangePredicateCandidate & candidate) -> bool {
		constexpr uint16_t reportDelta = 10;
		bool isDirty                   = false;
		if (candidate.newValue.Value() == 0 ||
		    (candidate.lastDirtyValue.Value() == 0 && candidate.newValue.Value() > reportDelta))
		{
			isDirty = true;
		}
		else if (isNewTransition &&
		            (candidate.newValue.Value() > static_cast<uint32_t>(lastRemainingTime + reportDelta) ||
		            static_cast<uint32_t>(candidate.newValue.Value() + reportDelta) < lastRemainingTime ||
		            candidate.newValue.Value() > static_cast<uint32_t>(candidate.lastDirtyValue.Value() + reportDelta)))
		{
			isDirty = true;
		}
		return isDirty;
	};

	if (state->quietRemainingTime.SetValue(remainingTimeDs, now, predicate) == AttributeDirtyState::kMustReport)
	{
		markDirty = MarkAttributeDirty::kYes;
	}

	Attributes::RemainingTime::Set(endpoint, state->quietRemainingTime.value().Value(), markDirty);
}
