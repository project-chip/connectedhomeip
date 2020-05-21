/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *      This provides unit testing for the CHIP ZCL Application
 *      layer's level control cluster server
 *
 */

#include "test-unit.h"

// Main command parsing controller.
ChipZclStatus_t chipZclCommandParse(ChipZclCommandContext_t * context);

int testCoreMessageDispatch()
{
    chipZclEndpointInit();

    ChipZclStatus_t status = CHIP_ZCL_STATUS_FAILURE;

    ChipZclAttributeId_t att[] = { 0x0000, 0x0001 };

    ChipZclGeneralCommandReadAttributesRequest_t request = {
        .count      = 2,
        .attributes = att,
    };

    ChipZclCommandContext_t context = {
        .endpointId      = 1,
        .mfgSpecific     = false,
        .clusterSpecific = false,
        .clusterId       = CHIP_ZCL_CLUSTER_BASIC,
        .commandId       = ZCL_READ_ATTRIBUTES_COMMAND_ID,
        .direction       = ZCL_DIRECTION_CLIENT_TO_SERVER,
        .request         = &request,
    };

    status = chipZclCommandParse(&context);

    (status == CHIP_ZCL_STATUS_SUCCESS) ? printf("SUCCESS 0x%X\n", status) : printf("FAILURE 0x%X\n", status);
    return status;
}
