/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Contains the functions for compatibility with ember ZCL inner state
 *          when calling ember callbacks.
 */

#include <access/AccessControl.h>
#include <app/ClusterInfo.h>
#include <app/Command.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/Engine.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/ember-compatibility-functions.h>
#include <app/util/error-mapping.h>
#include <app/util/util.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <protocols/interaction_model/Constants.h>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>

#include <zap-generated/endpoint_config.h>

#include <limits>

using namespace chip;
using namespace chip::Access;
using namespace chip::app;
using namespace chip::app::Compatibility;

namespace chip {
namespace app {
namespace Compatibility {
namespace {
constexpr uint32_t kTemporaryDataVersion = 0;
// On some apps, ATTRIBUTE_LARGEST can as small as 3, making compiler unhappy since data[kAttributeReadBufferSize] cannot hold
// uint64_t. Make kAttributeReadBufferSize at least 8 so it can fit all basic types.
constexpr size_t kAttributeReadBufferSize = (ATTRIBUTE_LARGEST >= 8 ? ATTRIBUTE_LARGEST : 8);
EmberAfClusterCommand imCompatibilityEmberAfCluster;
EmberApsFrame imCompatibilityEmberApsFrame;
EmberAfInterpanHeader imCompatibilityInterpanHeader;
Command * currentCommandObject;

// BasicType maps the type to basic int(8|16|32|64)(s|u) types.
EmberAfAttributeType BaseType(EmberAfAttributeType type)
{
    switch (type)
    {
    case ZCL_ACTION_ID_ATTRIBUTE_TYPE:  // Action Id
    case ZCL_FABRIC_IDX_ATTRIBUTE_TYPE: // Fabric Index
    case ZCL_BITMAP8_ATTRIBUTE_TYPE:    // 8-bit bitmap
    case ZCL_ENUM8_ATTRIBUTE_TYPE:      // 8-bit enumeration
        return ZCL_INT8U_ATTRIBUTE_TYPE;

    case ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE: // Endpoint Number
    case ZCL_GROUP_ID_ATTRIBUTE_TYPE:    // Group Id
    case ZCL_VENDOR_ID_ATTRIBUTE_TYPE:   // Vendor Id
    case ZCL_ENUM16_ATTRIBUTE_TYPE:      // 16-bit enumeration
    case ZCL_BITMAP16_ATTRIBUTE_TYPE:    // 16-bit bitmap
    case ZCL_STATUS_ATTRIBUTE_TYPE:      // Status Code
        static_assert(std::is_same<chip::EndpointId, uint16_t>::value,
                      "chip::EndpointId is expected to be uint8_t, change this when necessary");
        static_assert(std::is_same<chip::GroupId, uint16_t>::value,
                      "chip::GroupId is expected to be uint16_t, change this when necessary");
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

    case ZCL_EVENT_NO_ATTRIBUTE_TYPE:  // Event Number
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE: // Fabric Id
    case ZCL_NODE_ID_ATTRIBUTE_TYPE:   // Node Id
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:  // 64-bit bitmap
    case ZCL_EPOCH_US_ATTRIBUTE_TYPE:  // Epoch Microseconds
        static_assert(std::is_same<chip::EventNumber, uint64_t>::value,
                      "chip::EventNumber is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::FabricId, uint64_t>::value,
                      "chip::FabricId is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::NodeId, uint64_t>::value,
                      "chip::NodeId is expected to be uint64_t, change this when necessary");
        return ZCL_INT64U_ATTRIBUTE_TYPE;

    default:
        return type;
    }
}

Protocols::InteractionModel::Status CheckAccessControl(const SubjectDescriptor & aSubjectDescriptor, EndpointId aEndpoint,
                                                       ClusterId aCluster, bool aWrite)
{
    RequestPath requestPath = { .cluster = aCluster, .endpoint = aEndpoint };
    // TODO: get required privilege using ember APIs, as it could be custom
    Privilege privilege = aWrite ? Privilege::kOperate : Privilege::kView;
    CHIP_ERROR err      = GetAccessControl().Check(aSubjectDescriptor, requestPath, privilege);
    return (err == CHIP_NO_ERROR) ? Protocols::InteractionModel::Status::Success
                                  : (err == CHIP_ERROR_ACCESS_DENIED) ? Protocols::InteractionModel::Status::UnsupportedAccess
                                                                      : Protocols::InteractionModel::Status::Failure;
}

} // namespace

void SetupEmberAfObjects(Command * command, const ConcreteCommandPath & commandPath)
{
    Messaging::ExchangeContext * commandExchangeCtx = command->GetExchangeContext();

    imCompatibilityEmberApsFrame.clusterId           = commandPath.mClusterId;
    imCompatibilityEmberApsFrame.destinationEndpoint = commandPath.mEndpointId;
    imCompatibilityEmberApsFrame.sourceEndpoint      = 1; // source endpoint is fixed to 1 for now.
    imCompatibilityEmberApsFrame.sequence =
        (commandExchangeCtx != nullptr ? static_cast<uint8_t>(commandExchangeCtx->GetExchangeId() & 0xFF) : 0);

    imCompatibilityEmberAfCluster.commandId      = commandPath.mCommandId;
    imCompatibilityEmberAfCluster.apsFrame       = &imCompatibilityEmberApsFrame;
    imCompatibilityEmberAfCluster.interPanHeader = &imCompatibilityInterpanHeader;
    imCompatibilityEmberAfCluster.source         = commandExchangeCtx;

    emAfCurrentCommand   = &imCompatibilityEmberAfCluster;
    currentCommandObject = command;
}

bool IMEmberAfSendDefaultResponseWithCallback(EmberAfStatus status)
{
    if (currentCommandObject == nullptr)
    {
        // If this command is not handled by IM, then let ember send response.
        return false;
    }

    chip::app::ConcreteCommandPath commandPath(imCompatibilityEmberApsFrame.destinationEndpoint,
                                               imCompatibilityEmberApsFrame.clusterId, imCompatibilityEmberAfCluster.commandId);

    CHIP_ERROR err = currentCommandObject->AddStatus(commandPath, ToInteractionModelStatus(status));
    return CHIP_NO_ERROR == err;
}

void ResetEmberAfObjects()
{
    emAfCurrentCommand   = nullptr;
    currentCommandObject = nullptr;
}

} // namespace Compatibility

namespace {
// Common buffer for ReadSingleClusterData & WriteSingleClusterData
uint8_t attributeData[kAttributeReadBufferSize];

template <typename T>
CHIP_ERROR attributeBufferToNumericTlvData(TLV::TLVWriter & writer, bool isNullable)
{
    typename NumericAttributeTraits<T>::StorageType value;
    memcpy(&value, attributeData, sizeof(value));
    TLV::Tag tag = TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData));
    if (isNullable && NumericAttributeTraits<T>::IsNullValue(value))
    {
        return writer.PutNull(tag);
    }

