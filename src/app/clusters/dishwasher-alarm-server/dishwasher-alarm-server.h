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
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

class DishwasherAlarmServer
{
public:
    static DishwasherAlarmServer & Instance();

    Protocols::InteractionModel::Status GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask);
    Protocols::InteractionModel::Status GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state);
    Protocols::InteractionModel::Status GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch);
    Protocols::InteractionModel::Status GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * suppported);

    // Whenever there is change on Mask we should change State accordingly.
    Protocols::InteractionModel::Status SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask);
    Protocols::InteractionModel::Status SetLatchValue(EndpointId endpoint, const BitMask<AlarmMap> latch);
    // A change in supported value will result in a corresponding change in mask and state.
    Protocols::InteractionModel::Status SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported);

    /**
     * @brief Set the value of the State attribute
     * This method sets the State attribute to the value in newState, if allowed by the Mask
     * and Supported attributes.  When the State attribute changes, a Notify event will be
     * generated.
     * @param[in] endpoint The endpoint corresponding to the Dishwasher Alarm cluster.
     * @param[in] newState The desired new value for the State attribute.
     * @param[in] ignoreLatchState If false, the Latch attribute will be honored and the
     * method will not reset any bits in the State attribute which have their
     * associated Latch bit set.  If true, the Latch attribute is ignored and the value of
     * the State attribute will be set to the value of the newState parameter while
     * honoring the Mask and Supported attributes.
     * The default value for the ignoreLatchState parameter is false.
     */
    Protocols::InteractionModel::Status SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState,
                                                      bool ignoreLatchState = false);

    /**
     * @brief Reset the value of latched alarms in the State attribute.
     * When the State attribute changes a Notify event is generated.
     * @param[in] endpoint The endpoint corresponding to the Dishwasher Alarm cluster.
     * @param[in] alarms Each bit set to a 1 in this parameter corresponds to a bit in the
     * State attribute will SHALL be reset to false.
     */
    Protocols::InteractionModel::Status ResetLatchedAlarms(EndpointId endpoint, const BitMask<AlarmMap> alarms);

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
