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

class RefrigeratorAlarmServer
{
public:
    static RefrigeratorAlarmServer & Instance();

    EmberAfStatus GetMaskValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * mask);
    EmberAfStatus GetStateValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * state);
    EmberAfStatus GetSupportedValue(chip::EndpointId endpoint,
                                    chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * suppported);

    // A change in mask value will result in a corresponding change in state.
    EmberAfStatus SetMaskValue(chip::EndpointId endpoint,
                               const chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);

    // When State changes we are generating Notify event.
    EmberAfStatus SetStateValue(chip::EndpointId endpoint,
                                chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> newState);

    // A change in supported value will result in a corresponding change in mask and state.
    EmberAfStatus SetSupportedValue(chip::EndpointId endpoint,
                                    const chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> supported);

private:
    static RefrigeratorAlarmServer instance;

    void SendNotifyEvent(chip::EndpointId endpointId, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> becameActive,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> becameInactive,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> newState,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);
};
