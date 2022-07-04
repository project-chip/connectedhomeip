/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <memory>
#include <string>

#include "IniEscaping.h"
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>

namespace chip {
namespace IniEscaping {

namespace {

constexpr size_t kEscapeChunkSize = 4; // "\x12" --> 4 chars

constexpr bool NeedsEscape(char c)
{
    return (c <= 0x20) || (c == '=') || (c == '\\') || (c >= 0x7F);
}

constexpr bool IsLowercaseHex(char c)
{
    return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'));
}

bool IsValidEscape(const std::string & s)
{
    return (s.size() >= kEscapeChunkSize) && (s[0] == '\\') && (s[1] == 'x') && IsLowercaseHex(s[2]) && IsLowercaseHex(s[3]);
}

} // namespace

std::string EscapeKey(const std::string & key)
{
    std::string escapedKey;
    escapedKey.reserve(key.size());

    for (char c : key)
    {
        // Replace spaces, non-printable chars, `=` and the escape itself with hex-escaped (C-style) characters.
        if (NeedsEscape(c))
        {
            char escaped[kEscapeChunkSize + 1] = { 0 };
            snprintf(escaped, sizeof(escaped), "\\x%02x", (static_cast<unsigned>(c) & 0xff));
            escapedKey += escaped;
        }
        else
        {
            escapedKey += c;
        }
    }

    return escapedKey;
}

std::string UnescapeKey(const std::string & key)
{
    std::string unescaped;
    unescaped.reserve(key.size());

    size_t idx       = 0;
    size_t remaining = key.size();
    while (remaining > 0)
    {
        char c = key[idx];
        if (c == '\\')
        {
            // Don't process invalid escapes.
            if (remaining < kEscapeChunkSize)
            {
                return "";
            }

            auto escapeChunk = key.substr(idx, kEscapeChunkSize);
            if (!IsValidEscape(escapeChunk))
            {
                return "";
            }

            // We validated format, now extract the last two chars as hex
            auto hexDigits   = escapeChunk.substr(2, 2);
            uint8_t charByte = 0;
            if ((chip::Encoding::HexToBytes(hexDigits.data(), 2, &charByte, 1) != 1) || !NeedsEscape(static_cast<char>(charByte)))
            {
                return "";
            }

            unescaped += static_cast<char>(charByte);
            idx += kEscapeChunkSize;
        }
        else
        {
            unescaped += c;
            idx += 1;
        }

        remaining = key.size() - idx;
    }

    return unescaped;
}

std::string StringToBase64(const std::string & value)
{
    std::unique_ptr<char[]> buffer(new char[BASE64_ENCODED_LEN(value.length())]);

    uint32_t len =
        chip::Base64Encode32(reinterpret_cast<const uint8_t *>(value.data()), static_cast<uint32_t>(value.length()), buffer.get());
    if (len == UINT32_MAX)
    {
        return "";
    }

    return std::string(buffer.get(), len);
}

std::string Base64ToString(const std::string & b64Value)
{
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(b64Value.length())]);

    uint32_t len = chip::Base64Decode32(b64Value.data(), static_cast<uint32_t>(b64Value.length()), buffer.get());
    if (len == UINT32_MAX)
    {
        return "";
    }

    return std::string(reinterpret_cast<const char *>(buffer.get()), len);
}

} // namespace IniEscaping
} // namespace chip
