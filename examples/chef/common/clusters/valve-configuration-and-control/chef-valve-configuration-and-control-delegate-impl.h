/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-server.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <app/util/config.h>
#include <lib/core/CHIPError.h>

#ifdef MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

/** @brief
 *    Defines methods for implementing application-specific logic for the Valve Configuration and Control Cluster.
 */
class DelegateImpl : public ValveConfigurationAndControl::Delegate
{
public:
    DelegateImpl() {};

    // shall return current level if supported, otherwise null
    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;

    ~DelegateImpl() = default;
};

void Shutdown();

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

chip::Protocols::InteractionModel::Status
chefValveConfigurationAndControlWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                              const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer);
chip::Protocols::InteractionModel::Status
chefValveConfigurationAndControlReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                             const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                             uint16_t maxReadLength);

#endif // MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER

