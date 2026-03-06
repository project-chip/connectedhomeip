/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters::FanControl {

FanControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

Protocols::InteractionModel::Status SetFanMode(EndpointId endpointId, FanModeEnum value);
Protocols::InteractionModel::Status SetPercentSetting(EndpointId endpointId, DataModel::Nullable<chip::Percent> value);
Protocols::InteractionModel::Status SetSpeedSetting(EndpointId endpointId, DataModel::Nullable<uint8_t> value);
Protocols::InteractionModel::Status SetRockSetting(EndpointId endpointId, BitMask<RockBitmap> value);
Protocols::InteractionModel::Status SetWindSetting(EndpointId endpointId, BitMask<WindBitmap> value);
Protocols::InteractionModel::Status SetAirflowDirection(EndpointId endpointId, AirflowDirectionEnum value);

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate);
Delegate * GetDelegate(EndpointId aEndpoint);

} // namespace chip::app::Clusters::FanControl
