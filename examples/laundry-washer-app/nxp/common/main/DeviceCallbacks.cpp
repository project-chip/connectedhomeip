/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright 2024 NXP
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <laundry-washer-mode.h>

#include <lib/support/CodeUtils.h>

using namespace chip::app;
void OnTriggerEffect(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerEffect,
};

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;

void LaundryWasherApp::DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId,
                                                                    AttributeId attributeId, uint8_t type, uint16_t size,
                                                                    uint8_t * value)
{
    ChipLogProgress(DeviceLayer,
                    "endpointId " ChipLogFormatMEI " clusterId " ChipLogFormatMEI " attribute ID: " ChipLogFormatMEI
                    " Type: %u Value: %u, length %u",
                    ChipLogValueMEI(endpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId), type, *value, size);
    switch (clusterId)
    {
    case Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;
    }
}

void LaundryWasherApp::DeviceCallbacks::OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId,
                                                                           chip::AttributeId attributeId, uint8_t * value)
{
    switch (attributeId)
    {
    case Clusters::OnOff::Attributes::OnOff::Id:
        if ((value != nullptr) && (*value == true))
        {
            // Update the current mode to OnMode after device is on
            ModeBase::Instance * modeInstance = LaundryWasherMode::Instance();

            if (modeInstance != nullptr)
            {
                DataModel::Nullable<uint8_t> mode = modeInstance->GetOnMode();
                if (mode.IsNull() == false)
                {
                    modeInstance->UpdateCurrentMode(mode.Value());
                }
            }
        }
        break;

    default:;
    }
}

// This returns an instance of this class.
LaundryWasherApp::DeviceCallbacks & LaundryWasherApp::DeviceCallbacks::GetDefaultInstance()
{
    static LaundryWasherApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    return LaundryWasherApp::DeviceCallbacks::GetDefaultInstance();
}
