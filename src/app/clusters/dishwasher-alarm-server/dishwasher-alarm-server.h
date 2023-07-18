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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>

class DishwasherAlarmServer
{
public:
    static DishwasherAlarmServer & Instance();

    void ResetCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);

    EmberAfStatus GetMaskValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> * mask);
    EmberAfStatus GetStateValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> * state);
    EmberAfStatus GetLatchValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> * latch);
    EmberAfStatus GetSupportedValue(chip::EndpointId endpoint,chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> * suppported);


    bool HasSupportsLatch(chip::EndpointId endpoint);

    // Whenever there is change on Mask we should change State accordingly.
    EmberAfStatus SetMaskValue(chip::EndpointId endpoint, const chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> mask);
    // When State changes we are generating Notify event.
    EmberAfStatus SetStateValue(chip::EndpointId endpoint,
                                const chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> newState);
    EmberAfStatus SetLatchValue(chip::EndpointId endpoint,
                               const chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> latch);
    EmberAfStatus SetSupportedValue(chip::EndpointId endpoint,
                                    const chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> supported);
private:
    static DishwasherAlarmServer instance;

    void SendNotifyEvent(chip::EndpointId endpointId, chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> becameActive,
                         chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> becameInactive,
                         chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> newState,
                         chip::BitMask<chip::app::Clusters::DishwasherAlarm::AlarmMap> mask);
};
