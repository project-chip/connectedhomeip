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
 * @brief Implementation for the Operational Credentials Cluster
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

bool emberAfOperationalCredentialsClusterRemoveFabricCallback(chip::app::Command * commandObj, chip::FabricId fabricId,
                                                              chip::NodeId nodeId, uint16_t vendorId)
{
    // Go through admin pairing table and remove the element
    // Then fetch the attribute list and find the right element (or maybe just use the admin pairing table index)
    // Then rewrite the entire attribute list (no choice for now)
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOperationalCredentialsClusterGetFabricIdCallback(chip::app::Command * commandObj)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: GetFabricId");
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(chip::app::Command * commandObj, uint8_t * Label)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel");
    // Go look at admin table using fabricId + update the label
    // Then find equivalent value in attribute list and update it.
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
