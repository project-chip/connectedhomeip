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

#include <stdarg.h>
#include <stdio.h>

#include "chip-zcl.h"

void chipZclCorePrintln(const char * formatString, ...)
{
    va_list ap;
    va_start(ap, formatString);
    printf(formatString, ap);
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

ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                     ChipZclAttributeId_t attributeId, void * buffer, size_t bufferLength)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

ChipZclStatus_t chipZclWriteAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                      ChipZclAttributeId_t attributeId, const void * buffer, size_t bufferLength)
{
    return CHIP_ZCL_STATUS_SUCCESS;
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

// Buffer Management Stubs
uint8_t * chipZclGetBufferPointer(Buffer buffer)
{
    return 0;
}

Buffer chipZclReallyAllocateBuffer(uint16_t length, bool unused)
{
    return 0;
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