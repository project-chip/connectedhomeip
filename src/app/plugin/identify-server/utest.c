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
 *      This file provides unit test code for the CHIP ZCL Application Layer's
 *      Indentify Cluster Server
 *
 */

#include "utest.h"

// -----------------------------------------------------------------------------
// Identify cluster (0x0003).

// Identify cluster client (0x0003).
const ChipZclClusterSpec_t chipZclClusterIdentifyClientSpec = {
    CHIP_ZCL_ROLE_CLIENT,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_IDENTIFY,
};

// Identify cluster server (0x0003).
const ChipZclClusterSpec_t chipZclClusterIdentifyServerSpec = {
    CHIP_ZCL_ROLE_SERVER,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_IDENTIFY,
};

ChipZclStatus_t chipZclSendClusterIdentifyServerCommandIdentifyQueryResponse(
    const ChipZclCommandContext_t * context, const ChipZclClusterIdentifyServerCommandIdentifyQueryResponse_t * response)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

void chipZclClusterIdentifyServerCommandIdentifyRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterIdentifyServerCommandIdentifyRequest_t * request);
void chipZclClusterIdentifyServerCommandIdentifyQueryRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterIdentifyServerCommandIdentifyQueryRequest_t * request);

void chipZclIdentifyServerStartIdentifyingCallback(ChipZclEndpointId_t endpointId, uint16_t identifyTimeS)
{
    chipZclCorePrintln("Identifying...");
}

void chipZclIdentifyServerStopIdentifyingCallback(ChipZclEndpointId_t endpointId)
{
    chipZclCorePrintln("Identified");
}

int main()
{
    ChipZclCommandContext_t context;
    context.endpointId = 1;

    ChipZclClusterIdentifyServerCommandIdentifyRequest_t request1;
    request1.identifyTime = 0;

    chipZclClusterIdentifyServerCommandIdentifyRequestHandler(&context, &request1);

    ChipZclClusterIdentifyServerCommandIdentifyQueryRequest_t request2;
    request2.dummy = true;

    chipZclClusterIdentifyServerCommandIdentifyQueryRequestHandler(&context, &request2);

    printf("Success \n");
    return 0;
}
