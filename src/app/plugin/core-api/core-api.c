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
ChipZclRawBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength)
{
    ChipZclRawBuffer_t * buffer = chipZclRawAlloc(sizeof(ChipZclRawBuffer_t));
    buffer->buffer              = chipZclRawAlloc(allocatedLength * sizeof(uint8_t));
    buffer->endPosition         = 0;
    buffer->currentPosition     = 0;
    buffer->totalLength         = allocatedLength;
    return buffer;
}

/**
 * Function that returns a pointer to the raw buffer.
 */
uint8_t * chipZclBufferPointer(ChipZclRawBuffer_t * buffer)
{
    return buffer->buffer;
}

/**
 * Function that frees a buffer.
 */
void chipZclBufferFree(ChipZclRawBuffer_t * buffer)
{
    chipZclRawFree(buffer->buffer);
    chipZclRawFree(buffer);
}

void chipZclBufferFlip(ChipZclRawBuffer_t * buffer)
{
    buffer->endPosition     = buffer->currentPosition;
    buffer->currentPosition = 0;
}

void chipZclBufferClear(ChipZclRawBuffer_t * buffer)
{
    buffer->currentPosition = 0;
    buffer->endPosition     = buffer->totalLength;
}
