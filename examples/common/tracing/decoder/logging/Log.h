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

#pragma once

#include <lib/support/Span.h>

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace trace {
namespace logging {

class ScopedLogIndent
{
public:
    ScopedLogIndent(const char * name);
    ~ScopedLogIndent();
};

class ScopedLogIndentWithFlags
{
public:
    ScopedLogIndentWithFlags(const char * name, uint8_t flags);
    ~ScopedLogIndentWithFlags();
};

class ScopedLogIndentWithSize
{
public:
    ScopedLogIndentWithSize(const char * name, size_t size);
    ~ScopedLogIndentWithSize();
};

void Log(const char * name, chip::ByteSpan & data);

void Log(const char * name, chip::CharSpan & data);

void Log(const char * name, uint16_t value);

void Log(const char * name, uint32_t value);

void Log(const char * name);

void Log(const char * name, const char * value);

void LogAsHex(const char * name, uint8_t value);

void LogAsHex(const char * name, uint16_t value);

void LogAsHex(const char * name, uint64_t value);

void LogCertificate(const char * name, const chip::ByteSpan & data);

void LogCertificateRequest(const char * name, const chip::ByteSpan & data);

} // namespace logging
} // namespace trace
} // namespace chip
