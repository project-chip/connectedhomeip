/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/server/AppDelegate.h>
#include <app/server/Server.h>
#include <credentials/FabricTable.h>
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDStateObserver.h>
#endif

namespace chip {

class AmebaObserver : public AppDelegate,
                      public FabricTable::Delegate
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    ,
                      public chip::app::ICDStateObserver
#endif
{
public:
    // Commissioning Observer
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override
    {
        ChipLogProgress(DeviceLayer, "Ameba Observer: Commissioning error (0x%x)", err);
        // Handle commissioning errror here
    }
    // Fabric Observer
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        ChipLogProgress(DeviceLayer, "Ameba Observer: Fabric 0x%x has been Removed", fabricIndex);
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            // Customer code
        }
    }
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    void OnEnterActiveMode() override
    {
        // Request high-performance WiFi mode (disable power-save)
        matter_wifi_set_powersave_mode(0, 0);
        ChipLogProgress(DeviceLayer, "ICD: Entering Active Mode");
    }
    void OnEnterIdleMode() override
    {
        // Allow WiFi to use DTIM-based sleep
        matter_wifi_set_powersave_mode(1, 1);
        ChipLogProgress(DeviceLayer, "ICD: Entering Idle Mode");
    }
    void OnTransitionToIdle() override
    {
        // Called before the transition to idle; can be used to flush pending operations
        ChipLogProgress(DeviceLayer, "ICD: Transitioning to Idle");
    }
    void OnICDModeChange() override
    {
        // Called when the ICD mode changes (SIT <-> LIT)
        ChipLogProgress(DeviceLayer, "ICD: Mode Changed");
    }
#endif
};

} // namespace chip
