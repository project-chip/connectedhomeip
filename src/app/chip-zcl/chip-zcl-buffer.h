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

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Structure that describes the buffers passed between the CHIP layers and
 * the Zap layer.
 *
 * This structure can be in one of two modes: ready to be written to, or ready
 * to be read from.  When it's ready to be written to, currentPosition
 * represents the current write position (i.e. the offset of the first byte that
 * has not been written yet) and dataLength is 0.  When it's ready to be read
 * from, currentPosition represents the current read position and dataLength is
 * the total length of the data that can be read.
 */
typedef struct ChipZclBuffer_t ChipZclBuffer_t;

/**
 * Function that allocates a buffer.
 *
 * @param[in] allocatedLength the number of octets the resulting buffer should
 *                            be able to store.
 * @return A newly allocated buffer.  This should later be freed with
 *         chipZclBufferFree.
 */
ChipZclBuffer_t * chipZclBufferAlloc(uint16_t allocatedLength);

/**
 * Function that returns a raw pointer to the underlying buffer.
 *
 * @param[in] buffer the buffer we are working with.
 * @return A pointer to its underlying memory.
 */
uint8_t * chipZclBufferPointer(ChipZclBuffer_t * buffer);

/**
 * Function that frees a buffer and its storage.
 *
 * @param[in] buffer a buffer to free.  The buffer should have been allocated
 *            with chipZclBufferAlloc.
 */
void chipZclBufferFree(ChipZclBuffer_t * buffer);

/**
 * Function that returns the size of the used portion of the buffer, in octets,
 * when the buffer is ready for reading.  Always returns 0 for buffers that are
 * being written to.
 *
 * @param[in] buffer the buffer whose used length we want.
 * @return The number of bytes of data the given buffer holds.
 */
uint16_t chipZclBufferDataLength(ChipZclBuffer_t * buffer);

/**
 * Indicates that we are done writing to a buffer and prepares it for reading.
 *
 * @param[in] buffer the buffer we are done writing to.
 * @param[in] newLength the length of the written data
 */
void chipZclBufferSetDataLength(ChipZclBuffer_t * buffer, uint16_t newLength);

/**
 * Function that returns available space remaining in the buffer after any
 * data in the buffer.
 *
 * @param[in] buffer the buffer we are interested in
 * @return The number of bytes left available for writing after any data
 */
uint16_t chipZclBufferAvailableLength(ChipZclBuffer_t * buffer);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif // CHIP_ZCL_BUFFER
