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

#include <app/Command.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/ember-compatibility-functions.h>
#include <app/util/util.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/Constants.h>

#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-type.h>

#include <gen/endpoint_config.h>

using namespace chip;
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
    case ZCL_STATUS_ATTRIBUTE_TYPE:     // Status Code
    case ZCL_DATA_VER_ATTRIBUTE_TYPE:   // Data Version
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:   // 32-bit bitmap
        static_assert(std::is_same<chip::ClusterId, uint32_t>::value,
                      "chip::Cluster is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::FieldId, uint32_t>::value,
                      "chip::FieldId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::EventId, uint32_t>::value,
                      "chip::EventId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::CommandId, uint32_t>::value,
                      "chip::CommandId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::TransactionId, uint32_t>::value,
                      "chip::TransactionId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DeviceTypeId, uint32_t>::value,
                      "chip::DeviceTypeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::StatusCode, uint32_t>::value,
                      "chip::StatusCode is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DataVersion, uint32_t>::value,
                      "chip::DataVersion is expected to be uint32_t, change this when necessary");
        return ZCL_INT32U_ATTRIBUTE_TYPE;

    case ZCL_EVENT_NO_ATTRIBUTE_TYPE:  // Event Number
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE: // Fabric Id
    case ZCL_NODE_ID_ATTRIBUTE_TYPE:   // Node Id
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:  // 64-bit bitmap
        static_assert(std::is_same<chip::EventNumber, uint64_t>::value,
                      "chip::EventNumber is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::FabricId, uint64_t>::value,
                      "chip::FabricId is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::NodeId, uint64_t>::value,
                      "chip::NodeId is expected to be uint64_t, change this when necessary");
        return ZCL_INT64U_ATTRIBUTE_TYPE;

    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: // Character string
        return ZCL_OCTET_STRING_ATTRIBUTE_TYPE;

    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        return ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE;

    default:
        return type;
    }
}

} // namespace

void SetupEmberAfObjects(Command * command, ClusterId clusterId, CommandId commandId, EndpointId endpointId)
{
    Messaging::ExchangeContext * commandExchangeCtx = command->GetExchangeContext();

    imCompatibilityEmberApsFrame.clusterId           = clusterId;
    imCompatibilityEmberApsFrame.destinationEndpoint = endpointId;
    imCompatibilityEmberApsFrame.sourceEndpoint      = 1; // source endpoint is fixed to 1 for now.
    imCompatibilityEmberApsFrame.sequence =
        (commandExchangeCtx != nullptr ? static_cast<uint8_t>(commandExchangeCtx->GetExchangeId() & 0xFF) : 0);

    imCompatibilityEmberAfCluster.commandId      = commandId;
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

    chip::app::CommandPathParams returnStatusParam = { imCompatibilityEmberApsFrame.destinationEndpoint,
                                                       0, // GroupId
                                                       imCompatibilityEmberApsFrame.clusterId,
                                                       imCompatibilityEmberAfCluster.commandId,
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    CHIP_ERROR err = currentCommandObject->AddStatusCode(
        returnStatusParam,
        status == EMBER_ZCL_STATUS_SUCCESS ? chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess
                                           : chip::Protocols::SecureChannel::GeneralStatusCode::kFailure,
        chip::Protocols::InteractionModel::Id, static_cast<Protocols::InteractionModel::ProtocolCode>(status));
    return CHIP_NO_ERROR == err;
}

void ResetEmberAfObjects()
{
    emAfCurrentCommand   = nullptr;
    currentCommandObject = nullptr;
}

} // namespace Compatibility

bool ServerClusterCommandExists(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId)
{
    // TODO: Currently, we are using cluster catalog from the ember library, this should be modified or replaced after several
    // updates to Commands.
    return emberAfContainsServer(aEndPointId, aClusterId);
}

CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    static uint8_t data[kAttributeReadBufferSize];

    ChipLogDetail(DataManagement,
                  "Received Cluster Command: Cluster=%" PRIx32 " NodeId=0x" ChipLogFormatX64 " Endpoint=%" PRIx16
                  " FieldId=%" PRIx32 " ListIndex=%" PRIx16,
                  aClusterInfo.mClusterId, ChipLogValueX64(aClusterInfo.mNodeId), aClusterInfo.mEndpointId, aClusterInfo.mFieldId,
                  aClusterInfo.mListIndex);

    EmberAfAttributeType attributeType;
    EmberAfStatus status;
    status = emberAfReadAttribute(aClusterInfo.mEndpointId, aClusterInfo.mClusterId, aClusterInfo.mFieldId, CLUSTER_MASK_SERVER,
                                  data, sizeof(data), &attributeType);

    // TODO: Currently, all errors are considered as attribute not exists, should be fixed by mapping ember error code to IM error
    // codes.
    if (apDataExists != nullptr)
    {
        *apDataExists = (EMBER_ZCL_STATUS_SUCCESS == status);
    }

    VerifyOrReturnError(apWriter != nullptr, CHIP_NO_ERROR);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return apWriter->Put(
            chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
            Protocols::InteractionModel::ToUint16(Protocols::InteractionModel::ProtocolCode::UnsupportedAttribute));
    }

    // TODO: ZCL_STRUCT_ATTRIBUTE_TYPE is not included in this switch case currently, should add support for structures.
    switch (BaseType(attributeType))
    {
    case ZCL_NO_DATA_ATTRIBUTE_TYPE: // No data
        ReturnErrorOnFailure(apWriter->PutNull(TLV::ContextTag(AttributeDataElement::kCsTag_Data)));
        break;
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: // Boolean
        ReturnErrorOnFailure(apWriter->PutBoolean(TLV::ContextTag(AttributeDataElement::kCsTag_Data), !!data[0]));
        break;
    case ZCL_INT8U_ATTRIBUTE_TYPE: // Unsigned 8-bit integer
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), data[0]));
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE: // Unsigned 16-bit integer
    {
        uint16_t uint16_data;
        memcpy(&uint16_data, data, sizeof(uint16_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), uint16_data));
        break;
    }
    case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
    {
        uint32_t uint32_data;
        memcpy(&uint32_data, data, sizeof(uint32_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), uint32_data));
        break;
    }
    case ZCL_INT64U_ATTRIBUTE_TYPE: // Unsigned 64-bit integer
    {
        uint64_t uint64_data;
        memcpy(&uint64_data, data, sizeof(uint64_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), uint64_data));
        break;
    }
    case ZCL_INT8S_ATTRIBUTE_TYPE: // Signed 8-bit integer
    {
        int8_t int8_data;
        memcpy(&int8_data, data, sizeof(int8_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), int8_data));
        break;
    }
    case ZCL_INT16S_ATTRIBUTE_TYPE: // Signed 16-bit integer
    {
        int16_t int16_data;
        memcpy(&int16_data, data, sizeof(int16_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), int16_data));
        break;
    }
    case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
    {
        int32_t int32_data;
        memcpy(&int32_data, data, sizeof(int32_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), int32_data));
        break;
    }
    case ZCL_INT64S_ATTRIBUTE_TYPE: // Signed 64-bit integer
    {
        int64_t int64_data;
        memcpy(&int64_data, data, sizeof(int64_data));
        ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), int64_data));
        break;
    }
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: // Octet string
    {
        uint8_t * actualData = data + 1;
        uint8_t dataLength   = data[0];
        if (dataLength == 0xFF /* invalid data, put empty value instead */)
        {
            dataLength = 0;
        }
        ReturnErrorOnFailure(
            apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::ByteSpan(actualData, dataLength)));
        break;
    }
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: {
        uint8_t * actualData = data + 2; // The pascal string contains 2 bytes length
        uint16_t dataLength;
        memcpy(&dataLength, data, sizeof(dataLength));
        if (dataLength == 0xFFFF /* invalid data, put empty value instead */)
        {
            dataLength = 0;
        }
        ReturnErrorOnFailure(
            apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::ByteSpan(actualData, dataLength)));
        break;
    }
    case ZCL_ARRAY_ATTRIBUTE_TYPE: {
        TLV::TLVType containerType;
        ReturnErrorOnFailure(
            apWriter->StartContainer(TLV::ContextTag(AttributeDataElement::kCsTag_Data), TLV::kTLVType_List, containerType));
        // TODO: Encode data in TLV, now raw buffers
        ReturnErrorOnFailure(
            apWriter->PutBytes(TLV::AnonymousTag, data,
                               emberAfAttributeValueSize(aClusterInfo.mClusterId, aClusterInfo.mFieldId, attributeType, data)));
        ReturnErrorOnFailure(apWriter->EndContainer(containerType));
        break;
    }
    default:
        ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(attributeType));
        return apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
                             Protocols::InteractionModel::ToUint16(Protocols::InteractionModel::ProtocolCode::UnsupportedRead));
    }

    // TODO: Add DataVersion support
    ReturnErrorOnFailure(apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_DataVersion), kTemporaryDataVersion));
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
