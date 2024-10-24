/*
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
#include <app/codegen-data-model-provider/EmberAttributeDataBuffer.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/interaction_model/StatusCode.h>

#include <limits>

namespace chip {
namespace app {
namespace Ember {

namespace {

/// Maximum length of a string, inclusive
///
/// the max size value (0xFF and 0xFFFF) is reserved for NULL representation so
/// it is not available
constexpr uint32_t MaxLength(EmberAttributeDataBuffer::PascalStringType s)
{
    if (s == EmberAttributeDataBuffer::PascalStringType::kShort)
    {
        return std::numeric_limits<uint8_t>::max() - 1;
    }
    // EmberAttributeBuffer::PascalStringType::kLong:
    return std::numeric_limits<uint16_t>::max() - 1;
}

struct UnsignedDecodeInfo
{
    unsigned byteCount;
    uint64_t maxValue;

    constexpr UnsignedDecodeInfo(unsigned bytes) : byteCount(bytes), maxValue(NumericLimits::MaxUnsignedValue(bytes)) {}
};

constexpr UnsignedDecodeInfo GetUnsignedDecodeInfo(EmberAfAttributeType type)
{

    switch (type)
    {
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        return UnsignedDecodeInfo(1);
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        return UnsignedDecodeInfo(2);
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
        return UnsignedDecodeInfo(3);
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        return UnsignedDecodeInfo(4);
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
        return UnsignedDecodeInfo(5);
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
        return UnsignedDecodeInfo(6);
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
        return UnsignedDecodeInfo(7);
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        return UnsignedDecodeInfo(8);
    }
    chipDie();
}

struct SignedDecodeInfo
{
    unsigned byteCount;
    int64_t minValue;
    int64_t maxValue;

    constexpr SignedDecodeInfo(unsigned bytes) :
        byteCount(bytes), minValue(NumericLimits::MinSignedValue(bytes)), maxValue(NumericLimits::MaxSignedValue(bytes))
    {}
};

constexpr SignedDecodeInfo GetSignedDecodeInfo(EmberAfAttributeType type)
{

    switch (type)
    {
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        return SignedDecodeInfo(1);
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        return SignedDecodeInfo(2);
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
        return SignedDecodeInfo(3);
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        return SignedDecodeInfo(4);
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
        return SignedDecodeInfo(5);
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
        return SignedDecodeInfo(6);
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
        return SignedDecodeInfo(7);
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        return SignedDecodeInfo(8);
    }
    chipDie();
}

} // namespace

CHIP_ERROR EmberAttributeDataBuffer::DecodeUnsignedInteger(chip::TLV::TLVReader & reader, EndianWriter & writer)
{
    UnsignedDecodeInfo info = GetUnsignedDecodeInfo(mAttributeType);

    // Any size of integer can be read by TLV getting 64-bit integers
    uint64_t value;

    if (reader.GetType() == TLV::kTLVType_Null)
    {
        // we know mIsNullable due to the check at the top of ::Decode
        value = NumericLimits::UnsignedMaxValueToNullValue(info.maxValue);
    }
    else
    {
        ReturnErrorOnFailure(reader.Get(value));

        bool valid =
            // Value is in [0, max] RANGE
            (value <= info.maxValue)
            // Nullable values reserve a specific value to mean NULL
            && !(mIsNullable && (value == NumericLimits::UnsignedMaxValueToNullValue(info.maxValue)));

        VerifyOrReturnError(valid, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    writer.EndianPut(value, info.byteCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmberAttributeDataBuffer::DecodeSignedInteger(chip::TLV::TLVReader & reader, EndianWriter & writer)
{
    SignedDecodeInfo info = GetSignedDecodeInfo(mAttributeType);

    // Any size of integer can be read by TLV getting 64-bit integers
    int64_t value;

    if (reader.GetType() == TLV::kTLVType_Null)
    {
        // we know mIsNullable due to the check at the top of ::Decode
        value = NumericLimits::SignedMinValueToNullValue(info.minValue);
    }
    else
    {
        ReturnErrorOnFailure(reader.Get(value));

        bool valid =
            // Value is in [min, max] RANGE
            ((value >= info.minValue) && (value <= info.maxValue))
            // Nullable values reserve a specific value to mean NULL
            && !(mIsNullable && (value == NumericLimits::SignedMinValueToNullValue(info.minValue)));

        VerifyOrReturnError(valid, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    writer.EndianPutSigned(value, info.byteCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmberAttributeDataBuffer::DecodeAsString(chip::TLV::TLVReader & reader, PascalStringType stringType,
                                                    TLV::TLVType tlvType, EndianWriter & writer)
{
    // Handle null first, then the actual data
    if (reader.GetType() == TLV::kTLVType_Null)
    {
        // we know mIsNullable due to the check at the top of ::Decode
        switch (stringType)
        {
        case PascalStringType::kShort:
            writer.Put8(NumericAttributeTraits<uint8_t>::kNullValue);
            break;
        case PascalStringType::kLong:
            writer.Put16(NumericAttributeTraits<uint16_t>::kNullValue);
            break;
        }
        return CHIP_NO_ERROR;
    }

    const uint32_t stringLength = reader.GetLength();

    VerifyOrReturnError(reader.GetType() == tlvType, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(stringLength <= MaxLength(stringType), CHIP_ERROR_INVALID_ARGUMENT);

    // Size is a prefix, where 0xFF/0xFFFF is the null marker (if applicable)
    switch (stringType)
    {
    case PascalStringType::kShort:
        writer.Put8(static_cast<uint8_t>(stringLength));
        break;
    case PascalStringType::kLong:
        writer.Put16(static_cast<uint16_t>(stringLength));
        break;
    }

    // data copy
    const uint8_t * tlvData;
    ReturnErrorOnFailure(reader.GetDataPtr(tlvData));
    writer.Put(tlvData, stringLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EmberAttributeDataBuffer::Decode(chip::TLV::TLVReader & reader)
{
    // all methods below assume that nullable setting matches (this is to reduce code size
    // even though clarity suffers)
    VerifyOrReturnError(mIsNullable || reader.GetType() != TLV::kTLVType_Null, CHIP_ERROR_WRONG_TLV_TYPE);

    EndianWriter endianWriter(mDataBuffer.data(), mDataBuffer.size());

    switch (mAttributeType)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        // Boolean values:
        //   0x00 is FALSE
        //   0x01 is TRUE
        //   0xFF is NULL
        if (reader.GetType() == TLV::kTLVType_Null)
        {
            // we know mIsNullable due to the check at the top of ::Decode
            endianWriter.Put8(NumericAttributeTraits<bool>::kNullValue);
        }
        else
        {
            bool value;
            ReturnErrorOnFailure(reader.Get(value));
            endianWriter.Put8(value ? 1 : 0);
        }
        break;
    case ZCL_INT8U_ATTRIBUTE_TYPE:  // Unsigned 8-bit integer
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        ReturnErrorOnFailure(DecodeUnsignedInteger(reader, endianWriter));
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:  // Signed 8-bit integer
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        ReturnErrorOnFailure(DecodeSignedInteger(reader, endianWriter));
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE: { // 32-bit float
        float value;
        if (reader.GetType() == TLV::kTLVType_Null)
        {
            // we know mIsNullable due to the check at the top of ::Decode
            NumericAttributeTraits<float>::SetNull(value);
        }
        else
        {

            ReturnErrorOnFailure(reader.Get(value));
        }
        endianWriter.Put(&value, sizeof(value));
        break;
    }
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: { // 64-bit float
        double value;
        if (reader.GetType() == TLV::kTLVType_Null)
        {
            // we know mIsNullable due to the check at the top of ::Decode
            NumericAttributeTraits<double>::SetNull(value);
        }
        else
        {
            ReturnErrorOnFailure(reader.Get(value));
        }
        endianWriter.Put(&value, sizeof(value));
        break;
    }
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Char string
        ReturnErrorOnFailure(DecodeAsString(reader, PascalStringType::kShort, TLV::kTLVType_UTF8String, endianWriter));
        break;
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        ReturnErrorOnFailure(DecodeAsString(reader, PascalStringType::kLong, TLV::kTLVType_UTF8String, endianWriter));
        break;
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
        ReturnErrorOnFailure(DecodeAsString(reader, PascalStringType::kShort, TLV::kTLVType_ByteString, endianWriter));
        break;
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        ReturnErrorOnFailure(DecodeAsString(reader, PascalStringType::kLong, TLV::kTLVType_ByteString, endianWriter));
        break;
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", mAttributeType);
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }

    size_t written;
    if (!endianWriter.Fit(written))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    mDataBuffer.reduce_size(written);
    return CHIP_NO_ERROR;
}

} // namespace Ember
} // namespace app
} // namespace chip
