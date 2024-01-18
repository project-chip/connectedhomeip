/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <DeviceEnergyManagementManager.h>
#include <EnergyEvseManager.h>

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/**
 * The EVSEManufacturer example class
 */

class EVSEManufacturer
{
public:
    EVSEManufacturer(EnergyEvseManager * aInstance) { mInstance = aInstance; }
    EnergyEvseManager * GetInstance() { return mInstance; }
    EnergyEvseDelegate * GetDelegate()
    {
        if (mInstance)
        {
            return mInstance->GetDelegate();
        }
        return nullptr;
    }

    /**
     * @brief   Called at start up to apply hardware settings
     */
    CHIP_ERROR Init();

    /**
     * @brief   Called at shutdown
     */
    CHIP_ERROR Shutdown();

    /**
     * @brief   Main Callback handler from delegate to user code
     */
    static void ApplicationCallbackHandler(const EVSECbInfo * cb, intptr_t arg);

private:
    EnergyEvseManager * mInstance;

    int64_t mLastChargingEnergyMeter    = 0;
    int64_t mLastDischargingEnergyMeter = 0;
};

/** @brief Helper function to return the singleton EVSEManufacturer instance
 *
 * This is needed by the EVSEManufacturer class to support TestEventTriggers
 * which are called outside of any class context. This allows the EVSEManufacturer
 * class to return the relevant Delegate instance in which to invoke the test
 * events on.
 *
 * This function is typically found in main.cpp or wherever the singleton is created.
 */
EVSEManufacturer * GetEvseManufacturer();

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
