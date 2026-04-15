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

#pragma once

#include <app/server/Server.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>
#ifdef CONFIG_CHIP_WIFI
#include "WiFiManager.h"
#endif

class AppFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    static void Init()
    {
#ifndef CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
        static AppFabricTableDelegate sAppFabricDelegate;
        LogErrorOnFailure(chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sAppFabricDelegate));
        k_timer_init(&sFabricRemovedTimer, &OnFabricRemovedTimerCallback, nullptr);
#endif // CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
    }

private:
    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex)
    {
#ifndef CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
        auto & server = chip::Server::GetInstance();

        if (server.GetFabricTable().FabricCount() == 0)
        {
            // ScheduleFactoryReset in case of failed commissioning
            if (AppTaskCommon::IsCommissioningFailed())
            {
                chip::Server::GetInstance().ScheduleFactoryReset();
                return;
            }

            if (chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                server.GetCommissioningWindowManager().CloseCommissioningWindow();
            }

            k_timer_start(&sFabricRemovedTimer, K_MSEC(CONFIG_CHIP_LAST_FABRIC_REMOVED_ACTION_DELAY), K_NO_WAIT);
        }
#endif // CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
    }

    static void OnFabricRemovedTimerCallback(k_timer * timer)
    {
#ifndef CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) {
#ifdef CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_AND_REBOOT
                chip::Server::GetInstance().ScheduleFactoryReset();
#elif defined(CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_ONLY)
                ChipLogProgress(DeviceLayer, "Erasing settings partition");
                // TC-OPCREDS-3.6 (device doesn't need to reboot automatically after the last fabric is removed) can't use
                // FactoryReset
                void * storage = nullptr;
                int status     = settings_storage_get(&storage);

                if (!status)
                {
                    status = nvs_clear(static_cast<nvs_fs *>(storage));
                }

                if (!status)
                {
                    status = nvs_mount(static_cast<nvs_fs *>(storage));
                }

                if (status)
                {
                    ChipLogError(DeviceLayer, "Storage clear failed: %d", status);
                }
#ifdef CONFIG_TFLM_FEATURE
                AppTask::MicroSpeechProcessStop();
#endif
#elif defined(CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_AND_PAIRING_START)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
                LogErrorOnFailure(chip::DeviceLayer::ThreadStackMgr().ClearAllSrpHostAndServices());
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
       // Erase Matter data
                CHIP_ERROR err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().DoFactoryReset();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Factory reset err: %" CHIP_ERROR_FORMAT, err.Format());
                }
                // Erase Network credentials and disconnect
                chip::DeviceLayer::ConnectivityMgr().ErasePersistentInfo();
#ifdef CONFIG_CHIP_WIFI
                LogErrorOnFailure(chip::DeviceLayer::WiFiManager::Instance().Disconnect());
                chip::DeviceLayer::ConnectivityMgr().ClearWiFiStationProvision();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
                LogErrorOnFailure(chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
                LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(StartThreadPrescan, 0));
#else
                // Start the New BLE advertising
                if (!chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled())
                {
                    if (CHIP_NO_ERROR == chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow())
                    {
                        return;
                    }
                }
                ChipLogError(FabricProvisioning, "Could not start Bluetooth LE advertising");
#endif
#endif // CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_AND_REBOOT
            }));
        }
#endif // CONFIG_CHIP_LAST_FABRIC_REMOVED_NONE
    }

    inline static k_timer sFabricRemovedTimer;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION &&                                      \
    CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_AND_PAIRING_START
    class PairingScanCallback : public DeviceLayer::NetworkCommissioning::ThreadDriver::ScanCallback
    {
    public:
        void OnFinished(NetworkCommissioning::Status err, CharSpan debugText,
                        NetworkCommissioning::ThreadScanResponseIterator * networks) override
        {
            LogErrorOnFailure(chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow());
        }
    };

    static void StartThreadPrescan(intptr_t)
    {
        static PairingScanCallback sPairingScanCallback;
        LogErrorOnFailure(chip::DeviceLayer::ThreadStackMgrImpl().StartThreadScan(&sPairingScanCallback));
    }
#endif
};
