/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "ThermostaticRadiatorValveManager.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>
#include <assert.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == ThermostatUserInterfaceConfiguration::Id)
    {
        if (attributeId == ThermostatUserInterfaceConfiguration::Attributes::TemperatureDisplayMode::Id)
        {
            ChipLogProgress(Zcl, " set TemperatureDisplayMode: %u", *value);
            // check the output temperature accordingly
            ThermostaticRadiatorValveMgr().DisplayTemperature();
        }
    }
}

void emberAfThermostatClusterInitCallback(EndpointId endpoint)
{

    // Temp. code for testing purpose, need to be updated
    const auto logOnFailure = [](Protocols::InteractionModel::Status status, const char * attributeName) {
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(Zcl, "Failed to set DoorLock %s: %x", attributeName, to_underlying(status));
        }
    };

    logOnFailure(Thermostat::Attributes::FeatureMap::Set(endpoint, 0x23), "feature map");
}
