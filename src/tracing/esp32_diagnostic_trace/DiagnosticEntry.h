/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once
#include <lib/core/TLVCircularBuffer.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

static constexpr uint32_t kMaxStringValueSize = 64;

enum class ValueType
{
    kInvalidType = 0,
    kCharString,
    kUnsignedInteger,
    kSignedInteger
};

struct DiagnosticEntry
{
    // mutable label because modified in decode
    char label[kMaxStringValueSize + 1];
    union
    {
        // mutable stringValue because modified in decode
        char stringValue[kMaxStringValueSize + 1];
        uint32_t uintValue;
        int32_t intValue;
    };
    ValueType type;
    uint32_t timestamps_ms_since_boot;
};

enum class DiagTag : uint8_t
{
    kTimestamp = 0,
    kLabel     = 1,
    kValue     = 2,
};

CHIP_ERROR WriteString(chip::TLV::TLVWriter & writer, const DiagTag tag, const char * str);
CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, const DiagnosticEntry & entry);
CHIP_ERROR Decode(chip::TLV::TLVReader & reader, DiagnosticEntry & entry);

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
