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
using namespace chip::app::Clusters::TvChannel;

::TvChannelInfo tvChannelClusterChangeChannel(std::string match);
bool tvChannelClusterChangeChannelByNumber(uint16_t majorNumer, uint16_t minorNumber);
bool tvChannelClusterSkipChannel(uint16_t count);

void sendResponse(app::CommandHandler * command, ::TvChannelInfo channelInfo)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), TvChannel::Id, Commands::ChangeChannelResponse::Id };
    TLV::TLVWriter * writer       = nullptr;
    SuccessOrExit(err = command->PrepareCommand(path));
    VerifyOrExit((writer = command->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // TODO: Enable this once struct as param is supported
    // SuccessOrExit(err = writer->Put(TLV::ContextTag(0), channelInfo));
    // EmberAfTvChannelErrorType. errorType
    // SuccessOrExit(err = writer->Put(TLV::ContextTag(1), errorType));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send ChangeChannel. Error:%s", ErrorStr(err));
    }
}

bool emberAfTvChannelClusterChangeChannelCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::ChangeChannel::DecodableType & commandData)
{
    auto & match = commandData.match;

    std::string matchString(match.data(), match.size());
    // TODO: Enable this once struct as param is supported
    // TvChannelInfo channelInfo = tvChannelClusterChangeChannel(matchString);
    // sendResponse(command, channelInfo);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTvChannelClusterChangeChannelByNumberCallback(app::CommandHandler * command,
                                                          const app::ConcreteCommandPath & commandPath,
                                                          const Commands::ChangeChannelByNumber::DecodableType & commandData)
{
    auto & majorNumber = commandData.majorNumber;
    auto & minorNumber = commandData.minorNumber;

    bool success         = tvChannelClusterChangeChannelByNumber(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfTvChannelClusterSkipChannelCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                const Commands::SkipChannel::DecodableType & commandData)
{
    auto & count = commandData.count;

    bool success         = tvChannelClusterSkipChannel(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterTvChannelPluginServerInitCallback() {}
