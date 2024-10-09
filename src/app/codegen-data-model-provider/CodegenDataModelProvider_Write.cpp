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

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/RequiredPrivilege.h>
#include <app/codegen-data-model-provider/EmberMetadata.h>
#include <app/data-model/FabricScoped.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table-detail.h>
#include <app/util/attribute-table.h>
#include <app/util/ember-io-storage.h>
#include <app/util/ember-strings.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
namespace {

using namespace chip::app::Compatibility::Internal;
using Protocols::InteractionModel::Status;

class ContextAttributesChangeListener : public AttributesChangedListener
{
public:
    ContextAttributesChangeListener(const DataModel::InteractionModelContext & context) : mListener(context.dataModelChangeListener)
    {}
    void MarkDirty(const AttributePathParams & path) override { mListener->MarkDirty(path); }

private:
    DataModel::ProviderChangeListener * mListener;
};

/// Attempts to write via an attribute access interface (AAI)
///
/// If it returns a CHIP_ERROR, then this is a FINAL result (i.e. either failure or success)
///
/// If it returns std::nullopt, then there is no AAI to handle the given path
/// and processing should figure out the value otherwise (generally from other ember data)
std::optional<CHIP_ERROR> TryWriteViaAccessInterface(const ConcreteDataAttributePath & path, AttributeAccessInterface * aai,
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
    //   -  otherwise, if no decoding done, return that processing must continue via nullopt
    return decoder.TriedDecode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

/// Metadata of what a ember/pascal short string means (prepended by a u8 length)
struct ShortPascalString
{
    using LengthType                        = uint8_t;
    static constexpr LengthType kNullLength = 0xFF;

    static void SetLength(uint8_t * buffer, LengthType value) { *buffer = value; }
};

/// Metadata of what a ember/pascal LONG string means (prepended by a u16 length)
struct LongPascalString
{
    using LengthType                        = uint16_t;
    static constexpr LengthType kNullLength = 0xFFFF;

    // Encoding for ember string lengths is little-endian (see ember-strings.cpp)
    static void SetLength(uint8_t * buffer, LengthType value) { Encoding::LittleEndian::Put16(buffer, value); }
};

// ember assumptions ... should just work
static_assert(sizeof(ShortPascalString::LengthType) == 1);
static_assert(sizeof(LongPascalString::LengthType) == 2);

/// Convert the value stored in 'decoder' into an ember format span 'out'
///
/// The value converted will be of type T (e.g. CharSpan or ByteSpan) and it will be converted
/// via the given ENCODING (i.e. ShortPascalString or LongPascalString)
///
/// isNullable defines if the value of NULL is allowed to be converted.
template <typename T, class ENCODING>
CHIP_ERROR DecodeStringLikeIntoEmberBuffer(AttributeValueDecoder decoder, bool isNullable, MutableByteSpan & out)
{
    T workingValue;

    if (isNullable)
    {
        typename DataModel::Nullable<T> nullableWorkingValue;
        ReturnErrorOnFailure(decoder.Decode(nullableWorkingValue));

        if (nullableWorkingValue.IsNull())
        {
            VerifyOrReturnError(out.size() >= sizeof(typename ENCODING::LengthType), CHIP_ERROR_BUFFER_TOO_SMALL);
            ENCODING::SetLength(out.data(), ENCODING::kNullLength);
            out.reduce_size(sizeof(typename ENCODING::LengthType));
            return CHIP_NO_ERROR;
        }

        // continue encoding non-null value
        workingValue = nullableWorkingValue.Value();
    }
    else
    {
        ReturnErrorOnFailure(decoder.Decode(workingValue));
    }

    auto len = static_cast<typename ENCODING::LengthType>(workingValue.size());
    VerifyOrReturnError(out.size() >= sizeof(len) + len, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t * output_buffer = out.data();

    ENCODING::SetLength(output_buffer, len);
    output_buffer += sizeof(len);

    memcpy(output_buffer, workingValue.data(), workingValue.size());
    output_buffer += workingValue.size();

    out.reduce_size(static_cast<size_t>(output_buffer - out.data()));
    return CHIP_NO_ERROR;
}

/// Decodes a numeric data value of type T from the `decoder` into a ember-encoded buffer `out`
///
/// isNullable defines if the value of NULL is allowed to be decoded.
template <typename T>
CHIP_ERROR DecodeIntoEmberBuffer(AttributeValueDecoder & decoder, bool isNullable, MutableByteSpan & out)
{
    using Traits = NumericAttributeTraits<T>;
    typename Traits::StorageType storageValue;

    if (isNullable)
    {
        DataModel::Nullable<typename Traits::WorkingType> workingValue;
        ReturnErrorOnFailure(decoder.Decode(workingValue));

        if (workingValue.IsNull())
        {
            Traits::SetNull(storageValue);
        }
        else
        {
            // This guards against trying to decode something that overlaps nullable, for example
            // Nullable<uint8_t>(0xFF) is not representable because 0xFF is the encoding of NULL in ember
            // as well as odd-sized integers (e.g. full 32-bit value like 0x11223344 cannot be written
            // to a 3-byte odd-sized integger).
            VerifyOrReturnError(Traits::CanRepresentValue(isNullable, workingValue.Value()), CHIP_ERROR_INVALID_ARGUMENT);
            Traits::WorkingToStorage(workingValue.Value(), storageValue);
        }

        VerifyOrReturnError(out.size() >= sizeof(storageValue), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        typename Traits::WorkingType workingValue;
        ReturnErrorOnFailure(decoder.Decode(workingValue));

        Traits::WorkingToStorage(workingValue, storageValue);

        VerifyOrReturnError(out.size() >= sizeof(storageValue), CHIP_ERROR_INVALID_ARGUMENT);

        // Even non-nullable values may be outside range: e.g. odd-sized integers have working values
        // that are larger than the storage values (e.g. a uint32_t being stored as a 3-byte integer)
        VerifyOrReturnError(Traits::CanRepresentValue(isNullable, workingValue), CHIP_ERROR_INVALID_ARGUMENT);
    }

    const uint8_t * data = Traits::ToAttributeStoreRepresentation(storageValue);

    // The decoding + ToAttributeStoreRepresentation will result in data being
    // stored in native format/byteorder, suitable to directly be stored in the data store
    memcpy(out.data(), data, sizeof(storageValue));
    out.reduce_size(sizeof(storageValue));

    return CHIP_NO_ERROR;
}

/// Read the data from "decoder" into an ember-formatted buffer "out"
///
/// `out` is a in/out buffer:
///    - its initial size determines the maximum size of the buffer
///    - its output size reflects the actual data size
///
/// Uses the attribute `metadata` to determine how the data is to be encoded into out.
CHIP_ERROR DecodeValueIntoEmberBuffer(AttributeValueDecoder & decoder, const EmberAfAttributeMetadata * metadata,
                                      MutableByteSpan & out)
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
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
}

} // namespace

DataModel::ActionReturnStatus CodegenDataModelProvider::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                       AttributeValueDecoder & decoder)
{
    ChipLogDetail(DataManagement, "Writing attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI,
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId));

