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

CharSpan AttributeDefaultValue::ToCharSpan() const
{
    VerifyOrReturnValue(!rawData.empty(), CharSpan());
    if (emberAfIsLongStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 2, CharSpan());
        uint16_t len = Encoding::LittleEndian::Get16(rawData.data());
        if (len == 0xFFFF)
        {
            return CharSpan();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(2 + len), CharSpan());
        return CharSpan(reinterpret_cast<const char *>(rawData.data() + 2), len);
    }
    if (emberAfIsStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 1, CharSpan());
        uint8_t len = rawData[0];
        if (len == 0xFF)
        {
            return CharSpan();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(1 + len), CharSpan());
        return CharSpan(reinterpret_cast<const char *>(rawData.data() + 1), len);
    }
    return CharSpan();
}

ByteSpan AttributeDefaultValue::ToByteSpan() const
{
    VerifyOrReturnValue(!rawData.empty(), ByteSpan());
    if (emberAfIsLongStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 2, ByteSpan());
        uint16_t len = Encoding::LittleEndian::Get16(rawData.data());
        if (len == 0xFFFF)
        {
            return ByteSpan();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(2 + len), ByteSpan());
        return ByteSpan(rawData.data() + 2, len);
    }
    if (emberAfIsStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 1, ByteSpan());
        uint8_t len = rawData[0];
        if (len == 0xFF)
        {
            return ByteSpan();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(1 + len), ByteSpan());
        return ByteSpan(rawData.data() + 1, len);
    }
    return ByteSpan();
}

DataModel::Nullable<CharSpan> AttributeDefaultValue::ToNullableCharSpan() const
{
    VerifyOrReturnValue(!rawData.empty(), DataModel::Nullable<CharSpan>(CharSpan()));
    if (emberAfIsLongStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 2, DataModel::Nullable<CharSpan>(CharSpan()));
        uint16_t len = Encoding::LittleEndian::Get16(rawData.data());
        if (len == 0xFFFF)
        {
            return DataModel::Nullable<CharSpan>();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(2 + len), DataModel::Nullable<CharSpan>(CharSpan()));
        return DataModel::Nullable<CharSpan>(CharSpan(reinterpret_cast<const char *>(rawData.data() + 2), len));
    }
    if (emberAfIsStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 1, DataModel::Nullable<CharSpan>(CharSpan()));
        uint8_t len = rawData[0];
        if (len == 0xFF)
        {
            return DataModel::Nullable<CharSpan>();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(1 + len), DataModel::Nullable<CharSpan>(CharSpan()));
        return DataModel::Nullable<CharSpan>(CharSpan(reinterpret_cast<const char *>(rawData.data() + 1), len));
    }
    return DataModel::Nullable<CharSpan>(CharSpan());
}

DataModel::Nullable<ByteSpan> AttributeDefaultValue::ToNullableByteSpan() const
{
    VerifyOrReturnValue(!rawData.empty(), DataModel::Nullable<ByteSpan>(ByteSpan()));
    if (emberAfIsLongStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 2, DataModel::Nullable<ByteSpan>(ByteSpan()));
        uint16_t len = Encoding::LittleEndian::Get16(rawData.data());
        if (len == 0xFFFF)
        {
            return DataModel::Nullable<ByteSpan>();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(2 + len), DataModel::Nullable<ByteSpan>(ByteSpan()));
        return DataModel::Nullable<ByteSpan>(ByteSpan(rawData.data() + 2, len));
    }
    if (emberAfIsStringAttributeType(type))
    {
        VerifyOrReturnValue(rawData.size() >= 1, DataModel::Nullable<ByteSpan>(ByteSpan()));
        uint8_t len = rawData[0];
        if (len == 0xFF)
        {
            return DataModel::Nullable<ByteSpan>();
        }
        VerifyOrReturnValue(rawData.size() >= static_cast<size_t>(1 + len), DataModel::Nullable<ByteSpan>(ByteSpan()));
        return DataModel::Nullable<ByteSpan>(ByteSpan(rawData.data() + 1, len));
    }
    return DataModel::Nullable<ByteSpan>(ByteSpan());
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

Status emberAfGetAttributeDefaultValue(const EmberAfAttributeMetadata * am, AttributeDefaultValue & outDefault)
{
    VerifyOrReturnError(am != nullptr, Status::UnsupportedAttribute);

    outDefault.type = am->attributeType;

    const uint8_t * ptr                       = nullptr;
    size_t defaultValueSizeForBigEndianNudger = 0;
    (void) defaultValueSizeForBigEndianNudger;

    if ((am->mask & MATTER_ATTRIBUTE_FLAG_MIN_MAX) != 0U)
    {
        if (am->size <= 2)
        {
            ptr = reinterpret_cast<const uint8_t *>(&(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue));
            defaultValueSizeForBigEndianNudger = sizeof(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
        }
        else
        {
            ptr = am->defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
        }
    }
    else
    {
        if ((am->size <= 4) && !emberAfIsStringAttributeType(am->attributeType))
        {
            ptr                                = reinterpret_cast<const uint8_t *>(&(am->defaultValue.defaultValue));
            defaultValueSizeForBigEndianNudger = sizeof(am->defaultValue.defaultValue);
        }
        else
        {
            ptr = am->defaultValue.ptrToDefaultValue;
        }
    }

#if (CHIP_CONFIG_BIG_ENDIAN_TARGET)
    if (am->size < defaultValueSizeForBigEndianNudger && ptr != nullptr)
    {
        ptr += (defaultValueSizeForBigEndianNudger - am->size);
    }
#endif

    if (ptr == nullptr)
    {
        outDefault.rawData = ByteSpan();
    }
    else if (emberAfIsLongStringAttributeType(am->attributeType))
    {
        uint16_t len       = Encoding::LittleEndian::Get16(ptr);
        size_t totalSize   = (len == 0xFFFF) ? 2 : static_cast<size_t>(2 + len);
        outDefault.rawData = ByteSpan(ptr, totalSize);
    }
    else if (emberAfIsStringAttributeType(am->attributeType))
    {
        uint8_t len        = ptr[0];
        size_t totalSize   = (len == 0xFF) ? 1 : static_cast<size_t>(1 + len);
        outDefault.rawData = ByteSpan(ptr, totalSize);
    }
    else
    {
        outDefault.rawData = ByteSpan(ptr, am->size);
    }

    return Status::Success;
}

} // namespace app
} // namespace chip
