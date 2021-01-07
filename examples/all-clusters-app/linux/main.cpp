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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include "af.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <core/CHIPError.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>

#include "Server.h"

#include <cassert>
#include <iostream>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{}

bool emberAfBasicClusterMfgSpecificPingCallback(void)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    // Init ZCL Data Model and CHIP App Server
    InitServer();

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run All Clusters App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
