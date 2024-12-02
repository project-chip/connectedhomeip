/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include "AppTask.h"

#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/telink/wifi/TelinkWiFiDriver.h>
#endif

#include <zephyr/kernel.h>

#ifdef CONFIG_USB_DEVICE_STACK
#include <zephyr/usb/usb_device.h>
#endif /* CONFIG_USB_DEVICE_STACK */

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
app::Clusters::NetworkCommissioning::Instance sWiFiCommissioningInstance(0, &(NetworkCommissioning::TelinkWiFiDriver::Instance()));
#endif

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
static constexpr uint32_t kFactoryResetOnBootMaxCnt       = 5;
static constexpr char kFactoryResetOnBootStoreKey[]       = "TelinkFactoryResetOnBootCnt";
static constexpr uint32_t kFactoryResetUsualBootTimeoutMs = 5000;

static k_timer FactoryResetUsualBootTimer;

static void FactoryResetUsualBoot(struct k_timer * dummy)
{
    (void) dummy;
    (void) chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFactoryResetOnBootStoreKey);
    LOG_INF("Schedule factory counter deleted");
}

static void FactoryResetOnBoot(void)
{
    uint32_t FactoryResetOnBootCnt;
    CHIP_ERROR FactoryResetOnBootErr = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(
        kFactoryResetOnBootStoreKey, &FactoryResetOnBootCnt, sizeof(FactoryResetOnBootCnt));

    if (FactoryResetOnBootErr == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        FactoryResetOnBootCnt = 1;
        if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFactoryResetOnBootStoreKey, &FactoryResetOnBootCnt,
                                                                        sizeof(FactoryResetOnBootCnt)) != CHIP_NO_ERROR)
        {
            LOG_ERR("FactoryResetOnBootCnt write fail");
        }
        else
        {
            LOG_INF("Schedule factory counter %u", FactoryResetOnBootCnt);
        }
    }
    else if (FactoryResetOnBootErr == CHIP_NO_ERROR)
    {
        FactoryResetOnBootCnt++;
        if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFactoryResetOnBootStoreKey, &FactoryResetOnBootCnt,
                                                                        sizeof(FactoryResetOnBootCnt)) != CHIP_NO_ERROR)
        {
            LOG_ERR("FactoryResetOnBootCnt write fail");
        }
        else
        {
            LOG_INF("Schedule factory counter %u", FactoryResetOnBootCnt);
            if (FactoryResetOnBootCnt >= kFactoryResetOnBootMaxCnt)
            {
                GetAppTask().PowerOnFactoryReset();
            }
        }
    }
    else
    {
        LOG_ERR("FactoryResetOnBootCnt read fail");
    }
    k_timer_init(&FactoryResetUsualBootTimer, FactoryResetUsualBoot, nullptr);
    k_timer_start(&FactoryResetUsualBootTimer, K_MSEC(kFactoryResetUsualBootTimeoutMs), K_NO_WAIT);
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

int main(void)
{
#if defined(CONFIG_USB_DEVICE_STACK) && !defined(CONFIG_CHIP_PW_RPC)
    usb_enable(NULL);
#endif /* CONFIG_USB_DEVICE_STACK */

    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef CONFIG_CHIP_PW_RPC
    rpc::Init();
#endif

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("MemoryInit fail");
        goto exit;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitChipStack fail");
        goto exit;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("StartEventLoopTask fail");
        goto exit;
    }

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
    FactoryResetOnBoot();
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitThreadStack fail");
        goto exit;
    }

#if defined(CONFIG_CHIP_THREAD_DEVICE_ROLE_ROUTER)
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#elif defined(CONFIG_CHIP_THREAD_DEVICE_ROLE_END_DEVICE)
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#elif defined(CONFIG_CHIP_THREAD_DEVICE_ROLE_SLEEPY_END_DEVICE)
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
#error THREAD_DEVICE_ROLE not selected
#endif
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetThreadDeviceType fail");
        goto exit;
    }
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sWiFiCommissioningInstance.Init();
#else
    err = CHIP_ERROR_INTERNAL;
    goto exit;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD */

    err = GetAppTask().StartApp();

exit:
    LOG_ERR("Exit err %d", err.Format());
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}
