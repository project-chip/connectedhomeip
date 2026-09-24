/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    Copyright 2026 NXP
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

#include <stdint.h>

namespace chip::NXP::App {

/**
 * @brief This class describes a manager for battery monitoring functionality.
 * It provides periodic battery level measurement and updates to the Power Source cluster.
 *
 */
class BatteryApplicationManager
{
public:
    void Init();
    void StartPeriodicUpdate();
    void StopPeriodicUpdate();
    void UpdateBatteryStatus();

private:
    void RefreshBatteryLevel();
    void UpdatePowerSourceCluster(uint8_t batteryLevel);

    void * mBatteryTimer;

    friend BatteryApplicationManager & BatteryAppMgr();
    static BatteryApplicationManager sInstance;
};

inline BatteryApplicationManager & BatteryAppMgr()
{
    return BatteryApplicationManager::sInstance;
}

} // namespace chip::NXP::App
