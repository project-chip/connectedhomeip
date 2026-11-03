/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/dishwasher-alarm-server/DishwasherAlarmCluster.h>
#include <clusters/DishwasherAlarm/Events.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

void DishwasherAlarmCluster::SendNotifyEvent(AlarmBase::AlarmMap becameActive, AlarmBase::AlarmMap becameInactive,
                                             AlarmBase::AlarmMap newState, AlarmBase::AlarmMap mask)
{
    VerifyOrReturn(mContext != nullptr);

    DishwasherAlarm::Events::Notify::Type event{
        .active   = BitMask<DishwasherAlarm::AlarmBitmap>(becameActive.Raw()),
        .inactive = BitMask<DishwasherAlarm::AlarmBitmap>(becameInactive.Raw()),
        .state    = BitMask<DishwasherAlarm::AlarmBitmap>(newState.Raw()),
        .mask     = BitMask<DishwasherAlarm::AlarmBitmap>(mask.Raw()),
    };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters
