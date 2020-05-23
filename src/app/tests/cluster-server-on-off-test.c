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
 *      On Off Cluster Server
 *
 */

#include "ChipZclUnitTests.h"

#include <chip-zcl.h>

#include "gen.h"

#include <stdio.h>

void chipZclClusterOnOffServerCommandOffRequestHandler(const ChipZclCommandContext_t * context,
                                                       const ChipZclClusterOnOffServerCommandOffRequest_t * request);
void chipZclClusterOnOffServerCommandOnRequestHandler(const ChipZclCommandContext_t * context,
                                                      const ChipZclClusterOnOffServerCommandOnRequest_t * request);
void chipZclClusterOnOffServerCommandToggleRequestHandler(const ChipZclCommandContext_t * context,
                                                          const ChipZclClusterOnOffServerCommandToggleRequest_t * request);

int testClusterServerOnOff()
{
    ChipZclCommandContext_t context;
    context.endpointId = 1;

    ChipZclClusterOnOffServerCommandOffRequest_t request1;
    request1.dummy = true;

    chipZclClusterOnOffServerCommandOffRequestHandler(&context, &request1);

    ChipZclClusterOnOffServerCommandOnRequest_t request2;
    request2.dummy = true;

    chipZclClusterOnOffServerCommandOnRequestHandler(&context, &request2);

    ChipZclClusterOnOffServerCommandToggleRequest_t request3;
    request3.dummy = true;

    chipZclClusterOnOffServerCommandToggleRequestHandler(&context, &request3);

    printf("Success \n");
    return 0;
}
