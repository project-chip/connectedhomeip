/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include "attributes_service/attributes_service.pb.h"
#include "attributes_service/attributes_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app/AppConfig.h>
#include <app/AttributeValueEncoder.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <pigweed/rpc_services/AccessInterceptor.h>
#include <pigweed/rpc_services/AccessInterceptorRegistry.h>
#include <platform/PlatformManager.h>
#include <set>

namespace chip {
namespace rpc {

std::optional<::pw::Status> TryWriteViaAccessor(const chip::app::ConcreteDataAttributePath & path,
                                                chip::app::AttributeValueDecoder & decoder)
{
    std::set<PigweedDebugAccessInterceptor *> accessors = PigweedDebugAccessInterceptorRegistry::Instance().GetAllAccessors();

    for (PigweedDebugAccessInterceptor * accessor : accessors)
    {
        std::optional<::pw::Status> result = accessor->Write(path, decoder);
        if (result.has_value()) // Write was either a success or failure.
        {
            return result;
        }
        else if (decoder.TriedDecode())
        {
            ChipLogError(Support, "Interceptor tried decode but did not return status.");
            return ::pw::Status::FailedPrecondition();
        }
    }

    VerifyOrReturnError(!decoder.TriedDecode(), ::pw::Status::FailedPrecondition());

    return std::nullopt;
}

// Implementation class for chip.rpc.Attributes.
class Attributes : public pw_rpc::nanopb::Attributes::Service<Attributes>
{
public:
    static constexpr TLV::Tag kAttributeDataTag = TLV::ContextTag(1);
    static constexpr TLV::Tag kDataTag          = TLV::ContextTag(to_underlying(chip::app::AttributeDataIB::Tag::kData));

    CHIP_ERROR PositionOnDataElement(chip::TLV::TLVReader & reader)
    {
        // Expect the TLV to be the full structure as received from a read (or subset)
        // TLV is a full ReportDataMessage
        //   - Anonymous Structure (container of everything)
        //     - 1: Array (one element)
        //       - Anonymous (the element)
        //         - 1 (AttributeData/AttributeDataIB) - Structure
        //           - 0 - Data Version
        //           - 1 - Path (1: Node, 2: Endpoint, 3: Cluster, 4: Attribute, ...)
        //           - 2 - Data (variable - may be raw data or a Structure)

        TLV::TLVType unused_outer_type;

        // Enter anonymous wrapper
        ReturnErrorOnFailure(reader.Next()); // got to anonymous
        ReturnErrorOnFailure(reader.EnterContainer(unused_outer_type));

        // Enter the array
        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.EnterContainer(unused_outer_type));

        // enter the structure of data
        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.EnterContainer(unused_outer_type));

        // Find AttributeData Container
        {
            chip::TLV::TLVReader tmp;
            ReturnErrorOnFailure(reader.FindElementWithTag(kAttributeDataTag, tmp));
            reader = tmp;
        }

        // Enter into AttributeData Container
        ReturnErrorOnFailure(reader.EnterContainer(unused_outer_type));

        // Find Data Container
        {
            chip::TLV::TLVReader tmp;
            ReturnErrorOnFailure(reader.FindElementWithTag(kDataTag, tmp));
            reader = tmp;
        }

        return CHIP_NO_ERROR;
    }

    pw::Result<TLV::TLVReader> ReadIntoTlv(const chip_rpc_AttributeData & data, chip::MutableByteSpan tempBuffer)
    {
        TLV::TLVReader result;

        if (data.has_tlv_data)
        {
            result.Init(data.tlv_data.bytes, data.tlv_data.size);
            CHIP_ERROR err = PositionOnDataElement(result);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Support, "Failed to parse input TLV buffer: %" CHIP_ERROR_FORMAT, err.Format());
                return pw::Status::InvalidArgument();
            }

