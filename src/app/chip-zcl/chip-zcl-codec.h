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
 *      This file provides declarations required by the CHIP ZCL codec.
 *      An implementation of the codec needs to implement the functions
 *      declared in this header.
 *
 */
#ifndef CHIP_ZCL_CODEC
#define CHIP_ZCL_CODEC

#include "chip-zcl-struct.h"
#include "chip-zcl.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Starts the encoding process. if there is any kind of preamble of anything, this function is responsible for putting it
 * there.
 */
ChipZclStatus_t chipZclCodecEncodeStart(ChipZclRawBuffer_t * buffer);

/**
 * @brief Encodes a single value of a given type.
 */
ChipZclStatus_t chipZclCodecEncode(ChipZclRawBuffer_t * buffer, ChipZclType_t type, void * ptr, uint16_t ptrLen);

/**
 * @brief Ends the encoding process. After this call the buffer is ready to go back to the lower layers.
 */
ChipZclStatus_t chipZclCodecEncodeEnd(ChipZclRawBuffer_t * buffer);

/**
 * @brief Starts the decoding process. if there is any kind of preamble of anything, this function is responsible for decoding it.
 */
ChipZclStatus_t chipZclCodecDecodeStart(ChipZclRawBuffer_t * buffer);

/**
 * @brief Decodes a single value and puts it into the pointer. If retLen is not NULL, the size of decoded value is put there.
 */
ChipZclStatus_t chipZclCodecDecode(ChipZclRawBuffer_t * buffer, ChipZclType_t type, void * ptr, uint16_t ptrLen, uint16_t * retLen);

/**
 * @brief Ends the decoding process. After this call, buffer should no longer be used for further decoding.
 */
ChipZclStatus_t chipZclCodecDecodeEnd(ChipZclRawBuffer_t * buffer);

#endif // CHIP_ZCL_CODEC