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

#include <BoltLockManager.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

void emberAfBasicClusterInitCallback(EndpointId endpoint)
{
    uint16_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    char cString[16] = "00000000";

    if (ConfigurationMgr().GetManufacturingDate(year, month, dayOfMonth) == CHIP_NO_ERROR)
    {
        snprintf(cString, sizeof(cString), "%04u%02u%02u", year, month, dayOfMonth);
    }
    Basic::Attributes::ManufacturingDate::Set(endpoint, CharSpan(cString));
}

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type, uint16_t size,
                                       uint8_t * value)

{
    switch (attributePath.mClusterId)
    {
    case OnOff::Id:
        if (attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
        {
            printf("ZCL OnOff -> %u\n", *value);
            BoltLockMgr().InitiateAction(BoltLockManager::ACTOR_ZCL_CMD,
                                         (*value) ? BoltLockManager::LOCK_ACTION : BoltLockManager::UNLOCK_ACTION);
            return;
        }
        break;
    default:
        printf("Unhandled cluster ID: 0x%04lx\n", attributePath.mClusterId);
        return;
    }

    printf("ERROR clusterId: 0x%04lx, unknown attribute ID: 0x%04lx\n", attributePath.mClusterId, attributePath.mAttributeId);
}
