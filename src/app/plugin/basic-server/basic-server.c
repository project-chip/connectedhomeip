/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *      This file provides the ZCL Basic Cluster Server implementation
 *      used by the CHIP ZCL Application Layer
 *
 */

#ifdef CHIP_TEST
#include "utest.h"
#endif

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
#include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE

void zapClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(
    const ChipZclCommandContext_t * context, const ChipZclClusterBasicServerCommandResetToFactoryDefaultsRequest_t * request)
{
    chipZclCorePrintln("RX: ResetToFactoryDefaults");

    chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);

    chipZclResetAttributes(context->endpointId);
    chipZclReportingConfigurationsFactoryReset(context->endpointId);
}
