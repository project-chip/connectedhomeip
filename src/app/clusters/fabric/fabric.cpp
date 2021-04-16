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
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include "af.h"
#include <app/util/attribute-storage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

using namespace chip;

void emberAfPluginFabricServerInitCallback(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    for (uint8_t index = 0; index < emberAfEndpointCount(); index++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(index);
        if (!emberAfContainsCluster(endpoint, ZCL_FABRIC_CLUSTER_ID))
        {
            continue;
        }

        // Loop through admin pairing table list
        // For each index, write the attribute in
    }
    VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "emberAfPluginFabricServerInitCallback failed"));
}

bool emberAfFabricClusterRemoveFabricCallback(chip::FabricId fabricId, chip::NodeId nodeId, uint16_t vendorId)
{
    // Go through admin pairing table and remove the element
    // Then fetch the attribute list and find the right element (or maybe just use the admin pairing table index)
    // Then rewrite the entire attribute list (no choice for now)
    emberAfDoorLockClusterPrintln("Fabric: RemoveFabric");
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfFabricClusterGetFabricIdCallback()
{
    emberAfDoorLockClusterPrintln("Fabric: GetFabricID");
    uint64_t fabricID = emberAfCurrentCommand()->source;
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_FABRIC_CLUSTER_ID,
                              ZCL_GET_FABRIC_ID_RESPONSE_COMMAND_ID, "y", fabricID);
    EmberStatus sendStatus = emberAfSendResponse();
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfDoorLockClusterPrintln("Fabric: failed to send %s response: 0x%x", "get_fabric_id", sendStatus);
    }
    return true;
}

bool emberAfFabricClusterUpdateFabricLabelCallback(chip::ByteSpan label)
{
    emberAfDoorLockClusterPrintln("Fabric: UpdateFabricLabel");
    // Go look at admin table using fabricId + update the label
    // Then find equivalent value in attribute list and update it.
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}