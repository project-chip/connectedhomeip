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
#include <pw_unit_test/framework.h>

#include <cmath>

#include <data-model-providers/codegen/EmberAttributeDataBuffer.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <app/data-model/Encode.h>
#include <app/data-model/Nullable.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <limits>
#include <optional>

using namespace chip;
using namespace chip::app;

namespace {

/// encodes a simple value in a TLV buffer
class TLVEncodedValue
{
public:
    TLVEncodedValue()  = default;
    ~TLVEncodedValue() = default;

    template <typename T>
    TLV::TLVReader EncodeValue(const T & value)
    {
        const auto kTag = TLV::ContextTag(AttributeDataIB::Tag::kData);

        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));

        TLV::TLVType outer;

        VerifyOrDie(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer) == CHIP_NO_ERROR);
        VerifyOrDie(DataModel::Encode(writer, kTag, value) == CHIP_NO_ERROR);
        VerifyOrDie(writer.EndContainer(outer) == CHIP_NO_ERROR);

        VerifyOrDie(writer.Finalize() == CHIP_NO_ERROR);
        size_t fill = writer.GetLengthWritten();

        TLV::TLVReader reader;
        reader.Init(mBuffer, fill);
        VerifyOrDie(reader.Next() == CHIP_NO_ERROR);
        VerifyOrDie(reader.GetTag() == TLV::AnonymousTag());
        VerifyOrDie(reader.EnterContainer(outer) == CHIP_NO_ERROR);
        VerifyOrDie(reader.Next() == CHIP_NO_ERROR);
        VerifyOrDie(reader.GetTag() == kTag);

        return reader;
    }

private:
    static constexpr size_t kMaxSize = 128;
    uint8_t mBuffer[kMaxSize];
};

class EncodeResult
{
public:
    explicit EncodeResult() = default;
    EncodeResult(CHIP_ERROR error) : mResult(error) { VerifyOrDie(error != CHIP_NO_ERROR); }

    static EncodeResult Ok() { return EncodeResult(); }

    bool IsSuccess() const { return !mResult.has_value(); }

    bool operator==(const CHIP_ERROR & other) const { return mResult.has_value() && (*mResult == other); }

    const std::optional<CHIP_ERROR> & Value() const { return mResult; }

private:
    std::optional<CHIP_ERROR> mResult;
};

template <typename T>
bool IsEqual(const T & a, const T & b)
{
    return a == b;
}

template <>
bool IsEqual<float>(const float & a, const float & b)
{
    if (std::isnan(a) && std::isnan(b))
    {
        return true;
    }

    return a == b;
}

template <>
bool IsEqual<double>(const double & a, const double & b)
{
    if (std::isnan(a) && std::isnan(b))
    {
        return true;
    }

    return a == b;
}

template <>
bool IsEqual<ByteSpan>(const ByteSpan & a, const ByteSpan & b)
{
    return a.data_equal(b);
}

template <>
bool IsEqual<CharSpan>(const CharSpan & a, const CharSpan & b)
{
    return a.data_equal(b);
}

template <typename T>
bool IsEqual(const std::optional<T> & a, const std::optional<T> & b)
{
    if (a.has_value() != b.has_value())
    {
        return false;
    }

    if (!a.has_value())
    {
        return true;
    }

    return IsEqual(*a, *b);
}

template <typename T>
bool IsEqual(const DataModel::Nullable<T> & a, const DataModel::Nullable<T> & b)
{
    if (a.IsNull() != b.IsNull())
    {
        return false;
    }

    if (a.IsNull())
    {
        return true;
    }

    return IsEqual(a.Value(), b.Value());
}

/// Validates that an encoded value in ember takes a specific format
template <size_t kMaxSize = 128>
class EncodeTester
{
public:
    EncodeTester(const EmberAfAttributeMetadata * meta) : mMetaData(meta) {}
    ~EncodeTester() = default;

