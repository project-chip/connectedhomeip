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

#include <app/CommandHandler.h>
#include <app/common/gen/af-structs.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>

EmberAfTvChannelInfo tvChannelClusterChangeChannel(std::string match);
bool tvChannelClusterChangeChannelByNumber(uint16_t majorNumer, uint16_t minorNumber);
bool tvChannelClusterSkipChannel(uint16_t count);

void sendResponse(chip::app::CommandHandler * command, EmberAfTvChannelInfo channelInfo)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    chip::app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_TV_CHANNEL_CLUSTER_ID,
                                               ZCL_CHANGE_CHANNEL_RESPONSE_COMMAND_ID,
                                               (chip::app::CommandPathFlags::kEndpointIdValid) };
    chip::TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // TODO: Enable this once struct as param is supported
    // SuccessOrExit(err = writer->Put(chip::TLV::ContextTag(0), channelInfo));
    // EmberAfTvChannelErrorType. errorType
    // SuccessOrExit(err = writer->Put(chip::TLV::ContextTag(1), errorType));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send ChangeChannel. Error:%s", chip::ErrorStr(err));
    }
}

bool emberAfTvChannelClusterChangeChannelCallback(chip::app::CommandHandler * command, uint8_t * match)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string matchString(reinterpret_cast<char *>(match));
    // TODO: Enable this once struct as param is supported
    // EmberAfTvChannelInfo channelInfo = tvChannelClusterChangeChannel(matchString);
    // sendResponse(command, channelInfo);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTvChannelClusterChangeChannelByNumberCallback(chip::app::CommandHandler * command, uint16_t majorNumber,
                                                          uint16_t minorNumber)
{
    bool success         = tvChannelClusterChangeChannelByNumber(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfTvChannelClusterSkipChannelCallback(chip::app::CommandHandler * command, uint16_t count)
{
    bool success         = tvChannelClusterSkipChannel(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
