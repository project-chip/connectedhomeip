/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BLEApplicationManager.h"

#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>
#include <src/platform/nxp/common/ble/BLEManagerCommon.h>

#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::NXP::App;

BLEApplicationManager BLEApplicationManager::sInstance;

void app_gap_callback(gapGenericEvent_t * event)
{
    /*Demo Gap application callback*/
}

void app_gatt_callback(deviceId_t id, gattServerEvent_t * event)
{
    /*Demo Gatt application callback*/

    if (event->eventType == gEvtAttributeWrittenWithoutResponse_c)
    {
        if (event->eventData.attributeWrittenEvent.handle == value_uart_stream)
        {
            ChipLogProgress(DeviceLayer, "Write to custom service");
        }
    }
}

void BLEApplicationManager::Init(void)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();

    bleManager->RegisterAppCallbacks(app_gap_callback, app_gatt_callback);
    err = bleManager->AddWriteNotificationHandle((uint16_t) value_uart_stream);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error while adding BLE write notification handle");
    }
}

void BLEApplicationManager::EnableMultipleConnectionsHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(EnableMultipleConnections, 0);
}

void BLEApplicationManager::EnableMultipleConnections(intptr_t arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Check the status of the commissioning */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Multiple connections, start advertising");
        auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();
        bleManager->SetBLEServiceMode(kMultipleBLE_Enabled);
        err = ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Error during ConnectivityMgr().SetBLEAdvertisingEnabled()");
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device must be commissioned before adding multiple BLE connections");
    }
}
