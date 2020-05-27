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
 *      This file provides a stub implementation of functions required
 *      by the CHIP ZCL Application Layer
 *
 */

#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip-zcl.h"

void chipZclCorePrintln(const char * formatString, ...)
{
    va_list ap;
    va_start(ap, formatString);
    vprintf(formatString, ap);
    printf("\n");
    va_end(ap);
}

// Default Response Stubs
ChipZclStatus_t chipZclSendDefaultResponse(const ChipZclCommandContext_t * context, ChipZclStatus_t status)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

// Reporting Configuration Stubs
void chipZclReportingConfigurationsFactoryReset(ChipZclEndpointId_t endpointId)
{
    return;
}

// Endpoint Management Stubs
ChipZclEndpointId_t chipZclEndpointIndexToId(ChipZclEndpointIndex_t index, const ChipZclClusterSpec_t * clusterSpec)
{
    return CHIP_ZCL_ENDPOINT_NULL;
}

// Attribute Management Stubs
void chipZclResetAttributes(ChipZclEndpointId_t endpointId)
{
    return;
}

// Event Management Stubs
EventQueue emAppEventQueue;

Event * chipZclEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all)
{
    return 0;
}

void chEventControlSetDelayMS(ChipZclEventControl * event, uint32_t delay)
{
    return;
}

void chipZclEventSetDelayMs(Event * event, uint32_t delay)
{
    return;
}

int32_t chipZclCompareClusterSpec(const ChipZclClusterSpec_t * s1, const ChipZclClusterSpec_t * s2)
{
    return 0;
}

bool chipZclAreClusterSpecsEqual(const ChipZclClusterSpec_t * s1, const ChipZclClusterSpec_t * s2)
{
    return 0;
}

void chipZclReverseClusterSpec(const ChipZclClusterSpec_t * s1, ChipZclClusterSpec_t * s2)
{
    return;
}

/**
 * Raw memory allocation. Can be mapped to an equivalent of malloc().
 * Expected to return NULL if it failed.
 */
void * chipZclRawAlloc(uint16_t allocatedLength)
{
    return malloc(allocatedLength);
}

/**
 * Raw memory free. Can be mapped to an equivalent of free().
 */
void chipZclRawFree(void * allocatedMemory)
{
    free(allocatedMemory);
}
