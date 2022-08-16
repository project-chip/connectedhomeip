/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;

namespace chip {

namespace DeviceManager {

using namespace ::chip::DeviceLayer;

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

    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    if (CONFIG_NETWORK_LAYER_BLE)
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    }

    err = Platform::MemoryInit();
    SuccessOrExit(err);

    PlatformMgr().AddEventHandler(CHIPDeviceManager::CommonDeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "StartEventLoopTask() - ERROR!\r\n");
    }
    else
    {
        ChipLogProgress(Zcl, "StartEventLoopTask() - OK\r\n");
    }

exit:
    return err;
}
} // namespace DeviceManager
} // namespace chip

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                       uint16_t size, uint8_t * value)
{
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();

    EndpointId endpointId   = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }
    }
    else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        if (attributeId != ZCL_CURRENT_LEVEL_ATTRIBUTE_ID)
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
    else if (clusterId == ZCL_COLOR_CONTROL_CLUSTER_ID)
    {
        uint8_t hue, saturation;

        if ((attributeId != ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID) &&
            (attributeId != ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID))
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
        {
            hue = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                       &saturation, sizeof(uint8_t));
        }
        if (attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID)
        {
            saturation = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                       sizeof(uint8_t));
        }
        ChipLogProgress(Zcl, "New hue: %d, New saturation: %d ", hue, saturation);
    }
    else if (clusterId == ZCL_IDENTIFY_CLUSTER_ID)
    {
        if (attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID)
        {
            if (cb != nullptr)
            {
                cb->PostAttributeChangeCallback(endpointId, clusterId, attributeId, mask, type, size, value);
            }
            ChipLogProgress(Zcl, "ZCL_IDENTIFY_TIME_ATTRIBUTE_ID value: %u ", *value);
        }
    }
    else
    {
        // ChipLogProgress(Zcl, "Unknown cluster ID: %" PRIx32, clusterId);
        return;
    }
}
