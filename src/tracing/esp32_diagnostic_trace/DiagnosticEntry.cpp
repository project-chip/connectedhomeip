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
#include <tracing/esp32_diagnostic_trace/DiagnosticEntry.h>
using namespace chip::TLV;

namespace chip {
namespace Tracing {
namespace Diagnostics {

CHIP_ERROR Encode(TLVWriter & writer, const DiagnosticEntry & entry)
{
    TLVType DiagnosticOuterContainer = kTLVType_NotSpecified;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, DiagnosticOuterContainer));

    // Write timestamp
    ReturnErrorOnFailure(writer.Put(ContextTag(DiagTag::TIMESTAMP), entry.timestamp));

    // Write label
    if (entry.label != nullptr)
    {
        uint32_t labelSize = strlen(entry.label) > kMaxStringValueSize ? kMaxStringValueSize : strlen(entry.label);
        if (labelSize >= kMaxStringValueSize)
        {
            char labelBuffer[kMaxStringValueSize + 1];
            memcpy(labelBuffer, entry.label, kMaxStringValueSize);
            labelBuffer[kMaxStringValueSize] = '\0';
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), labelBuffer));
        }
        else
        {
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), entry.label, labelSize));
        }
    }

    // Write value based on type
    switch (entry.type)
    {
    case ValueType::kInvalidType:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case ValueType::kCharString:
        if (entry.stringValue != nullptr)
        {
            uint32_t valueSize = strlen(entry.stringValue) > kMaxStringValueSize ? kMaxStringValueSize : strlen(entry.stringValue);
            if (valueSize >= kMaxStringValueSize)
            {
                char valueBuffer[kMaxStringValueSize + 1];
                memcpy(valueBuffer, entry.stringValue, kMaxStringValueSize);
                valueBuffer[kMaxStringValueSize] = '\0';
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), valueBuffer));
            }
            else
            {
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), entry.stringValue, valueSize));
            }
        }
        break;

    case ValueType::kUnsignedInteger:
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::VALUE), entry.uintValue));
        break;

    case ValueType::kSignedInteger:
        ReturnErrorOnFailure(writer.Put(chip::TLV::ContextTag(DiagTag::VALUE), entry.intValue));
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

    // Read timestamp
    ReturnErrorOnFailure(reader.Next(ContextTag(DiagTag::TIMESTAMP)));
    ReturnErrorOnFailure(reader.Get(entry.timestamp));

    // Read label
    ReturnErrorOnFailure(reader.Next(ContextTag(DiagTag::LABEL)));
    uint32_t labelSize = reader.GetLength();
    if (labelSize > kMaxStringValueSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    ReturnErrorOnFailure(reader.GetString(entry.label, kMaxStringValueSize + 1));

    // Read value
    ReturnErrorOnFailure(reader.Next());
    switch (entry.type)
    {
    case ValueType::kInvalidType:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case ValueType::kCharString: {
        uint32_t valueSize = reader.GetLength();
        if (valueSize > kMaxStringValueSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        ReturnErrorOnFailure(reader.GetString(entry.stringValue, kMaxStringValueSize + 1));
        break;
    }

    case ValueType::kUnsignedInteger:
        ReturnErrorOnFailure(reader.Get(entry.uintValue));
        break;

    case ValueType::kSignedInteger:
        ReturnErrorOnFailure(reader.Get(entry.intValue));
        break;
    }

    ReturnErrorOnFailure(reader.ExitContainer(containerType));
    return CHIP_NO_ERROR;
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
