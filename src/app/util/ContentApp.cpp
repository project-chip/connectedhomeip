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
 * @file Contains shell commands for a commissionee (eg. end device) related to commissioning.
 */

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/ContentAppPlatform.h>
#include <cstdio>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::AppPlatform;

namespace chip {
namespace AppPlatform {

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_APPLICATION_BASIC_CLUSTER_REVISION (1u)

EmberAfStatus ContentApp::HandleReadAttribute(ClusterId clusterId, chip::AttributeId attributeId, uint8_t * buffer,
                                              uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "Read Attribute for device %s cluster %d attribute=%d)",
                    GetApplicationBasic()->GetApplicationName(), static_cast<uint16_t>(clusterId),
                    static_cast<uint16_t>(attributeId));

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;
    if (clusterId == ZCL_APPLICATION_BASIC_CLUSTER_ID)
    {
        ret = GetApplicationBasic()->HandleReadAttribute(attributeId, buffer, maxReadLength);
    }
    if (clusterId == ZCL_ACCOUNT_LOGIN_CLUSTER_ID)
    {
        ret = GetAccountLogin()->HandleReadAttribute(attributeId, buffer, maxReadLength);
    }
    return ret;
}

EmberAfStatus ContentApp::HandleWriteAttribute(ClusterId clusterId, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "Read Attribute for device %s cluster %d attribute=%d)",
                    GetApplicationBasic()->GetApplicationName(), static_cast<uint16_t>(clusterId),
                    static_cast<uint16_t>(attributeId));

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    if (clusterId == ZCL_APPLICATION_BASIC_CLUSTER_ID)
    {
        ret = GetApplicationBasic()->HandleWriteAttribute(attributeId, buffer);
    }
    if (clusterId == ZCL_ACCOUNT_LOGIN_CLUSTER_ID)
    {
        ret = GetAccountLogin()->HandleWriteAttribute(attributeId, buffer);
    }
    return ret;
}

EmberAfStatus ApplicationBasic::HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic::HandleReadAttribute: attrId=%d, maxReadLength=%d",
                    static_cast<uint16_t>(attributeId), maxReadLength);

    if ((attributeId == ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        uint8_t bufferMemory[254];
        MutableByteSpan zclString(bufferMemory);
        MakeZclCharString(zclString, GetVendorName());
        buffer = zclString.data();
    }
    else if ((attributeId == ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *(uint16_t *) buffer = GetVendorId();
    }
    else if ((attributeId == ZCL_APPLICATION_NAME_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        uint8_t bufferMemory[254];
        MutableByteSpan zclString(bufferMemory);
        MakeZclCharString(zclString, GetApplicationName());
        buffer = zclString.data();
    }
    else if ((attributeId == ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *(uint16_t *) buffer = GetProductId();
    }
    else if ((attributeId == ZCL_APPLICATION_STATUS_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = (uint8_t) GetApplicationStatus();
    }
    else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
    {
        *buffer = (uint16_t) ZCL_APPLICATION_BASIC_CLUSTER_REVISION;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus ApplicationBasic::HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "ApplicationBasic::HandleWriteAttribute: attrId=%d", static_cast<uint16_t>(attributeId));

    if (attributeId == ZCL_APPLICATION_STATUS_ATTRIBUTE_ID)
    {
        if (*buffer)
        {
            SetApplicationStatus(app::Clusters::ApplicationBasic::ApplicationBasicStatus::kActiveVisibleFocus);
        }
        else
        {
            SetApplicationStatus(app::Clusters::ApplicationBasic::ApplicationBasicStatus::kActiveVisibleNotFocus);
        }
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus AccountLogin::HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "AccountLogin::HandleReadAttribute: attrId=%d, maxReadLength=%d",
                    static_cast<uint16_t>(attributeId), maxReadLength);
    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus AccountLogin::HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "AccountLogin::HandleWriteAttribute: attrId=%d", static_cast<uint16_t>(attributeId));
    return EMBER_ZCL_STATUS_FAILURE;
}

} // namespace AppPlatform
} // namespace chip
