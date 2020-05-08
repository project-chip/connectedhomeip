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
 *      This file provides the Silicon Labs implementation of functions
 *      specifically only required by the Silicon Labs implementation
 *      of these functions
 *
 */

#include "chip-zcl.h"

// Default Response Stubs
ChipZclStatus_t chipZclSendDefaultResponse(const ChipZclCommandContext_t * context, ChipZclStatus_t status)
{
    return emberZclDefaultResponse(context, status);
}

// Reporting Configuration Stubs
void chipZclReportingConfigurationsFactoryReset(ChipZclEndpointId_t endpointId)
{
    emberZclReportingConfigurationsFactoryReset(endpointId);
}

// Attribute Management Stubs
void chipZclResetAttributes(ChipZclEndpointId_t endpointId)
{
    emberZclResetAttributes(endpointId);
}

ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                     ChipZclAttributeId_t attributeId, void * buffer, size_t bufferLength)
{
    return emberZclReadAttribute(endpointId, clusterSpec, attributeId, buffer, bufferLength);
}

ChipZclStatus_t chipZclWriteAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                      ChipZclAttributeId_t attributeId, const void * buffer, size_t bufferLength)
{
    return emberZclWriteAttribute(endpointId, clusterSpec, attributeId, buffer, bufferLength);
}

// Event Management Stubs
EventQueue emAppEventQueue;

Event * chipZclEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all)
{
    return emFindEvents(queue, actions, predicate, data, all);
}

void chipZclEventSetDelayMs(Event * event, uint32_t delay)
{
    emberEventSetDelayMs(event, delay);
}

// Buffer Management Stubs
uint8_t * chipZclGetBufferPointer(Buffer buffer)
{
    return emGetBufferPointer(buffer);
}

Buffer chipZclReallyAllocateBuffer(uint16_t length, bool unused)
{
    return emReallyAllocateBuffer(length, unused);
}