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
#include <lib/support/CHIPMemString.h>
#include <tracing/esp32_diagnostics/DiagnosticEntry.h>
using namespace chip::TLV;

namespace chip {
namespace Tracing {
namespace Diagnostics {

CHIP_ERROR WriteString(TLVWriter & writer, const DiagTag tag, const char * string)
{
    char buffer[kMaxStringValueSize + 1];
    Platform::CopyString(buffer, string);
    return writer.PutString(chip::TLV::ContextTag(tag), buffer);
}

CHIP_ERROR Encode(TLVWriter & writer, const DiagnosticEntry & entry)
{
    TLVType DiagnosticOuterContainer = kTLVType_NotSpecified;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, DiagnosticOuterContainer));
    ReturnErrorOnFailure(writer.Put(ContextTag(DiagTag::kTimestamp), entry.timestamps_ms_since_boot));
    ReturnErrorOnFailure(WriteString(writer, DiagTag::kLabel, entry.label));
    switch (entry.type)
    {
    case ValueType::kInvalidType:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case ValueType::kCharString:
        ReturnErrorOnFailure(WriteString(writer, DiagTag::kValue, entry.stringValue));
        break;
    case ValueType::kUnsignedInteger:
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::kValue), entry.uintValue));
        break;
    case ValueType::kSignedInteger:
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::kValue), entry.intValue));
        break;
    }
    ReturnErrorOnFailure(writer.EndContainer(DiagnosticOuterContainer));
    ReturnErrorOnFailure(writer.Finalize());
    ChipLogDetail(DeviceLayer, "Diagnostic Value written to storage successfully. label: %s\n", entry.label);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Decode(TLVReader & reader, DiagnosticEntry & entry)
{
    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(ContextTag(DiagTag::kTimestamp)));
    ReturnErrorOnFailure(reader.Get(entry.timestamps_ms_since_boot));
    ReturnErrorOnFailure(reader.Next(ContextTag(DiagTag::kLabel)));
    uint32_t labelSize = reader.GetLength();
    if (labelSize > kMaxStringValueSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    ReturnErrorOnFailure(reader.GetString(entry.label, kMaxStringValueSize + 1));
    ReturnErrorOnFailure(reader.Next(ContextTag(DiagTag::kValue)));
    switch (reader.GetType())
    {
    case kTLVType_NotSpecified:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case kTLVType_UTF8String: {
        uint32_t valueSize = reader.GetLength();
        if (valueSize > kMaxStringValueSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        ReturnErrorOnFailure(reader.GetString(entry.stringValue, kMaxStringValueSize + 1));
        entry.type = ValueType::kCharString;
        break;
    }
    case kTLVType_UnsignedInteger:
        ReturnErrorOnFailure(reader.Get(entry.uintValue));
        entry.type = ValueType::kUnsignedInteger;
        break;
    case kTLVType_SignedInteger:
        ReturnErrorOnFailure(reader.Get(entry.intValue));
        entry.type = ValueType::kSignedInteger;
        break;
    default:
        ChipLogError(DeviceLayer, "Invalid value type encountered while decoding DiagnosticEntry");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return reader.ExitContainer(containerType);
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
