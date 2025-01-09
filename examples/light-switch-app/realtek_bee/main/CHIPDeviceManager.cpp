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
#include <platform/Realtek_bee/FactoryDataProvider.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

#include "Globals.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;

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

    ChipLogProgress(DeviceLayer, "Start to init MemoryInit");
    err = Platform::MemoryInit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "Start to init InitChipStack");
    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "Factorydata init");
    err = mFactoryDataProvider.Init();
    SuccessOrExit(err);
    SetCommissionableDataProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);

#if CONFIG_NETWORK_LAYER_BLE
    ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

    PlatformMgr().AddEventHandler(CHIPDeviceManager::CommonDeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);

#if CHIP_ENABLE_OPENTHREAD
    ChipLogProgress(DeviceLayer, "Initializing OpenThread stack");
    err = ThreadStackMgr().InitThreadStack();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "Set Thread Device Type");

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else // CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if CHIP_DEVICE_CONFIG_THREAD_SSED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#endif
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "Start OpenThread task");
    err = ThreadStackMgrImpl().StartThreadTask();
    SuccessOrExit(err);
    ChipLogProgress(DeviceLayer, "Start OpenThread task done!!");
#endif // CHIP_ENABLE_OPENTHREAD

exit:
    return err;
}

void CHIPDeviceManager::Shutdown()
{
    PlatformMgr().Shutdown();
}

} // namespace DeviceManager
} // namespace chip
