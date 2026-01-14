/**
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

/**
 * @file API declarations for valve configuration and control cluster.
 */

#pragma once

#include "valve-configuration-and-control-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);
Delegate * GetDefaultDelegate(EndpointId endpoint);

CHIP_ERROR CloseValve(chip::EndpointId ep);
CHIP_ERROR SetValveLevel(chip::EndpointId ep, DataModel::Nullable<Percent> level, DataModel::Nullable<uint32_t> openDuration);
CHIP_ERROR UpdateCurrentLevel(chip::EndpointId ep, chip::Percent currentLevel);
CHIP_ERROR UpdateCurrentState(chip::EndpointId ep, ValveConfigurationAndControl::ValveStateEnum currentState);
CHIP_ERROR EmitValveFault(chip::EndpointId ep, chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault);
void UpdateAutoCloseTime(uint64_t time);

inline bool HasFeature(EndpointId ep, Feature feature)
{
    uint32_t map;
    bool success = (Attributes::FeatureMap::Get(ep, &map) == Protocols::InteractionModel::Status::Success);
    return success ? (map & to_underlying(feature)) : false;
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
