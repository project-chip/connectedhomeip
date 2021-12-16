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

#include "app/util/attribute-storage.h"
#include "attributes_service/attributes_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>

namespace chip {
namespace rpc {

// Implementation class for chip.rpc.Attributes.
class Attributes : public pw_rpc::nanopb::Attributes::Service<Attributes>
{
public:
    ::pw::Status Write(const chip_rpc_AttributeWrite & request, pw_protobuf_Empty & response)
    {
        const void * data;
        switch (request.data.which_data)
        {
        case chip_rpc_AttributeData_data_bool_tag:
            data = &request.data.data.data_bool;
            break;
        case chip_rpc_AttributeData_data_uint8_tag:
            data = &request.data.data.data_uint8;
            break;
        case chip_rpc_AttributeData_data_uint16_tag:
            data = &request.data.data.data_uint16;
            break;
        case chip_rpc_AttributeData_data_uint32_tag:
            data = &request.data.data.data_uint32;
            break;
        case chip_rpc_AttributeData_data_int8_tag:
            data = &request.data.data.data_int8;
            break;
        case chip_rpc_AttributeData_data_int16_tag:
            data = &request.data.data.data_int16;
            break;
        case chip_rpc_AttributeData_data_int32_tag:
            data = &request.data.data.data_int32;
            break;
        case chip_rpc_AttributeData_data_bytes_tag:
            data = &request.data.data.data_bytes;
            break;
        default:
            return pw::Status::InvalidArgument();
        }
        RETURN_STATUS_IF_NOT_OK(
            emberAfWriteServerAttribute(request.metadata.endpoint, request.metadata.cluster, request.metadata.attribute_id,
                                        const_cast<uint8_t *>(static_cast<const uint8_t *>(data)), request.metadata.type));
        return pw::OkStatus();
    }

    ::pw::Status Read(const chip_rpc_AttributeMetadata & request, chip_rpc_AttributeData & response)
    {
        void * data;
        size_t size = 0;
        switch (request.type)
        {
        case chip_rpc_AttributeType_ZCL_BOOLEAN_ATTRIBUTE_TYPE:
            data                = &response.data.data_bool;
            size                = sizeof(response.data.data_bool);
            response.which_data = chip_rpc_AttributeData_data_bool_tag;
            break;
        case chip_rpc_AttributeType_ZCL_ENUM8_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_BITMAP8_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT8U_ATTRIBUTE_TYPE:
            data                = &response.data.data_uint8;
            size                = sizeof(response.data.data_uint8);
            response.which_data = chip_rpc_AttributeData_data_uint8_tag;
            break;
        case chip_rpc_AttributeType_ZCL_ENUM16_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_BITMAP16_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT16U_ATTRIBUTE_TYPE:
            data                = &response.data.data_uint16;
            size                = sizeof(response.data.data_uint16);
            response.which_data = chip_rpc_AttributeData_data_uint16_tag;
            break;
        case chip_rpc_AttributeType_ZCL_BITMAP32_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT32U_ATTRIBUTE_TYPE:
            data                = &response.data.data_uint32;
            size                = sizeof(response.data.data_uint32);
            response.which_data = chip_rpc_AttributeData_data_uint32_tag;
            break;
        case chip_rpc_AttributeType_ZCL_ARRAY_ATTRIBUTE_TYPE:
            // We don't know how to read these; need to get the right
            // AttributeAccessInterface.
            return pw::Status::InvalidArgument();
        case chip_rpc_AttributeType_ZCL_BITMAP64_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT24U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT40U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT48U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT56U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT64U_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT8S_ATTRIBUTE_TYPE:
            data                = &response.data.data_int8;
            size                = sizeof(response.data.data_int8);
            response.which_data = chip_rpc_AttributeData_data_int8_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT16S_ATTRIBUTE_TYPE:
            data                = &response.data.data_int16;
            size                = sizeof(response.data.data_int16);
            response.which_data = chip_rpc_AttributeData_data_int16_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT24S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT32S_ATTRIBUTE_TYPE:
            data                = &response.data.data_int32;
            size                = sizeof(response.data.data_int32);
            response.which_data = chip_rpc_AttributeData_data_int32_tag;
            break;
        case chip_rpc_AttributeType_ZCL_INT40S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT48S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT56S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_INT64S_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_SINGLE_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_DOUBLE_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
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
            data = response.data.data_bytes.bytes;
            size = emberAfGetDataSize(request.type);
            if (size > sizeof(response.data.data_bytes.bytes))
            {
                return pw::Status::OutOfRange();
            }
            response.data.data_bytes.size = size;
            response.which_data           = chip_rpc_AttributeData_data_bytes_tag;
            break;
        case chip_rpc_AttributeType_ZCL_NO_DATA_ATTRIBUTE_TYPE:
        case chip_rpc_AttributeType_ZCL_UNKNOWN_ATTRIBUTE_TYPE:
        default:
            return pw::Status::InvalidArgument();
        }
        RETURN_STATUS_IF_NOT_OK(emberAfReadServerAttribute(request.endpoint, request.cluster, request.attribute_id,
                                                           static_cast<uint8_t *>(data), size));
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
