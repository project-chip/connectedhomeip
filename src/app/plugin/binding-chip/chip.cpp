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
 *      This file provides the CHIP implementation of functions required
 *      by the CHIP ZCL Application Layer
 *
 */

#include "chip-zcl.h"

#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <stdarg.h>

void chipZclCorePrintln(const char * formatString, ...)
{
    va_list args;
    va_start(args, formatString);
    chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Detail, formatString, args);
    va_end(args);
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
 * ChipZclBuffer_t is merely a type alias
 */
struct ChipZclBuffer_t : public chip::System::PacketBuffer
{
};

/**
 * Function that allocates a buffer.
 */
ChipZclBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength)
{
    return static_cast<ChipZclBuffer_t *>(chip::System::PacketBuffer::NewWithAvailableSize(allocatedLength));
}

/**
 * Function that frees a buffer and its storage.
 */
void chipZclBufferFree(ChipZclBuffer_t * buffer)
{
    chip::System::PacketBuffer::Free(buffer);
}

/**
 * Function that returns a pointer to the raw buffer.
 */
uint8_t * chipZclBufferPointer(ChipZclBuffer_t * buffer)
{
    return buffer->Start();
}

/**
 * Function that returns the size of the used portion of the buffer.
 */
uint16_t chipZclBufferDataLength(ChipZclBuffer_t * buffer)
{
    return buffer->DataLength();
}

/**
 * returns space available for writing after any data already in the buffer
 */
uint16_t chipZclBufferAvailableLength(ChipZclBuffer_t * buffer)
{
    return buffer->AvailableDataLength();
}

/**
 *  sets data length for a buffer that's being written to
 */
void chipZclBufferSetDataLength(ChipZclBuffer_t * buffer, uint16_t newLength)
{
    buffer->SetDataLength(newLength);
}
