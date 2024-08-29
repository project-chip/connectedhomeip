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
#include <app/codegen-data-model-provider/CodegenDataModelProvider.h>

#include <optional>
#include <variant>

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueEncoder.h>
#include <app/GlobalAttributes.h>
#include <app/RequiredPrivilege.h>
#include <app/codegen-data-model-provider/EmberMetadata.h>
#include <app/data-model/FabricScoped.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/ember-global-attribute-access-interface.h>
#include <app/util/ember-io-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
namespace {

using namespace chip::app::Compatibility::Internal;
using Protocols::InteractionModel::Status;

/// Attempts to read via an attribute access interface (AAI)
///
/// If it returns a CHIP_ERROR, then this is a FINAL result (i.e. either failure or success).
///
/// If it returns std::nullopt, then there is no AAI to handle the given path
/// and processing should figure out the value otherwise (generally from other ember data)
std::optional<CHIP_ERROR> TryReadViaAccessInterface(const ConcreteAttributePath & path, AttributeAccessInterface * aai,
                                                    AttributeValueEncoder & encoder)
{
    // Processing can happen only if an attribute access interface actually exists..
    if (aai == nullptr)
    {
        return std::nullopt;
    }

    CHIP_ERROR err = aai->Read(path, encoder);

    if (err != CHIP_NO_ERROR)
    {
        // Implementation of 8.4.3.2 of the spec for path expansion
        if (path.mExpanded && (err == CHIP_IM_GLOBAL_STATUS(UnsupportedRead)))
        {
            return CHIP_NO_ERROR;
        }

        return err;
    }

    // If the encoder tried to encode, then a value should have been written.
    //   - if encode, assume DONE (i.e. FINAL CHIP_NO_ERROR)
    //   - if no encode, say that processing must continue
    return encoder.TriedEncode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

/// Metadata of what a ember/pascal short string means (prepended by a u8 length)
struct ShortPascalString
{
    using LengthType                        = uint8_t;
    static constexpr LengthType kNullLength = 0xFF;

    static size_t GetLength(ByteSpan buffer)
    {
        VerifyOrDie(buffer.size() >= 1);
        // NOTE: we do NOT use emberAfStringLength from ember-strings.h because that will result in 0
        //       length for null sizes (i.e. 0xFF is translated to 0 and we do not want that here)
        return buffer[0];
    }
};

/// Metadata of what a ember/pascal LONG string means (prepended by a u16 length)
struct LongPascalString
{
    using LengthType                        = uint16_t;
    static constexpr LengthType kNullLength = 0xFFFF;

    static size_t GetLength(ByteSpan buffer)
    {
        // NOTE: we do NOT use emberAfLongStringLength from ember-strings.h because that will result in 0
        //       length for null sizes (i.e. 0xFFFF is translated to 0 and we do not want that here)
        VerifyOrDie(buffer.size() >= 2);
        const uint8_t * data = buffer.data();
        return Encoding::LittleEndian::Read16(data);
    }
};

// ember assumptions ... should just work
static_assert(sizeof(ShortPascalString::LengthType) == 1);
static_assert(sizeof(LongPascalString::LengthType) == 2);

/// Given a ByteSpan containing data from ember, interpret it
/// as a span of type OUT (i.e. ByteSpan or CharSpan) given a ENCODING
/// where ENCODING is Short or Long pascal strings.
template <class OUT_TYPE, class ENCODING>
std::optional<OUT_TYPE> ExtractEmberString(ByteSpan data)
{
    constexpr size_t kLengthTypeSize = sizeof(typename ENCODING::LengthType);
    VerifyOrDie(kLengthTypeSize <= data.size());
    auto len = ENCODING::GetLength(data);

    if (len == ENCODING::kNullLength)
    {
        return std::nullopt;
    }

    VerifyOrDie(len + sizeof(len) <= data.size());
    return std::make_optional<OUT_TYPE>(reinterpret_cast<typename OUT_TYPE::pointer>(data.data() + kLengthTypeSize), len);
}

/// Encode a value inside `encoder`
///
/// The value encoded will be of type T (e.g. CharSpan or ByteSpan) and it will be decoded
/// via the given ENCODING (i.e. ShortPascalString or LongPascalString)
///
/// isNullable defines if the value of NULL is allowed to be encoded.
template <typename T, class ENCODING>
CHIP_ERROR EncodeStringLike(ByteSpan data, bool isNullable, AttributeValueEncoder & encoder)
{
    std::optional<T> value = ExtractEmberString<T, ENCODING>(data);
    if (!value.has_value())
    {
        if (isNullable)
        {
            return encoder.EncodeNull();
        }
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // encode value as-is
    return encoder.Encode(*value);
}

/// Encodes a numeric data value of type T from the given ember-encoded buffer `data`.
///
/// isNullable defines if the value of NULL is allowed to be encoded.
template <typename T>
CHIP_ERROR EncodeFromSpan(ByteSpan data, bool isNullable, AttributeValueEncoder & encoder)
{
    typename NumericAttributeTraits<T>::StorageType value;

    VerifyOrReturnError(data.size() >= sizeof(value), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(&value, data.data(), sizeof(value));

    if (isNullable && NumericAttributeTraits<T>::IsNullValue(value))
    {
        return encoder.EncodeNull();
    }

    if (!NumericAttributeTraits<T>::CanRepresentValue(isNullable, value))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return encoder.Encode(NumericAttributeTraits<T>::StorageToWorking(value));
}

/// Converts raw ember data from `data` into the encoder
///
/// Uses the attribute `metadata` to determine how the data is encoded into `data` and
/// write a suitable value into `encoder`.
CHIP_ERROR EncodeEmberValue(ByteSpan data, const EmberAfAttributeMetadata * metadata, AttributeValueEncoder & encoder)
{
    VerifyOrReturnError(metadata != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const bool isNullable = metadata->IsNullable();

    switch (AttributeBaseType(metadata->attributeType))
    {
    case ZCL_NO_DATA_ATTRIBUTE_TYPE: // No data
        return encoder.EncodeNull();
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        return EncodeFromSpan<bool>(data, isNullable, encoder);
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        return EncodeFromSpan<uint8_t>(data, isNullable, encoder);
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        return EncodeFromSpan<uint16_t>(data, isNullable, encoder);
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
        return EncodeFromSpan<OddSizedInteger<3, false>>(data, isNullable, encoder);
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        return EncodeFromSpan<uint32_t>(data, isNullable, encoder);
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
        return EncodeFromSpan<OddSizedInteger<5, false>>(data, isNullable, encoder);
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
        return EncodeFromSpan<OddSizedInteger<6, false>>(data, isNullable, encoder);
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
        return EncodeFromSpan<OddSizedInteger<7, false>>(data, isNullable, encoder);
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        return EncodeFromSpan<uint64_t>(data, isNullable, encoder);
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        return EncodeFromSpan<int8_t>(data, isNullable, encoder);
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        return EncodeFromSpan<int16_t>(data, isNullable, encoder);
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
        return EncodeFromSpan<OddSizedInteger<3, true>>(data, isNullable, encoder);
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        return EncodeFromSpan<int32_t>(data, isNullable, encoder);
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
        return EncodeFromSpan<OddSizedInteger<5, true>>(data, isNullable, encoder);
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
        return EncodeFromSpan<OddSizedInteger<6, true>>(data, isNullable, encoder);
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
        return EncodeFromSpan<OddSizedInteger<7, true>>(data, isNullable, encoder);
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        return EncodeFromSpan<int64_t>(data, isNullable, encoder);
    case ZCL_SINGLE_ATTRIBUTE_TYPE: // 32-bit float
        return EncodeFromSpan<float>(data, isNullable, encoder);
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: // 64-bit float
        return EncodeFromSpan<double>(data, isNullable, encoder);
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Char string
        return EncodeStringLike<CharSpan, ShortPascalString>(data, isNullable, encoder);
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        return EncodeStringLike<CharSpan, LongPascalString>(data, isNullable, encoder);
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
        return EncodeStringLike<ByteSpan, ShortPascalString>(data, isNullable, encoder);
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        return EncodeStringLike<ByteSpan, LongPascalString>(data, isNullable, encoder);
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(metadata->attributeType));
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
}

} // namespace

/// separated-out ReadAttribute implementation (given existing complexity)
///
/// Generally will:
///    - validate ACL (only for non-internal requests)
///    - Try to read attribute via the AttributeAccessInterface
///    - Try to read the value from ember RAM storage
DataModel::ActionReturnStatus CodegenDataModelProvider::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                      AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    // ACL check for non-internal requests
    if (!request.operationFlags.Has(DataModel::OperationFlags::kInternal))
    {
        ReturnErrorCodeIf(!request.subjectDescriptor.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

        Access::RequestPath requestPath{ .cluster     = request.path.mClusterId,
                                         .endpoint    = request.path.mEndpointId,
                                         .requestType = Access::RequestType::kAttributeReadRequest,
                                         .entityId    = request.path.mAttributeId };
        CHIP_ERROR err = Access::GetAccessControl().Check(*request.subjectDescriptor, requestPath,
                                                          RequiredPrivilege::ForReadAttribute(request.path));
        if (err != CHIP_NO_ERROR)
        {
            ReturnErrorCodeIf((err != CHIP_ERROR_ACCESS_DENIED) && (err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL), err);

            // Implementation of 8.4.3.2 of the spec for path expansion
            if (request.path.mExpanded)
            {
                return CHIP_NO_ERROR;
            }

            // access denied and access restricted have specific codes for IM
            return err == CHIP_ERROR_ACCESS_DENIED ? CHIP_IM_GLOBAL_STATUS(UnsupportedAccess)
                                                   : CHIP_IM_GLOBAL_STATUS(AccessRestricted);
        }
    }

    auto metadata = Ember::FindAttributeMetadata(request.path);

    // Explicit failure in finding a suitable metadata
    if (const Status * status = std::get_if<Status>(&metadata))
    {
        VerifyOrDie((*status == Status::UnsupportedEndpoint) || //
                    (*status == Status::UnsupportedCluster) ||  //
                    (*status == Status::UnsupportedAttribute));
        return *status;
    }

    // Read via AAI
    std::optional<CHIP_ERROR> aai_result;
    if (const EmberAfCluster ** cluster = std::get_if<const EmberAfCluster *>(&metadata))
    {
        Compatibility::GlobalAttributeReader aai(*cluster);
        aai_result = TryReadViaAccessInterface(request.path, &aai, encoder);
    }
    else
    {
        aai_result = TryReadViaAccessInterface(
            request.path, AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId),
            encoder);
    }
    ReturnErrorCodeIf(aai_result.has_value(), *aai_result);

    if (!std::holds_alternative<const EmberAfAttributeMetadata *>(metadata))
    {
        // if we only got a cluster, this was for a global attribute. We cannot read ember attributes
        // at this point, so give up (although GlobalAttributeReader should have returned something here).
        chipDie();
    }
    const EmberAfAttributeMetadata * attributeMetadata = std::get<const EmberAfAttributeMetadata *>(metadata);

    // At this point, we have to use ember directly to read the data.
    EmberAfAttributeSearchRecord record;
    record.endpoint                            = request.path.mEndpointId;
    record.clusterId                           = request.path.mClusterId;
    record.attributeId                         = request.path.mAttributeId;
    Protocols::InteractionModel::Status status = emAfReadOrWriteAttribute(
        &record, &attributeMetadata, gEmberAttributeIOBufferSpan.data(), static_cast<uint16_t>(gEmberAttributeIOBufferSpan.size()),
        /* write = */ false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
    }

    return EncodeEmberValue(gEmberAttributeIOBufferSpan, attributeMetadata, encoder);
}

} // namespace app
} // namespace chip
