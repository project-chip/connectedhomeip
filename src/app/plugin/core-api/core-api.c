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

#include <memory.h>

#include "chip-zcl.h"

/**
 * Function that allocates a buffer.
 */
ChipZclBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength)
{
    ChipZclBuffer_t * buffer = chipZclRawAlloc(sizeof(ChipZclBuffer_t));
    buffer->buffer           = chipZclRawAlloc(allocatedLength * sizeof(uint8_t));
    buffer->dataLength       = 0;
    buffer->currentPosition  = 0;
    buffer->totalLength      = allocatedLength;
    return buffer;
}

/**
 * Creates a zcl buffer out of a raw chunk of memory.
 */
ChipZclBuffer_t * chipZclBufferCreate(uint8_t * rawBuffer, uint16_t rawLength)
{
    ChipZclBuffer_t * buffer = chipZclRawAlloc(sizeof(ChipZclBuffer_t));
    buffer->buffer           = rawBuffer;
    buffer->dataLength       = 0;
    buffer->currentPosition  = 0;
    buffer->totalLength      = rawLength;
    return buffer;
}

/**
 * Function that returns a pointer to the raw buffer.
 */
uint8_t * chipZclBufferPointer(ChipZclBuffer_t * buffer)
{
    return buffer->buffer;
}

/**
 * Function that returns the size of the used portion of the buffer.
 */
uint16_t chipZclBufferUsedLength(ChipZclBuffer_t * buffer)
{
    return buffer->dataLength;
}

/**
 * Function that frees a buffer and its storage.
 */
void chipZclBufferFree(ChipZclBuffer_t * buffer)
{
    chipZclRawFree(buffer->buffer);
    chipZclBufferFreeOnlyBuffer(buffer);
}

/**
 * Function that frees a buffer but not its storage.
 */
void chipZclBufferFreeOnlyBuffer(ChipZclBuffer_t * buffer)
{
    chipZclRawFree(buffer);
}

void chipZclBufferReset(ChipZclBuffer_t * buffer)
{
    buffer->currentPosition = 0;
}

void chipZclBufferFinishWriting(ChipZclBuffer_t * buffer)
{
    buffer->dataLength      = buffer->currentPosition;
    buffer->currentPosition = 0;
}

void chipZclBufferClear(ChipZclBuffer_t * buffer)
{
    buffer->currentPosition = 0;
    buffer->dataLength      = 0;
}

ChipZclStatus_t chipZclClusterCommandParse(ChipZclCommandContext_t * context);

ChipZclStatus_t chipZclProcessIncoming(uint8_t * rawBuffer, uint16_t rawBufferLength)
{
    ChipZclBuffer_t * buffer          = chipZclBufferCreate(rawBuffer, rawBufferLength);
    ChipZclCommandContext_t * context = chipZclRawAlloc(sizeof(ChipZclCommandContext_t));

    chipZclDecodeZclHeader(buffer, context);
    chipZclClusterCommandParse(context);

    chipZclRawFree(context);
    chipZclBufferFreeOnlyBuffer(buffer);

    return CHIP_ZCL_STATUS_SUCCESS;
}
