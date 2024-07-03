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
#include <app/codegen-data-model/CodegenDataModel.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/RequiredPrivilege.h>
#include <app/codegen-data-model/EmberMetadata.h>
#include <app/data-model/FabricScoped.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/ember-io-storage.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
namespace {

using namespace chip::app::Compatibility::Internal;

/// Attempts to write via an attribute access interface (AAI)
///
/// If it returns a CHIP_ERROR, then this is a FINAL result (i.e. either failure or success)
///
/// If it returns std::nullopt, then there is no AAI to handle the given path
/// and processing should figure out the value otherwise (generally from other ember data)
std::optional<CHIP_ERROR> TryWriteViaAccessInterface(const ConcreteAttributePath & path, AttributeAccessInterface * aai,
                                                     AttributeValueDecoder & decoder)
{
    // Processing can happen only if an attribute access interface actually exists..
    if (aai == nullptr)
    {
        return std::nullopt;
    }

    CHIP_ERROR err = aai->Write(path, decoder);

    if (err != CHIP_NO_ERROR)
    {
        return std::make_optional(err);
    }

    // If the decoder tried to decode, then a value should have been read for processing.
    //   - if decoding was done, assume DONE (i.e. final CHIP_NO_ERROR)
    //   - if no encode, say that processing must continue
    return decoder.TriedDecode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

/// Metadata of what a ember/pascal short string means (prepended by a u8 length)
struct ShortPascalString
{
    using LengthType                        = uint8_t;
    static constexpr LengthType kNullLength = 0xFF;
};

/// Metadata of what a ember/pascal LONG string means (prepended by a u16 length)
struct LongPascalString
{
    using LengthType                        = uint16_t;
    static constexpr LengthType kNullLength = 0xFFFF;
};

// ember assumptions ... should just work
static_assert(sizeof(ShortPascalString::LengthType) == 1);
static_assert(sizeof(LongPascalString::LengthType) == 2);

/// Encode the value stored in 'decoder' into an ember format string 'out'
///
/// The value encoded will be of type T (e.g. CharSpan or ByteSpan) and it will be encoded
/// via the given ENCODING (i.e. ShortPascalString or LongPascalString)
///
/// isNullable defines if the value of NULL is allowed to be encoded.
template <typename T, class ENCODING>
CHIP_ERROR DecodeStringLikeIntoEmberBuffer(AttributeValueDecoder decoder, bool isNullable, MutableByteSpan out)
{
    T workingValue;

    if (isNullable)
    {
        typename DataModel::Nullable<T> nullableWorkingValue;
        ReturnErrorOnFailure(decoder.Decode(nullableWorkingValue));

        if (nullableWorkingValue.IsNull())
        {
            VerifyOrReturnError(out.size() >= sizeof(typename ENCODING::LengthType), CHIP_ERROR_BUFFER_TOO_SMALL);

            typename ENCODING::LengthType nullLength = ENCODING::kNullLength;
            memcpy(out.data(), &nullLength, sizeof(nullLength));
            return CHIP_NO_ERROR;
        }

        // continue encoding non-null value
        workingValue = nullableWorkingValue.Value();
    }
    else
    {
        ReturnErrorOnFailure(decoder.Decode(workingValue));
    }

    typename ENCODING::LengthType len = static_cast<typename ENCODING::LengthType>(workingValue.size());
    VerifyOrReturnError(out.size() >= sizeof(len) + len, CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(out.data(), &len, sizeof(len));
    memcpy(out.data() + sizeof(len), workingValue.data(), workingValue.size());
    return CHIP_NO_ERROR;
}

/// Encodes a numeric data value of type T from the given ember-encoded buffer `data`.
///
/// isNullable defines if the value of NULL is allowed to be encoded.
template <typename T>
CHIP_ERROR DecodeIntoEmberBuffer(AttributeValueDecoder & decoder, bool isNullable, MutableByteSpan out)
{
    if (isNullable)
    {
        using Traits = NumericAttributeTraits<T>;

        DataModel::Nullable<typename Traits::WorkingType> workingValue;
        ReturnErrorOnFailure(decoder.Decode(workingValue));

        typename Traits::StorageType storageValue;
        if (workingValue.IsNull())
        {
            Traits::SetNull(storageValue);
        }
        else
        {
            VerifyOrReturnError(Traits::CanRepresentValue(isNullable, *workingValue), CHIP_ERROR_INVALID_ARGUMENT);
            Traits::WorkingToStorage(*workingValue, storageValue);
        }

        VerifyOrReturnError(out.size() >= sizeof(storageValue), CHIP_ERROR_INVALID_ARGUMENT);

        const uint8_t * data = Traits::ToAttributeStoreRepresentation(storageValue);
        memcpy(out.data(), data, sizeof(storageValue));
    }
    else
    {
        using Traits = NumericAttributeTraits<T>;

        typename Traits::WorkingType workingValue;
        ReturnErrorOnFailure(decoder.Decode(workingValue));

        typename Traits::StorageType storageValue;
        Traits::WorkingToStorage(workingValue, storageValue);

        VerifyOrReturnError(out.size() >= sizeof(storageValue), CHIP_ERROR_INVALID_ARGUMENT);

        // This guards against trying to encode something that overlaps nullable, for example
        // Nullable<uint8_t>(0xFF) is not representable because 0xFF is the encoding of NULL in ember
        VerifyOrReturnError(Traits::CanRepresentValue(isNullable, workingValue), CHIP_ERROR_INVALID_ARGUMENT);

        const uint8_t * data = Traits::ToAttributeStoreRepresentation(storageValue);
        memcpy(out.data(), data, sizeof(storageValue));
    }

    return CHIP_NO_ERROR;
}

/// Read the data from "decoder" into an ember-formatted buffer "out"
///
/// Uses the attribute `metadata` to determine how the data is to be encoded into out.
CHIP_ERROR DecodeValueIntoEmberBuffer(AttributeValueDecoder & decoder, const EmberAfAttributeMetadata * metadata,
                                      MutableByteSpan out)
{
    VerifyOrReturnError(metadata != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const bool isNullable = metadata->IsNullable();

    switch (AttributeBaseType(metadata->attributeType))
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        return DecodeIntoEmberBuffer<bool>(decoder, isNullable, out);
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        return DecodeIntoEmberBuffer<uint8_t>(decoder, isNullable, out);
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        return DecodeIntoEmberBuffer<uint16_t>(decoder, isNullable, out);
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<3, false>>(decoder, isNullable, out);
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        return DecodeIntoEmberBuffer<uint32_t>(decoder, isNullable, out);
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<5, false>>(decoder, isNullable, out);
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<6, false>>(decoder, isNullable, out);
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<7, false>>(decoder, isNullable, out);
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        return DecodeIntoEmberBuffer<uint64_t>(decoder, isNullable, out);
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        return DecodeIntoEmberBuffer<int8_t>(decoder, isNullable, out);
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        return DecodeIntoEmberBuffer<int16_t>(decoder, isNullable, out);
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<3, true>>(decoder, isNullable, out);
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        return DecodeIntoEmberBuffer<int32_t>(decoder, isNullable, out);
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<5, true>>(decoder, isNullable, out);
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<6, true>>(decoder, isNullable, out);
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
        return DecodeIntoEmberBuffer<OddSizedInteger<7, true>>(decoder, isNullable, out);
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        return DecodeIntoEmberBuffer<int64_t>(decoder, isNullable, out);
    case ZCL_SINGLE_ATTRIBUTE_TYPE: // 32-bit float
        return DecodeIntoEmberBuffer<float>(decoder, isNullable, out);
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: // 64-bit float
        return DecodeIntoEmberBuffer<double>(decoder, isNullable, out);
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Char string
        return DecodeStringLikeIntoEmberBuffer<CharSpan, ShortPascalString>(decoder, isNullable, out);
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        return DecodeStringLikeIntoEmberBuffer<CharSpan, LongPascalString>(decoder, isNullable, out);
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
        return DecodeStringLikeIntoEmberBuffer<ByteSpan, ShortPascalString>(decoder, isNullable, out);
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        return DecodeStringLikeIntoEmberBuffer<ByteSpan, LongPascalString>(decoder, isNullable, out);
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(metadata->attributeType));
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }
    return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
}

} // namespace

CHIP_ERROR CodegenDataModel::WriteAttribute(const InteractionModel::WriteAttributeRequest & request,
                                            AttributeValueDecoder & decoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    // ACL check for non-internal requests
    if (!request.operationFlags.Has(InteractionModel::OperationFlags::kInternal))
    {
        ReturnErrorCodeIf(!request.subjectDescriptor.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

        Access::RequestPath requestPath{ .cluster = request.path.mClusterId, .endpoint = request.path.mEndpointId };
        ReturnErrorOnFailure(Access::GetAccessControl().Check(*request.subjectDescriptor, requestPath,
                                                              RequiredPrivilege::ForWriteAttribute(request.path)));
    }

    auto metadata = Ember::FindAttributeMetadata(request.path);

    // Explicit failure in finding a suitable metadata
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&metadata))
    {
        VerifyOrDie(*err != CHIP_NO_ERROR);
        return *err;
    }

