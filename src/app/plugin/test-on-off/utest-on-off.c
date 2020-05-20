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
 *      This file provides unit testing for the CHIP ZCL Command Handling
 *      related to the on-off-server functionality. It constructs an
 *      incoming message related to on-off and passes it into the CHIP
 *      ZCL message dispatch code
 *
 */

#include "chip-zcl.h"
#include "gen.h"

#include <stdio.h>

int main()
{
    ChipZclCommandContext_t context;
    context.mfgSpecific    = false;
    context.clusterId      = CHIP_ZCL_CLUSTER_ON_OFF;
    context.commandId      = ZCL_ON_COMMAND_ID;
    context.direction      = ZCL_DIRECTION_CLIENT_TO_SERVER;
    ChipZclStatus_t status = chipZclClusterCommandParse(&context);
    printf("Success: 0x%X \n", status);
}
