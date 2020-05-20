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
 *      This file provides unit testing for the CHIP ZCL Application
 *      layer's basic cluster server
 *
 */
#include "utest.h"

void zapClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterBasicServerCommandResetToFactoryDefaultsRequest_t * request);

int main()
{
    ChipZclCommandContext_t context;
    ChipZclClusterBasicServerCommandResetToFactoryDefaultsRequest_t request;

    context.endpointId = 1;
    request.dummy      = true;

    zapClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(&context, &request);

    printf("Success \n");
    return 0;
}
