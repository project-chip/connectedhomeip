/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    switch (attributePath.mClusterId)
    {
    case Identify::Id:
        if (attributePath.mAttributeId == Identify::Attributes::IdentifyTime::Id)
        {
            uint16_t identifyTime;
            if (EMBER_ZCL_STATUS_SUCCESS == Identify::Attributes::IdentifyTime::Get(attributePath.mEndpointId, &identifyTime))
            {
                ChipLogProgress(Zcl, "IdentifyTime %u", identifyTime);
                return;
            }
        }
        break;
    case OnOffSwitchConfiguration::Id:
        printf("OnOff Switch Configuration attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u\n",
               ChipLogValueMEI(attributePath.mAttributeId), type, *value, size);
        return;
    default:
        printf("Unhandled cluster ID: 0x%04lx\n", attributePath.mClusterId);
        return;
    }

    printf("ERROR clusterId: 0x%04lx, unknown attribute ID: 0x%04lx\n", attributePath.mClusterId, attributePath.mAttributeId);
}