            return result;
        }

        TLV::TLVWriter writer;

        writer.Init(tempBuffer);

        CHIP_ERROR write_status;

        TLV::TLVType outer;
        VerifyOrReturnError(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer) == CHIP_NO_ERROR,
                            pw::Status::Internal());

        switch (data.which_data)
        {
        case chip_rpc_AttributeData_data_bool_tag:
            write_status = writer.Put(kDataTag, data.data.data_bool);
            break;
        case chip_rpc_AttributeData_data_uint8_tag:
            write_status = writer.Put(kDataTag, data.data.data_uint8);
            break;
        case chip_rpc_AttributeData_data_uint16_tag:
            write_status = writer.Put(kDataTag, data.data.data_uint16);
            break;
        case chip_rpc_AttributeData_data_uint32_tag:
            write_status = writer.Put(kDataTag, data.data.data_uint32);
            break;
        case chip_rpc_AttributeData_data_int8_tag:
            write_status = writer.Put(kDataTag, data.data.data_int8);
            break;
        case chip_rpc_AttributeData_data_int16_tag:
            write_status = writer.Put(kDataTag, data.data.data_int16);
            break;
        case chip_rpc_AttributeData_data_int32_tag:
            write_status = writer.Put(kDataTag, data.data.data_int32);
            break;
        case chip_rpc_AttributeData_data_single_tag:
            write_status = writer.Put(kDataTag, data.data.data_single);
            break;
        case chip_rpc_AttributeData_data_bytes_tag:
            write_status = writer.PutBytes(kDataTag, data.data.data_bytes.bytes, data.data.data_bytes.size);
            break;
        case chip_rpc_AttributeData_data_string_tag:
            write_status = writer.PutString(kDataTag, data.data.data_string);
            break;
        default:
            return pw::Status::InvalidArgument();
        }

        if (write_status != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to encode TLV data: %" CHIP_ERROR_FORMAT, write_status.Format());
            return pw::Status::Internal();
        }

        VerifyOrReturnValue(writer.EndContainer(outer) == CHIP_NO_ERROR, pw::Status::Internal());
        VerifyOrReturnValue(writer.Finalize() == CHIP_NO_ERROR, pw::Status::Internal());
        result.Init(tempBuffer.data(), writer.GetLengthWritten());

        VerifyOrReturnError(result.Next() == CHIP_NO_ERROR, pw::Status::Internal());
        VerifyOrReturnError(result.EnterContainer(outer) == CHIP_NO_ERROR, pw::Status::Internal());

        // This positions on the data element
        VerifyOrReturnError(result.Next() == CHIP_NO_ERROR, pw::Status::Internal());

        return result;
    }

    ::pw::Status Write(const chip_rpc_AttributeWrite & request, pw_protobuf_Empty & response)
    {
        app::ConcreteAttributePath path(request.metadata.endpoint, request.metadata.cluster, request.metadata.attribute_id);

        DeviceLayer::StackLock lock;

        // TODO: this assumes a singleton data model provider
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();

        app::DataModel::ServerClusterFinder serverClusterFinder(provider);
        auto info = serverClusterFinder.Find(path);
        if (!info.has_value())
        {
            return ::pw::Status::NotFound();
        }

        Access::SubjectDescriptor subjectDescriptor{ .authMode = chip::Access::AuthMode::kInternalDeviceAccess };
        app::DataModel::WriteAttributeRequest write_request;
        write_request.path = path;
        write_request.operationFlags.Set(app::DataModel::OperationFlags::kInternal);
        write_request.subjectDescriptor = &subjectDescriptor;

        uint8_t raw_value_buffer[64]; // enough to hold general types
        pw::Result<TLV::TLVReader> tlvReader = ReadIntoTlv(request.data, chip::MutableByteSpan(raw_value_buffer));

        if (!tlvReader.status().ok())
        {
            return tlvReader.status();
        }

        app::AttributeValueDecoder decoder(tlvReader.value(), subjectDescriptor);

        std::optional<::pw::Status> interceptResult = TryWriteViaAccessor(write_request.path, decoder);
        if (interceptResult.has_value())
        {
            return *interceptResult;
        }
        ChipLogProgress(Support, "No custom PigweedRPC Attribute Accessor registration found, using fake write access.");

        app::DataModel::ActionReturnStatus result = provider->WriteAttribute(write_request, decoder);

        if (!result.IsSuccess())
        {
            app::DataModel::ActionReturnStatus::StringStorage storage;
            ChipLogError(Support, "Failed to write data: %s", result.c_str(storage));
            return ::pw::Status::Internal();
        }

        return pw::OkStatus();
    }

    ::pw::Status Read(const chip_rpc_AttributeMetadata & request, chip_rpc_AttributeData & response)
    {
        app::ConcreteAttributePath path(request.endpoint, request.cluster, request.attribute_id);
        MutableByteSpan tlvBuffer(response.tlv_data.bytes);
        PW_TRY(ReadAttributeIntoTlvBuffer(path, tlvBuffer));
        // NOTE: TLV will be a full AttributeReportIB (so not purely the data)
        response.tlv_data.size = tlvBuffer.size();
        response.has_tlv_data  = true;

        switch (request.type)
        {
        case chip_rpc_AttributeType_ZCL_BOOLEAN_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_Boolean, response.data.data_bool));
            response.which_data = chip_rpc_AttributeData_data_bool_tag;
            break;
        case chip_rpc_AttributeType_ZCL_ENUM8_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UnsignedInteger, response.data.data_uint8));
            response.which_data = chip_rpc_AttributeData_data_uint8_tag;
            break;
        case chip_rpc_AttributeType_ZCL_ENUM16_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UnsignedInteger, response.data.data_uint16));
            response.which_data = chip_rpc_AttributeData_data_uint16_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT8U_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UnsignedInteger, response.data.data_uint8));
            response.which_data = chip_rpc_AttributeData_data_uint8_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT16U_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UnsignedInteger, response.data.data_uint16));
            response.which_data = chip_rpc_AttributeData_data_uint16_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT32U_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UnsignedInteger, response.data.data_uint32));
            response.which_data = chip_rpc_AttributeData_data_uint32_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT8S_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_SignedInteger, response.data.data_int8));
            response.which_data = chip_rpc_AttributeData_data_int8_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT16S_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_SignedInteger, response.data.data_int16));
            response.which_data = chip_rpc_AttributeData_data_int16_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT32S_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_SignedInteger, response.data.data_int32));
            response.which_data = chip_rpc_AttributeData_data_int32_tag;
            break;
        case chip_rpc_AttributeType_ZCL_SINGLE_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_FloatingPointNumber, response.data.data_single));
            response.which_data = chip_rpc_AttributeData_data_single_tag;
            break;
        case chip_rpc_AttributeType_ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_ByteString, response.data.data_bytes));
            response.which_data = chip_rpc_AttributeData_data_bytes_tag;
            break;
        case chip_rpc_AttributeType_ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
            PW_TRY(TlvBufferGetData(tlvBuffer, TLV::kTLVType_UTF8String, response.data.data_string));
            response.which_data = chip_rpc_AttributeData_data_string_tag;
            break;
        case chip_rpc_AttributeType_ZCL_BITMAP8_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_BITMAP16_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_BITMAP32_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_ARRAY_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_BITMAP64_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT24U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT40U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT48U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT56U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT64U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT24S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT40S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT48S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT56S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT64S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_DOUBLE_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_STRUCT_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_TOD_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_DATE_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_UTC_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_EPOCH_US_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_EPOCH_S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_SYSTIME_US_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_PERCENT_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_PERCENT100THS_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_CLUSTER_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_ATTRIB_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_FIELD_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_EVENT_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_COMMAND_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_ACTION_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_TRANS_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_NODE_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_VENDOR_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_FABRIC_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_GROUP_ID_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_STATUS_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_DATA_VER_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_EVENT_NO_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_FABRIC_IDX_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_IPADR_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_IPV4ADR_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_IPV6ADR_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_IPV6PRE_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_HWADR_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_NO_DATA_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_UNKNOWN_ATTRIBUTE_TYPE:
        default:
            break;
            // These are currently not returned as decoded data, but can be
            // decoded from the returned TLV data.
        }

        return pw::OkStatus();
    }

