/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
#include <protocols/interaction_model/StatusCode.h>

class RefrigeratorAlarmServer
{
public:
    static RefrigeratorAlarmServer & Instance();

    chip::Protocols::InteractionModel::Status GetMaskValue(
        chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * mask);
    chip::Protocols::InteractionModel::Status GetStateValue(
        chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * state);
    chip::Protocols::InteractionModel::Status GetSupportedValue(
        chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * suppported);

    chip::Protocols::InteractionModel::Status
    SetMaskValue(chip::EndpointId endpoint, const chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);

    chip::Protocols::InteractionModel::Status
    SetStateValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> newState);

private:
    static RefrigeratorAlarmServer instance;
};

#include <app/clusters/refrigerator-alarm-server/CodegenIntegration.h>
