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

/****************************************************************************
 * @file
 * @brief Routines for the Application Launcher plugin, the
 *server implementation of the Application Launcher cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <app/util/af.h>

using namespace chip;

ApplicationLauncherResponse applicationLauncherClusterLaunchApp(EmberAfApplicationLauncherApp application, std::string data);

bool emberAfApplicationLauncherClusterLaunchAppCallback(EndpointId endpoint, app::CommandHandler * commandObj, uint8_t *, uint8_t *)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void sendResponse(app::CommandHandler * command, ApplicationLauncherResponse response)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_APPLICATION_LAUNCHER_CLUSTER_ID,
                                         ZCL_LAUNCH_APP_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), response.status));
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(1), reinterpret_cast<const char *>(response.data)));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send LaunchAppResponse. Error:%s", ErrorStr(err));
    }
}

EmberAfApplicationLauncherApp getApplicationFromCommand(uint16_t catalogVendorId, uint8_t * applicationId)
{
    EmberAfApplicationLauncherApp application = {};
    application.applicationId                 = applicationId;
    application.catalogVendorId               = catalogVendorId;
    return application;
}

bool emberAfApplicationLauncherClusterLaunchAppCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t * requestData,
                                                        uint16_t requestApplicationCatalogVendorId, uint8_t * requestApplicationId)
{
    EmberAfApplicationLauncherApp application = getApplicationFromCommand(requestApplicationCatalogVendorId, requestApplicationId);
    // TODO: Char is not null terminated, verify this code once #7963 gets merged.
    std::string reqestDataString(reinterpret_cast<char *>(requestData));
    ApplicationLauncherResponse response = applicationLauncherClusterLaunchApp(application, reqestDataString);
    sendResponse(command, response);
    return true;
}
