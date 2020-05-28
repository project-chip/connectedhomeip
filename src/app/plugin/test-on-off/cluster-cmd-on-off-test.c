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

// Test function that creates a command context and populates it with some data.
void testInitCommandContext(ChipZclCommandContext_t * context)
{
    context->endpointId      = 1;
    context->clusterId       = CHIP_ZCL_CLUSTER_ON_OFF;
    context->clusterSpecific = true;
    context->mfgSpecific     = false;
    context->commandId       = ZCL_ON_COMMAND_ID;
    context->direction       = ZCL_DIRECTION_CLIENT_TO_SERVER;
}

// Function that tests that the command encoder/decoder works correctly.
int testEncodingDecoding(ChipZclBuffer_t * buffer, ChipZclCommandContext_t * context)
{
    chipZclEndpointInit();

    // Create another context to test decoding of header
    ChipZclCommandContext_t context2;
    chipZclDecodeZclHeader(buffer, &context2);

    if (context2.endpointId != context->endpointId)
    {
        printf("ERROR: Endpoint ID doesnt match.");
        return 1;
    }
    if (context2.clusterId != context->clusterId)
    {
        printf("ERROR: Cluster ID doesnt match.");
        return 1;
    }
    if (context2.commandId != context->commandId)
    {
        printf("ERROR: Command ID doesnt match.");
        return 1;
    }
    if (context2.mfgSpecific != context->mfgSpecific)
    {
        printf("ERROR: MFG specific doesnt match.");
        return 1;
    }
    if (context2.clusterSpecific != context->clusterSpecific)
    {
        printf("ERROR: Cluster specific doesnt match.");
        return 1;
    }
    printf("SUCCESS: header encoder/decoder");

    return 0;
}

bool globalAttributeChangedFlag = false;

void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    printf("Attribute change callback: ep=%d clusterId=%d attributeId=%d\n", endpoint, clusterId, attributeId);
    globalAttributeChangedFlag = true;
}

int testClusterCmdOnOff(void)
{
    // First construct an incoming buffer for test, give it 1024 bytes for no good reason.
    ChipZclBuffer_t * buffer = chipZclBufferAlloc(1024);

    ChipZclCommandContext_t context;
    testInitCommandContext(&context);

    // Encode the header into the buffer
    chipZclEncodeZclHeader(buffer, &context);

    if (testEncodingDecoding(buffer, &context) != 0)
    {
        return 1;
    }

    uint8_t * rawBuffer   = chipZclBufferPointer(buffer);
    uint16_t bufferLength = chipZclBufferDataLength(buffer);

    printf("Buffer for processing is ready, length: %d\n", bufferLength);

    // At this point, we have a buffer encoded with the command context that contains the command.
    // So we will be testing top-level entry API.
    ChipZclStatus_t status = chipZclProcessIncoming(rawBuffer, bufferLength);

    if (status == CHIP_ZCL_STATUS_SUCCESS)
    {
        if (globalAttributeChangedFlag == false)
        {
            printf("ERROR: attribute change callback did not fire!\n");
            return 1;
        }
        else
        {
            printf("SUCCESS: attribute got changed and attrribute change callback did fire.\n");
            return 0;
        }
    }
    else
    {
        printf("ERROR: Process incoming failed with: %d\n", status);
        return 1;
    }
}
