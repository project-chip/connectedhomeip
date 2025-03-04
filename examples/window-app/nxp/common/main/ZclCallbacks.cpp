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

#include "WindowCoveringManager.h"
#include <app/clusters/window-covering-server/window-covering-delegate.h>

using namespace ::chip;
using namespace ::chip::app::Clusters::WindowCovering;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
	/*
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        // propagate event to device manager
        cb->PostAttributeChangeCallback(path.mEndpointId, path.mClusterId, path.mAttributeId, type, size, value);
    }
	*/
	if (path.mClusterId == app::Clusters::WindowCovering::Id)
    {
        ChipLogProgress(Zcl, "-------------------------------------------------------------------------------");
        ChipLogProgress(Zcl, "Window covering cluster ID: " ChipLogFormatMEI " Type: %u Value: %u, length: %u",
                        ChipLogValueMEI(path.mAttributeId), type, *value, size);			 
        ChipLogProgress(Zcl, "-------------------------------------------------------------------------------");
    }
}

void MatterWindowCoveringClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    if (attributePath.mEndpointId == WindowCoveringManager::Endpoint())
    {
        switch (attributePath.mAttributeId)
        {
        case Attributes::TargetPositionLiftPercent100ths::Id:
            WindowCoveringManager::WindowCoveringMgr().StartMove(WindowCoveringType::Lift);
			ChipLogProgress(Zcl, "==============================================================================");
			ChipLogProgress(Zcl, "************ Start Move Window Covering, Type: Lift ************");
			ChipLogProgress(Zcl, "==============================================================================");
            break;
        case Attributes::TargetPositionTiltPercent100ths::Id:
            WindowCoveringManager::WindowCoveringMgr().StartMove(WindowCoveringType::Tilt);
			ChipLogProgress(Zcl, "==============================================================================");
			ChipLogProgress(Zcl, "************ Start Move Window Covering Type: Tilt) ************");
			ChipLogProgress(Zcl, "==============================================================================");
            break;
        case Attributes::CurrentPositionLiftPercent100ths::Id:
            WindowCoveringManager::WindowCoveringMgr().PositionUpdate(WindowCoveringType::Lift);
            break;
        case Attributes::CurrentPositionTiltPercent100ths::Id:
            WindowCoveringManager::WindowCoveringMgr().PositionUpdate(WindowCoveringType::Tilt);
            break;
        default:
            WindowCoveringManager::WindowCoveringMgr().SchedulePostAttributeChange(attributePath.mEndpointId, attributePath.mAttributeId);
            break;
        };
    }
}