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
#include <app/util/attribute-metadata.h>

#include <algorithm>
#include <cstring>

#include <app-common/zap-generated/attribute-type.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

bool EmberAfAttributeMetadata::IsBoolean() const
{
    return attributeType == ZCL_BOOLEAN_ATTRIBUTE_TYPE;
}

bool EmberAfAttributeMetadata::IsSignedIntegerAttribute() const
{
    return chip::app::IsSignedAttributeType(attributeType);
}

bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

namespace chip {
namespace app {

using Protocols::InteractionModel::Status;

bool AttributeDefaultValue::DecodeStringPayload(ByteSpan & outPayload) const
{
    outPayload = ByteSpan();

    VerifyOrReturnValue(!rawData.empty(), false);

    size_t prefixSize;
    size_t length;

    if (emberAfIsLongStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 2, false);
        uint16_t len = Encoding::LittleEndian::Get16(rawData.data());
        VerifyOrReturnValue(len != 0xFFFF, false); // Null sentinel
        prefixSize = 2;
        length     = len;
    }
    else if (emberAfIsStringAttributeType(type))
    {
        uint8_t len = rawData[0];
        VerifyOrReturnValue(len != 0xFF, false); // Null sentinel
        prefixSize = 1;
        length     = len;
    }
    else
    {
        return false;
    }

    VerifyOrReturnValue(rawData.size() >= prefixSize + length, false);
    outPayload = rawData.SubSpan(prefixSize, length);
    return true;
}

CharSpan AttributeDefaultValue::ToCharSpan() const
{
    ByteSpan payload;
    VerifyOrReturnValue(DecodeStringPayload(payload), CharSpan());
    return CharSpan(Uint8::to_const_char(payload.data()), payload.size());
}

ByteSpan AttributeDefaultValue::ToByteSpan() const
{
    ByteSpan payload;
    VerifyOrReturnValue(DecodeStringPayload(payload), ByteSpan());
    return payload;
}

DataModel::Nullable<CharSpan> AttributeDefaultValue::ToNullableCharSpan() const
{
    ByteSpan payload;
    VerifyOrReturnValue(DecodeStringPayload(payload), DataModel::Nullable<CharSpan>());
    return DataModel::Nullable<CharSpan>(CharSpan(Uint8::to_const_char(payload.data()), payload.size()));
}

DataModel::Nullable<ByteSpan> AttributeDefaultValue::ToNullableByteSpan() const
{
    ByteSpan payload;
    VerifyOrReturnValue(DecodeStringPayload(payload), DataModel::Nullable<ByteSpan>());
    return DataModel::Nullable<ByteSpan>(payload);
}

bool AttributeDefaultValue::SetOwnedValue(ByteSpan data, EmberAfAttributeType attributeType)
{
    VerifyOrReturnValue(data.size() <= kMaxOwnedValueSize, false);

    memcpy(mOwnedValue, data.data(), data.size());
    rawData = ByteSpan(mOwnedValue, data.size());
    type    = attributeType;
    return true;
}

void AttributeDefaultValue::CopyScalar(void * outBuffer, size_t bufferSize) const
{
    if (outBuffer == nullptr || bufferSize == 0)
    {
        return;
    }
    if (rawData.empty())
    {
        memset(outBuffer, 0, bufferSize);
        return;
    }
    size_t copySize = std::min(bufferSize, rawData.size());
    memcpy(outBuffer, rawData.data(), copySize);
    if (copySize < bufferSize)
    {
        memset(reinterpret_cast<uint8_t *>(outBuffer) + copySize, 0, bufferSize - copySize);
    }
}

namespace {

/// Decodes the Pascal-style length prefix at the start of `ptr` and reports the number of bytes the
/// value occupies, prefix included. `declaredSize` is the attribute size from the metadata, which
/// covers both the prefix and the payload.
///
/// Returns false when the prefix claims more bytes than the attribute declares: such a declaration is
/// inconsistent and the resulting span would reach past the underlying buffer.
bool PascalStringSize(const uint8_t * ptr, uint16_t declaredSize, bool isLongString, size_t & outSize)
{
    const size_t prefixSize = isLongString ? 2u : 1u;
    VerifyOrReturnValue(declaredSize >= prefixSize, false);

    const uint16_t length = isLongString ? Encoding::LittleEndian::Get16(ptr) : ptr[0];
    const bool isNull     = isLongString ? (length == 0xFFFF) : (length == 0xFF);

    outSize = isNull ? prefixSize : (prefixSize + length);
    return outSize <= declaredSize;
}

} // namespace

Status emberAfGetAttributeDefaultValue(const EmberAfAttributeMetadata & am, AttributeDefaultValue & outDefault)
{
    outDefault.type = am.attributeType;

    if (am.HasEmptyDefault())
    {
        outDefault.rawData = ByteSpan();
        return Status::NotFound;
    }

    const bool isLongString  = emberAfIsLongStringAttributeType(am.attributeType);
    const bool isShortString = emberAfIsStringAttributeType(am.attributeType);
    const bool isStringType  = isLongString || isShortString;

    const uint8_t * ptr = nullptr;

    // Size of the union member that `ptr` points into when the value is stored inline. Only
    // relevant on big-endian targets; see the adjustment below.
    size_t inlineStorageSize = 0;
    (void) inlineStorageSize;

    if ((am.mask & MATTER_ATTRIBUTE_FLAG_MIN_MAX) != 0U)
    {
        if (am.defaultValue.ptrToMinMaxValue != nullptr)
        {
            // This is intentionally 2 and not 4 bytes since defaultValue in min/max attributes is
            // still uint16_t.
            static_assert(sizeof(am.defaultValue.ptrToMinMaxValue->defaultValue.defaultValue) == 2,
                          "if statement relies on size of max/min defaultValue being 2");
            if (am.size <= 2)
            {
                ptr = reinterpret_cast<const uint8_t *>(&(am.defaultValue.ptrToMinMaxValue->defaultValue.defaultValue));
                inlineStorageSize = sizeof(am.defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
            }
            else
            {
                ptr = am.defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
            }
        }
    }
    else
    {
        // Non-string scalars <= 4 bytes are stored inline in defaultValue.defaultValue.
        // Strings and long strings always store a pointer in defaultValue.ptrToDefaultValue,
        // even if their size is <= 4 bytes.
        if ((am.size <= 4) && !isStringType)
        {
            ptr               = reinterpret_cast<const uint8_t *>(&(am.defaultValue.defaultValue));
            inlineStorageSize = sizeof(am.defaultValue.defaultValue);
        }
        else
        {
            ptr = am.defaultValue.ptrToDefaultValue;
        }
    }

#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
    // Inline defaults occupy the full union member regardless of the attribute size. On big-endian
    // targets the significant bytes sit at the end of that member, so nudge the pointer forward.
    if (am.size < inlineStorageSize && ptr != nullptr)
    {
        ptr += (inlineStorageSize - am.size);
    }
#endif

    if (ptr == nullptr)
    {
        outDefault.rawData = ByteSpan();
        return Status::NotFound;
    }

    if (isStringType)
    {
        size_t totalSize = 0;
        if (!PascalStringSize(ptr, am.size, isLongString, totalSize))
        {
            outDefault.rawData = ByteSpan();
            return Status::NotFound;
        }
        outDefault.rawData = ByteSpan(ptr, totalSize);
    }
    else
    {
        outDefault.rawData = ByteSpan(ptr, am.size);
    }

    return Status::Success;
}

} // namespace app
} // namespace chip
