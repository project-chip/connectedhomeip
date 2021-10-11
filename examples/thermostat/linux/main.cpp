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

#include "AppMain.h"

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/Command.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

bool emberAfBasicClusterMfgSpecificPingCallback(Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

// MatterPreAttributeChangeCallback() is called for every cluster.
// As of 8/17/21 cluster specific PreAttributeChangeCalbacks are not yet implemented.

Protocols::InteractionModel::Status MatterPreAttributeChangeCallback(const ConcreteAttributePath & attributePath, uint8_t mask,
                                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;
    if (attributePath.mClusterId == Thermostat::Id)
    {
        status = MatterThermostatClusterServerPreAttributeChangedCallback(attributePath, type, size, value);
    }
    return status;
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
