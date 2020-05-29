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

#include "ChipZclUnitTests.h"

#include <chip-zcl.h>

#include "gen.h"

#include <stdio.h>

// -----------------------------------------------------------------------------
// Level Control cluster (0x0008).

// Level Control cluster client (0x0008).
const ChipZclClusterSpec_t chipZclClusterLevelControlClientSpec = {
    CHIP_ZCL_ROLE_CLIENT,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_LEVEL_CONTROL,
};

// Level Control cluster server (0x0008).
const ChipZclClusterSpec_t chipZclClusterLevelControlServerSpec = {
    CHIP_ZCL_ROLE_SERVER,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_LEVEL_CONTROL,
};

// On/off cluster server (0x0006).
const ChipZclClusterSpec_t chipZclClusterOnOffServerSpec = {
    CHIP_ZCL_ROLE_SERVER,
    CHIP_ZCL_MANUFACTURER_CODE_NULL,
    CHIP_ZCL_CLUSTER_ON_OFF,
};

void chipZclClusterLevelControlServerCommandMoveToLevelRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandMoveToLevelRequest_t * request);

void chipZclClusterLevelControlServerCommandMoveRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandMoveRequest_t * request);

void chipZclClusterLevelControlServerCommandStepRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandStepRequest_t * request);

void chipZclClusterLevelControlServerCommandStopRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandStopRequest_t * request);

void chipZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t * request);

void chipZclClusterLevelControlServerCommandMoveWithOnOffRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandMoveWithOnOffRequest_t * request);

void chipZclClusterLevelControlServerCommandStepWithOnOffRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandStepWithOnOffRequest_t * request);

void chipZclClusterLevelControlServerCommandStopWithOnOffRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterLevelControlServerCommandStopWithOnOffRequest_t * request);

int testClusterServerLevelControl()
{
    ChipZclCommandContext_t context;
    context.endpointId = 1;

    ChipZclClusterLevelControlServerCommandMoveToLevelRequest_t request1;
    request1.level          = 20;
    request1.transitionTime = 5;
    chipZclClusterLevelControlServerCommandMoveToLevelRequestHandler(&context, &request1);

    ChipZclClusterLevelControlServerCommandMoveRequest_t request2;
    request2.moveMode = 0;
    request2.rate     = 5;
    chipZclClusterLevelControlServerCommandMoveRequestHandler(&context, &request2);

    ChipZclClusterLevelControlServerCommandStepRequest_t request3;
    request3.stepMode       = 0;
    request3.stepSize       = 1;
    request3.transitionTime = 5;
    chipZclClusterLevelControlServerCommandStepRequestHandler(&context, &request3);

    ChipZclClusterLevelControlServerCommandStopRequest_t request4;
    request4.dummy = true;
    chipZclClusterLevelControlServerCommandStopRequestHandler(&context, &request4);

    ChipZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t request5;
    request5.level          = 255;
    request5.transitionTime = 5;
    chipZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(&context, &request5);

    ChipZclClusterLevelControlServerCommandMoveWithOnOffRequest_t request6;
    request6.moveMode = 0;
    request6.rate     = 5;
    chipZclClusterLevelControlServerCommandMoveWithOnOffRequestHandler(&context, &request6);

    ChipZclClusterLevelControlServerCommandStepWithOnOffRequest_t request7;
    request7.stepMode       = 0;
    request7.stepSize       = 5;
    request7.transitionTime = 5;
    chipZclClusterLevelControlServerCommandStepWithOnOffRequestHandler(&context, &request7);

    ChipZclClusterLevelControlServerCommandStopWithOnOffRequest_t request8;
    request8.dummy = true;
    chipZclClusterLevelControlServerCommandStopWithOnOffRequestHandler(&context, &request8);

    printf("Success \n");
    return 0;
}
