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

#include "Log.h"

#include "ToCertificateString.h"

#include <lib/support/BytesToHex.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace trace {
namespace logging {

namespace {

constexpr uint8_t kSpacePerIndent = 4;
uint8_t gIndentLevel              = 0;

void ENFORCE_FORMAT(1, 2) LogFormatted(const char * format, ...)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE + 1] = {};

    int indentation = gIndentLevel * kSpacePerIndent;
    snprintf(buffer, sizeof(buffer), "%*s", indentation, "");

    va_list args;
    va_start(args, format);
    vsnprintf(&buffer[indentation], sizeof(buffer) - static_cast<size_t>(indentation), format, args);
    va_end(args);

    ChipLogDetail(DataManagement, "%s", buffer);
}

const char * ToHexString(chip::ByteSpan source, MutableCharSpan destination)
{
    memset(destination.data(), '\0', destination.size());

    if (source.size() == 0)
    {
        return destination.data();
    }

    CHIP_ERROR err =
        Encoding::BytesToHex(source.data(), source.size(), destination.data(), destination.size(), Encoding::HexFlags::kUppercase);
    if (CHIP_NO_ERROR != err)
    {
        LogErrorOnFailure(err);
        return destination.data();
    }

    return destination.data();
}

} // namespace

void Log(const char * name, ByteSpan & data)
{
    if (data.empty())
    {
        return;
    }

    // If the size of the data is larger than half of the maximum size for a log message (minus 1 for null-termination),
    // reduce the size of the data to fit within this limit.
    // The limit is half the size of the message because we will be logging a hex representation of that data, at 2 chars per byte.
    if (data.size() > (CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE / 2) - 1)
    {
        data.reduce_size((CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE / 2) - 1);
    }

    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = { 0 };
    chip::MutableCharSpan destination(buffer);

    // Check if the size of the data can be cast to uint16_t.
    // If yes, log the name and size of the data followed by its hexadecimal string representation.
    // If no, log the name and a marker indicating the size exceeds UINT16_MAX, followed by the data's hexadecimal string.
    CanCastTo<uint16_t>(data.size())
        ? LogFormatted("%s (%u) = %s", name, static_cast<uint16_t>(data.size()), ToHexString(data, destination))
        : LogFormatted("%s (>UINT16_MAX) = %s", name, ToHexString(data, destination));
}

void Log(const char * name, CharSpan & data)
{
    if (data.empty())
    {
        return;
    }

    CanCastTo<uint16_t>(data.size())
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

void LogAsHex(const char * name, uint8_t value)
{
    LogFormatted("%s = 0x%02x", name, value);
}

void LogAsHex(const char * name, uint16_t value)
{
    LogFormatted("%s = 0x%04x", name, value);
}

void LogAsHex(const char * name, uint64_t value)
{
    LogFormatted("%s = " ChipLogFormatX64, name, ChipLogValueX64(value));
}

void LogCertificate(const char * name, const ByteSpan & data)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {};
    chip::MutableCharSpan destination(buffer);

    CanCastTo<uint16_t>(data.size()) ? LogFormatted("%s (%u) =", name, static_cast<uint16_t>(data.size()))
                                     : LogFormatted("%s (>UINT16_MAX) =", name);
    LogFormatted("{\n%s", ToCertificateString(data, destination));
    LogFormatted("}");
    LogFormatted(" ");
}

void LogCertificateRequest(const char * name, const ByteSpan & data)
{
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {};
    chip::MutableCharSpan destination(buffer);

    CanCastTo<uint16_t>(data.size()) ? LogFormatted("%s (%u) =", name, static_cast<uint16_t>(data.size()))
                                     : LogFormatted("%s (>UINT16_MAX) =", name);
    LogFormatted("{\n%s", ToCertificateRequestString(data, destination));
    LogFormatted("}");
    LogFormatted(" ");
}

void IncreaseLogIndent()
{
    LogFormatted("{");
    gIndentLevel++;
}

void DecreaseLogIndent()
{
    gIndentLevel--;
    LogFormatted("}");

    if (gIndentLevel == 0)
    {
        LogFormatted(" ");
    }
}

ScopedLogIndent::ScopedLogIndent(const char * name)
{
    LogFormatted("%s =", name);
    IncreaseLogIndent();
}

ScopedLogIndent::~ScopedLogIndent()
{
    DecreaseLogIndent();
}

ScopedLogIndentWithFlags::ScopedLogIndentWithFlags(const char * name, uint8_t flags)
{
    LogFormatted("%s (0x%02x) =", name, flags);
    IncreaseLogIndent();
}

ScopedLogIndentWithFlags::~ScopedLogIndentWithFlags()
{
    DecreaseLogIndent();
}

ScopedLogIndentWithSize::ScopedLogIndentWithSize(const char * name, size_t size)
{
    CanCastTo<uint16_t>(size) ? LogFormatted("%s (%u bytes) =", name, static_cast<uint16_t>(size))
                              : LogFormatted("%s (>UINT16_MAX) =", name);
    IncreaseLogIndent();
}

ScopedLogIndentWithSize::~ScopedLogIndentWithSize()
{
    DecreaseLogIndent();
}

} // namespace logging
} // namespace trace
} // namespace chip
