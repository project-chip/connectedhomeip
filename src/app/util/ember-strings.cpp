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
#include <app/util/ember-strings.h>

#include <lib/core/CHIPEncoding.h>

using namespace chip;

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
    uint16_t length = Encoding::LittleEndian::Get16(buffer);
    return (length == 0xFFFF ? 0 : length);
}

void emberAfCopyString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = 0; // Zero out the length of string
    }
    else if (src[0] == 0xFF)
    {
        dest[0] = src[0];
    }
    else
    {
        uint8_t length = emberAfStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 1, src + 1, length);
        dest[0] = length;
    }
}

void emberAfCopyLongString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = dest[1] = 0; // Zero out the length of string
    }
    else if ((src[0] == 0xFF) && (src[1] == 0xFF))
    {
        dest[0] = 0xFF;
        dest[1] = 0xFF;
    }
    else
    {
        uint16_t length = emberAfLongStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 2, src + 2, length);
        Encoding::LittleEndian::Put16(dest, length);
    }
}
