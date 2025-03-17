/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

static constexpr size_t kMaxStringValueSize = 128;

enum class ValueType
{
    kCharString,
    kUnsignedInteger,
    kSignedInteger
};

struct DiagnosticEntry
{
    char * label;
    union
    {
        char * stringValue;
        uint32_t uintValue;
        int32_t intValue;
    };
    ValueType type;
    uint32_t timestamp;
};

enum class DiagTag : uint8_t
{
    TIMESTAMP = 1,
    LABEL     = 2,
    VALUE     = 3
};

CHIP_ERROR Encode(chip::TLV::CircularTLVWriter & writer, const DiagnosticEntry & entry);
CHIP_ERROR Decode(chip::TLV::CircularTLVReader & reader, DiagnosticEntry & entry);

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
