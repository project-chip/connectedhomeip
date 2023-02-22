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

#include <app/util/af.h>
#include <app/util/config.h>
#include <app/util/util.h>
#include <lib/support/TypeTraits.h>

using namespace chip;

//------------------------------------------------------------------------------

// these variables are for storing responses that are created by zcl-utils
// in functions called from emberIncomingMsgHandler. The response is sent
// from emberAfTick (meant to be called immediately after emberTick).
// There is only space for one response as each call to emberTick will
// only result in a single call to emberIncomingMsgHandler. If the device
// receives multiple ZCL messages, the stack will queue these and hand
// these to the application via emberIncomingMsgHandler one at a time.
uint8_t appResponseData[EMBER_AF_RESPONSE_BUFFER_LEN];
uint16_t appResponseLength;

//------------------------------------------------------------------------------
// Utilities for adding bytes to the response buffer: appResponseData. These
// functions take care of incrementing appResponseLength.

uint8_t * emberAfPutInt8uInResp(uint8_t value)
{
    // emberAfDebugPrint("try %x max %x\r\n", appResponseLength, EMBER_AF_RESPONSE_BUFFER_LEN);
    if (appResponseLength < EMBER_AF_RESPONSE_BUFFER_LEN)
    {
        // emberAfDebugPrint("put %x at spot %x\r\n", value, appResponseLength);
        appResponseData[appResponseLength] = value;
        appResponseLength++;
        return &appResponseData[appResponseLength - 1];
    }

    return nullptr;
}

uint16_t * emberAfPutInt16uInResp(uint16_t value)
{
    uint8_t * low  = emberAfPutInt8uInResp(EMBER_LOW_BYTE(value));
    uint8_t * high = emberAfPutInt8uInResp(EMBER_HIGH_BYTE(value));

    if (low && high)
    {
        return (uint16_t *) low;
    }

    return nullptr;
}

uint8_t * emberAfPutBlockInResp(const uint8_t * data, uint16_t length)
{
    if ((appResponseLength + length) < EMBER_AF_RESPONSE_BUFFER_LEN)
    {
        memmove(appResponseData + appResponseLength, data, length);
        appResponseLength = static_cast<uint16_t>(appResponseLength + length);
        return &appResponseData[appResponseLength - length];
    }

    return nullptr;
}

uint8_t * emberAfPutStringInResp(const uint8_t * buffer)
{
    uint8_t length = emberAfStringLength(buffer);
    return emberAfPutBlockInResp(buffer, static_cast<uint16_t>(length + 1));
}

void emberAfPutInt16sInResp(int16_t value)
{
    emberAfPutInt16uInResp(static_cast<uint16_t>(value));
}

// ------------------------------------
// Utilities for reading from RAM buffers (reading from incoming message
// buffer)
// ------------------------------------

// retrieves an uint64_t which contains between 1 and 8 bytes of relevant data
// depending on number of bytes requested.
uint64_t emberAfGetInt(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen, uint8_t bytes)
{
    uint64_t result = 0;
    uint8_t i       = bytes;
    if ((currentIndex + bytes) > msgLen)
    {
        emberAfDebugPrintln("GetInt, %x bytes short", bytes);
        emberAfDebugFlush();
        return 0;
    }
    while (i > 0)
    {
        result = (result << 8) + message[(currentIndex + i) - 1];
        i--;
    }
    return result;
}

uint16_t emberAfGetInt16u(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen)
{
    return static_cast<uint16_t>(emberAfGetInt(message, currentIndex, msgLen, 2));
}

uint8_t emberAfStringLength(const uint8_t * buffer)
{
    // The first byte specifies the length of the string.  A length of 0xFF means
    // the string is invalid and there is no character data.
    return (buffer[0] == 0xFF ? 0 : buffer[0]);
}

uint16_t emberAfLongStringLength(const uint8_t * buffer)
{
    // The first two bytes specify the length of the long string.  A length of
    // 0xFFFF means the string is invalid and there is no character data.
    uint16_t length = emberAfGetInt16u(buffer, 0, 2);
    return (length == 0xFFFF ? 0 : length);
}
