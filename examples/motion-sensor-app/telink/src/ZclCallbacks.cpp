/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AppTask.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t, uint16_t, uint8_t * value)
{
    if (attributePath.mClusterId == OccupancySensing::Id &&
        attributePath.mAttributeId == OccupancySensing::Attributes::Occupancy::Id)
    {
        ChipLogProgress(Zcl, "Occupancy changed externally: %u", *value);

        if (!GetAppTask().IsSyncClusterToButtonAction())
        {
            GetAppTask().PostMotionActionRequest((*value & 0x01) ? MotionSensorManager::Action::kSetDetected
                                                                 : MotionSensorManager::Action::kSetUndetected);
        }
        GetAppTask().SetSyncClusterToButtonAction(false);
    }
}

void emberAfOccupancySensingClusterInitCallback(chip::EndpointId)
{
    ChipLogProgress(Zcl, "Occupancy cluster init");
    GetAppTask().UpdateClusterState();
}