private:
    static constexpr uint8_t kReportContextTag = 0x01;

    ::pw::Status ReadAttributeIntoTlvBuffer(const app::ConcreteAttributePath & path, MutableByteSpan & tlvBuffer)
    {
        Access::SubjectDescriptor subjectDescriptor{ .authMode = chip::Access::AuthMode::kInternalDeviceAccess };
        app::AttributeReportIBs::Builder attributeReports;
        TLV::TLVWriter writer;
        TLV::TLVType outer;
        DeviceLayer::StackLock lock;

        writer.Init(tlvBuffer);
        PW_TRY(ChipErrorToPwStatus(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer)));
        PW_TRY(ChipErrorToPwStatus(attributeReports.Init(&writer, kReportContextTag)));

        // TODO: this assumes a singleton data model provider
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();

        app::DataModel::ReadAttributeRequest request;
        request.path = path;
        request.operationFlags.Set(app::DataModel::OperationFlags::kInternal);
        request.subjectDescriptor = &subjectDescriptor;

        app::DataModel::ServerClusterFinder serverClusterFinder(provider);
        auto info = serverClusterFinder.Find(path);

        if (!info.has_value())
        {
            return ::pw::Status::NotFound();
        }

        app::AttributeValueEncoder encoder(attributeReports, subjectDescriptor, path, info->dataVersion,
                                           false /* isFabricFiltered */, nullptr /* attributeEncodingState */);
        app::DataModel::ActionReturnStatus result = provider->ReadAttribute(request, encoder);

        if (!result.IsSuccess())
        {
            app::DataModel::ActionReturnStatus::StringStorage storage;
            ChipLogError(Support, "Failed to read data: %s", result.c_str(storage));
            return ::pw::Status::Internal();
        }

        attributeReports.EndOfContainer();
        PW_TRY(ChipErrorToPwStatus(writer.EndContainer(outer)));
        PW_TRY(ChipErrorToPwStatus(writer.Finalize()));
        tlvBuffer.reduce_size(writer.GetLengthWritten());

        return ::pw::OkStatus();
    }

    template <typename T>
    CHIP_ERROR TlvGet(TLV::TLVReader & reader, T & value)
    {
        return reader.Get(value);
    }

    CHIP_ERROR TlvGet(TLV::TLVReader & reader, chip_rpc_AttributeData_data_bytes_t & value)
    {
        value.size = reader.GetLength();
        return reader.GetBytes(value.bytes, sizeof(value.bytes));
    }

    template <size_t N>
    CHIP_ERROR TlvGet(TLV::TLVReader & reader, char (&value)[N])
    {
        return reader.GetString(value, N);
    }

    template <typename T>
    ::pw::Status TlvBufferGetData(ByteSpan tlvBuffer, TLV::TLVType expectedDataType, T & responseData)
    {
        TLV::TLVReader reader;
        reader.Init(tlvBuffer);
        // Open outer container
        PW_TRY(ChipErrorToPwStatus(reader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag())));
        TLV::TLVType readerRoot;
        PW_TRY(ChipErrorToPwStatus(reader.EnterContainer(readerRoot)));

        // Open report container
        PW_TRY(ChipErrorToPwStatus(reader.Next(TLV::TLVType::kTLVType_Array, TLV::ContextTag(kReportContextTag))));
        TLV::TLVType readerArray;
        PW_TRY(ChipErrorToPwStatus(reader.EnterContainer(readerArray)));

        // Skip first array element which is the empty array from spec 10.5.4.3
        PW_TRY(ChipErrorToPwStatus(reader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag())));

        // Parse the AttributeDataIB to pull out data
        app::AttributeReportIB::Parser reportParser;
        PW_TRY(ChipErrorToPwStatus(reportParser.Init(reader)));
        app::AttributeDataIB::Parser dataParser;
        PW_TRY(ChipErrorToPwStatus(reportParser.GetAttributeData(&dataParser)));
        TLV::TLVReader dataReader;
        PW_TRY(ChipErrorToPwStatus(dataParser.GetData(&dataReader)));
        PW_TRY(CheckTlvTagAndType(&dataReader, TLV::ContextTag(0x2), expectedDataType));
        PW_TRY(ChipErrorToPwStatus(TlvGet(dataReader, responseData)));

        return ::pw::OkStatus();
    }

    static ::pw::Status ChipErrorToPwStatus(CHIP_ERROR err)
    {
        if (err == CHIP_NO_ERROR)
        {
            return ::pw::OkStatus();
        }
        else if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return ::pw::Status::ResourceExhausted();
        }
        return ::pw::Status::Internal();
    }

    static ::pw::Status CheckTlvTagAndType(TLV::TLVReader * reader, TLV::Tag expectedTag, TLV::TLVType expectedType)
    {
        if (reader->GetTag() != expectedTag || reader->GetType() != expectedType)
        {
            return ::pw::Status::NotFound();
        }
        return ::pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
