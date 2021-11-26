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

/**
 *
 *    Copyright (c) 2021 Silicon Labs
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
 * @brief Routines for the Content Launch plugin, the
 *server implementation of the Content Launch cluster.
 *******************************************************************************
 ******************************************************************************/

#include "content-launch-server.h"
#include <app-common/zap-generated/af-structs.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <list>

using namespace chip;

ContentLaunchResponse contentLauncherClusterLaunchContent(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                          const chip::CharSpan & data);
ContentLaunchResponse contentLauncherClusterLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                      ContentLaunchBrandingInformation & brandingInformation);

bool emberAfContentLauncherClusterLaunchContentCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentLauncher::Commands::LaunchContent::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Clusters::ContentLauncher::Commands::LaunchContentResponse::Type response;

    auto & autoplay = commandData.autoPlay;
    auto & data     = commandData.data;
    std::list<ContentLaunchParamater> parameterList;

    ContentLaunchResponse resp = contentLauncherClusterLaunchContent(parameterList, autoplay, data);
    VerifyOrExit(resp.err == CHIP_NO_ERROR, err = resp.err);

    response.contentLaunchStatus = resp.status;
    response.data                = resp.data;

    err = commandObj->AddResponseData(commandPath, response);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchContentCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfContentLauncherClusterLaunchURLCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Clusters::ContentLauncher::Commands::LaunchURLResponse::Type response;

    auto & contentUrl    = commandData.contentURL;
    auto & displayString = commandData.displayString;
    ContentLaunchBrandingInformation brandingInformation;

    ContentLaunchResponse resp = contentLauncherClusterLaunchUrl(contentUrl, displayString, brandingInformation);
    VerifyOrExit(resp.err == CHIP_NO_ERROR, err = resp.err);

    response.contentLaunchStatus = resp.status;
    response.data                = resp.data;

    err = commandObj->AddResponseData(commandPath, response);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfContentLauncherClusterLaunchURLCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

void MatterContentLauncherPluginServerInitCallback() {}
