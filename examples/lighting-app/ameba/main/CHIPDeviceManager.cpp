/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *      This file implements the CHIP Device Interface that is used by
 *      applications to interact with the CHIP stack
 *
 */

#include <stdlib.h>

#include "CHIPDeviceManager.h"
#include <app/util/basic-types.h>
#include <core/ErrorStr.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/Ameba/FactoryDataProvider.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

#include "Globals.h"
#include "LEDWidget.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;
using namespace ::chip::Credentials;

namespace chip {

namespace DeviceManager {

using namespace ::chip::DeviceLayer;

chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

void CHIPDeviceManager::CommonDeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    CHIPDeviceManagerCallbacks * cb = reinterpret_cast<CHIPDeviceManagerCallbacks *>(arg);
    if (cb != nullptr)
    {
        cb->DeviceEventCallback(event, reinterpret_cast<intptr_t>(cb));
    }
}

CHIP_ERROR CHIPDeviceManager::Init(CHIPDeviceManagerCallbacks * cb)
{
    CHIP_ERROR err;
    mCB = cb;

    err = Platform::MemoryInit();
    SuccessOrExit(err);

    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = mFactoryDataProvider.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error initializing FactoryData!");
        ChipLogError(DeviceLayer, "Check if you have flashed it correctly!");
    }
    SetCommissionableDataProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);

#if CONFIG_NETWORK_LAYER_BLE
    ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

    PlatformMgr().AddEventHandler(CHIPDeviceManager::CommonDeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);

exit:
    return err;
}
} // namespace DeviceManager
} // namespace chip

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();

    EndpointId endpointId   = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == app::Clusters::OnOff::Id)
    {
        if (attributeId != app::Clusters::OnOff::Attributes::OnOff::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        statusLED1.Set(*value);
    }
    else if (clusterId == app::Clusters::LevelControl::Id)
    {
        if (attributeId != app::Clusters::LevelControl::Attributes::CurrentLevel::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }
        if (size == 1)
        {
            // ChipLogProgress(Zcl, "New level: %u ", *value);
        }
        else
        {
            ChipLogError(Zcl, "wrong length for level: %d\n", size);
        }
    }
    else if (clusterId == app::Clusters::ColorControl::Id)
    {
        using namespace app::Clusters::ColorControl::Attributes;

        uint8_t hue, saturation;

        if ((attributeId != CurrentHue::Id) && (attributeId != CurrentSaturation::Id))
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        if (attributeId == CurrentHue::Id)
        {
            hue = *value;
            CurrentSaturation::Get(endpointId, &saturation);
        }
        if (attributeId == CurrentSaturation::Id)
        {
            saturation = *value;
            CurrentHue::Get(endpointId, &hue);
        }
        ChipLogProgress(Zcl, "New hue: %d, New saturation: %d ", hue, saturation);
    }
    else if (clusterId == app::Clusters::Identify::Id)
    {
        if (attributeId == app::Clusters::Identify::Attributes::IdentifyTime::Id)
        {
            if (cb != nullptr)
            {
                cb->PostAttributeChangeCallback(endpointId, clusterId, attributeId, type, size, value);
            }
            ChipLogProgress(Zcl, "IdentifyTime value: %u ", *value);
        }
    }
    else
    {
        // ChipLogProgress(Zcl, "Unknown cluster ID: %" PRIx32, clusterId);
        return;
    }
}
