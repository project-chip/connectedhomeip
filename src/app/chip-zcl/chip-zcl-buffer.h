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
 *      This file provides declarations required by the CHIP ZCL Application
 *      layer buffer system. Essentially an API to the lower layers.
 *
 */
#ifndef CHIP_ZCL_BUFFER
#define CHIP_ZCL_BUFFER

#include <stdint.h>

/**
 * Structure that describes the raw buffers passed between the CHIP layers and the Zap layer.
 */
typedef struct
{
    uint8_t * buffer;
    uint16_t totalLength;
    uint16_t currentPosition;
    uint16_t endPosition;
} ChipZclRawBuffer_t;

/**
 * Raw memory allocation. Can be mapped to an equivalent of malloc().
 * Expected to return NULL if it failed.
 * Expected to follow the same alignment requirements as malloc() does.
 */
void * chipZclRawAlloc(uint16_t allocatedLength);

/**
 * Raw memory free. Can be mapped to an equivalent of free().
 */
void chipZclRawFree(void * allocatedMemory);

/**
 * Function that allocates a buffer.
 */
ChipZclRawBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength);

/**
 * Function that returns a raw pointer to the underlying buffer.
 */
uint8_t * chipZclBufferPointer(ChipZclRawBuffer_t * buffer);

/**
 * Function that frees a buffer.
 */
void chipZclBufferFree(ChipZclRawBuffer_t * buffer);

/**
 * Function that resets a buffer.
 * After this call, the buffer is reading for reading from beginning again.
 */
void chipZclBufferReset(ChipZclRawBuffer_t * buffer);

/**
 * Function that returns the size of the used portion of the buffer.
 */
uint16_t chipZclBufferUsedLength(ChipZclRawBuffer_t * buffer);

/**
 * Flips the buffer between reading and writing.
 */
void chipZclBufferFlip(ChipZclRawBuffer_t * buffer);

/**
 * Creates a ZCL buffer out of raw chunk of memory.
 */
ChipZclRawBuffer_t * chipZclBufferCreate(uint8_t * rawBuffer, uint16_t rawLength);

#endif // CHIP_ZCL_BUFFER
