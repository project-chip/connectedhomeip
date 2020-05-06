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

#include "utest.h"

// -----------------------------------------------------------------------------
// Level Control cluster (0x0008).

// Level Control cluster client (0x0008).
const ZapClusterSpec_t zapClusterLevelControlClientSpec = {
    ZAP_ROLE_CLIENT,
    ZAP_MANUFACTURER_CODE_NULL,
    ZAP_CLUSTER_LEVEL_CONTROL,
};

// Level Control cluster server (0x0008).
const ZapClusterSpec_t zapClusterLevelControlServerSpec = {
    ZAP_ROLE_SERVER,
    ZAP_MANUFACTURER_CODE_NULL,
    ZAP_CLUSTER_LEVEL_CONTROL,
};

// On/off cluster server (0x0006).
const ZapClusterSpec_t zapClusterOnOffServerSpec = {
    ZAP_ROLE_SERVER,
    ZAP_MANUFACTURER_CODE_NULL,
    ZAP_CLUSTER_ON_OFF,
};

void zapClusterLevelControlServerCommandMoveToLevelRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelRequest_t * request);

void zapClusterLevelControlServerCommandMoveRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandMoveRequest_t * request);

void zapClusterLevelControlServerCommandStepRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandStepRequest_t * request);

void zapClusterLevelControlServerCommandStopRequestHandler(const ZapCommandContext_t * context,
                                                           const ZapClusterLevelControlServerCommandStopRequest_t * request);

void zapClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t * request);

void zapClusterLevelControlServerCommandMoveWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandMoveWithOnOffRequest_t * request);

void zapClusterLevelControlServerCommandStepWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStepWithOnOffRequest_t * request);

void zapClusterLevelControlServerCommandStopWithOnOffRequestHandler(
    const ZapCommandContext_t * context, const ZapClusterLevelControlServerCommandStopWithOnOffRequest_t * request);

int main()
{
    ZapCommandContext_t context;
    context.endpointId = 1;

    ZapClusterLevelControlServerCommandMoveToLevelRequest_t request1;
    request1.level          = 20;
    request1.transitionTime = 5;
    zapClusterLevelControlServerCommandMoveToLevelRequestHandler(&context, &request1);

    ZapClusterLevelControlServerCommandMoveRequest_t request2;
    request2.moveMode = 0;
    request2.rate     = 5;
    zapClusterLevelControlServerCommandMoveRequestHandler(&context, &request2);

    ZapClusterLevelControlServerCommandStepRequest_t request3;
    request3.stepMode       = 0;
    request3.stepSize       = 1;
    request3.transitionTime = 5;
    zapClusterLevelControlServerCommandStepRequestHandler(&context, &request3);

    ZapClusterLevelControlServerCommandStopRequest_t request4;
    request4.dummy = true;
    zapClusterLevelControlServerCommandStopRequestHandler(&context, &request4);

    ZapClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t request5;
    request5.level          = 255;
    request5.transitionTime = 5;
    zapClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(&context, &request5);

    ZapClusterLevelControlServerCommandMoveWithOnOffRequest_t request6;
    request6.moveMode = 0;
    request6.rate     = 5;
    zapClusterLevelControlServerCommandMoveWithOnOffRequestHandler(&context, &request6);

    ZapClusterLevelControlServerCommandStepWithOnOffRequest_t request7;
    request7.stepMode       = 0;
    request7.stepSize       = 5;
    request7.transitionTime = 5;
    zapClusterLevelControlServerCommandStepWithOnOffRequestHandler(&context, &request7);

    ZapClusterLevelControlServerCommandStopWithOnOffRequest_t request8;
    request8.dummy = true;
    zapClusterLevelControlServerCommandStopWithOnOffRequestHandler(&context, &request8);

    printf("Success \n");
    return 0;
}
