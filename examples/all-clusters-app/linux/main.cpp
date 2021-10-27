/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/Command.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/af.h>

#include "AppMain.h"

using namespace chip;

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

Protocols::InteractionModel::Status MatterPreAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t mask,
                                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;
    if (attributePath.mClusterId == app::Clusters::Thermostat::Id)
    {
        status = MatterThermostatClusterServerPreAttributeChangedCallback(attributePath, type, size, value);
    }
    else if (attributePath.mClusterId == app::Clusters::ThermostatUserInterfaceConfiguration::Id)
    {
        status =
            MatterThermostatUserInterfaceConfigurationClusterServerPreAttributeChangedCallback(attributePath, type, size, value);
    }
    return status;
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