    template <typename T, size_t N>
    EncodeResult TryEncode(const T & value, const uint8_t (&arr)[N])
    {
        ByteSpan expected(arr);
        MutableByteSpan out_span(mEmberAttributeDataBuffer);
        Ember::EmberAttributeDataBuffer buffer(mMetaData, out_span);

        TLVEncodedValue tlvEncoded;
        TLV::TLVReader reader = tlvEncoded.EncodeValue(value);

        CHIP_ERROR err = buffer.Decode(reader);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (expected.size() != out_span.size())
        {
            ChipLogError(Test, "Decode mismatch in size: expected %u, got %u", static_cast<unsigned>(expected.size()),
                         static_cast<unsigned>(out_span.size()));
            return CHIP_ERROR_INTERNAL;
        }

        if (!expected.data_equal(out_span))
        {
            ChipLogError(Test, "Decode mismatch in content for %u bytes", static_cast<unsigned>(expected.size()));
            return CHIP_ERROR_INTERNAL;
        }

        return EncodeResult::Ok();
    }

    template <typename T, size_t N>
    EncodeResult TryDecode(const T & value, const uint8_t (&arr)[N])
    {
        // Write data to TLV
        {
            uint8_t mutableBuffer[N];
            memcpy(mutableBuffer, arr, N);

            MutableByteSpan data_span(mutableBuffer);
            Ember::EmberAttributeDataBuffer buffer(mMetaData, data_span);

            TLV::TLVWriter writer;
            writer.Init(mEmberAttributeDataBuffer, sizeof(mEmberAttributeDataBuffer));
            ReturnErrorOnFailure(buffer.Encode(writer, TLV::AnonymousTag()));
            ReturnErrorOnFailure(writer.Finalize());
        }

        // Data was written in TLV. Take it back out

        TLV::TLVReader reader;
        reader.Init(mEmberAttributeDataBuffer, sizeof(mEmberAttributeDataBuffer));

        ReturnErrorOnFailure(reader.Next());

        T encodedValue;
        ReturnErrorOnFailure(DataModel::Decode(reader, encodedValue));

        if (!IsEqual(encodedValue, value))
        {
            ChipLogError(Test, "Encode mismatch: different data");
            return CHIP_ERROR_INTERNAL;
        }

        return EncodeResult::Ok();
    }

private:
    const EmberAfAttributeMetadata * mMetaData;
    uint8_t mEmberAttributeDataBuffer[kMaxSize];
};

const EmberAfAttributeMetadata * CreateFakeMeta(EmberAfAttributeType type, bool nullable)
{
    static EmberAfAttributeMetadata meta = {
        .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint8_t *>(nullptr)),
        .attributeId   = 0,
        .size          = 0, // likely not valid, however not used for tests
        .attributeType = ZCL_UNKNOWN_ATTRIBUTE_TYPE,
        .mask          = 0,
    };

    meta.attributeType = type;
    meta.mask          = nullable ? MATTER_ATTRIBUTE_FLAG_NULLABLE : 0;

    return &meta;
}

} // namespace

// All the tests below assume buffer ordering in little endian format
// Since currently all chip platforms in CI are little endian, we just kept tests
// as-is
static_assert(!CHIP_CONFIG_BIG_ENDIAN_TARGET);

