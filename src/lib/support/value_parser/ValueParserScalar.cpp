/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <cctype>

#include <lib/support/BytesToHex.h>
#include <lib/support/value_parser/ValueParserScalar.h>

namespace chip {
namespace ValueParser {

bool IsSplitter(char ch)
{
    return ch == ',' || ch == ')' || ch == ']';
}

bool IsTokenEnd(char ch)
{
    return isspace(ch) || ch == '\0' || ch == ',' || ch == ')' || ch == ']';
}

const char * SkipSpaces(const char * iter)
{
    while (isspace(*iter))
    {
        iter++;
    }
    return iter;
}

CHIP_ERROR ParseValue(const char *& iter, bool & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    if (strncmp("true", iter, 4) == 0)
    {
        value = true;
        iter += 4;
        return IsTokenEnd(*iter) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (strncmp("false", iter, 5) == 0)
    {
        value = false;
        iter += 5;
        return IsTokenEnd(*iter) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ParseValue(const char *& iter, chip::CharSpan & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    const char * begin = iter;
    for (; *iter && !IsSplitter(*iter); iter++)
        ;
    value = chip::CharSpan(begin, static_cast<size_t>(iter - begin));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseValue(const char *& iter, chip::ByteSpan & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    std::vector<uint8_t> data;
    if (strncmp(iter, "hex:", 4) != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    iter += 4;
    const char * end = iter + strlen(iter);
    for (; iter + 1 < end && !IsTokenEnd(*iter); iter += 2)
    {
        uint8_t converted;
        CHIP_ERROR error = chip::Encoding::MakeU8FromAsciiHex(iter, &converted);
        if (error != CHIP_NO_ERROR)
        {
            return error;
        }
        data.push_back(converted);
    }
    if (IsTokenEnd(*iter))
    {
        if (!data.empty())
        {
            uint8_t * data_storage = new uint8_t[data.size()];
            std::copy(std::begin(data), std::end(data), data_storage);
            value = chip::ByteSpan(data_storage, data.size());
            freeFunctions.push_back([data_storage]() { delete[] data_storage; });
        }
        else
        {
            value = chip::ByteSpan();
        }
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

} // namespace ValueParser
} // namespace chip
