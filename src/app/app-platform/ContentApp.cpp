/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Contains functions relating to Content App of the Video Player.
 */

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
