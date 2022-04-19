/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/af.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters::PumpConfigurationAndControl;
using namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes;

void emberAfPumpConfigurationAndControlClusterServerInitCallback(EndpointId endpoint)
{
    emberAfDebugPrintln("Initialize PCC Server Cluster [EP:%d]", endpoint);
    // TODO
}

void MatterPumpConfigurationAndControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    emberAfDebugPrintln("PCC Server Cluster Attribute changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                        (unsigned int) attributePath.mAttributeId);

    switch (attributePath.mAttributeId)
    {
    case ControlMode::Id: {
        PumpControlMode controlMode;
        ControlMode::Get(attributePath.mEndpointId, &controlMode);
        EffectiveControlMode::Set(attributePath.mEndpointId, controlMode);
    }
    break;
    case OperationMode::Id: {
        PumpOperationMode operationMode;
        OperationMode::Get(attributePath.mEndpointId, &operationMode);
        EffectiveOperationMode::Set(attributePath.mEndpointId, operationMode);
    }
    break;
    default:
        emberAfDebugPrintln("PCC Server: unhandled attribute ID");
    }
}

void MatterPumpConfigurationAndControlPluginServerInitCallback() {}
