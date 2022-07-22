/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "LightingManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

void emberAfBasicClusterInitCallback(EndpointId endpoint)
{
    uint16_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    char cString[16] = "00000000";

    if (GetDeviceInstanceInfoProvider()->GetManufacturingDate(year, month, dayOfMonth) == CHIP_NO_ERROR)
    {
        snprintf(cString, sizeof(cString), "%04u%02u%02u", year, month, dayOfMonth);
    }
    Basic::Attributes::ManufacturingDate::Set(endpoint, CharSpan(cString));
}

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
