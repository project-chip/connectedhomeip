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
#include <app/EventLogging.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void logBooleanStateEvent(bool state)
{
    chip::EventNumber eventNumber;
    chip::app::Clusters::BooleanState::Events::StateChange::Type event{ state };
    if (CHIP_NO_ERROR != chip::app::LogEvent(event, 1, eventNumber))
    {
        ChipLogProgress(Zcl, "booleanstate: failed to reacord state-change event");
    }
}

void ContactSensorApp::DeviceCallbacks::PostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                    chip::AttributeId attributeId, uint8_t type, uint16_t size,
                                                                    uint8_t * value)
{
    if (clusterId == BooleanState::Id)
    {
        if (attributeId != BooleanState::Attributes::StateValue::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().RestoreState();
#endif
        logBooleanStateEvent(static_cast<bool>(*value));
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    static ContactSensorApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}
