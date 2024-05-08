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
#include <app/codegen-interaction-model/Model.h>

#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/GlobalAttributes.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>

namespace chip {
namespace app {
namespace CodegenDataModel {
namespace {

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
    if (err != CHIP_NO_ERROR)
    {
        ReturnErrorCodeIf(err != CHIP_ERROR_ACCESS_DENIED, err);
        if (path.mExpanded)
        {
            return CHIP_NO_ERROR;
        }

        return SendFailureStatus(path, aAttributeReports, Protocols::InteractionModel::Status::UnsupportedAccess, nullptr);
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
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

    CHIP_ERROR err = CheckAccessPrivilege(request.path, request.subjectDescriptor);
    if (err == CHIP_ERROR_ACCESS_DENIED && request.path.mExpanded)
    {
        // return success, however without filling in the attribute value at all as there is nothing to be done here...
        return CHIP_NO_ERROR;
    }

#if 0


    {
        // Special handling for mandatory global attributes: these are always for attribute list, using a special
        // reader (which can be lightweight constructed even from nullptr).
        GlobalAttributeReader reader(attributeCluster);
        AttributeAccessInterface * attributeOverride =
            (attributeCluster != nullptr) ? &reader : GetAttributeAccessOverride(aPath.mEndpointId, aPath.mClusterId);
        if (attributeOverride)
        {
            bool triedEncode = false;
            ReturnErrorOnFailure(ReadViaAccessInterface(aSubjectDescriptor, aIsFabricFiltered, aPath, aAttributeReports,
                                                        apEncoderState, attributeOverride, &triedEncode));
            ReturnErrorCodeIf(triedEncode, CHIP_NO_ERROR);
        }
    }

    // Read attribute using Ember, if it doesn't have an override.

    TLV::TLVWriter backup;
    aAttributeReports.Checkpoint(backup);

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

    EmberAfAttributeSearchRecord record;
    record.endpoint    = aPath.mEndpointId;
    record.clusterId   = aPath.mClusterId;
    record.attributeId = aPath.mAttributeId;
    Status status      = emAfReadOrWriteAttribute(&record, &attributeMetadata, attributeData, sizeof(attributeData),
                                                  /* write = */ false);

    if (status == Status::Success)
    {
        EmberAfAttributeType attributeType = attributeMetadata->attributeType;
        bool isNullable                    = attributeMetadata->IsNullable();
        TLV::TLVWriter * writer            = attributeDataIBBuilder.GetWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_NO_ERROR);
        TLV::Tag tag = TLV::ContextTag(AttributeDataIB::Tag::kData);
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
        case ZCL_INT24U_ATTRIBUTE_TYPE: // Unsigned 24-bit integer
        {
            using IntType = OddSizedInteger<3, false>;
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
            break;
        }
        case ZCL_INT32U_ATTRIBUTE_TYPE: // Unsigned 32-bit integer
        {
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<uint32_t>(*writer, isNullable));
            break;
        }
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
        case ZCL_INT24S_ATTRIBUTE_TYPE: // Signed 24-bit integer
        {
            using IntType = OddSizedInteger<3, true>;
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<IntType>(*writer, isNullable));
            break;
        }
        case ZCL_INT32S_ATTRIBUTE_TYPE: // Signed 32-bit integer
        {
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<int32_t>(*writer, isNullable));
            break;
        }
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
        {
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<int64_t>(*writer, isNullable));
            break;
        }
        case ZCL_SINGLE_ATTRIBUTE_TYPE: // 32-bit float
        {
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<float>(*writer, isNullable));
            break;
        }
        case ZCL_DOUBLE_ATTRIBUTE_TYPE: // 64-bit float
        {
            ReturnErrorOnFailure(attributeBufferToNumericTlvData<double>(*writer, isNullable));
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
        default:
            ChipLogError(DataManagement, "Attribute type 0x%x not handled", static_cast<int>(attributeType));
            status = Status::UnsupportedRead;
        }
    }

    if (status == Protocols::InteractionModel::Status::Success)
    {
        return SendSuccessStatus(attributeReport, attributeDataIBBuilder);
    }

    return SendFailureStatus(aPath, aAttributeReports, status, &backup);
#endif

    // TODO: this needs an implementation
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace CodegenDataModel
} // namespace app
} // namespace chip
