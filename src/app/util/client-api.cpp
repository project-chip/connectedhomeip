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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief
 *******************************************************************************
 ******************************************************************************/

#include "app/util/common.h"
#include <app/util/client-api.h>
#include <app/util/util.h>
#include <stdarg.h>

using namespace chip;

uint8_t * emAfZclBuffer   = NULL;
uint16_t emAfZclBufferLen = 0;

// Pointer to where this API should put the length
uint16_t * emAfResponseLengthPtr = NULL;

// Pointer to where the API should put the cluster ID
EmberApsFrame * emAfCommandApsFrame = NULL;

/////////////////

// Method that fills in the buffer.
// It returns number of bytes filled
// and 0 on error.
static uint16_t vFillBuffer(uint8_t * buffer, uint16_t bufferLen, uint8_t frameControl, uint16_t manufacturerCode,
                            CommandId commandId, const char * format, va_list argPointer)
{
    uint64_t value;
    uint8_t valueLen;
    uint8_t * data;
    uint16_t dataLen;
    uint8_t i;
    uint16_t bytes = 0;

    // The destination buffer must be at least large enough to hold the ZCL
    // overhead: frame control, manufacturer code (if applicable), sequence
    // number, and command id.  If it is, add these in order.
    if (bufferLen < EMBER_AF_ZCL_OVERHEAD ||
        (manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE && bufferLen < EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD))
    {
        emberAfDebugPrintln("ERR: Buffer too short for ZCL header");
        return 0;
    }
    if (manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE)
    {
        frameControl |= ZCL_MANUFACTURER_SPECIFIC_MASK;
    }
    buffer[bytes++] = frameControl;
    if (manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE)
    {
        buffer[bytes++] = EMBER_LOW_BYTE(manufacturerCode);
        buffer[bytes++] = EMBER_HIGH_BYTE(manufacturerCode);
    }
    buffer[bytes++] = emberAfNextSequence();
    buffer[bytes++] = EMBER_BYTE_0(commandId);
    buffer[bytes++] = EMBER_BYTE_1(commandId);
    buffer[bytes++] = EMBER_BYTE_2(commandId);
    buffer[bytes++] = EMBER_BYTE_3(commandId);

    // Each argument comes in as an integer value, a pointer to a buffer, or a
    // pointer to a buffer followed by an integer length.
    for (i = 0; format[i] != 0; i++)
    {
        char cmd;
        value    = 0;
        valueLen = 0;
        data     = 0;
        cmd      = format[i];
        if (cmd <= 's')
        {
            //  0--9, A--G, L, S, b, l, and s all have a pointer to a buffer.  The
            // length of that buffer is implied by 0--9 and A--G (0 to 16 bytes).
            // For L, S, and b, a separate integer specifies the length.  That length
            // will precede the data in the destination buffer for L and S, which
            // turns them into regular ZigBee strings.  In this case, care must be
            // taken to account for invalid strings, which have length 0xFFFF or 0xFF
            // for L and S respectively.  In the case of invalid strings, the length
            // byte(s) are copied to the destination buffer but the string itself is
            // not.  Finally, l and s are just ZigBee strings and the length of the
            // string data is contained within the buffer itself and the entire
            // buffer is copied as is to the destination buffer.  Note that
            // emberAf(Long)StringLength handles invalid strings, but it does not
            // include the length byte(s) in the total length, so the overhead
            // must be maually accounted for when copying.
            data     = (uint8_t *) va_arg(argPointer, uint8_t *);
            valueLen = 0;
            if (cmd == 'L' || cmd == 'S' || cmd == 'b')
            {
                dataLen = (uint16_t) va_arg(argPointer, int);
                if (cmd == 'L')
                {
                    value    = dataLen;
                    valueLen = (value == 0xFFFF ? 0 : 2);
                }
                else if (cmd == 'S')
                {
                    value    = (uint8_t) dataLen;
                    valueLen = (value == 0xFF ? 0 : 1);
                }
                else
                {
                    // MISRA requires ..else if.. to have terminating else.
                }
            }
            else if (cmd == 'l')
            {
                dataLen = static_cast<uint16_t>(emberAfLongStringLength(data) + 2);
            }
            else if (cmd == 's')
            {
                dataLen = static_cast<uint16_t>(emberAfStringLength(data) + 1);
            }
            else if ('0' <= cmd && cmd <= '9')
            {
                dataLen = static_cast<uint16_t>(cmd - '0');
            }
            else if ('A' <= cmd && cmd <= 'G')
            {
                dataLen = static_cast<uint16_t>(cmd - 'A' + 10);
            }
            else
            {
                emberAfDebugPrintln("ERR: Unknown format '%c'", cmd);
                return 0;
            }
        }
        else
        {
            // u, v, x, w, and y are one-, two-, three-, four-byte or eight-byte integers.  u and v
            // must be extracted as an int while x and w come through as an uint32_t, y as a uint64_t.
            // In all cases, the value is copied to the destination buffer in little-
            // endian format.
            dataLen = 0;
            if (cmd == 'u')
            {
                valueLen = 1;
            }
            else if (cmd == 'v')
            {
                valueLen = 2;
            }
            else if (cmd == 'x')
            {
                valueLen = 3;
            }
            else if (cmd == 'w')
            {
                valueLen = 4;
            }
            else if (cmd == 'y')
            {
                valueLen = 8;
            }
            else
            {
                emberAfDebugPrintln("ERR: Unknown format '%c'", cmd);
                return 0;
            }
            if (valueLen <= 2)
            {
                value = static_cast<uint64_t>(va_arg(argPointer, int));
            }
            else if (valueLen <= 4)
            {
                value = static_cast<uint64_t>(va_arg(argPointer, uint32_t));
            }
            else
            {
                value = va_arg(argPointer, uint64_t);
            }
        }

        // The destination buffer must be at least as large as the running total
        // plus the length of the integer value (if applicable) plus the length of
        // the data (if applicable).
        if (bufferLen < bytes + dataLen + valueLen)
        {
            emberAfDebugPrintln("ERR: Buffer too short for %d bytes for format '%c'", dataLen + valueLen, cmd);
            return 0;
        }

        // If there is an integer value, add it to destination buffer in little-
        // endian format.
        for (; 0 < valueLen; valueLen--)
        {
            buffer[bytes++] = EMBER_LOW_BYTE(value);
            value           = value >> 8;
        }

        // Finally, if there is data, add it to the destination buffer as is.  If
        // the data length is zero, data may actually be NULL.  Even if the length
        // argument is zero, passing NULL as either the source or destination to
        // memcpy is invalid and the behavior is undefined.  We avoid that with an
        // explicit check.
        if (dataLen != 0)
        {
            if (data == NULL)
            {
                emberAfDebugPrintln("ERR: Missing data for %d bytes for format '%c'", dataLen, cmd);
                return 0;
            }
            memcpy(buffer + bytes, data, dataLen);
            bytes = static_cast<uint16_t>(bytes + dataLen);
        }
    }

    return bytes;
}

