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

#include "ApplicationLauncherManager.h"

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>

#include <app/util/af.h>
#include <app/util/basic-types.h>

using namespace std;

CHIP_ERROR ApplicationLauncherManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err);
exit:
    return err;
}

vector<uint16_t> ApplicationLauncherManager::proxyGetApplicationList()
{
    vector<uint16_t> applications;
    applications.push_back(123);
    applications.push_back(456);
    return applications;
}

ApplicationLaunchResponse ApplicationLauncherManager::proxyApplicationLaunchRequest(EmberAfApplicationLauncherApp application,
                                                                                    string data)
{
    // TODO: Insert your code
    ApplicationLaunchResponse response;
    response.data   = "data";
    response.status = EMBER_ZCL_APPLICATION_LAUNCHER_STATUS_SUCCESS;
    // TODO: Update once storing a structure attribute is supported
    // emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_LAUNCH_CLUSTER_ID, ZCL_APPLICATION_LAUNCHER_CURRENT_APP_APPLICATION_ID,
    //                             (uint8_t *) &application, ZCL_STRUCT_ATTRIBUTE_TYPE);

    return response;
}

static void sendResponse(const char * responseName, ApplicationLaunchResponse response)
{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                              ZCL_APPLICATION_LAUNCHER_CLUSTER_ID, ZCL_LAUNCH_APP_RESPONSE_COMMAND_ID, "us", response.status,
                              &response.data);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%s", responseName, chip::ErrorStr(status));
    }
}

EmberAfApplicationLauncherApp getApplicationFromCommand(EmberAfClusterCommand * cmd, unsigned char * reqestData,
                                                        uint8_t * requestApplication)
{
    string reqestDataString(reinterpret_cast<char *>(reqestData));

    uint16_t extensionFieldSetsLen =
        static_cast<uint16_t>(cmd->bufLen - (cmd->payloadStartIndex + emberAfStringLength((uint8_t *) reqestData) + 1));
    uint16_t catalogVendorId = emberAfGetInt16u(requestApplication, 0, extensionFieldSetsLen);
    uint8_t * applicationId  = emberAfGetString(requestApplication, 0, extensionFieldSetsLen);

    EmberAfApplicationLauncherApp application = {};
    application.applicationId                 = applicationId;
    application.catalogVendorId               = catalogVendorId;
    return application;
}

bool emberAfApplicationLauncherClusterLaunchAppCallback(unsigned char * reqestData, uint8_t * requestApplication)
{
    EmberAfApplicationLauncherApp application = getApplicationFromCommand(emberAfCurrentCommand(), reqestData, requestApplication);
    string reqestDataString(reinterpret_cast<char *>(reqestData));

    ApplicationLaunchResponse response = ApplicationLauncherManager().proxyApplicationLaunchRequest(application, reqestDataString);

    sendResponse("LaunchAppResponse", response);
    return true;
}
