/*
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

#include "Hex.h"

namespace chip {

uint8_t HexDigitToInt(char c)
{
    if ('0' <= c && c <= '9')
    {
        return static_cast<uint8_t>(c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        return static_cast<uint8_t>(0x0a + c - 'a');
    }
    else if ('A' <= c && c <= 'F')
    {
        return static_cast<uint8_t>(0x0a + c - 'A');
    }

    return kInvalidDigitValue;
}

} // namespace chip
