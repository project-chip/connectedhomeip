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
#include <optional>

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueEncoder.h>
#include <app/GlobalAttributes.h>
#include <app/RequiredPrivilege.h>
#include <app/codegen-interaction-model/Model.h>
#include <app/data-model/FabricScoped.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/ember-global-attribute-access-interface.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
#if 0
namespace DataModel {

// Ensure odd sized integers are not considered fabric scoped for the purpose of
// encoding via AttributeValueEncoder (they are just integers)
template <int ByteSize, bool IsSigned>
class IsFabricScoped<OddSizedInteger<ByteSize, IsSigned>>
{
public:
    static constexpr bool value = false;
};
} // namespace DataModel
#endif

namespace CodegenDataModel {
namespace {
// On some apps, ATTRIBUTE_LARGEST can as small as 3, making compiler unhappy since data[kAttributeReadBufferSize] cannot hold
// uint64_t. Make kAttributeReadBufferSize at least 8 so it can fit all basic types.
constexpr size_t kAttributeReadBufferSize = (ATTRIBUTE_LARGEST >= 8 ? ATTRIBUTE_LARGEST : 8);
uint8_t attributeReadBufferSpace[kAttributeReadBufferSize];

// BasicType maps the type to basic int(8|16|32|64)(s|u) types.
// TODO: this code should be SHARED!
EmberAfAttributeType BaseType(EmberAfAttributeType type)
{
    switch (type)
    {
    case ZCL_ACTION_ID_ATTRIBUTE_TYPE:  // Action Id
    case ZCL_FABRIC_IDX_ATTRIBUTE_TYPE: // Fabric Index
    case ZCL_BITMAP8_ATTRIBUTE_TYPE:    // 8-bit bitmap
    case ZCL_ENUM8_ATTRIBUTE_TYPE:      // 8-bit enumeration
    case ZCL_STATUS_ATTRIBUTE_TYPE:     // Status Code
    case ZCL_PERCENT_ATTRIBUTE_TYPE:    // Percentage
        static_assert(std::is_same<chip::Percent, uint8_t>::value,
                      "chip::Percent is expected to be uint8_t, change this when necessary");
        return ZCL_INT8U_ATTRIBUTE_TYPE;

    case ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE:   // Endpoint Number
    case ZCL_GROUP_ID_ATTRIBUTE_TYPE:      // Group Id
    case ZCL_VENDOR_ID_ATTRIBUTE_TYPE:     // Vendor Id
    case ZCL_ENUM16_ATTRIBUTE_TYPE:        // 16-bit enumeration
    case ZCL_BITMAP16_ATTRIBUTE_TYPE:      // 16-bit bitmap
    case ZCL_PERCENT100THS_ATTRIBUTE_TYPE: // 100ths of a percent
        static_assert(std::is_same<chip::EndpointId, uint16_t>::value,
                      "chip::EndpointId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::GroupId, uint16_t>::value,
                      "chip::GroupId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::Percent100ths, uint16_t>::value,
                      "chip::Percent100ths is expected to be uint16_t, change this when necessary");
        return ZCL_INT16U_ATTRIBUTE_TYPE;

    case ZCL_CLUSTER_ID_ATTRIBUTE_TYPE: // Cluster Id
    case ZCL_ATTRIB_ID_ATTRIBUTE_TYPE:  // Attribute Id
    case ZCL_FIELD_ID_ATTRIBUTE_TYPE:   // Field Id
    case ZCL_EVENT_ID_ATTRIBUTE_TYPE:   // Event Id
    case ZCL_COMMAND_ID_ATTRIBUTE_TYPE: // Command Id
    case ZCL_TRANS_ID_ATTRIBUTE_TYPE:   // Transaction Id
    case ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE: // Device Type Id
    case ZCL_DATA_VER_ATTRIBUTE_TYPE:   // Data Version
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:   // 32-bit bitmap
    case ZCL_EPOCH_S_ATTRIBUTE_TYPE:    // Epoch Seconds
    case ZCL_ELAPSED_S_ATTRIBUTE_TYPE:  // Elapsed Seconds
        static_assert(std::is_same<chip::ClusterId, uint32_t>::value,
                      "chip::Cluster is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::EventId, uint32_t>::value,
                      "chip::EventId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::CommandId, uint32_t>::value,
                      "chip::CommandId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::TransactionId, uint32_t>::value,
                      "chip::TransactionId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DeviceTypeId, uint32_t>::value,
                      "chip::DeviceTypeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DataVersion, uint32_t>::value,
                      "chip::DataVersion is expected to be uint32_t, change this when necessary");
        return ZCL_INT32U_ATTRIBUTE_TYPE;

    case ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE: // Amperage milliamps
    case ZCL_ENERGY_MWH_ATTRIBUTE_TYPE:  // Energy milliwatt-hours
    case ZCL_POWER_MW_ATTRIBUTE_TYPE:    // Power milliwatts
    case ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE:  // Voltage millivolts
        return ZCL_INT64S_ATTRIBUTE_TYPE;

    case ZCL_EVENT_NO_ATTRIBUTE_TYPE:   // Event Number
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE:  // Fabric Id
    case ZCL_NODE_ID_ATTRIBUTE_TYPE:    // Node Id
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:   // 64-bit bitmap
    case ZCL_EPOCH_US_ATTRIBUTE_TYPE:   // Epoch Microseconds
    case ZCL_POSIX_MS_ATTRIBUTE_TYPE:   // POSIX Milliseconds
    case ZCL_SYSTIME_MS_ATTRIBUTE_TYPE: // System time Milliseconds
    case ZCL_SYSTIME_US_ATTRIBUTE_TYPE: // System time Microseconds
        static_assert(std::is_same<chip::EventNumber, uint64_t>::value,
                      "chip::EventNumber is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::FabricId, uint64_t>::value,
                      "chip::FabricId is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::NodeId, uint64_t>::value,
                      "chip::NodeId is expected to be uint64_t, change this when necessary");
        return ZCL_INT64U_ATTRIBUTE_TYPE;

    case ZCL_TEMPERATURE_ATTRIBUTE_TYPE: // Temperature
        return ZCL_INT16S_ATTRIBUTE_TYPE;

    default:
        return type;
    }
}

// Will set at most one of the out-params (aAttributeCluster or
// aAttributeMetadata) to non-null.  Both null means attribute not supported,
// aAttributeCluster non-null means this is a supported global attribute that
// does not have metadata.
CHIP_ERROR FindAttributeMetadata(const ConcreteAttributePath & aPath, const EmberAfCluster ** aAttributeCluster,
                                 const EmberAfAttributeMetadata ** aAttributeMetadata)
{
    *aAttributeCluster  = nullptr;
    *aAttributeMetadata = nullptr;

    for (auto & attr : GlobalAttributesNotInMetadata)
    {
        if (attr == aPath.mAttributeId)
        {
            *aAttributeCluster = emberAfFindServerCluster(aPath.mEndpointId, aPath.mClusterId);
            return CHIP_NO_ERROR;
        }
    }

    *aAttributeMetadata = emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (*aAttributeMetadata == nullptr)
    {
        const EmberAfEndpointType * type = emberAfFindEndpointType(aPath.mEndpointId);
        if (type == nullptr)
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
        }

        const EmberAfCluster * cluster = emberAfFindClusterInType(type, aPath.mClusterId, CLUSTER_MASK_SERVER);
        if (cluster == nullptr)
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
        }

        // Since we know the attribute is unsupported and the endpoint/cluster are
        // OK, this is the only option left.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckAccessPrivilege(const ConcreteAttributePath & path, const chip::Access::SubjectDescriptor & descriptor)
{
    Access::RequestPath requestPath{ .cluster = path.mClusterId, .endpoint = path.mEndpointId };
    Access::Privilege requestPrivilege = RequiredPrivilege::ForReadAttribute(path);
    CHIP_ERROR err                     = Access::GetAccessControl().Check(descriptor, requestPath, requestPrivilege);

    // access denied sent as-is
    ReturnErrorCodeIf(err == CHIP_ERROR_ACCESS_DENIED, CHIP_ERROR_ACCESS_DENIED);

    // anything else is an opaque failure
    ReturnErrorCodeIf(err != CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));

    return CHIP_NO_ERROR;
}

/// Attempts to read via an attribute access interface (AAI)
///
/// Expected returns:
///   - CHIP_IM_GLOBAL_STATUS(UnsupportedRead) IF AND ONLY IF processing denied by the AAI (considered final)
///   -
///
/// If it returns a VALUE, then this is a FINAL result (i.e. either failure or success).
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

    // TODO: AAI seems to NEVER be able to return anything else EXCEPT
    // UnsupportedRead to be handled upward

    CHIP_ERROR err = aai->Read(path, encoder);

    // explict translate UnsupportedAccess to Access denied. This is to allow callers to determine a
    // translation for this: usually wildcard subscriptions MAY just ignore these where as direct reads
    // MUST translate them to UnsupportedAccess
    ReturnErrorCodeIf(err == CHIP_IM_GLOBAL_STATUS(UnsupportedAccess), CHIP_ERROR_ACCESS_DENIED);

    if (err != CHIP_NO_ERROR)
    {
        return std::make_optional(err);
    }

