/*
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
 * @brief Implementation for the Test Server Cluster
 ***************************************************************************/

#include <app/Command.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

using namespace chip;

void emberAfPluginTestClusterServerInitCallback(void) {}

bool emberAfTestClusterClusterTestCallback(chip::app::Command *)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfTestClusterClusterTestSpecificCallback(chip::app::Command * apCommandObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t returnValue = 7;
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_TEST_CLUSTER_ID,
                              ZCL_TEST_SPECIFIC_RESPONSE_COMMAND_ID, "u", returnValue);

    EmberStatus sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        ChipLogError(Zcl, "Test Cluster: failed to send TestSpecific response: 0x%x", sendStatus);
    }

    if (apCommandObj != nullptr)
    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_TEST_CLUSTER_ID,
                                             ZCL_TEST_SPECIFIC_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer = nullptr;

        SuccessOrExit(err = apCommandObj->PrepareCommand(&cmdParams));
        writer = apCommandObj->GetCommandDataElementTLVWriter();
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), returnValue));
        SuccessOrExit(err = apCommandObj->FinishCommand());
    }

exit:
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Test Cluster: failed to send TestSpecific response: %x", err);
    }
    return true;
}

bool emberAfTestClusterClusterTestNotHandledCallback(chip::app::Command *)
{
    return false;
}
