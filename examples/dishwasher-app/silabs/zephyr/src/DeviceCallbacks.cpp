/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "DeviceCallbacks.h"
#include "DishwasherManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DeviceEnergyManagement;
using namespace ::chip::app::Clusters::OperationalState;

void DishwasherApp::DeviceCallbacks::PostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                 chip::AttributeId attributeId, uint8_t type, uint16_t size,
                                                                 uint8_t * value)
{
    if ((clusterId == OperationalState::Id) && (attributeId == OperationalState::Attributes::OperationalState::Id))
    {
        VerifyOrReturn(value != nullptr);
        OperationalState::OperationalStateEnum state = static_cast<OperationalState::OperationalStateEnum>(*value);
        DishwasherManager * mgr = GetDishwasherManager();
        if (mgr != nullptr)
        {
            mgr->UpdateOperationState(state);
        }
        UpdateEpmAttributesForOperationalState(state);
    }
    else if (clusterId == Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::Zephyr::App::GetDeviceCallbacks()
{
    static DishwasherApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}
