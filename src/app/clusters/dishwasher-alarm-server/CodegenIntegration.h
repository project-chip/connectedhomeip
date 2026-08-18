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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/dishwasher-alarm-server/DishwasherAlarmCluster.h>
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-delegate.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters::DishwasherAlarm {

DishwasherAlarmCluster * FindClusterOnEndpoint(EndpointId endpointId);

class DishwasherAlarmServer
{
public:
    static DishwasherAlarmServer & Instance();

    Protocols::InteractionModel::Status GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask);
    Protocols::InteractionModel::Status GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state);
    Protocols::InteractionModel::Status GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch);
    Protocols::InteractionModel::Status GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * suppported);

    Protocols::InteractionModel::Status SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask);

    Protocols::InteractionModel::Status SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState,
                                                      bool ignoreLatchState = false);

    Protocols::InteractionModel::Status ResetLatchedAlarms(EndpointId endpoint, const BitMask<AlarmMap> alarms);

protected:
    bool HasResetFeature(EndpointId endpoint);

private:
    static DishwasherAlarmServer instance;
};

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

Delegate * GetDelegate(EndpointId endpoint);

} // namespace chip::app::Clusters::DishwasherAlarm
