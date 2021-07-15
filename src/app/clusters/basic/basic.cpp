/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *
 */

#include "basic.h"

#include <platform/CHIPDeviceLayer.h>

#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include <cstring>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {
template <size_t BufferLength>
uint8_t * MakeZclCharString(uint8_t (&zclString)[BufferLength], char (&cString)[BufferLength])
{
    static_assert(BufferLength <= 256, "Too long string to fit in ZCL_CHAR_STRING type");
    zclString[0] = static_cast<uint8_t>(strlen(cString));
    memcpy(&zclString[1], cString, zclString[0]);
    return zclString;
}
} // namespace

void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint)
{
    uint16_t vendorId;
    uint16_t productId;
    uint16_t productRevision;
    uint32_t firmwareRevision;
    char cString[65];
    uint8_t zclString[65];

    if (ConfigurationMgr().GetVendorName(cString, sizeof(cString)) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_VENDOR_NAME_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              MakeZclCharString(zclString, cString), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetVendorId(vendorId) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_VENDOR_ID_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              reinterpret_cast<uint8_t *>(&vendorId), ZCL_INT16U_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetProductName(cString, sizeof(cString)) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_PRODUCT_NAME_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              MakeZclCharString(zclString, cString), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetProductId(productId) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_PRODUCT_ID_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              reinterpret_cast<uint8_t *>(&productId), ZCL_INT16U_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetProductRevisionString(cString, sizeof(cString)) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_HARDWARE_VERSION_STRING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              MakeZclCharString(zclString, cString), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetProductRevision(productRevision) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_HARDWARE_VERSION_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              reinterpret_cast<uint8_t *>(&productRevision), ZCL_INT16U_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetFirmwareRevisionString(cString, sizeof(cString)) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_SOFTWARE_VERSION_STRING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              MakeZclCharString(zclString, cString), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    }

    if (ConfigurationMgr().GetFirmwareRevision(firmwareRevision) == CHIP_NO_ERROR)
    {
        emberAfWriteAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_SOFTWARE_VERSION_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                              reinterpret_cast<uint8_t *>(&firmwareRevision), ZCL_INT32U_ATTRIBUTE_TYPE);
    }
}
