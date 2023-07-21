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
    EmberAfStatus SetLatchValue(EndpointId endpoint, const BitMask<AlarmMap> latch);
    // A change in supported value will result in a corresponding change in mask and state.
    EmberAfStatus SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported);

    /**
     * @brief Set value of State attribute
     * When State changes we are generating Notify event.
     * @param[in] endpoint The endpoint corresponding to the Dishwasher Alarm cluster.
     * @param[in] newState The value of State which want to set.
     * @param[in] ignoreLatchState The default value false means that if each bit set in the Latch
     * attribute is true, the corresponding value of State attribute can not be reset to false.
     * Otherwise, the ignoreLatchState value true means that the corresponding value of State
     * attribute can be set true or reset to false.
     */
    EmberAfStatus SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState, bool ignoreLatchState = false);

    /**
     * @brief Reset value of State attribute
     * When State changes we are generating Notify event.
     * @param[in] endpoint The endpoint corresponding to the Dishwasher Alarm cluster.
     * @param[in] alarms Each bit set in this field corresponds to State that SHALL be reset to false.
     * Even if the each bit set in the Latch attribute is true.
     */
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
