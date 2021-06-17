/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "TargetNavigatorManager.h"

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <support/CodeUtils.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR TargetNavigatorManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err);
exit:
    return err;
}

std::vector<EmberAfNavigateTargetTargetInfo> TargetNavigatorManager::proxyGetTargetInfoList()
{
    // TODO: Insert code here
    std::vector<EmberAfNavigateTargetTargetInfo> targets;
    int maximumVectorSize = 2;
    char name[]           = "exampleName";

    for (int i = 0; i < maximumVectorSize; ++i)
    {
        EmberAfNavigateTargetTargetInfo targetInfo;
        targetInfo.name       = chip::ByteSpan(chip::Uint8::from_char(name), sizeof(name));
        targetInfo.identifier = static_cast<uint8_t>(1 + i);
        targets.push_back(targetInfo);
    }

    return targets;
}

TargetNavigatorLaunchResponse TargetNavigatorManager::proxyNavigateTargetRequest(uint8_t target, std::string data)
{
    // TODO: Insert code here
    TargetNavigatorLaunchResponse response;
    response.data   = "data response";
    response.status = EMBER_ZCL_APPLICATION_LAUNCHER_STATUS_SUCCESS;
    return response;
}

static void sendResponse(const char * responseName, TargetNavigatorLaunchResponse response)
{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_TARGET_NAVIGATOR_CLUSTER_ID,
                              ZCL_NAVIGATE_TARGET_RESPONSE_COMMAND_ID, "us", response.status, &response.data);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%s", responseName, chip::ErrorStr(status));
    }
}

bool emberAfTargetNavigatorClusterNavigateTargetCallback(unsigned char target, unsigned char * data)
{
    std::string dataString(reinterpret_cast<char *>(data));
    TargetNavigatorLaunchResponse response = TargetNavigatorManager().proxyNavigateTargetRequest(target, dataString);
    sendResponse("NavigateTargetResponse", response);
    return true;
}