    if (!NumericAttributeTraits<T>::CanRepresentValue(isNullable, value))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return writer.Put(tag, static_cast<T>(value));
}

} // anonymous namespace

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // TODO: Currently, we are using cluster catalog from the ember library, this should be modified or replaced after several
    // updates to Commands.
    return emberAfContainsServer(aCommandPath.mEndpointId, aCommandPath.mClusterId);
}

CHIP_ERROR ReadSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, const ConcreteAttributePath & aPath,
                                 AttributeReportIB::Builder & aAttributeReport)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=%" PRIx16 " AttributeId=" ChipLogFormatMEI,
                  ChipLogValueMEI(aPath.mClusterId), aPath.mEndpointId, ChipLogValueMEI(aPath.mAttributeId));

    TLV::TLVWriter backup;
    aAttributeReport.Checkpoint(backup);

    AttributeDataIB::Builder attributeDataIBBuilder = aAttributeReport.CreateAttributeData();
    AttributePathIB::Builder attributePathIBBuilder = attributeDataIBBuilder.CreatePath();
    attributePathIBBuilder.Endpoint(aPath.mEndpointId)
        .Cluster(aPath.mClusterId)
        .Attribute(aPath.mAttributeId)
        .EndOfAttributePathIB();
    ReturnErrorOnFailure(attributePathIBBuilder.GetError());

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Failure;

    {
        TLV::TLVWriter * writer = attributeDataIBBuilder.GetWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_NO_ERROR);

        bool accessControlChecked = false;

        AttributeAccessInterface * attrOverride = findAttributeAccessOverride(aPath.mEndpointId, aPath.mClusterId);
        if (attrOverride != nullptr)
        {
            // TODO: We should probably clone the writer and convert failures here
            // into status responses, unless our caller already does that.

            AttributeValueEncoder valueEncoder(writer, aSubjectDescriptor.fabricIndex);
            ReturnErrorOnFailure(attrOverride->Read(aPath, valueEncoder));

            auto accessControlStatus =
                CheckAccessControl(aSubjectDescriptor, aPath.mEndpointId, aPath.mClusterId, /* write = */ false);
            if (accessControlStatus != Protocols::InteractionModel::Status::Success)
            {
                status = accessControlStatus;
                ExitNow();
            }
            accessControlChecked = true;

            if (valueEncoder.TriedEncode())
            {
                // TODO: Add DataVersion support
                attributeDataIBBuilder.DataVersion(kTemporaryDataVersion).EndOfAttributeDataIB();
                ReturnErrorOnFailure(attributeDataIBBuilder.GetError());
                return CHIP_NO_ERROR;
            }
        }

        EmberAfAttributeMetadata * metadata = nullptr;
        {
            EmberAfAttributeSearchRecord record;
            record.endpoint           = aPath.mEndpointId;
            record.clusterId          = aPath.mClusterId;
            record.clusterMask        = CLUSTER_MASK_SERVER;
            record.attributeId        = aPath.mAttributeId;
            record.manufacturerCode   = EMBER_AF_NULL_MANUFACTURER_CODE;
            EmberAfStatus emberStatus = emAfReadOrWriteAttribute(&record, &metadata, attributeData, sizeof(attributeData),
                                                                 /* write = */ false);
            if (emberStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = ToInteractionModelStatus(emberStatus);
                ExitNow();
            }
        }

        if (!accessControlChecked)
        {
            auto accessControlStatus =
                CheckAccessControl(aSubjectDescriptor, aPath.mEndpointId, aPath.mClusterId, /* write = */ false);
            if (accessControlStatus != Protocols::InteractionModel::Status::Success)
            {
                status = accessControlStatus;
                ExitNow();
            }
            accessControlChecked = true;
        }

        {
            EmberAfAttributeType attributeType = metadata->attributeType;
            bool isNullable                    = metadata->IsNullable();
            writer                             = attributeDataIBBuilder.GetWriter();
            TLV::Tag tag                       = TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData));
            switch (BaseType(attributeType))
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
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint16_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint32_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint64_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<int8_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<int16_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<int32_t>(*writer, isNullable));
                break;
            }
            case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
            {
                ReturnErrorOnFailure(attributeBufferToNumericTlvData<int64_t>(*writer, isNullable));
                break;
            }
            case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Char string
            {
                char * actualData  = reinterpret_cast<char *>(attributeData + 1);
                uint8_t dataLength = attributeData[0];
                if (dataLength == 0xFF)
                {
                    if (isNullable)
                    {
                        ReturnErrorOnFailure(writer->PutNull(tag));
                    }
                    else
                    {
                        return CHIP_ERROR_INCORRECT_STATE;
                    }
                }
                else
                {
                    ReturnErrorOnFailure(writer->PutString(tag, actualData, dataLength));
                }
                break;
            }
            case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE: {
                char * actualData = reinterpret_cast<char *>(attributeData + 2); // The pascal string contains 2 bytes length
                uint16_t dataLength;
                memcpy(&dataLength, attributeData, sizeof(dataLength));
                if (dataLength == 0xFFFF)
                {
                    if (isNullable)
                    {
                        ReturnErrorOnFailure(writer->PutNull(tag));
                    }
                    else
                    {
                        return CHIP_ERROR_INCORRECT_STATE;
                    }
                }
                else
                {
                    ReturnErrorOnFailure(writer->PutString(tag, actualData, dataLength));
                }
                break;
            }
            case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
            {
                uint8_t * actualData = attributeData + 1;
                uint8_t dataLength   = attributeData[0];
                if (dataLength == 0xFF)
                {
                    if (isNullable)
                    {
                        ReturnErrorOnFailure(writer->PutNull(tag));
                    }
                    else
                    {
                        return CHIP_ERROR_INCORRECT_STATE;
                    }
                }
                else
                {
                    ReturnErrorOnFailure(writer->Put(tag, chip::ByteSpan(actualData, dataLength)));
                }
                break;
            }
            case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: {
                uint8_t * actualData = attributeData + 2; // The pascal string contains 2 bytes length
                uint16_t dataLength;
                memcpy(&dataLength, attributeData, sizeof(dataLength));
                if (dataLength == 0xFFFF)
                {
                    if (isNullable)
                    {
                        ReturnErrorOnFailure(writer->PutNull(tag));
                    }
                    else
                    {
                        return CHIP_ERROR_INCORRECT_STATE;
                    }
                }
                else
                {
                    ReturnErrorOnFailure(writer->Put(tag, chip::ByteSpan(actualData, dataLength)));
                }
                break;
            }
            case ZCL_ARRAY_ATTRIBUTE_TYPE: {
                // We only get here for attributes of list type that have no override
                // registered.  There should not be any nonempty lists like that.
                uint16_t size = emberAfAttributeValueSize(aPath.mClusterId, aPath.mAttributeId, attributeType, attributeData);
                if (size != 2)
                {
                    // The value returned by emberAfAttributeValueSize for a list
                    // includes the space needed to store the list length (2 bytes) plus
                    // the space needed to store the actual list items.  We expect it to
                    // return 2 here, indicating a zero-length list.  If it doesn't,
                    // something has gone wrong.
                    return CHIP_ERROR_INCORRECT_STATE;
                }

                // Just encode an empty array.
                TLV::TLVType containerType;
                ReturnErrorOnFailure(writer->StartContainer(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)),
                                                            TLV::kTLVType_Array, containerType));
                ReturnErrorOnFailure(writer->EndContainer(containerType));
                break;
            }
            default:
                ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(attributeType));
                status = Protocols::InteractionModel::Status::UnsupportedRead;
                ExitNow();
            }
        }
    }

    status = Protocols::InteractionModel::Status::Success;

