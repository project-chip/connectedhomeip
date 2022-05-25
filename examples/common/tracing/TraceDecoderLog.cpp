/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "TraceDecoderLog.h"

#include "decoder/TraceDecoderToCertificateString.h"
#include "decoder/TraceDecoderToHexString.h"

#include <lib/support/SafeInt.h>

namespace chip {
namespace trace {

constexpr uint8_t kSpacePerIndent = 4;
uint8_t gIndentLevel              = 0;

void ENFORCE_FORMAT(1, 2) LogFormatted(const char * format, ...)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {};

    uint8_t indentation = gIndentLevel * kSpacePerIndent;
    snprintf(buffer, sizeof(buffer), "%*s", indentation, "");

    va_list args;
    va_start(args, format);
    vsnprintf(&buffer[indentation], sizeof(buffer) - indentation, format, args);
    va_end(args);

    ChipLogDetail(DataManagement, "%s", buffer);
}

void Log(const char * name, chip::ByteSpan & data)
{
    if (data.empty())
    {
        return;
    }

    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    chip::CanCastTo<uint16_t>(data.size())
        ? LogFormatted("%s (%u) = %s", name, static_cast<uint16_t>(data.size()),
                       chip::trace::ToHexString(data, buffer, sizeof(buffer)))
        : LogFormatted("%s (>UINT16_MAX) = %s", name, chip::trace::ToHexString(data, buffer, sizeof(buffer)));
}

void Log(const char * name, chip::CharSpan & data)
{
    if (data.empty())
    {
        return;
    }

    chip::CanCastTo<uint16_t>(data.size())
        ? LogFormatted("%s (%u) = %.*s", name, static_cast<uint16_t>(data.size()), static_cast<int>(data.size()), data.data())
        : LogFormatted("%s (>UINT16_MAX) = %.*s", name, static_cast<int>(data.size()), data.data());
}

void Log(const char * name, uint16_t value)
{
    LogFormatted("%s = %u", name, value);
}

void Log(const char * name, uint32_t value)
{
    LogFormatted("%s = %u", name, value);
}

void Log(const char * name, const char * value)
{
    LogFormatted("%s = %s", name, value);
}

void Log(const char * name)
{
    LogFormatted("%s", name);
}

void LogAsHex(const char * name, uint16_t value)
{
    LogFormatted("%s = 0x%04x", name, value);
}

void LogAsHex(const char * name, uint64_t value)
{
    LogFormatted("%s = " ChipLogFormatX64, name, ChipLogValueX64(value));
}

void LogCertificate(const char * name, const chip::ByteSpan & data)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {};

    chip::CanCastTo<uint16_t>(data.size()) ? LogFormatted("%s (%u) =", name, static_cast<uint16_t>(data.size()))
                                           : LogFormatted("%s (>UINT16_MAX) =", name);
    LogFormatted("{\n%s", chip::trace::ToCertificateString(data, buffer, sizeof(buffer)));
    LogFormatted("}");
    LogFormatted("");
}

void LogCertificateRequest(const char * name, const chip::ByteSpan & data)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {};

    chip::CanCastTo<uint16_t>(data.size()) ? LogFormatted("%s (%u) =", name, static_cast<uint16_t>(data.size()))
                                           : LogFormatted("%s (>UINT16_MAX) =", name);
    LogFormatted("{\n%s", chip::trace::ToCertificateRequestString(data, buffer, sizeof(buffer)));
    LogFormatted("}");
    LogFormatted("");
}

void IncreaseLogIndent(const char * name, uint8_t flags)
{
    LogFormatted("%s (0x%02x) =", name, flags);
    LogFormatted("{");
    gIndentLevel++;
}

void IncreaseLogIndent(const char * name)
{
    LogFormatted("%s =", name);
    LogFormatted("{");
    gIndentLevel++;
}

void DecreaseLogIndent()
{
    gIndentLevel--;
    LogFormatted("}");
    LogFormatted("");
}

} // namespace trace
} // namespace chip