    // TODO: ordering is to check writability/existence BEFORE ACL and this seems wrong, however
    //       existing unit tests (TC_AcessChecker.py) validate that we get UnsupportedWrite instead of UnsupportedAccess
    //
    //       This should likely be fixed in spec (probably already fixed by
    //       https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/9024)
    //       and tests and implementation
    //
    //       Open issue that needs fixing: https://github.com/project-chip/connectedhomeip/issues/33735
    auto metadata = Ember::FindAttributeMetadata(request.path);

    // Explicit failure in finding a suitable metadata
    if (const Status * status = std::get_if<Status>(&metadata))
    {
        VerifyOrDie((*status == Status::UnsupportedEndpoint) || //
                    (*status == Status::UnsupportedCluster) ||  //
                    (*status == Status::UnsupportedAttribute));
        return *status;
    }

    const EmberAfAttributeMetadata ** attributeMetadata = std::get_if<const EmberAfAttributeMetadata *>(&metadata);

    // All the global attributes that we do not have metadata for are
    // read-only. Specifically only the following list-based attributes match the
    // "global attributes not in metadata" (see GlobalAttributes.h :: GlobalAttributesNotInMetadata):
    //   - AttributeList
    //   - EventList
    //   - AcceptedCommands
    //   - GeneratedCommands
    //
    // Given the above, UnsupportedWrite should be correct (attempt to write to a read-only list)
    bool isReadOnly = (attributeMetadata == nullptr) || (*attributeMetadata)->IsReadOnly();

    // Internal is allowed to bypass timed writes and read-only.
    if (!request.operationFlags.Has(DataModel::OperationFlags::kInternal))
    {
        VerifyOrReturnError(!isReadOnly, Status::UnsupportedWrite);
    }

    // ACL check for non-internal requests
    bool checkAcl = !request.operationFlags.Has(DataModel::OperationFlags::kInternal);

