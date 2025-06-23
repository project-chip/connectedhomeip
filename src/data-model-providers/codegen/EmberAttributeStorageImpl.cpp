/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "lib/core/CHIPError.h"
#include <data-model-providers/codegen/EmberAttributeStorageImpl.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <lib/support/ScopedBuffer.h>

namespace chip {
namespace app {
namespace Storage {
namespace {

// EmberAfAttributeType possible values are part of the unnamed enum in `attribute-type.h`,
// of the form ZCL_*_ATTRIBUTE_TYPE
EmberAfAttributeType attributeType(AttributeStorage::Buffer & buffer)
{
    switch (buffer.type())
    {
    case AttributeStorage::Buffer::Type::kPrimitive:
        switch (buffer.size())
        {
        case 1:
            return ZCL_INT8U_ATTRIBUTE_TYPE;
        case 2:
            return ZCL_INT16U_ATTRIBUTE_TYPE;
        case 3:
            return ZCL_INT24U_ATTRIBUTE_TYPE;
        case 4:
            return ZCL_INT32U_ATTRIBUTE_TYPE;
        case 5:
            return ZCL_INT40U_ATTRIBUTE_TYPE;
        case 6:
            return ZCL_INT48U_ATTRIBUTE_TYPE;
        case 7:
            return ZCL_INT56U_ATTRIBUTE_TYPE;
        case 8:
            return ZCL_INT64U_ATTRIBUTE_TYPE;
        default:
            // This one is a bit nonsense..
            return ZCL_NO_DATA_ATTRIBUTE_TYPE;
        }
    case AttributeStorage::Buffer::Type::kMutableByteSpan:
        // Generally ember persistence did not persist raw values.
        return ZCL_STRUCT_ATTRIBUTE_TYPE;
    case AttributeStorage::Buffer::Type::kStringOneByteLength:
    case AttributeStorage::Buffer::Type::kBytesOneByteLength:
        return ZCL_OCTET_STRING_ATTRIBUTE_TYPE;
    case AttributeStorage::Buffer::Type::kStringTwoByteLength:
    case AttributeStorage::Buffer::Type::kBytesTwoByteLength:
        return ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE;
    }

    // This should NEVER be reached, however I did have compilers ocmplain about it.
    return ZCL_NO_DATA_ATTRIBUTE_TYPE;
}

} // namespace

CHIP_ERROR EmberAttributeStorageImpl::Write(const ConcreteAttributePath & path, const Value & value)
{
    AttributePersistenceProvider * provider = GetAttributePersistenceProvider();

    // note that data is already in the format that EMBER functions expect:
    //  - numeric is "as is"
    //  - pascal strings are already length-prefixed
    //  - raw values are just bytes
    return provider->WriteValue(path, value.data());
}

CHIP_ERROR EmberAttributeStorageImpl::Read(const ConcreteAttributePath & path, Buffer buffer)
{
    VerifyOrReturnError(buffer.size() > 0, CHIP_ERROR_BUFFER_TOO_SMALL);

    AttributePersistenceProvider * provider = GetAttributePersistenceProvider();
    const EmberAfAttributeMetadata fakeMetadata{
        .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
        .attributeId   = path.mAttributeId,
        .size          = static_cast<uint16_t>(buffer.size()),
        .attributeType = attributeType(buffer),
        .mask          = 0,
    };

    switch (buffer.type())
    {
    case AttributeStorage::Buffer::Type::kPrimitive: {
        MutableByteSpan raw{ reinterpret_cast<uint8_t *>(buffer.data()), buffer.size() };
        // numbers are just read as-is
        ReturnErrorOnFailure(provider->ReadValue(path, &fakeMetadata, raw));

        // Read size MUST match expected size.
        VerifyOrReturnError(raw.size() == fakeMetadata.size, CHIP_ERROR_INVALID_INTEGER_VALUE);
        return CHIP_NO_ERROR;
    }
    case AttributeStorage::Buffer::Type::kMutableByteSpan: {
        // raw data is a mutable byte buffer that we modify
        auto raw = reinterpret_cast<MutableByteSpan *>(buffer.data());
        return provider->ReadValue(path, &fakeMetadata, *raw);
    }
    case AttributeStorage::Buffer::Type::kBytesOneByteLength:
    case AttributeStorage::Buffer::Type::kStringOneByteLength: {
        MutableByteSpan raw{ reinterpret_cast<uint8_t *>(buffer.data()), buffer.size() };
        ReturnErrorOnFailure(provider->ReadValue(path, &fakeMetadata, raw));

        // first byte should be a valid size, If we read an empty value, empty it out
        if (raw.empty())
        {
            *reinterpret_cast<uint8_t *>(buffer.data()) = 0;
        }
        else
        {
            // we have valid data, validate it is not out of bounds
            VerifyOrReturnError(raw[0] + 1 <= fakeMetadata.size, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        }

        return CHIP_NO_ERROR;
    }
    case AttributeStorage::Buffer::Type::kBytesTwoByteLength:
    case AttributeStorage::Buffer::Type::kStringTwoByteLength: {
        VerifyOrReturnError(buffer.size() > 1, CHIP_ERROR_BUFFER_TOO_SMALL);

        MutableByteSpan raw{ reinterpret_cast<uint8_t *>(buffer.data()), buffer.size() };
        ReturnErrorOnFailure(provider->ReadValue(path, &fakeMetadata, raw));

        // first byte should be a valid size, If we read an empty value, empty it out
        if (raw.empty())
        {
            *reinterpret_cast<uint8_t *>(buffer.data()) = 0;
            *(reinterpret_cast<uint8_t *>(buffer.data()) + 1) = 0;
        }
        else
        {
            // we have valid data, validate it is not out of bounds
            uint16_t len;
            memcpy(&len, raw.data(), 2);
            VerifyOrReturnError(len + 2 <= fakeMetadata.size, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        }

        return CHIP_NO_ERROR;
    }
    }

    // should never be reached because switch above covers all cases
    // added here to make come compilers happy.
    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace Storage
} // namespace app
} // namespace chip
