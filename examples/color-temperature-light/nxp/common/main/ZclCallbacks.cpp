/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>

#include "LightingManager.h"

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
	/*
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        // propagate event to device manager
        cb->PostAttributeChangeCallback(path.mEndpointId, path.mClusterId, path.mAttributeId, type, size, value);
    }*/
	if (path.mClusterId == OnOff::Id)
    {
        ChipLogProgress(Zcl, "==============================================================================");
        ChipLogProgress(Zcl, "OnOff attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                                 ChipLogValueMEI(path.mAttributeId), type, *value, size);
        ChipLogProgress(Zcl, "==============================================================================");
		  
		if (path.mAttributeId != OnOff::Attributes::OnOff::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mAttributeId));
            return;
        }

        LightingMgr().InitiateAction(0, *value ? LightingManager::TURNON_ACTION : LightingManager::TURNOFF_ACTION);
		  
    }
/*
    else if (path.mClusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                                 ChipLogValueMEI(path.mAttributeId), type, *value, size);
        ChipLogProgress(Zcl, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
	else if (path.mClusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "-------------------------------------------------------------------------------");
		ChipLogProgress(Zcl,
                        "Color Control attribute ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        ChipLogValueMEI(path.mAttributeId), type, *value, size);
		ChipLogProgress(Zcl, "-------------------------------------------------------------------------------");
     }
    else
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mAttributeId));
    }
*/
}