    // For chunking, ACL check is not re-done if the previous write was successful for the exact same
    // path. We apply this everywhere as a shortcut, although realistically this is only for AccessControl cluster
    if (checkAcl && request.previousSuccessPath.has_value())
    {
        // NOTE: explicit cast/check only for attribute path and nothing else.
        //
        //       In particular `request.path` is a DATA path (contains a list index)
        //       and we do not want request.previousSuccessPath to be auto-cast to a
        //       data path with a empty list and fail the compare.
        //
        //       This could be `request.previousSuccessPath != request.path` (where order
        //       is important) however that would seem more brittle (relying that a != b
        //       behaves differently than b != a due to casts). Overall Data paths are not
        //       the same as attribute paths.
        //
        //       Also note that Concrete path have a mExpanded that is not used in compares.
        const ConcreteAttributePath & attributePathA = request.path;
        const ConcreteAttributePath & attributePathB = *request.previousSuccessPath;

        checkAcl = (attributePathA != attributePathB);
    }

    if (checkAcl)
    {
        ReturnErrorCodeIf(!request.subjectDescriptor.has_value(), Status::UnsupportedAccess);

        Access::RequestPath requestPath{ .cluster     = request.path.mClusterId,
                                         .endpoint    = request.path.mEndpointId,
                                         .requestType = Access::RequestType::kAttributeWriteRequest,
                                         .entityId    = request.path.mAttributeId };
        CHIP_ERROR err = Access::GetAccessControl().Check(*request.subjectDescriptor, requestPath,
                                                          RequiredPrivilege::ForWriteAttribute(request.path));

        if (err != CHIP_NO_ERROR)
        {
            VerifyOrReturnValue(err != CHIP_ERROR_ACCESS_DENIED, Status::UnsupportedAccess);
            VerifyOrReturnValue(err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL, Status::AccessRestricted);

            return err;
        }
    }

    // Internal is allowed to bypass timed writes and read-only.
    if (!request.operationFlags.Has(DataModel::OperationFlags::kInternal))
    {
        VerifyOrReturnError(!(*attributeMetadata)->MustUseTimedWrite() || request.writeFlags.Has(DataModel::WriteFlags::kTimed),
                            Status::NeedsTimedInteraction);
    }

    // Extra check: internal requests can bypass the read only check, however global attributes
    // have no underlying storage, so write still cannot be done
    VerifyOrReturnError(attributeMetadata != nullptr, Status::UnsupportedWrite);

    if (request.path.mDataVersion.HasValue())
    {
        std::optional<DataModel::ClusterInfo> clusterInfo = GetClusterInfo(request.path);
        if (!clusterInfo.has_value())
        {
            ChipLogError(DataManagement, "Unable to get cluster info for Endpoint 0x%x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return Status::DataVersionMismatch;
        }

        if (request.path.mDataVersion.Value() != clusterInfo->dataVersion)
        {
            ChipLogError(DataManagement, "Write Version mismatch for Endpoint 0x%x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return Status::DataVersionMismatch;
        }
    }

    ContextAttributesChangeListener change_listener(CurrentContext());

    AttributeAccessInterface * aai =
        AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId);
    std::optional<CHIP_ERROR> aai_result = TryWriteViaAccessInterface(request.path, aai, decoder);
    if (aai_result.has_value())
    {
        if (*aai_result == CHIP_NO_ERROR)
        {
            // TODO: this is awkward since it provides AAI no control over this, specifically
            //       AAI may not want to increase versions for some attributes that are Q
            emberAfAttributeChanged(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId, &change_listener);
        }
        return *aai_result;
    }

    MutableByteSpan dataBuffer = gEmberAttributeIOBufferSpan;
    ReturnErrorOnFailure(DecodeValueIntoEmberBuffer(decoder, *attributeMetadata, dataBuffer));

    Protocols::InteractionModel::Status status;

    if (dataBuffer.size() > (*attributeMetadata)->size)
    {
        ChipLogDetail(Zcl, "Data to write exceeds the attribute size claimed.");
        return Status::InvalidValue;
    }

    EmberAfWriteDataInput dataInput(dataBuffer.data(), (*attributeMetadata)->attributeType);

    dataInput.SetChangeListener(&change_listener);
    // TODO: dataInput.SetMarkDirty() should be according to `ChangesOmmited`

    if (request.operationFlags.Has(DataModel::OperationFlags::kInternal))
    {
        // Internal requests use the non-External interface that has less enforcement
        // than the external version (e.g. does not check/enforce writable settings, does not
        // validate attribute types) - see attribute-table.h documentation for details.
        status = emberAfWriteAttribute(request.path, dataInput);
    }
    else
    {
        status = emAfWriteAttributeExternal(request.path, dataInput);
    }

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
