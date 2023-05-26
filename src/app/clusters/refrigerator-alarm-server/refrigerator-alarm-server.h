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

    bool ResetCommand(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                      const chip::app::Clusters::RefrigeratorAlarm::Commands::Reset::DecodableType & commandData);

    EmberAfStatus getMaskValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * mask);
    EmberAfStatus getLatchValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * latch);
    EmberAfStatus getStateValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * state);

    EmberAfStatus setMaskValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);

    bool SendNotifyEvent(chip::EndpointId endpointId, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> active,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> inActive,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> state,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);

private:
    static RefrigeratorAlarmServer instance;
};