exit:
    if (status == Protocols::InteractionModel::Status::Success)
    {
        // TODO: Add DataVersion support
        attributeDataIBBuilder.DataVersion(kTemporaryDataVersion).EndOfAttributeDataIB();
        ReturnErrorOnFailure(attributeDataIBBuilder.GetError());
    }
    else
    {
        aAttributeReport.Rollback(backup);
        AttributeStatusIB::Builder attributeStatusIBBuilder;
        attributeStatusIBBuilder = aAttributeReport.CreateAttributeStatus();
        attributePathIBBuilder   = attributeStatusIBBuilder.CreatePath();
        attributePathIBBuilder.Endpoint(aPath.mEndpointId)
            .Cluster(aPath.mClusterId)
            .Attribute(aPath.mAttributeId)
            .EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePathIBBuilder.GetError());
        StatusIB::Builder statusIBBuilder = attributeStatusIBBuilder.CreateErrorStatus();
        statusIBBuilder.EncodeStatusIB(StatusIB(status));
        ReturnErrorOnFailure(statusIBBuilder.GetError());
        attributeStatusIBBuilder.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatusIBBuilder.GetError());
    }
    return CHIP_NO_ERROR;
}

namespace {

template <typename T>
CHIP_ERROR numericTlvDataToAttributeBuffer(TLV::TLVReader & aReader, bool isNullable, uint16_t & dataLen)
{
    typename NumericAttributeTraits<T>::StorageType value;
    static_assert(sizeof(value) <= sizeof(attributeData), "Value cannot fit into attribute data");
    if (isNullable && aReader.GetType() == TLV::kTLVType_Null)
    {
        value = NumericAttributeTraits<T>::kNullValue;
    }
    else
    {
        T val;
        ReturnErrorOnFailure(aReader.Get(val));
        VerifyOrReturnError(NumericAttributeTraits<T>::CanRepresentValue(isNullable, val), CHIP_ERROR_INVALID_ARGUMENT);
        value = val;
    }
    dataLen = sizeof(value);
    memcpy(attributeData, &value, sizeof(value));
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
        memcpy(&attributeData[0], &len, sizeof(len));
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
        VerifyOrReturnError(len + sizeof(len) /* length at the beginning of data */ <= sizeof(attributeData),
                            CHIP_ERROR_MESSAGE_TOO_LONG);
        memcpy(&attributeData[0], &len, sizeof(len));
        memcpy(&attributeData[sizeof(len)], data, len);
        dataLen = static_cast<uint16_t>(len + sizeof(len));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR prepareWriteData(const EmberAfAttributeMetadata * metadata, TLV::TLVReader & aReader, uint16_t & dataLen)
{
    EmberAfAttributeType expectedType = BaseType(metadata->attributeType);
    bool isNullable                   = metadata->IsNullable();
    switch (expectedType)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        return numericTlvDataToAttributeBuffer<bool>(aReader, isNullable, dataLen);
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        return numericTlvDataToAttributeBuffer<uint8_t>(aReader, isNullable, dataLen);
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
        return numericTlvDataToAttributeBuffer<uint16_t>(aReader, isNullable, dataLen);
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        return numericTlvDataToAttributeBuffer<uint32_t>(aReader, isNullable, dataLen);
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
        return numericTlvDataToAttributeBuffer<uint64_t>(aReader, isNullable, dataLen);
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
        return numericTlvDataToAttributeBuffer<int8_t>(aReader, isNullable, dataLen);
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
        return numericTlvDataToAttributeBuffer<int16_t>(aReader, isNullable, dataLen);
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        return numericTlvDataToAttributeBuffer<int32_t>(aReader, isNullable, dataLen);
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
        return numericTlvDataToAttributeBuffer<int64_t>(aReader, isNullable, dataLen);
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

// TODO: Refactor WriteSingleClusterData and all dependent functions to take ConcreteAttributePath instead of ClusterInfo
// as the input argument.
CHIP_ERROR WriteSingleClusterData(const SubjectDescriptor & aSubjectDescriptor, ClusterInfo & aClusterInfo,
                                  TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = aClusterInfo.mEndpointId;
    attributePathParams.mClusterId   = aClusterInfo.mClusterId;
    attributePathParams.mAttributeId = aClusterInfo.mAttributeId;

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Failure;

    {
        bool accessControlChecked = false;

        AttributeAccessInterface * attrOverride = findAttributeAccessOverride(aClusterInfo.mEndpointId, aClusterInfo.mClusterId);
        if (attrOverride != nullptr)
        {
            ConcreteAttributePath path(aClusterInfo.mEndpointId, aClusterInfo.mClusterId, aClusterInfo.mAttributeId);
            AttributeValueDecoder valueDecoder(aReader);
            ReturnErrorOnFailure(attrOverride->Write(path, valueDecoder));

            auto accessControlStatus =
                CheckAccessControl(aSubjectDescriptor, aClusterInfo.mEndpointId, aClusterInfo.mClusterId, /* write = */ true);
            if (accessControlStatus != Protocols::InteractionModel::Status::Success)
            {
                status = accessControlStatus;
                ExitNow();
            }
            accessControlChecked = true;

            if (valueDecoder.TriedDecode())
            {
                status = Protocols::InteractionModel::Status::Success;
                ExitNow();
            }
        }

        // Passing nullptr as buf to emberAfReadAttribute means we only need attribute type here,
        // and ember will not do data read & copy in this case.
        const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(
            aClusterInfo.mEndpointId, aClusterInfo.mClusterId, aClusterInfo.mAttributeId, CLUSTER_MASK_SERVER, 0);
        if (metadata == nullptr)
        {
            status = Protocols::InteractionModel::Status::UnsupportedAttribute;
            ExitNow();
        }

        if (!accessControlChecked)
        {
            auto accessControlStatus =
                CheckAccessControl(aSubjectDescriptor, aClusterInfo.mEndpointId, aClusterInfo.mClusterId, /* write = */ true);
            if (accessControlStatus != Protocols::InteractionModel::Status::Success)
            {
                status = accessControlStatus;
                ExitNow();
            }
            accessControlChecked = true;
        }

        {
            uint16_t dataLen = 0;
            auto error       = prepareWriteData(metadata, aReader, dataLen);
            if (error != CHIP_NO_ERROR)
            {
                ChipLogDetail(Zcl, "Failed to prepare data to write: %s", ErrorStr(error));
                status = Protocols::InteractionModel::Status::InvalidValue;
                ExitNow();
            }
            if (dataLen > metadata->size)
            {
                ChipLogDetail(Zcl, "Data to write exceedes the attribute size claimed.");
                status = Protocols::InteractionModel::Status::InvalidValue;
                ExitNow();
            }

            status = ToInteractionModelStatus(emberAfWriteAttributeExternal(aClusterInfo.mEndpointId, aClusterInfo.mClusterId,
                                                                            aClusterInfo.mAttributeId, CLUSTER_MASK_SERVER, 0,
                                                                            attributeData, metadata->attributeType));
        }
    }

exit:
    return apWriteHandler->AddStatus(attributePathParams, status);
}

} // namespace app
} // namespace chip

void MatterReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                            uint16_t manufacturerCode, EmberAfAttributeType type, uint8_t * data)
{
    IgnoreUnusedVariable(manufacturerCode);
    IgnoreUnusedVariable(type);
    IgnoreUnusedVariable(data);
    IgnoreUnusedVariable(mask);

    MatterReportingAttributeChangeCallback(endpoint, clusterId, attributeId);
}

void MatterReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    ClusterInfo info;
    info.mClusterId   = clusterId;
    info.mAttributeId = attributeId;
    info.mEndpointId  = endpoint;

    InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(info);

    // Schedule work to run asynchronously on the CHIP thread. The scheduled work won't execute until the current execution context
    // has completed. This ensures that we can 'gather up' multiple attribute changes that have occurred in the same execution
    // context without requiring any explicit 'start' or 'end' change calls into the engine to book-end the change.
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
}
