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
    uint16_t index;
} ChipZclRawBuffer_t;

/**
 * Function that allocates a buffer.
 */
ChipZclRawBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength);

/**
 * Function that frees a buffer.
 */
void chipZclBufferFree(ChipZclRawBuffer_t * buffer);

#endif // CHIP_ZCL_BUFFER