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
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

TvChannelInfo tvChannelClusterChangeChannel(std::string match);
bool tvChannelClusterChangeChannelByNumber(uint16_t majorNumer, uint16_t minorNumber);
bool tvChannelClusterSkipChannel(uint16_t count);

void sendResponse(app::CommandHandler * command, TvChannelInfo channelInfo)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_TV_CHANNEL_CLUSTER_ID,
                                         ZCL_CHANGE_CHANNEL_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
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

bool emberAfTvChannelClusterChangeChannelCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t * match)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string matchString(reinterpret_cast<char *>(match));
    // TODO: Enable this once struct as param is supported
    // TvChannelInfo channelInfo = tvChannelClusterChangeChannel(matchString);
    // sendResponse(command, channelInfo);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTvChannelClusterChangeChannelByNumberCallback(EndpointId endpoint, app::CommandHandler * command, uint16_t majorNumber,
                                                          uint16_t minorNumber)
{
    bool success         = tvChannelClusterChangeChannelByNumber(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfTvChannelClusterSkipChannelCallback(EndpointId endpoint, app::CommandHandler * command, uint16_t count)
{
    bool success         = tvChannelClusterSkipChannel(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
