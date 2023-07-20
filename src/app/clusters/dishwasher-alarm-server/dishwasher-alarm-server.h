/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include "dishwasher-alarm-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

class DishwasherAlarmServer
{
public:
    static DishwasherAlarmServer & Instance();

    EmberAfStatus GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask);
    EmberAfStatus GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state);
    EmberAfStatus GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch);
    EmberAfStatus GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * suppported);

    // Whenever there is change on Mask we should change State accordingly.
    EmberAfStatus SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask);
    // When State changes we are generating Notify event.
    EmberAfStatus SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState);
    EmberAfStatus SetLatchValue(EndpointId endpoint, const BitMask<AlarmMap> latch);
    // A change in supported value will result in a corresponding change in mask and state.
    EmberAfStatus SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported);
    EmberAfStatus ResetStateValue(EndpointId endpoint, const BitMask<AlarmMap> alarms);

    // check whether the Alarm featureMap has enabled Reset feature.
    bool HasResetFeature(EndpointId endpoint);

private:
    static DishwasherAlarmServer instance;

    void SendNotifyEvent(EndpointId endpointId, BitMask<AlarmMap> becameActive, BitMask<AlarmMap> becameInactive,
                         BitMask<AlarmMap> newState, BitMask<AlarmMap> mask);
};

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
