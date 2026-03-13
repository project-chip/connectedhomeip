/*
 *    Copyright (c) 2026 Project CHIP Authors
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
 * @file
 * Stub implementations of ZAP-generated Thermostat attribute accessors used by
 * thermostat-server.cpp but absent from the generic test environment.
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Attributes {

namespace FeatureMap {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    *value = 0;
    return Protocols::InteractionModel::Status::Success;
}
} // namespace FeatureMap

namespace RemoteSensing {
Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Thermostat::RemoteSensingBitmap> * value)
{
    *value = BitMask<chip::app::Clusters::Thermostat::RemoteSensingBitmap>{};
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Thermostat::RemoteSensingBitmap> value)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        chip::BitMask<chip::app::Clusters::Thermostat::RemoteSensingBitmap> value,
                                        MarkAttributeDirty markDirty)
{
    return Protocols::InteractionModel::Status::Success;
}
} // namespace RemoteSensing

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
