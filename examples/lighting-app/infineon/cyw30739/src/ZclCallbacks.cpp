/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightingManager.h"
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
    case OnOff::Id:
        if (attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
        {
            printf("ZCL OnOff -> %u\n", *value);
            LightMgr().InitiateAction(LightingManager::ACTOR_ZCL_CMD,
                                      (*value == 0) ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION, *value);
            return;
        }
        break;
    case LevelControl::Id:
        if (attributePath.mAttributeId == LevelControl::Attributes::CurrentLevel::Id)
        {
            printf("ZCL CurrentLevel -> %u\n", *value);
            LightMgr().InitiateAction(LightingManager::ACTOR_ZCL_CMD, LightingManager::LEVEL_ACTION, *value);
            return;
        }
        break;
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
    default:
        printf("Unhandled cluster ID: 0x%04lx\n", attributePath.mClusterId);
        return;
    }

    printf("ERROR clusterId: 0x%04lx, unknown attribute ID: 0x%04lx\n", attributePath.mClusterId, attributePath.mAttributeId);
}
