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
 * @brief Routines for the TV Channel plugin, the
 *server implementation of the TV Channel cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Channel;

::ChannelInfo ChannelClusterChangeChannel(std::string match);
bool ChannelClusterChangeChannelByNumber(uint16_t majorNumer, uint16_t minorNumber);
bool ChannelClusterSkipChannel(uint16_t count);

bool emberAfChannelClusterChangeChannelRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                       const Commands::ChangeChannelRequest::DecodableType & commandData)
{
    Commands::ChangeChannelResponse::Type response;
    response.channelMatch.majorNumber       = 1;
    response.channelMatch.minorNumber       = 0;
    response.channelMatch.name              = chip::CharSpan("name", strlen("name"));
    response.channelMatch.callSign          = chip::CharSpan("callSign", strlen("callSign"));
    response.channelMatch.affiliateCallSign = chip::CharSpan("affiliateCallSign", strlen("affiliateCallSign"));
    response.errorType                      = (ErrorTypeEnum) 0;

    CHIP_ERROR err = command->AddResponseData(commandPath, response);
    if (err != CHIP_NO_ERROR)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfChannelClusterChangeChannelByNumberRequestCallback(
    app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
    const Commands::ChangeChannelByNumberRequest::DecodableType & commandData)
{
    auto & majorNumber = commandData.majorNumber;
    auto & minorNumber = commandData.minorNumber;

    bool success         = ChannelClusterChangeChannelByNumber(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfChannelClusterSkipChannelRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::SkipChannelRequest::DecodableType & commandData)
{
    auto & count = commandData.count;

    bool success         = ChannelClusterSkipChannel(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterChannelPluginServerInitCallback() {}
