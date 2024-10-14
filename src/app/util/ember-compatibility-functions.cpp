/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
#include <app/util/ember-compatibility-functions.h>

#include <access/AccessControl.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteEventPath.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>
#include <app/RequiredPrivilege.h>
#include <app/reporting/Engine.h>
#include <app/reporting/reporting.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table-detail.h>
#include <app/util/attribute-table.h>
#include <app/util/config.h>
#include <app/util/ember-global-attribute-access-interface.h>
#include <app/util/ember-io-storage.h>
#include <app/util/odd-sized-integers.h>
#include <app/util/util.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/Constants.h>

#include <app-common/zap-generated/attribute-type.h>

#include <zap-generated/endpoint_config.h>

#include <limits>

using chip::Protocols::InteractionModel::Status;

using namespace chip;
using namespace chip::app;
using namespace chip::Access;
using namespace chip::app::Compatibility;
using namespace chip::app::Compatibility::Internal;

namespace chip {
namespace app {
namespace {

template <typename T>
CHIP_ERROR attributeBufferToNumericTlvData(TLV::TLVWriter & writer, bool isNullable)
{
    typename NumericAttributeTraits<T>::StorageType value;
    memcpy(&value, gEmberAttributeIOBufferSpan.data(), sizeof(value));
    TLV::Tag tag = TLV::ContextTag(AttributeDataIB::Tag::kData);
    if (isNullable && NumericAttributeTraits<T>::IsNullValue(value))
    {
        return writer.PutNull(tag);
    }

    if (!NumericAttributeTraits<T>::CanRepresentValue(isNullable, value))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return NumericAttributeTraits<T>::Encode(writer, tag, value);
}

} // anonymous namespace

Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    using Protocols::InteractionModel::Status;

    const EmberAfEndpointType * type = emberAfFindEndpointType(aCommandPath.mEndpointId);
    if (type == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }

    const EmberAfCluster * cluster = emberAfFindClusterInType(type, aCommandPath.mClusterId, CLUSTER_MASK_SERVER);
    if (cluster == nullptr)
    {
        return Status::UnsupportedCluster;
    }

    auto * commandHandler =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(aCommandPath.mEndpointId, aCommandPath.mClusterId);
    if (commandHandler)
    {
        struct Context
        {
            bool commandExists;
            CommandId targetCommand;
        } context{ false, aCommandPath.mCommandId };

        CHIP_ERROR err = commandHandler->EnumerateAcceptedCommands(
            aCommandPath,
            [](CommandId command, void * closure) -> Loop {
                auto * ctx = static_cast<Context *>(closure);
                if (ctx->targetCommand == command)
                {
                    ctx->commandExists = true;
                    return Loop::Break;
                }
                return Loop::Continue;
            },
            &context);

        // We now have three cases:
        // 1) handler returned CHIP_ERROR_NOT_IMPLEMENTED.  In that case we
        //    should fall back to looking at cluster->acceptedCommandList
        // 2) handler returned success.  In that case, the handler is the source
        //    of truth about the set of accepted commands, and
        //    context.commandExists indicates whether a aCommandPath.mCommandId
        //    was in the set, and we should return either Success or
        //    UnsupportedCommand accordingly.
        // 3) Some other status was returned.  In this case we should probably
        //    err on the side of not allowing the command, since we have no idea
        //    whether to allow it or not.
        if (err != CHIP_ERROR_NOT_IMPLEMENTED)
        {
            if (err == CHIP_NO_ERROR)
            {
                return context.commandExists ? Status::Success : Status::UnsupportedCommand;
            }

            return Status::Failure;
        }
    }

    for (const CommandId * cmd = cluster->acceptedCommandList; cmd != nullptr && *cmd != kInvalidCommandId; cmd++)
    {
        if (*cmd == aCommandPath.mCommandId)
        {
            return Status::Success;
        }
    }

    return Status::UnsupportedCommand;
}

namespace {

CHIP_ERROR ReadClusterDataVersion(const ConcreteClusterPath & aConcreteClusterPath, DataVersion & aDataVersion)
{
    DataVersion * version = emberAfDataVersionStorage(aConcreteClusterPath);
    if (version == nullptr)
    {
        ChipLogError(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " not found in ReadClusterDataVersion!",
                     aConcreteClusterPath.mEndpointId, ChipLogValueMEI(aConcreteClusterPath.mClusterId));
        return CHIP_ERROR_NOT_FOUND;
    }
    aDataVersion = *version;
    return CHIP_NO_ERROR;
}

// Helper function for trying to read an attribute value via an
// AttributeAccessInterface.  On failure, the read has failed.  On success, the
// aTriedEncode outparam is set to whether the AttributeAccessInterface tried to encode a value.
CHIP_ERROR ReadViaAccessInterface(const SubjectDescriptor & subjectDescriptor, bool aIsFabricFiltered,
                                  const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                  AttributeEncodeState * aEncoderState, AttributeAccessInterface * aAccessInterface,
                                  bool * aTriedEncode)
{
    AttributeEncodeState state(aEncoderState);
    DataVersion version = 0;
    ReturnErrorOnFailure(ReadClusterDataVersion(aPath, version));
    AttributeValueEncoder valueEncoder(aAttributeReports, subjectDescriptor, aPath, version, aIsFabricFiltered, state);
    CHIP_ERROR err = aAccessInterface->Read(aPath, valueEncoder);

    if (err == CHIP_IM_GLOBAL_STATUS(UnsupportedRead) && aPath.mExpanded)
    {
        //
        // Set this to true to ensure our caller will return immediately without proceeding further.
        //
        *aTriedEncode = true;
        return CHIP_NO_ERROR;
    }

    if (err != CHIP_NO_ERROR)
    {
        // If the err is not CHIP_NO_ERROR, means the encoding was aborted, then the valueEncoder may save its state.
        // The state is used by list chunking feature for now.
        if (aEncoderState != nullptr)
        {
            *aEncoderState = valueEncoder.GetState();
        }
        return err;
    }

    *aTriedEncode = valueEncoder.TriedEncode();
    return CHIP_NO_ERROR;
}

// Determine the appropriate status response for an unsupported attribute for
// the given path.  Must be called when the attribute is known to be unsupported
// (i.e. we found no attribute metadata for it).
Protocols::InteractionModel::Status UnsupportedAttributeStatus(const ConcreteAttributePath & aPath)
{
    using Protocols::InteractionModel::Status;

    const EmberAfEndpointType * type = emberAfFindEndpointType(aPath.mEndpointId);
    if (type == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }

    const EmberAfCluster * cluster = emberAfFindClusterInType(type, aPath.mClusterId, CLUSTER_MASK_SERVER);
    if (cluster == nullptr)
    {
        return Status::UnsupportedCluster;
    }

    // Since we know the attribute is unsupported and the endpoint/cluster are
    // OK, this is the only option left.
    return Status::UnsupportedAttribute;
}

// Will set at most one of the out-params (aAttributeCluster or
// aAttributeMetadata) to non-null.  Both null means attribute not supported,
// aAttributeCluster non-null means this is a supported global attribute that
// does not have metadata.
void FindAttributeMetadata(const ConcreteAttributePath & aPath, const EmberAfCluster ** aAttributeCluster,
                           const EmberAfAttributeMetadata ** aAttributeMetadata)
{
    *aAttributeCluster  = nullptr;
    *aAttributeMetadata = nullptr;

    for (auto & attr : GlobalAttributesNotInMetadata)
    {
        if (attr == aPath.mAttributeId)
        {
            *aAttributeCluster = emberAfFindServerCluster(aPath.mEndpointId, aPath.mClusterId);
            return;
        }
    }

    *aAttributeMetadata = emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
}

} // anonymous namespace

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    for (auto & attr : GlobalAttributesNotInMetadata)
    {
        if (attr == aPath.mAttributeId)
        {
            return (emberAfFindServerCluster(aPath.mEndpointId, aPath.mClusterId) != nullptr);
        }
    }
    return (emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId) != nullptr);
}

CHIP_ERROR ReadSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeEncodeState * apEncoderState)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(aPath.mClusterId), aPath.mEndpointId, ChipLogValueMEI(aPath.mAttributeId), aPath.mExpanded);

    // Check attribute existence. This includes attributes with registered metadata, but also specially handled
    // mandatory global attributes (which just check for cluster on endpoint).

    const EmberAfCluster * attributeCluster            = nullptr;
    const EmberAfAttributeMetadata * attributeMetadata = nullptr;
    FindAttributeMetadata(aPath, &attributeCluster, &attributeMetadata);

    if (attributeCluster == nullptr && attributeMetadata == nullptr)
    {
        return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(UnsupportedAttributeStatus(aPath));
    }

    // Check access control. A failed check will disallow the operation, and may or may not generate an attribute report
    // depending on whether the path was expanded.

    {
        Access::RequestPath requestPath{ .cluster     = aPath.mClusterId,
                                         .endpoint    = aPath.mEndpointId,
                                         .requestType = Access::RequestType::kAttributeReadRequest,
                                         .entityId    = aPath.mAttributeId };
        Access::Privilege requestPrivilege = RequiredPrivilege::ForReadAttribute(aPath);
        CHIP_ERROR err                     = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath, requestPrivilege);
        if (err != CHIP_NO_ERROR)
        {
            ReturnErrorCodeIf((err != CHIP_ERROR_ACCESS_DENIED) && (err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL), err);
            if (aPath.mExpanded)
            {
                return CHIP_NO_ERROR;
            }
            return err == CHIP_ERROR_ACCESS_DENIED ? CHIP_IM_GLOBAL_STATUS(UnsupportedAccess)
                                                   : CHIP_IM_GLOBAL_STATUS(AccessRestricted);
        }
    }

    {
        // Special handling for mandatory global attributes: these are always for attribute list, using a special
        // reader (which can be lightweight constructed even from nullptr).
        GlobalAttributeReader reader(attributeCluster);
        AttributeAccessInterface * attributeOverride = (attributeCluster != nullptr)
            ? &reader
            : AttributeAccessInterfaceRegistry::Instance().Get(aPath.mEndpointId, aPath.mClusterId);
        if (attributeOverride)
        {
            bool triedEncode = false;
            ReturnErrorOnFailure(ReadViaAccessInterface(aSubjectDescriptor, aIsFabricFiltered, aPath, aAttributeReports,
                                                        apEncoderState, attributeOverride, &triedEncode));
            ReturnErrorCodeIf(triedEncode, CHIP_NO_ERROR);
        }
    }

    // Read attribute using Ember, if it doesn't have an override.

    EmberAfAttributeSearchRecord record;
    record.endpoint    = aPath.mEndpointId;
    record.clusterId   = aPath.mClusterId;
    record.attributeId = aPath.mAttributeId;
    Status status      = emAfReadOrWriteAttribute(&record, &attributeMetadata, gEmberAttributeIOBufferSpan.data(),
                                                  static_cast<uint16_t>(gEmberAttributeIOBufferSpan.size()),
                                                  /* write = */ false);

    if (status != Status::Success)
    {
        return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
    }

    // data available, return the corresponding record
    AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
    ReturnErrorOnFailure(aAttributeReports.GetError());

    AttributeDataIB::Builder & attributeDataIBBuilder = attributeReport.CreateAttributeData();
    ReturnErrorOnFailure(attributeDataIBBuilder.GetError());

    DataVersion version = 0;
    ReturnErrorOnFailure(ReadClusterDataVersion(aPath, version));
    attributeDataIBBuilder.DataVersion(version);
    ReturnErrorOnFailure(attributeDataIBBuilder.GetError());

    AttributePathIB::Builder & attributePathIBBuilder = attributeDataIBBuilder.CreatePath();
    ReturnErrorOnFailure(attributeDataIBBuilder.GetError());

    CHIP_ERROR err = attributePathIBBuilder.Endpoint(aPath.mEndpointId)
                         .Cluster(aPath.mClusterId)
                         .Attribute(aPath.mAttributeId)
                         .EndOfAttributePathIB();
    ReturnErrorOnFailure(err);

    TLV::TLVWriter * writer = attributeDataIBBuilder.GetWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_NO_ERROR);

    const EmberAfAttributeType attributeType = attributeMetadata->attributeType;
    const bool isNullable                    = attributeMetadata->IsNullable();
    const TLV::Tag tag                       = TLV::ContextTag(AttributeDataIB::Tag::kData);

    switch (AttributeBaseType(attributeType))
    {
    case ZCL_NO_DATA_ATTRIBUTE_TYPE: // No data
        ReturnErrorOnFailure(writer->PutNull(tag));
        break;
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<bool>(*writer, isNullable));
        break;
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint8_t>(*writer, isNullable));
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint16_t>(*writer, isNullable));
        break;
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
    {
        using IntType = OddSizedInteger<3, false>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint32_t>(*writer, isNullable));
        break;
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
    {
        using IntType = OddSizedInteger<5, false>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
    {
        using IntType = OddSizedInteger<6, false>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
    {
        using IntType = OddSizedInteger<7, false>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint64_t>(*writer, isNullable));
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<int8_t>(*writer, isNullable));
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<int16_t>(*writer, isNullable));
        break;
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
    {
        using IntType = OddSizedInteger<3, true>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<int32_t>(*writer, isNullable));
        break;
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
    {
        using IntType = OddSizedInteger<5, true>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
    {
        using IntType = OddSizedInteger<6, true>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
    {
        using IntType = OddSizedInteger<7, true>;
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
        break;
    }
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<int64_t>(*writer, isNullable));
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE: // 32-bit float
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<float>(*writer, isNullable));
        break;
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: // 64-bit float
        ReturnErrorOnFailure(attributeBufferToNumericTlvData<double>(*writer, isNullable));
        break;
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Char string
    {
        char * actualData  = reinterpret_cast<char *>(gEmberAttributeIOBufferSpan.data() + 1);
        uint8_t dataLength = gEmberAttributeIOBufferSpan[0];
        if (dataLength == 0xFF)
        {
            VerifyOrReturnError(isNullable, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(writer->PutNull(tag));
        }
        else
        {
            ReturnErrorOnFailure(writer->PutString(tag, actualData, dataLength));
        }
        break;
    }
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE: {
        char * actualData =
            reinterpret_cast<char *>(gEmberAttributeIOBufferSpan.data() + 2); // The pascal string contains 2 bytes length
        uint16_t dataLength;
        memcpy(&dataLength, gEmberAttributeIOBufferSpan.data(), sizeof(dataLength));
        if (dataLength == 0xFFFF)
        {
            VerifyOrReturnError(isNullable, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(writer->PutNull(tag));
        }
        else
        {
            ReturnErrorOnFailure(writer->PutString(tag, actualData, dataLength));
        }
        break;
    }
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
    {
        uint8_t * actualData = gEmberAttributeIOBufferSpan.data() + 1;
        uint8_t dataLength   = gEmberAttributeIOBufferSpan[0];
        if (dataLength == 0xFF)
        {
            VerifyOrReturnError(isNullable, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(writer->PutNull(tag));
        }
        else
        {
            ReturnErrorOnFailure(writer->Put(tag, chip::ByteSpan(actualData, dataLength)));
        }
        break;
    }
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: {
        uint8_t * actualData = gEmberAttributeIOBufferSpan.data() + 2; // The pascal string contains 2 bytes length
        uint16_t dataLength;
        memcpy(&dataLength, gEmberAttributeIOBufferSpan.data(), sizeof(dataLength));
        if (dataLength == 0xFFFF)
        {
            VerifyOrReturnError(isNullable, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(writer->PutNull(tag));
        }
        else
        {
            ReturnErrorOnFailure(writer->Put(tag, chip::ByteSpan(actualData, dataLength)));
        }
        break;
    }
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(attributeType));
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }

    // If we got this far, placing the data to be read in the output TLVWriter succeeded.
    // Try to terminate our attribute report to signal success.
    ReturnErrorOnFailure(attributeDataIBBuilder.EndOfAttributeDataIB());
    return attributeReport.EndOfAttributeReportIB();
}

namespace {

template <typename T>
CHIP_ERROR numericTlvDataToAttributeBuffer(TLV::TLVReader & aReader, bool isNullable, uint16_t & dataLen)
{
    typename NumericAttributeTraits<T>::StorageType value;
    VerifyOrDie(sizeof(value) <= gEmberAttributeIOBufferSpan.size());

    if (isNullable && aReader.GetType() == TLV::kTLVType_Null)
    {
        NumericAttributeTraits<T>::SetNull(value);
    }
    else
    {
        typename NumericAttributeTraits<T>::WorkingType val;
        ReturnErrorOnFailure(aReader.Get(val));
        VerifyOrReturnError(NumericAttributeTraits<T>::CanRepresentValue(isNullable, val), CHIP_ERROR_INVALID_ARGUMENT);
        NumericAttributeTraits<T>::WorkingToStorage(val, value);
    }
    dataLen = sizeof(value);
    memcpy(gEmberAttributeIOBufferSpan.data(), &value, sizeof(value));
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR stringTlvDataToAttributeBuffer(TLV::TLVReader & aReader, bool isOctetString, bool isNullable, uint16_t & dataLen)
{
    const uint8_t * data = nullptr;
    T len;
    if (isNullable && aReader.GetType() == TLV::kTLVType_Null)
    {
        // Null is represented by an 0xFF or 0xFFFF length, respectively.
        len = std::numeric_limits<T>::max();
        memcpy(gEmberAttributeIOBufferSpan.data(), &len, sizeof(len));
        dataLen = sizeof(len);
    }
    else
    {
        VerifyOrReturnError((isOctetString && aReader.GetType() == TLV::TLVType::kTLVType_ByteString) ||
                                (!isOctetString && aReader.GetType() == TLV::TLVType::kTLVType_UTF8String),
                            CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(CanCastTo<T>(aReader.GetLength()), CHIP_ERROR_MESSAGE_TOO_LONG);
        ReturnErrorOnFailure(aReader.GetDataPtr(data));
        len = static_cast<T>(aReader.GetLength());
        VerifyOrReturnError(len != std::numeric_limits<T>::max(), CHIP_ERROR_MESSAGE_TOO_LONG);
        VerifyOrReturnError(len + sizeof(len) /* length at the beginning of data */ <= gEmberAttributeIOBufferSpan.size(),
                            CHIP_ERROR_MESSAGE_TOO_LONG);
        memcpy(gEmberAttributeIOBufferSpan.data(), &len, sizeof(len));
        memcpy(gEmberAttributeIOBufferSpan.data() + sizeof(len), data, len);
        dataLen = static_cast<uint16_t>(len + sizeof(len));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR prepareWriteData(const EmberAfAttributeMetadata * attributeMetadata, TLV::TLVReader & aReader, uint16_t & dataLen)
{
    EmberAfAttributeType expectedType = AttributeBaseType(attributeMetadata->attributeType);
    bool isNullable                   = attributeMetadata->IsNullable();
    switch (expectedType)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        return numericTlvDataToAttributeBuffer<bool>(aReader, isNullable, dataLen);
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        return numericTlvDataToAttributeBuffer<uint8_t>(aReader, isNullable, dataLen);
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        return numericTlvDataToAttributeBuffer<uint16_t>(aReader, isNullable, dataLen);
    case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
    {
        using IntType = OddSizedInteger<3, false>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        return numericTlvDataToAttributeBuffer<uint32_t>(aReader, isNullable, dataLen);
    case ZCL_INT40U_ATTRIBUTE_TYPE: // Unsigned 40-bit integer
    {
        using IntType = OddSizedInteger<5, false>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT48U_ATTRIBUTE_TYPE: // Unsigned 48-bit integer
    {
        using IntType = OddSizedInteger<6, false>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT56U_ATTRIBUTE_TYPE: // Unsigned 56-bit integer
    {
        using IntType = OddSizedInteger<7, false>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        return numericTlvDataToAttributeBuffer<uint64_t>(aReader, isNullable, dataLen);
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        return numericTlvDataToAttributeBuffer<int8_t>(aReader, isNullable, dataLen);
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        return numericTlvDataToAttributeBuffer<int16_t>(aReader, isNullable, dataLen);
    case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
    {
        using IntType = OddSizedInteger<3, true>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        return numericTlvDataToAttributeBuffer<int32_t>(aReader, isNullable, dataLen);
    case ZCL_INT40S_ATTRIBUTE_TYPE: // Signed 40-bit integer
    {
        using IntType = OddSizedInteger<5, true>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT48S_ATTRIBUTE_TYPE: // Signed 48-bit integer
    {
        using IntType = OddSizedInteger<6, true>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT56S_ATTRIBUTE_TYPE: // Signed 56-bit integer
    {
        using IntType = OddSizedInteger<7, true>;
        return numericTlvDataToAttributeBuffer<IntType>(aReader, isNullable, dataLen);
    }
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        return numericTlvDataToAttributeBuffer<int64_t>(aReader, isNullable, dataLen);
    case ZCL_SINGLE_ATTRIBUTE_TYPE: // 32-bit float
        return numericTlvDataToAttributeBuffer<float>(aReader, isNullable, dataLen);
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: // 64-bit float
        return numericTlvDataToAttributeBuffer<double>(aReader, isNullable, dataLen);
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:  // Char string
        return stringTlvDataToAttributeBuffer<uint8_t>(aReader, expectedType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE, isNullable,
                                                       dataLen);
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: // Long octet string
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:  // Long char string
        return stringTlvDataToAttributeBuffer<uint16_t>(aReader, expectedType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, isNullable,
                                                        dataLen);
    default:
        ChipLogError(DataManagement, "Attribute type %x not handled", static_cast<int>(expectedType));
        return CHIP_ERROR_INVALID_DATA_LIST;
    }
}
} // namespace

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aPath)
{
    return emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
}

CHIP_ERROR WriteSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    // Check attribute existence. This includes attributes with registered metadata, but also specially handled
    // mandatory global attributes (which just check for cluster on endpoint).
    const EmberAfCluster * attributeCluster            = nullptr;
    const EmberAfAttributeMetadata * attributeMetadata = nullptr;
    FindAttributeMetadata(aPath, &attributeCluster, &attributeMetadata);

    if (attributeCluster == nullptr && attributeMetadata == nullptr)
    {
        return apWriteHandler->AddStatus(aPath, UnsupportedAttributeStatus(aPath));
    }

    // All the global attributes we don't have metadata for are readonly.
    if (attributeMetadata == nullptr || attributeMetadata->IsReadOnly())
    {
        return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::UnsupportedWrite);
    }

    {
        Access::RequestPath requestPath{ .cluster     = aPath.mClusterId,
                                         .endpoint    = aPath.mEndpointId,
                                         .requestType = Access::RequestType::kAttributeWriteRequest,
                                         .entityId    = aPath.mAttributeId };
        Access::Privilege requestPrivilege = RequiredPrivilege::ForWriteAttribute(aPath);
        CHIP_ERROR err                     = CHIP_NO_ERROR;
        if (!apWriteHandler->ACLCheckCacheHit({ aPath, requestPrivilege }))
        {
            err = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath, requestPrivilege);
        }
        if (err != CHIP_NO_ERROR)
        {
            ReturnErrorCodeIf((err != CHIP_ERROR_ACCESS_DENIED) && (err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL), err);
            // TODO: when wildcard/group writes are supported, handle them to discard rather than fail with status
            return apWriteHandler->AddStatus(aPath,
                                             err == CHIP_ERROR_ACCESS_DENIED
                                                 ? Protocols::InteractionModel::Status::UnsupportedAccess
                                                 : Protocols::InteractionModel::Status::AccessRestricted);
        }
        apWriteHandler->CacheACLCheckResult({ aPath, requestPrivilege });
    }

    if (attributeMetadata->MustUseTimedWrite() && !apWriteHandler->IsTimedWrite())
    {
        return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::NeedsTimedInteraction);
    }

    if (aPath.mDataVersion.HasValue() && !IsClusterDataVersionEqual(aPath, aPath.mDataVersion.Value()))
    {
        ChipLogError(DataManagement, "Write Version mismatch for Endpoint %x, Cluster " ChipLogFormatMEI, aPath.mEndpointId,
                     ChipLogValueMEI(aPath.mClusterId));
        return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::DataVersionMismatch);
    }

    if (auto * attrOverride = AttributeAccessInterfaceRegistry::Instance().Get(aPath.mEndpointId, aPath.mClusterId))
    {
        AttributeValueDecoder valueDecoder(aReader, aSubjectDescriptor);
        ReturnErrorOnFailure(attrOverride->Write(aPath, valueDecoder));

        if (valueDecoder.TriedDecode())
        {
            MatterReportingAttributeChangeCallback(aPath);
            return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
        }
    }

    CHIP_ERROR preparationError = CHIP_NO_ERROR;
    uint16_t dataLen            = 0;
    if ((preparationError = prepareWriteData(attributeMetadata, aReader, dataLen)) != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "Failed to prepare data to write: %" CHIP_ERROR_FORMAT, preparationError.Format());
        return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::InvalidValue);
    }

    if (dataLen > attributeMetadata->size)
    {
        ChipLogDetail(Zcl, "Data to write exceedes the attribute size claimed.");
        return apWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::InvalidValue);
    }

    auto status = emAfWriteAttributeExternal(
        aPath, EmberAfWriteDataInput(gEmberAttributeIOBufferSpan.data(), attributeMetadata->attributeType));
    return apWriteHandler->AddStatus(aPath, status);
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    DataVersion * version = emberAfDataVersionStorage(aConcreteClusterPath);
    if (version == nullptr)
    {
        ChipLogError(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " not found in IsClusterDataVersionEqual!",
                     aConcreteClusterPath.mEndpointId, ChipLogValueMEI(aConcreteClusterPath.mClusterId));
        return false;
    }

    return (*(version)) == aRequiredVersion;
}

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    CHIP_ERROR err;
    auto deviceTypeList = emberAfDeviceTypeListFromEndpoint(endpoint, err);
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    for (auto & device : deviceTypeList)
    {
        if (device.deviceId == deviceType)
        {
            return true;
        }
    }

    return false;
}

Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath)
{
    using Protocols::InteractionModel::Status;

    const EmberAfEndpointType * type = emberAfFindEndpointType(aPath.mEndpointId);
    if (type == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }

    const EmberAfCluster * cluster = emberAfFindClusterInType(type, aPath.mClusterId, CLUSTER_MASK_SERVER);
    if (cluster == nullptr)
    {
        return Status::UnsupportedCluster;
    }

    // No way to tell. Just claim supported.
    return Status::Success;
}

} // namespace app
} // namespace chip