    // All the global attributes that we do not have metadata for are
    // read-only (i.e. cannot write atribute_list/event_list/accepted_cmds/generated_cmds)
    //
    // so if no metadata available, we wil lreturn an error
    const EmberAfAttributeMetadata ** attributeMetadata = std::get_if<const EmberAfAttributeMetadata *>(&metadata);
    if (attributeMetadata == nullptr)
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }

    if ((*attributeMetadata)->IsReadOnly() && !request.operationFlags.Has(InteractionModel::OperationFlags::kInternal))
    {
        // Internal is allowed to try to bypass read-only updates, however otherwise we deny read-only
        // updates
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }

    if ((*attributeMetadata)->MustUseTimedWrite() && !request.writeFlags.Has(InteractionModel::WriteFlags::kTimed))
    {
        return CHIP_IM_GLOBAL_STATUS(NeedsTimedInteraction);
    }

    if (request.path.mDataVersion.HasValue())
    {
        std::optional<InteractionModel::ClusterInfo> clusterInfo = GetClusterInfo(request.path);
        if (!clusterInfo.has_value())
        {
            ChipLogError(DataManagement, "Unable to get cluster info for Endpoint %x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return CHIP_IM_GLOBAL_STATUS(DataVersionMismatch);
        }

        if (request.path.mDataVersion.Value() != clusterInfo->dataVersion)
        {
            ChipLogError(DataManagement, "Write Version mismatch for Endpoint %x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return CHIP_IM_GLOBAL_STATUS(DataVersionMismatch);
        }
    }

    std::optional<CHIP_ERROR> aai_result = TryWriteViaAccessInterface(
        request.path, GetAttributeAccessOverride(request.path.mEndpointId, request.path.mClusterId), decoder);
    ReturnErrorCodeIf(aai_result.has_value(), *aai_result);

    ReturnErrorOnFailure(DecodeValueIntoEmberBuffer(decoder, *attributeMetadata, gEmberAttributeIOBufferSpan));

    EmberAfAttributeSearchRecord record;
    record.endpoint    = request.path.mEndpointId;
    record.clusterId   = request.path.mClusterId;
    record.attributeId = request.path.mAttributeId;

    Protocols::InteractionModel::Status status = emAfReadOrWriteAttribute(
        &record, attributeMetadata, gEmberAttributeIOBufferSpan.data(), static_cast<uint16_t>(gEmberAttributeIOBufferSpan.size()),
        /* write = */ true);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return ChipError(ChipError::SdkPart::kIMGlobalStatus, to_underlying(status), __FILE__, __LINE__);
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