////////////////////// Public API ////////////////////////

void emberAfSetExternalBuffer(uint8_t * buffer, uint16_t bufferLen, uint16_t * lenPtr, EmberApsFrame * apsFrame)
{
    emAfZclBuffer         = buffer;
    emAfZclBufferLen      = bufferLen;
    emAfResponseLengthPtr = lenPtr;
    emAfCommandApsFrame   = apsFrame;
}

uint16_t emberAfFillExternalManufacturerSpecificBuffer(uint8_t frameControl, ClusterId clusterId, uint16_t manufacturerCode,
                                                       CommandId commandId, const char * format, ...)
{
    uint16_t returnValue;
    va_list argPointer;

    va_start(argPointer, format);
    returnValue = vFillBuffer(emAfZclBuffer, emAfZclBufferLen, frameControl, manufacturerCode, commandId, format, argPointer);
    va_end(argPointer);
    *emAfResponseLengthPtr         = returnValue;
    emAfCommandApsFrame->clusterId = clusterId;
    emAfCommandApsFrame->options   = EMBER_AF_DEFAULT_APS_OPTIONS;
    return returnValue;
}

uint16_t emberAfFillExternalBuffer(uint8_t frameControl, ClusterId clusterId, CommandId commandId, const char * format, ...)
{
    uint16_t returnValue;
    va_list argPointer;

    va_start(argPointer, format);
    returnValue =
        vFillBuffer(emAfZclBuffer, emAfZclBufferLen, frameControl, EMBER_AF_NULL_MANUFACTURER_CODE, commandId, format, argPointer);
    va_end(argPointer);
    *emAfResponseLengthPtr         = returnValue;
    emAfCommandApsFrame->clusterId = clusterId;
    emAfCommandApsFrame->options   = EMBER_AF_DEFAULT_APS_OPTIONS;
    return returnValue;
}

