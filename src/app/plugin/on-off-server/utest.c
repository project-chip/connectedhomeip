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

#include "utest.h" /* TODO pass all that stuff in -D */

// On/off cluster client (0x0006).
// On/off cluster client (0x0006).
const ZapClusterSpec_t zapClusterOnOffClientSpec = {
    ZAP_ROLE_CLIENT,
    ZAP_MANUFACTURER_CODE_NULL,
    ZAP_CLUSTER_ON_OFF,
};

// On/off cluster server (0x0006).
const ZapClusterSpec_t zapClusterOnOffServerSpec = {
    ZAP_ROLE_SERVER,
    ZAP_MANUFACTURER_CODE_NULL,
    ZAP_CLUSTER_ON_OFF,
};

void zapClusterOnOffServerCommandOffRequestHandler(const ZapCommandContext_t * context,
                                                   const ZapClusterOnOffServerCommandOffRequest_t * request);
void zapClusterOnOffServerCommandOnRequestHandler(const ZapCommandContext_t * context,
                                                  const ZapClusterOnOffServerCommandOnRequest_t * request);
void zapClusterOnOffServerCommandToggleRequestHandler(const ZapCommandContext_t * context,
                                                      const ZapClusterOnOffServerCommandToggleRequest_t * request);

int main()
{
    ZapCommandContext_t context;
    context.endpointId = 1;

    ZapClusterOnOffServerCommandOffRequest_t request1;
    request1.dummy = true;

    zapClusterOnOffServerCommandOffRequestHandler(&context, &request1);

    ZapClusterOnOffServerCommandOnRequest_t request2;
    request2.dummy = true;

    zapClusterOnOffServerCommandOnRequestHandler(&context, &request2);

    ZapClusterOnOffServerCommandToggleRequest_t request3;
    request3.dummy = true;

    zapClusterOnOffServerCommandToggleRequestHandler(&context, &request3);

    printf("Success \n");
    return 0;
}
