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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;

ApplicationLauncherResponse applicationLauncherClusterLaunchApp(::ApplicationLauncherApp application, std::string data);

bool emberAfApplicationLauncherClusterLaunchAppCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath, EndpointId endpoint,
                                                        uint8_t *, uint8_t *, Commands::LaunchApp::DecodableType & commandData)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void sendResponse(app::CommandHandler * command, ApplicationLauncherResponse response)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), ApplicationLauncher::Id, Commands::LaunchAppResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = command->PrepareCommand(path));
    VerifyOrExit((writer = command->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), response.status));
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(1), reinterpret_cast<const char *>(response.data)));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send LaunchAppResponse. Error:%s", ErrorStr(err));
    }
}

::ApplicationLauncherApp getApplicationFromCommand(uint16_t catalogVendorId, CharSpan applicationId)
{
    ::ApplicationLauncherApp application = {};
    // TODO: Need to figure out what types we're using here.
    // application.applicationId            = applicationId;
    application.catalogVendorId = catalogVendorId;
    return application;
}

bool emberAfApplicationLauncherClusterLaunchAppCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::LaunchApp::DecodableType & commandData)
{
    auto & requestData                       = commandData.data;
    auto & requestApplicationCatalogVendorId = commandData.catalogVendorId;
    auto & requestApplicationId              = commandData.applicationId;

    ::ApplicationLauncherApp application = getApplicationFromCommand(requestApplicationCatalogVendorId, requestApplicationId);
    std::string reqestDataString(requestData.data(), requestData.size());
    ApplicationLauncherResponse response = applicationLauncherClusterLaunchApp(application, reqestDataString);
    sendResponse(command, response);
    return true;
}

void MatterApplicationLauncherPluginServerInitCallback() {}