TEST(TestEmberAttributeBuffer, TestEncodeUnsignedTypes)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8U_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<uint8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint8_t>(0xFD, { 0xFD }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint8_t>(255, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<uint8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint8_t>(0xFD, { 0xFD }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<uint8_t>>(DataModel::NullNullable, { 0xFF }).IsSuccess());

        // Not allowed to encode null-equivalent
        EXPECT_EQ(tester.TryEncode<uint8_t>(0xFF, { 0xFF }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16U_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<uint16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(0xFD, { 0xFD, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(255, { 0xFF, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(0xABCD, { 0xCD, 0xAB }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(0xFFFF, { 0xFF, 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<uint16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(0xFD, { 0xFD, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(255, { 0xFF, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint16_t>(0xABCD, { 0xCD, 0xAB }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<uint16_t>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());

        // Not allowed to encode null-equivalent
        EXPECT_EQ(tester.TryEncode<uint16_t>(0xFFFF, { 0xFF, 0xFF }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<uint64_t>(0, { 0, 0, 0, 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x1234567, { 0x67, 0x45, 0x23, 0x01, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0xAABBCCDDEEFF1122, { 0x22, 0x11, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA }).IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<uint64_t>(std::numeric_limits<uint64_t>::max() - 1, { 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                .IsSuccess());

        EXPECT_TRUE(tester
                        .TryEncode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable,
                                                                  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                        .IsSuccess());

        EXPECT_EQ(
            tester.TryEncode<uint64_t>(std::numeric_limits<uint64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }),
            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64U_ATTRIBUTE_TYPE, false /* nullable */));

        // we should be able to encode the maximum value
        EXPECT_TRUE(
            tester.TryEncode<uint64_t>(std::numeric_limits<uint64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                .IsSuccess());
    }

    /// Odd sized integers
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24U_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint32_t>(0xFFFFFF, { 0xFF, 0xFF, 0xFF }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<uint32_t>(0x1000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(tester.TryEncode<uint32_t>(0xFF000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0xFF, 0xFF, 0xFF }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<uint32_t>(0x1000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // cannot encode null equivalent value
        EXPECT_EQ(tester.TryEncode<uint32_t>(0xFFFFFF, { 0x56, 0x34, 0x12 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT40U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0, { 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x123456, { 0x56, 0x34, 0x12, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x123456FFFF, { 0xFF, 0xFF, 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<uint64_t>(0x10011001100, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // cannot encode null equivalent value
        EXPECT_EQ(tester.TryEncode<uint64_t>(0xFFFFFFFFFF, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    // Double-check tests, not as exhaustive, to cover all other unsigned values and get
    // more test line coverage
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT32U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0 }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT48U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0, 0, 0 }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT56U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0, 0, 0, 0 }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestEncodeSignedTypes)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(127, { 127 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(-10, { 0xF6 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(-128, { 0x80 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(127, { 127 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(-10, { 0xF6 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int8_t>(-127, { 0x81 }).IsSuccess());

        // NULL canot be encoded
        EXPECT_EQ(tester.TryEncode<int8_t>(std::numeric_limits<int8_t>::min(), { 0x80 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    {

        EncodeTester tester(CreateFakeMeta(ZCL_INT16S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(127, { 127, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(-10, { 0xF6, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(-128, { 0x80, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(-1234, { 0x2E, 0xFB }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(std::numeric_limits<int16_t>::min(), { 0x0, 0x80 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(127, { 127, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int16_t>(-10, { 0xF6, 0xFF }).IsSuccess());

        // NULL canot be encoded
        EXPECT_EQ(tester.TryEncode<int16_t>(std::numeric_limits<int16_t>::min(), { 0x80 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    // Odd size integers
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-1, { 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-10, { 0xF6, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-1234, { 0x2E, 0xFB, 0xFF }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<int32_t>(0x1000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(tester.TryEncode<int32_t>(0x0F000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(tester.TryEncode<int32_t>(-0x1000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<int32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-1, { 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-10, { 0xF6, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int32_t>(-1234, { 0x2E, 0xFB, 0xFF }).IsSuccess());

        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0x00, 0x00, 0x80 }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<int32_t>(0x1000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // cannot encode null equivalent value - this is the minimum negative value
        // for 24-bit
        EXPECT_EQ(tester.TryEncode<int32_t>(-(1 << 24) - 1, { 0x56, 0x34, 0x12 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        // Out of range for signed - these are unsigned values that are larger
        EXPECT_EQ(tester.TryEncode<int32_t>(0xFFFFFF, { 0x56, 0x34, 0x12 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(tester.TryEncode<int32_t>(0x800000, { 0x56, 0x34, 0x12 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT40S_ATTRIBUTE_TYPE, true /* nullable */));

        // NOTE: to generate encoded values, you an use commands like:
        //
        //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<q", -12345678910)]))'
        //
        //    OUTPUT: 0xC2, 0xE3, 0x23, 0x20, 0xFD, 0xFF, 0xFF, 0xFF
        //
        EXPECT_TRUE(tester.TryEncode<int64_t>(0, { 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int64_t>(0x123456, { 0x56, 0x34, 0x12, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int64_t>(-123456789, { 0xeb, 0x32, 0xa4, 0xf8, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<int64_t>(-12345678910, { 0xc2, 0xe3, 0x23, 0x20, 0xfd }).IsSuccess());

        EXPECT_TRUE(
            tester.TryEncode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable, { 0x00, 0x00, 0x00, 0x00, 0x80 }).IsSuccess());

        // Out of range
        EXPECT_EQ(tester.TryEncode<int64_t>(0x10011001100, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // cannot encode null equivalent value
        EXPECT_EQ(tester.TryEncode<int64_t>(-(1LL << 40) - 1, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // negative out of range
        EXPECT_EQ(tester.TryEncode<int64_t>(-0x10000000000, { 0 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    // Double-check tests, not as exhaustive, to cover all other unsigned values and get
    // more test line coverage
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT32S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT48S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT56S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());

        // min/max ranges too
        EXPECT_TRUE(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::min() + 1, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F })
                .IsSuccess());

        // Reserved value for NULL
        EXPECT_EQ(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::min(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 }),
            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64S_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());

        EXPECT_TRUE(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::min(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::min() + 1, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<int64_t>(std::numeric_limits<int64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F })
                .IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestEncodeBool)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryEncode<bool>(true, { 1 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<bool>(false, { 0 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryEncode<bool>(true, { 1 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<bool>(false, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<bool>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestEncodeFloatingPoint)
{
    // NOTE: to generate encoded values, you an use commands like:
    //
    //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<f", -123.55)]))'
    //    OUTPUT: 0x9A, 0x19, 0xF7, 0x42
    //
    //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<f", float("nan"))]))'
    //    OUTPUT: 0x00, 0x00, 0xC0, 0x7F
    //
    {
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<float>(123.55f, { 0x9A, 0x19, 0xF7, 0x42 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<float>(123.55f, { 0x9A, 0x19, 0xF7, 0x42 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<float>>(DataModel::NullNullable, { 0, 0, 0xC0, 0x7F }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<double>(123.55, { 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x5E, 0x40 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<double>(123.55, { 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x5E, 0x40 }).IsSuccess());
        EXPECT_TRUE(
            tester.TryEncode<DataModel::Nullable<double>>(DataModel::NullNullable, { 0, 0, 0, 0, 0, 0, 0xF8, 0x7F }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestEncodeStrings)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<CharSpan>(""_span, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<CharSpan>("test"_span, { 4, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<CharSpan>("foo"_span, { 3, 'f', 'o', 'o' }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<CharSpan>(""_span, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<CharSpan>("test"_span, { 4, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<CharSpan>(""_span, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<CharSpan>("test"_span, { 4, 0, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<CharSpan>("foo"_span, { 3, 0, 'f', 'o', 'o' }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<CharSpan>("test"_span, { 4, 0, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }

    const uint8_t kOctetData[] = { 1, 2, 3 };

    // Binary data
    {
        EncodeTester tester(CreateFakeMeta(ZCL_OCTET_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan({}), { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan(kOctetData), { 3, 1, 2, 3 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_OCTET_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan({}), { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan(kOctetData), { 3, 1, 2, 3 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<ByteSpan>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan({}), { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan(kOctetData), { 3, 0, 1, 2, 3 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan({}), { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<ByteSpan>(ByteSpan(kOctetData), { 3, 0, 1, 2, 3 }).IsSuccess());
        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<ByteSpan>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestEncodeFailures)
{
    {
        // attribute type that is not handled
        EncodeTester tester(CreateFakeMeta(ZCL_UNKNOWN_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_EQ(tester.TryEncode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0 }), CHIP_IM_GLOBAL_STATUS(Failure));
    }

    {
        // Insufficient space
        EncodeTester<3> tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));

        // Empty is ok
        EXPECT_TRUE(tester.TryEncode<CharSpan>(""_span, { 0 }).IsSuccess());

        // Short strings (with and without count) is wrong.
        EXPECT_EQ(tester.TryEncode<CharSpan>("test"_span, { 0 }), CHIP_ERROR_NO_MEMORY);
        EXPECT_EQ(tester.TryEncode<CharSpan>("foo"_span, { 3, 'f', 'o' }), CHIP_ERROR_NO_MEMORY);

        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        // Insufficient space
        EncodeTester<3> tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));

        // Empty is ok
        EXPECT_TRUE(tester.TryEncode<CharSpan>(""_span, { 0, 0 }).IsSuccess());

        // Short strings (with and without count) is wrong.
        EXPECT_EQ(tester.TryEncode<CharSpan>("test"_span, { 0 }), CHIP_ERROR_NO_MEMORY);
        EXPECT_EQ(tester.TryEncode<CharSpan>("foo"_span, { 0, 3, 'f', 'o' }), CHIP_ERROR_NO_MEMORY);
        EXPECT_EQ(tester.TryEncode<CharSpan>("test"_span, { 0xFF }), CHIP_ERROR_NO_MEMORY);

        EXPECT_TRUE(tester.TryEncode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }

    {
        // Insufficient space even for length
        EncodeTester<1> tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_EQ(tester.TryEncode<CharSpan>(""_span, { 0 }), CHIP_ERROR_NO_MEMORY);
    }

    // bad type casts
    {
        EncodeTester tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryEncode<ByteSpan>(ByteSpan({}), { 0 }), CHIP_ERROR_WRONG_TLV_TYPE);
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT32U_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryEncode<bool>(true, { 0 }), CHIP_ERROR_WRONG_TLV_TYPE);
    }
}

TEST(TestEmberAttributeBuffer, TestNoData)
{
    EncodeTester tester(CreateFakeMeta(ZCL_NO_DATA_ATTRIBUTE_TYPE, true /* nullable */));

    // support a always-null type
    EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0 }).IsSuccess());
    EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<float>>(DataModel::NullNullable, { 0 }).IsSuccess());
    EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<bool>>(DataModel::NullNullable, { 0 }).IsSuccess());
}

TEST(TestEmberAttributeBuffer, TestDecodeFailures)
{
    {
        // attribute type that is not handled
        EncodeTester tester(CreateFakeMeta(ZCL_UNKNOWN_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_EQ(tester.TryDecode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0 }), CHIP_IM_GLOBAL_STATUS(Failure));
    }

    {
        // Insufficient input
        EncodeTester<3> tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_EQ(tester.TryDecode<CharSpan>("test"_span, { 10 }), CHIP_ERROR_BUFFER_TOO_SMALL);
        EXPECT_EQ(tester.TryDecode<CharSpan>("foo"_span, { 3, 'f', 'o' }), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    {
        // Insufficient data buffer - should never happen, but test that we will error out
        EncodeTester tester(CreateFakeMeta(ZCL_INT32U_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryDecode<uint32_t>(123, { 1, 2, 3 }), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    {
        // Insufficient data buffer - should never happen, but test that we will error out
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryDecode<float>(1.5f, { 1, 2, 3 }), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    {
        // Insufficient data buffer - should never happen, but test that we will error out
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryDecode<double>(1.5, { 1, 2, 3 }), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    {
        // Bad boolean data
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryDecode<bool>(true, { 123 }), CHIP_ERROR_INVALID_ARGUMENT);
    }
}

TEST(TestEmberAttributeBuffer, TestDecodeSignedTypes)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(127, { 127 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(-10, { 0xF6 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(-128, { 0x80 }).IsSuccess());

        // longer data is ok
        EXPECT_TRUE(tester.TryDecode<int8_t>(-128, { 0x80, 1, 2, 3, 4 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(127, { 127 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(-10, { 0xF6 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int8_t>(-127, { 0x81 }).IsSuccess());

        // NULL can be decoded
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<int8_t>>(DataModel::NullNullable, { 0x80 }).IsSuccess());

        // decoding as nullable proceeds as normal
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<int8_t>>(-127, { 0x81 }).IsSuccess());
    }

    {

        EncodeTester tester(CreateFakeMeta(ZCL_INT16S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(127, { 127, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(-10, { 0xF6, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(-128, { 0x80, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(-1234, { 0x2E, 0xFB }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(std::numeric_limits<int16_t>::min(), { 0x0, 0x80 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(127, { 127, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int16_t>(-10, { 0xF6, 0xFF }).IsSuccess());

        // NULL decoding
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<int16_t>>(DataModel::NullNullable, { 0x00, 0x80 }).IsSuccess());
    }

    // Odd size integers
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24S_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-1, { 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-10, { 0xF6, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-1234, { 0x2E, 0xFB, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24S_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<int32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-1, { 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-10, { 0xF6, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int32_t>(-1234, { 0x2E, 0xFB, 0xFF }).IsSuccess());

        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0x00, 0x00, 0x80 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT40S_ATTRIBUTE_TYPE, true /* nullable */));

        // NOTE: to generate encoded values, you an use commands like:
        //
        //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<q", -12345678910)]))'
        //
        //    OUTPUT: 0xC2, 0xE3, 0x23, 0x20, 0xFD, 0xFF, 0xFF, 0xFF
        //
        EXPECT_TRUE(tester.TryDecode<int64_t>(0, { 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int64_t>(0x123456, { 0x56, 0x34, 0x12, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int64_t>(-123456789, { 0xeb, 0x32, 0xa4, 0xf8, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<int64_t>(-12345678910, { 0xc2, 0xe3, 0x23, 0x20, 0xfd }).IsSuccess());

        EXPECT_TRUE(
            tester.TryDecode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable, { 0x00, 0x00, 0x00, 0x00, 0x80 }).IsSuccess());
    }

    // Double-check tests, not as exhaustive, to cover all other unsigned values and get
    // more test line coverage
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT32S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT48S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT56S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64S_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());

        // min/max ranges too
        EXPECT_TRUE(
            tester.TryDecode<int64_t>(std::numeric_limits<int64_t>::min() + 1, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<int64_t>(std::numeric_limits<int64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F })
                .IsSuccess());

        EXPECT_TRUE(tester
                        .TryDecode<DataModel::Nullable<int64_t>>(DataModel::NullNullable,
                                                                 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                        .IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64S_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<int64_t>(-1234, { 0x2E, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());

        EXPECT_TRUE(
            tester.TryDecode<int64_t>(std::numeric_limits<int64_t>::min(), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<int64_t>(std::numeric_limits<int64_t>::min() + 1, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 })
                .IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<int64_t>(std::numeric_limits<int64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F })
                .IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestDecodeUnsignedTypes)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8U_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<uint8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint8_t>(0xFD, { 0xFD }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint8_t>(255, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT8U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<uint8_t>(0, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint8_t>(123, { 123 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint8_t>(0xFD, { 0xFD }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint8_t>>(DataModel::NullNullable, { 0xFF }).IsSuccess());

        // NULL decoding should work
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint8_t>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16U_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<uint16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(0xFD, { 0xFD, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(255, { 0xFF, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(0xABCD, { 0xCD, 0xAB }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(0xFFFF, { 0xFF, 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT16U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<uint16_t>(0, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(123, { 123, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(0xFD, { 0xFD, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(255, { 0xFF, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint16_t>(0xABCD, { 0xCD, 0xAB }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint16_t>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());

        // NULL SUPPORT
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint16_t>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64U_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<uint64_t>(0, { 0, 0, 0, 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x1234567, { 0x67, 0x45, 0x23, 0x01, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0xAABBCCDDEEFF1122, { 0x22, 0x11, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA }).IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<uint64_t>(std::numeric_limits<uint64_t>::max() - 1, { 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                .IsSuccess());

        EXPECT_TRUE(tester
                        .TryDecode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable,
                                                                  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                        .IsSuccess());

        EXPECT_TRUE(tester
                        .TryDecode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable,
                                                                  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                        .IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT64U_ATTRIBUTE_TYPE, false /* nullable */));

        // we should be able to encode the maximum value
        EXPECT_TRUE(
            tester.TryDecode<uint64_t>(std::numeric_limits<uint64_t>::max(), { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
                .IsSuccess());
    }

    /// Odd sized integers
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24U_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint32_t>(0xFFFFFF, { 0xFF, 0xFF, 0xFF }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT24U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint32_t>(0, { 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint32_t>(0x123456, { 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint32_t>>(DataModel::NullNullable, { 0xFF, 0xFF, 0xFF }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<uint32_t>>(0x1234, { 0x34, 0x12, 0x00 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT40U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0, { 0, 0, 0, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x123456, { 0x56, 0x34, 0x12, 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x123456FFFF, { 0xFF, 0xFF, 0x56, 0x34, 0x12 }).IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<DataModel::Nullable<uint64_t>>(DataModel::NullNullable, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }).IsSuccess());
    }

    // Double-check tests, not as exhaustive, to cover all other unsigned values and get
    // more test line coverage
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT32U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0 }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT48U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0, 0, 0 }).IsSuccess());
    }
    {
        EncodeTester tester(CreateFakeMeta(ZCL_INT56U_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<uint64_t>(0x1234, { 0x34, 0x12, 0, 0, 0, 0, 0 }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestDecodeStrings)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<CharSpan>(""_span, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<CharSpan>("test"_span, { 4, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<CharSpan>("foo"_span, { 3, 'f', 'o', 'o' }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<CharSpan>(""_span, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<CharSpan>("test"_span, { 4, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<CharSpan>(""_span, { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<CharSpan>("test"_span, { 4, 0, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<CharSpan>("foo"_span, { 3, 0, 'f', 'o', 'o' }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<CharSpan>("test"_span, { 4, 0, 't', 'e', 's', 't' }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<CharSpan>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }

    const uint8_t kOctetData[] = { 1, 2, 3 };

    // Binary data
    {
        EncodeTester tester(CreateFakeMeta(ZCL_OCTET_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan({}), { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan(kOctetData), { 3, 1, 2, 3 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_OCTET_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan({}), { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan(kOctetData), { 3, 1, 2, 3 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<ByteSpan>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan({}), { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan(kOctetData), { 3, 0, 1, 2, 3 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan({}), { 0, 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<ByteSpan>(ByteSpan(kOctetData), { 3, 0, 1, 2, 3 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<ByteSpan>>(DataModel::NullNullable, { 0xFF, 0xFF }).IsSuccess());
    }
}

TEST(TestEmberAttributeBuffer, TestDecodeBool)
{
    {
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, false /* nullable */));

        EXPECT_TRUE(tester.TryDecode<bool>(true, { 1 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<bool>(false, { 0 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, true /* nullable */));

        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<bool>>(true, { 1 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<bool>>(false, { 0 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<bool>>(DataModel::NullNullable, { 0xFF }).IsSuccess());
    }

    {
        // Boolean that is NOT nullable
        EncodeTester tester(CreateFakeMeta(ZCL_BOOLEAN_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_EQ(tester.TryDecode<DataModel::Nullable<bool>>(DataModel::NullNullable, { 0xFF }), CHIP_ERROR_INVALID_ARGUMENT);
        EXPECT_EQ(tester.TryDecode<bool>(true, { 0xFF }), CHIP_ERROR_INVALID_ARGUMENT);
    }
}

TEST(TestEmberAttributeBuffer, TestDecodeFloatingPoint)
{
    // NOTE: to generate encoded values, you an use commands like:
    //
    //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<f", -123.55)]))'
    //    OUTPUT: 0x9A, 0x19, 0xF7, 0x42
    //
    //    python -c 'import struct; print(", ".join(["0x%X" % v for v in struct.pack("<f", float("nan"))]))'
    //    OUTPUT: 0x00, 0x00, 0xC0, 0x7F
    //
    {
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<float>(123.55f, { 0x9A, 0x19, 0xF7, 0x42 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<float>(123.55f, { 0x9A, 0x19, 0xF7, 0x42 }).IsSuccess());
        EXPECT_TRUE(tester.TryDecode<DataModel::Nullable<float>>(DataModel::NullNullable, { 0, 0, 0xC0, 0x7F }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_SINGLE_ATTRIBUTE_TYPE, false /* nullable */));
        // non-nullable float
        EXPECT_TRUE(tester.TryDecode<float>(std::nanf("0"), { 0, 0, 0xC0, 0x7F }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, false /* nullable */));
        EXPECT_TRUE(tester.TryDecode<double>(123.55, { 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x5E, 0x40 }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, true /* nullable */));
        EXPECT_TRUE(tester.TryDecode<double>(123.55, { 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x5E, 0x40 }).IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<DataModel::Nullable<double>>(123.55, { 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x5E, 0x40 }).IsSuccess());
        EXPECT_TRUE(
            tester.TryDecode<DataModel::Nullable<double>>(DataModel::NullNullable, { 0, 0, 0, 0, 0, 0, 0xF8, 0x7F }).IsSuccess());
    }

    {
        EncodeTester tester(CreateFakeMeta(ZCL_DOUBLE_ATTRIBUTE_TYPE, false /* nullable */));
        // non-nullable double
        EXPECT_TRUE(tester.TryDecode<double>(std::nan("0"), { 0, 0, 0, 0, 0, 0, 0xF8, 0x7F }).IsSuccess());
    }
}
