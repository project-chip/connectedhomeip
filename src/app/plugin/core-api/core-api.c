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
    ChipZclBuffer_t buffer = { rawBuffer, 0, 0, rawBufferLength };

    ChipZclCommandContext_t context = { 0 };

    chipZclDecodeZclHeader(&buffer, &context);
    return chipZclClusterCommandParse(&context);
}
