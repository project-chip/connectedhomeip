#include <tracing/esp32_diagnostic_trace/DiagnosticEntry.h>
using namespace chip::TLV;

namespace chip {
namespace Tracing {
namespace Diagnostics {

CHIP_ERROR Encode(CircularTLVWriter & writer, const DiagnosticEntry & entry)
{
    TLVType DiagnosticOuterContainer = kTLVType_NotSpecified;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, DiagnosticOuterContainer));

    // Write timestamp
    ReturnErrorOnFailure(writer.Put(ContextTag(DiagTag::TIMESTAMP), entry.timestamp));

    // Write label
    if (entry.label != nullptr)
    {
        if (strlen(entry.label) > kMaxStringValueSize)
        {
            char labelBuffer[kMaxStringValueSize + 1];
            memcpy(labelBuffer, entry.label, kMaxStringValueSize);
            labelBuffer[kMaxStringValueSize] = '\0';
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), labelBuffer));
        }
        else
        {
            ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::LABEL), entry.label));
        }
    }

    // Write value based on type
    switch (entry.type)
    {
    case ValueType::kCharString:
        if (entry.stringValue != nullptr)
        {
            if (strlen(entry.stringValue) > kMaxStringValueSize)
            {
                char valueBuffer[kMaxStringValueSize + 1];
                memcpy(valueBuffer, entry.stringValue, kMaxStringValueSize);
                valueBuffer[kMaxStringValueSize] = '\0';
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), valueBuffer));
            }
            else
            {
                ReturnErrorOnFailure(writer.PutString(chip::TLV::ContextTag(DiagTag::VALUE), entry.stringValue));
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
    ChipLogProgress(DeviceLayer, "Diagnostic Value written to storage successfully. label: %s\n", entry.label);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Decode(CircularTLVReader & reader, DiagnosticEntry & entry)
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
