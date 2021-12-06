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
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>
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

CHIP_ERROR ApplicationLauncherManager::proxyGetApplicationList(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        ReturnErrorOnFailure(encoder.Encode(123u));
        ReturnErrorOnFailure(encoder.Encode(456u));
        return CHIP_NO_ERROR;
    });
}

ApplicationLauncherResponse applicationLauncherClusterLaunchApp(ApplicationLauncherApp application, std::string data)
{
    // TODO: Insert your code
    ApplicationLauncherResponse response;
    const char * testData = "data";
    response.data         = (uint8_t *) testData;
    response.status       = EMBER_ZCL_APPLICATION_LAUNCHER_STATUS_SUCCESS;
    // TODO: Update once storing a structure attribute is supported
    // emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_LAUNCH_CLUSTER_ID, ZCL_APPLICATION_LAUNCHER_CURRENT_APP_APPLICATION_ID,
    //                             (uint8_t *) &application, ZCL_STRUCT_ATTRIBUTE_TYPE);

    return response;
}
