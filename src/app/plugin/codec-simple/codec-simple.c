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
 *      This file provides a simple implementation of the codec. It is the most
 *      basic C implementation, providing no specific safety and proper error handling
 *      but will do ok in a typical unit test case.
 *
 */
#include "chip-zcl-codec.h"

#include <memory.h>
#include <stdint.h>

/**
 * @brief Starts the encoding process. if there is any kind of preamble of anything, this function is responsible for putting it
 * there.
 */
ChipZclStatus_t chipZclCodecEncodeStart(ChipZclRawBuffer_t * buffer)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Encodes a single value of a given type.
 */
ChipZclStatus_t chipZclCodecEncode(ChipZclRawBuffer_t * buffer, ChipZclType_t type, void * ptr, uint16_t ptrLen)
{
    switch (type)
    {
    case CHIP_ZCL_STRUCT_TYPE_INTEGER:
        memcpy(&(buffer->buffer[buffer->currentPosition]), ptr, ptrLen);
        buffer->currentPosition += ptrLen;
        return CHIP_ZCL_STATUS_SUCCESS;

    case CHIP_ZCL_STRUCT_TYPE_STRING:
        // In this simple case, length is encoded as the first 2 bytes.
        memcpy(&(buffer->buffer[buffer->currentPosition]), &ptrLen, 2);
        buffer->currentPosition += 2;
        memcpy(&(buffer->buffer[buffer->currentPosition]), ptr, ptrLen);
        buffer->currentPosition += ptrLen;
        return CHIP_ZCL_STATUS_SUCCESS;

    default:
        return CHIP_ZCL_STATUS_FAILURE;
    }
}

/**
 * @brief Ends the encoding process. After this call the buffer is ready to go back to the lower layers.
 */
ChipZclStatus_t chipZclCodecEncodeEnd(ChipZclRawBuffer_t * buffer)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Starts the decoding process. if there is any kind of preamble of anything, this function is responsible for decoding it.
 */
ChipZclStatus_t chipZclCodecDecodeStart(ChipZclRawBuffer_t * buffer)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Decodes a single value and puts it into the pointer. If retLen is not NULL, the size of decoded value is put there.
 */
ChipZclStatus_t chipZclCodecDecode(ChipZclRawBuffer_t * buffer, ChipZclType_t type, void * ptr, uint16_t ptrLen, uint16_t * retLen)
{
    uint16_t encodedLength;
    switch (type)
    {
    case CHIP_ZCL_STRUCT_TYPE_INTEGER:
        memcpy(ptr, &(buffer->buffer[buffer->currentPosition]), ptrLen);
        if (retLen != NULL)
            *retLen = ptrLen;
        buffer->currentPosition += ptrLen;
        return CHIP_ZCL_STATUS_SUCCESS;

    case CHIP_ZCL_STRUCT_TYPE_STRING:
        memcpy(&encodedLength, &(buffer->buffer[buffer->currentPosition]), 2);
        buffer->currentPosition += 2;
        if (encodedLength > ptrLen)
            return CHIP_ZCL_STATUS_FAILURE;
        memmove(ptr, &(buffer->buffer[buffer->currentPosition]), encodedLength);
        buffer->currentPosition += encodedLength;
        return CHIP_ZCL_STATUS_SUCCESS;
    default:
        return CHIP_ZCL_STATUS_FAILURE;
    }
}

/**
 * @brief Ends the decoding process. After this call, buffer should no longer be used for further decoding.
 */
ChipZclStatus_t chipZclCodecDecodeEnd(ChipZclRawBuffer_t * buffer)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}
