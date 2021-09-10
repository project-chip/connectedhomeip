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
 * @brief Routines for the Target Navigator plugin, the
 *server implementation of the Target Navigator cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/clusters/target-navigator-server/target-navigator-server.h>
#include <app/util/af.h>

using namespace chip;

TargetNavigatorResponse targetNavigatorClusterNavigateTarget(uint8_t target, std::string data);

void sendResponse(app::CommandHandler * command, TargetNavigatorResponse response)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_TARGET_NAVIGATOR_CLUSTER_ID,
                                         ZCL_NAVIGATE_TARGET_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), response.status));
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(1), reinterpret_cast<const char *>(response.data)));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send NavigateTargetResponse. Error:%s", ErrorStr(err));
    }
}

bool emberAfTargetNavigatorClusterNavigateTargetCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t target,
                                                         uint8_t * data)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string dataString(reinterpret_cast<char *>(data));
    TargetNavigatorResponse response = targetNavigatorClusterNavigateTarget(target, dataString);
    sendResponse(command, response);
    return true;
}