    // if no attempt was made to encode anything, assume the AAI did not even
    // try to handle it, so handling has to be deferred
    return encoder.TriedEncode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

static constexpr uint8_t kEmberStringNullLength      = 0xFF;
static constexpr uint16_t kEmberLongStringNullLength = 0xFFFF;

template <class T>
std::optional<T> ExtractEmberShortString(ByteSpan data)
{
    uint8_t len = data[0];

    if (len == kEmberStringNullLength)
    {
        return std::nullopt;
    }
    VerifyOrDie(static_cast<size_t>(len + 1) <= data.size());

    return std::make_optional<T>(reinterpret_cast<typename T::pointer>(data.data() + 1), len);
}

template <class T>
std::optional<T> ExtractEmberLongString(ByteSpan data)
{
    uint16_t len;

    VerifyOrDie(sizeof(len) <= data.size());
    memcpy(&len, data.data(), sizeof(len));

    if (len == kEmberLongStringNullLength)
    {
        return std::nullopt;
    }
    VerifyOrDie(static_cast<size_t>(len + 1) <= data.size());

    return std::make_optional<T>(reinterpret_cast<typename T::pointer>(data.data() + 1), len);
}

// TODO: string handling:
//    - length (1 or 2 bytes, null is 0xFF or 0xFFFF)
//    - either Put(ByteSpan) or PutString() -> this seems to need a Encoder equivalent?
//       - Span calls EncodeString
//       - ByteSpan calls Encode
//

template <typename T>
CHIP_ERROR EncodeStringLike(std::optional<T> data, bool isNullable, AttributeValueEncoder & encoder)
{
    if (!data.has_value())
    {
        if (isNullable)
        {
            return encoder.EncodeNull();
        }
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // encode value as-is
    return encoder.Encode(*data);
}

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
CHIP_ERROR EncodeEmberValue(ByteSpan data, const EmberAfAttributeMetadata * metadata, AttributeValueEncoder & encoder)
{
    VerifyOrReturnError(metadata != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const bool isNullable = metadata->IsNullable();

    switch (BaseType(metadata->attributeType))
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
        return EncodeStringLike(ExtractEmberShortString<CharSpan>(data), isNullable, encoder);
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        return EncodeStringLike(ExtractEmberLongString<CharSpan>(data), isNullable, encoder);
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
        return EncodeStringLike(ExtractEmberShortString<ByteSpan>(data), isNullable, encoder);
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        return EncodeStringLike(ExtractEmberLongString<ByteSpan>(data), isNullable, encoder);
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(metadata->attributeType));
        return CHIP_IM_GLOBAL_STATUS(UnsupportedRead);
    }
}

} // namespace

/// separated-out ReadAttribute implementation (given existing complexity)
CHIP_ERROR Model::ReadAttribute(const InteractionModel::ReadAttributeRequest & request, InteractionModel::ReadState & state,
                                AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    const EmberAfCluster * attributeCluster            = nullptr;
    const EmberAfAttributeMetadata * attributeMetadata = nullptr;

    ReturnErrorOnFailure(FindAttributeMetadata(request.path, &attributeCluster, &attributeMetadata));
    VerifyOrDie(attributeMetadata != nullptr); // this is the contract of FindAttributeMetadata

    // ACL check for non-internal requests
    if (!request.operationFlags.Has(InteractionModel::OperationFlags::kInternal))
    {
        ReturnErrorCodeIf(!request.subjectDescriptor.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(CheckAccessPrivilege(request.path, *request.subjectDescriptor));
    }

    std::optional<CHIP_ERROR> aai_result;

    if (attributeCluster != nullptr)
    {
        aai_result = TryReadViaAccessInterface(
            request.path, GetAttributeAccessOverride(request.path.mEndpointId, request.path.mClusterId), encoder);
    }
    else
    {
        Compatibility::GlobalAttributeReader aai(attributeCluster);
        aai_result = TryReadViaAccessInterface(request.path, &aai, encoder);
    }

    if (aai_result.has_value())
    {
        // save the reading state for later use (if lists are being decoded)
        state.listEncodeStart = encoder.GetState().CurrentEncodingListIndex();
        return *aai_result;
    }
    state.listEncodeStart = kInvalidListIndex;

    // At this point, we have to use ember directly to read the data.
    // Available methods:
    //   - emberAfReadOrWriteAttribute should do the processing of read vs write
    //   - we write to a fixed buffer, so the actual type has to be determined
    //     by the attributeType (base type)

    // TODO: ember reading
    //
    EmberAfAttributeSearchRecord record;
    record.endpoint    = request.path.mEndpointId;
    record.clusterId   = request.path.mClusterId;
    record.attributeId = request.path.mAttributeId;
    Protocols::InteractionModel::Status status =
        emAfReadOrWriteAttribute(&record, &attributeMetadata, attributeReadBufferSpace, sizeof(attributeReadBufferSpace),
                                 /* write = */ false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return ChipError(ChipError::SdkPart::kIMGlobalStatus, to_underlying(status), __FILE__, __LINE__);
    }

    return EncodeEmberValue(ByteSpan(attributeReadBufferSpace), attributeMetadata, encoder);
}

} // namespace CodegenDataModel
} // namespace app
} // namespace chip
