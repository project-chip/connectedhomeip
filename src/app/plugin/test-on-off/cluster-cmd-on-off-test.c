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

#include "ChipZclOnOffTest.h"

#include "chip-zcl.h"

#include "gen.h"

#include <stdio.h>

int testClusterCmdOnOff(void)
{
    // First construct an incoming buffer for test, give it 1024 bytes for no good reason.
    ChipZclRawBuffer_t * buffer = chipZclBufferAlloc(1024);

    ChipZclCommandContext_t context;
    context.endpointId      = 1;
    context.clusterId       = CHIP_ZCL_CLUSTER_ON_OFF;
    context.clusterSpecific = true;
    context.mfgSpecific     = false;
    context.commandId       = ZCL_ON_COMMAND_ID;
    context.direction       = ZCL_DIRECTION_CLIENT_TO_SERVER;

    // Encode the header into the buffer
    chipZclEncodeZclHeader(buffer, &context);

    // Create another context to test decoding of header
    ChipZclCommandContext_t context2;
    chipZclBufferFlip(buffer);
    chipZclDecodeZclHeader(buffer, &context2);

    if (context2.endpointId != context.endpointId)
    {
        printf("ERROR: Endpoint ID doesnt match.");
        return 1;
    }
    if (context2.clusterId != context.clusterId)
    {
        printf("ERROR: Cluster ID doesnt match.");
        return 1;
    }
    if (context2.commandId != context.commandId)
    {
        printf("ERROR: Command ID doesnt match.");
        return 1;
    }
    if (context2.mfgSpecific != context.mfgSpecific)
    {
        printf("ERROR: MFG specific doesnt match.");
        return 1;
    }
    if (context2.clusterSpecific != context.clusterSpecific)
    {
        printf("ERROR: Cluster specific doesnt match.");
        return 1;
    }

    return chipZclClusterCommandParse(&context);
}
