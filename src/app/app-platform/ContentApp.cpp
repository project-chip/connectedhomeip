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
 * @file Contains functions relating to Content App of the Video Player.
 */

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::AppPlatform;

namespace chip {
namespace AppPlatform {

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_APPLICATION_BASIC_CLUSTER_REVISION (1u)

EmberAfStatus ContentApp::HandleReadAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer,
                                              uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer,
                    "Read Attribute for endpoint " ChipLogFormatMEI " cluster " ChipLogFormatMEI " attribute " ChipLogFormatMEI,
                    ChipLogValueMEI(mEndpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId));

    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus ContentApp::HandleWriteAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer,
                    "Read Attribute for endpoint " ChipLogFormatMEI " cluster " ChipLogFormatMEI " attribute " ChipLogFormatMEI,
                    ChipLogValueMEI(mEndpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId));

    return EMBER_ZCL_STATUS_FAILURE;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
