/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

#include "DeviceCallbacks.h"
#if CONFIG_ENABLE_FEEDBACK
#include "UserInterfaceFeedback.h"
#endif

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void LightingApp::DeviceCallbacks::PostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                               chip::AttributeId attributeId, uint8_t type, uint16_t size,
                                                               uint8_t * value)
{
    if (clusterId == OnOff::Id)
    {
        if (attributeId != OnOff::Attributes::OnOff::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().RestoreState();
#endif
    }
    else if (clusterId == LevelControl::Id)
    {
        if (attributeId != LevelControl::Attributes::CurrentLevel::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
#if LIGHTING_MANAGER_ENABLE_DIMMABLE_LED
        if (*value > 1 && *value < 254)
        {
            ChipLogProgress(Zcl, "Setting value: %d", *value);
            // The cluster attribute value will be read in RestoreState and the proper action will be taken: e.g. ApplyDim.
#if CONFIG_ENABLE_FEEDBACK
            FeedbackMgr().RestoreState();
#endif
        }
#endif
    }
    else if (clusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    static LightingApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}