uint16_t emberAfFillBuffer(uint8_t * buffer, uint16_t bufferLen, uint8_t frameControl, CommandId commandId, const char * format,
                           ...)
{
    uint16_t returnValue;
    va_list argPointer;
    va_start(argPointer, format);
    returnValue = vFillBuffer(buffer, bufferLen, frameControl, EMBER_AF_NULL_MANUFACTURER_CODE, commandId, format, argPointer);
    va_end(argPointer);
    return returnValue;
}

EmberStatus emberAfSendCommandUnicastToBindingsWithCallback(EmberAfMessageSentFunction callback)
{
    return emberAfSendUnicastToBindingsWithCallback(emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer, callback);
}

EmberStatus emberAfSendCommandUnicastToBindings(void)
{
    return emberAfSendCommandUnicastToBindingsWithCallback(NULL);
}

// EmberStatus emberAfSendCommandMulticastWithCallback(GroupId multicastId, EmberAfMessageSentFunction callback)
// {
//     return emberAfSendMulticastWithCallback(multicastId, emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer, callback);
// }

// EmberStatus emberAfSendCommandMulticastWithAliasWithCallback(GroupId multicastId, EmberNodeId alias, uint8_t sequence,
//                                                              EmberAfMessageSentFunction callback)
// {
//     return emberAfSendMulticastWithAliasWithCallback(multicastId, emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer,
//     alias,
//                                                      sequence, callback);
// }

// EmberStatus emberAfSendCommandMulticast(GroupId multicastId)
// {
//     return emberAfSendCommandMulticastWithCallback(multicastId, NULL);
// }

// EmberStatus emberAfSendCommandMulticastWithAlias(GroupId multicastId, EmberNodeId alias, uint8_t sequence)
// {
//     return emberAfSendCommandMulticastWithAliasWithCallback(multicastId, alias, sequence, NULL);
// }

EmberStatus emberAfSendCommandMulticastToBindings(void)
{
    return emberAfSendMulticastToBindings(emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer);
}

// EmberStatus emberAfSendCommandUnicastWithCallback(const chip::MessageSendDestination & destination, EmberAfMessageSentFunction
// callback)
// {
//     return emberAfSendUnicastWithCallback(destination, emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer, callback);
// }

// EmberStatus emberAfSendCommandUnicast(const chip::MessageSendDestination & destination)
// {
//     return emberAfSendCommandUnicastWithCallback(destination, NULL);
// }

// EmberStatus emberAfSendCommandBroadcastWithCallback(EmberNodeId destination, EmberAfMessageSentFunction callback)
// {
//     return emberAfSendBroadcastWithCallback(destination, emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer, callback);
// }

// EmberStatus emberAfSendCommandBroadcastWithAliasWithCallback(EmberNodeId destination, EmberNodeId alias, uint8_t sequence,
//                                                              EmberAfMessageSentFunction callback)
// {
//     return emberAfSendBroadcastWithAliasWithCallback(destination, emAfCommandApsFrame, *emAfResponseLengthPtr, emAfZclBuffer,
//     alias,
//                                                      sequence, callback);
// }

// EmberStatus emberAfSendCommandBroadcastWithAlias(EmberNodeId destination, EmberNodeId alias, uint8_t sequence)
// {
//     return emberAfSendCommandBroadcastWithAliasWithCallback(destination, alias, sequence, NULL);
// }

// EmberStatus emberAfSendCommandBroadcast(EmberNodeId destination)
// {
//     return emberAfSendCommandBroadcastWithCallback(destination, NULL);
// }

// EmberStatus emberAfSendCommandInterPan(EmberPanId panId, const EmberEUI64 destinationLongId, EmberNodeId destinationShortId,
//                                        GroupId multicastId)
// {
//     return emberAfSendInterPan(panId, destinationLongId, destinationShortId, multicastId, emAfCommandApsFrame->clusterId,
//                                *emAfResponseLengthPtr, emAfZclBuffer);
// }

EmberApsFrame * emberAfGetCommandApsFrame(void)
{
    return emAfCommandApsFrame;
}

void emberAfSetCommandEndpoints(EndpointId sourceEndpoint, EndpointId destinationEndpoint)
{
    emAfCommandApsFrame->sourceEndpoint      = sourceEndpoint;
    emAfCommandApsFrame->destinationEndpoint = destinationEndpoint;
}
