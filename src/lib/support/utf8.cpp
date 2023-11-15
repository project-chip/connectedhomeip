/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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
#include "utf8.h"

namespace chip {
namespace Utf8 {

namespace {
/**
   State machine for UTF8 valid bytes

Table 3-7. Well-Formed UTF-8 Byte Sequences

Code Points       | First B  | Second B   | Third B | Fourth B
------------------+----------+------------+---------+---------
U+0000..U+007F    | 00..7F   |            |         |
U+0080..U+07FF    | C2..DF   | 80..BF     |         |
U+0800..U+0FFF    | E0       | A0..BF (A) | 80..BF  |
U+1000..U+CFFF    | E1..EC   | 80..BF     | 80..BF  |
U+D000..U+D7FF    | ED       | 80..9F (B) | 80..BF  |
U+E000..U+FFFF    | EE..EF   | 80..BF     | 80..BF  |
U+10000..U+3FFFF  | F0       | 90..BF (C) | 80..BF  | 80..BF
U+40000..U+FFFFF  | F1..F3   | 80..BF     | 80..BF  | 80..BF
U+100000..U+10FFFF| F4       | 80..8F (D) | 80..BF  | 80..BF
*/

enum class ParserState
{
    kFirstByte,
    kSecondByte_A,
    kSecondByte_B,
    kSecondByte_C,
    kSecondByte_D,
    kExtraOneByte,    // 0x80 .. 0xBF  once
    kExtraTwoBytes,   // 0x80 .. 0xBF  twice
    kExtraThreeBytes, // 0x80 .. 0xBF  three times
    //
    kInvalid, // some error
};

ParserState NextState(ParserState state, uint8_t value)
{
    switch (state)
    {
    case ParserState::kFirstByte:
        if (value <= 0x7F)
        {
            return ParserState::kFirstByte;
        }
        else if ((value >= 0xC2) && (value <= 0xDF))
        {
            return ParserState::kExtraOneByte;
        }
        else if (value == 0xE0)
        {
            return ParserState::kSecondByte_A;
        }
        else if ((value >= 0xE1) && (value <= 0xEC))
        {
            return ParserState::kExtraTwoBytes;
        }
        else if (value == 0xED)
        {
            return ParserState::kSecondByte_B;
        }
        else if ((value >= 0xEE) && (value <= 0xEF))
        {
            return ParserState::kExtraTwoBytes;
        }
        else if (value == 0xF0)
        {
            return ParserState::kSecondByte_C;
        }
        else if ((value >= 0xF1) && (value <= 0xF3))
        {
            return ParserState::kExtraThreeBytes;
        }
        else if (value == 0xF4)
        {
            return ParserState::kSecondByte_D;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kSecondByte_A:
        if (value >= 0xA0 && value <= 0xBF)
        {
            return ParserState::kExtraOneByte;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kSecondByte_B:
        if (value >= 0x80 && value <= 0x9F)
        {
            return ParserState::kExtraOneByte;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kSecondByte_C:
        if (value >= 0x90 && value <= 0xBF)
        {
            return ParserState::kExtraTwoBytes;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kSecondByte_D:
        if (value >= 0x80 && value <= 0x8F)
        {
            return ParserState::kExtraTwoBytes;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kExtraOneByte:
        if (value >= 0x80 && value <= 0xBF)
        {
            return ParserState::kFirstByte;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kExtraTwoBytes:
        if (value >= 0x80 && value <= 0xBF)
        {
            return ParserState::kExtraOneByte;
        }
        else
        {
            return ParserState::kInvalid;
        }
    case ParserState::kExtraThreeBytes:
        if (value >= 0x80 && value <= 0xBF)
        {
            return ParserState::kExtraTwoBytes;
        }
        else
        {
            return ParserState::kInvalid;
        }
    default:
        return ParserState::kInvalid;
    }
}

} // namespace

bool IsValid(CharSpan span)
{
    ParserState state = ParserState::kFirstByte;

    const char * data    = span.data();
    const size_t kLength = span.size();

    // Every byte should be valid
    for (size_t i = 0; i < kLength; i++)
    {
        state = NextState(state, static_cast<uint8_t>(data[i]));

        if (state == ParserState::kInvalid)
        {
            return false;
        }
    }

    // finally no continuation should be expected
    return state == ParserState::kFirstByte;
}

} // namespace Utf8
} // namespace chip
