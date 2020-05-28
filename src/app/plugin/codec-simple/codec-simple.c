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
ChipZclStatus_t chipZclCodecEncodeStart(ChipZclCodec_t * me, ChipZclBuffer_t * buffer)
{
    me->buffer    = buffer;
    me->cursor    = chipZclBufferDataLength(buffer);
    me->available = chipZclBufferAvailableLength(buffer);

    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t put(ChipZclCodec_t * me, void * ptr, uint16_t ptrLen)
{
    if (me->available < ptrLen)
    {
        return CHIP_ZCL_STATUS_FAILURE;
    }

    memmove(chipZclBufferPointer(me->buffer) + me->cursor, ptr, ptrLen);
    me->cursor += ptrLen;
    me->available -= ptrLen;

    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t get(ChipZclCodec_t * me, void * ptr, uint16_t ptrLen)
{
    if (me->available < ptrLen)
    {
        return CHIP_ZCL_STATUS_FAILURE;
    }

    memmove(ptr, chipZclBufferPointer(me->buffer) + me->cursor, ptrLen);
    me->cursor += ptrLen;
    me->available -= ptrLen;

    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Encodes a single value of a given type.
 */
ChipZclStatus_t chipZclCodecEncode(ChipZclCodec_t * me, ChipZclType_t type, void * ptr, uint16_t ptrLen)
{
    switch (type)
    {
    case CHIP_ZCL_STRUCT_TYPE_INTEGER:
        return put(me, ptr, ptrLen);

    case CHIP_ZCL_STRUCT_TYPE_STRING:
        // In this simple case, length is encoded as the first 2 bytes.
        {
            ChipZclStatus_t status = put(me, &ptrLen, 2);

            if (CHIP_ZCL_STATUS_SUCCESS != status)
            {
                return status;
            }
        }
        return put(me, ptr, ptrLen);

    default:
        return CHIP_ZCL_STATUS_FAILURE;
    }
}

/**
 * @brief Ends the encoding process. After this call the buffer is ready to go back to the lower layers.
 */
ChipZclStatus_t chipZclCodecEncodeEnd(ChipZclCodec_t * me)
{
    chipZclBufferSetDataLength(me->buffer, me->cursor);
    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Starts the decoding process. if there is any kind of preamble of anything, this function is responsible for decoding it.
 */
ChipZclStatus_t chipZclCodecDecodeStart(ChipZclCodec_t * me, ChipZclBuffer_t * buffer)
{
    me->buffer    = buffer;
    me->available = chipZclBufferDataLength(buffer);
    me->cursor    = 0;
    return CHIP_ZCL_STATUS_SUCCESS;
}

/**
 * @brief Decodes a single value and puts it into the pointer. If retLen is not NULL, the size of decoded value is put there.
 */
ChipZclStatus_t chipZclCodecDecode(ChipZclCodec_t * me, ChipZclType_t type, void * ptr, uint16_t ptrLen, uint16_t * retLen)
{
    uint16_t dummy;
    if (NULL == retLen)
    {
        retLen = &dummy;
    }

    switch (type)
    {
    case CHIP_ZCL_STRUCT_TYPE_INTEGER:
        *retLen = ptrLen;
        return get(me, ptr, *retLen);

    case CHIP_ZCL_STRUCT_TYPE_STRING:
        // gotta read length first
        {
            ChipZclStatus_t status;

            *retLen = 0;
            status  = get(me, retLen, 2);

            if (CHIP_ZCL_STATUS_SUCCESS != status)
            {
                return status;
            }
        }
        if (*retLen > ptrLen)
        {
            return CHIP_ZCL_STATUS_FAILURE;
        }
        return get(me, ptr, *retLen);

    default:
        return CHIP_ZCL_STATUS_FAILURE;
    }
}

/**
 * @brief Call after decoding to verify that everything has been decoded
 */
ChipZclStatus_t chipZclCodecDecodeEnd(ChipZclCodec_t * me)
{
    if (chipZclBufferDataLength(me->buffer) == me->cursor)
    {
        return CHIP_ZCL_STATUS_SUCCESS;
    }
    else
    {
        return CHIP_ZCL_STATUS_FAILURE;
    }
}

void chipZclEncodeZclHeader(ChipZclBuffer_t * buffer, ChipZclCommandContext_t * context)
{
    ChipZclCodec_t codec;
    uint8_t mask = 0;
    if (context->clusterSpecific)
    {
        mask |= 0x01;
    }
    if (context->mfgSpecific)
    {
        mask |= 0x02;
    }

    chipZclCodecEncodeStart(&codec, buffer);
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &mask, sizeof(mask));
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->endpointId), sizeof(context->endpointId));
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->commandId), sizeof(context->commandId));
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->direction), sizeof(context->direction));
    if (mask & 0x01)
    {
        chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->clusterId), sizeof(context->clusterId));
    }
    if (mask & 0x02)
    {
        chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->mfgCode), sizeof(context->mfgCode));
    }
    chipZclCodecEncodeEnd(&codec);
}

/**
 * This function takes the buffer and decodes it into ZCL header data in the context.
 */
void chipZclDecodeZclHeader(ChipZclBuffer_t * buffer, ChipZclCommandContext_t * context)
{
    ChipZclCodec_t codec;
    uint8_t mask = 0;
    chipZclCodecDecodeStart(&codec, buffer);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &mask, sizeof(mask), NULL);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->endpointId), sizeof(context->endpointId), NULL);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->commandId), sizeof(context->commandId), NULL);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->direction), sizeof(context->direction), NULL);
    if (mask & 0x01)
    {
        context->clusterSpecific = true;
        chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->clusterId), sizeof(context->clusterId), NULL);
    }
    else
    {
        context->clusterSpecific = false;
    }
    if (mask & 0x02)
    {
        context->mfgSpecific = true;
        chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(context->mfgCode), sizeof(context->mfgCode), NULL);
    }
    else
    {
        context->mfgSpecific = false;
    }
    chipZclCodecDecodeEnd(&codec);
}
