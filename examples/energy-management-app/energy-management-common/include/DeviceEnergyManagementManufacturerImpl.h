/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

/**
 * The DeviceEnergyManagementManufacturer example class
 */

class DeviceEnergyManagementManufacturer
{
public:
    DeviceEnergyManagementManufacturer(DeviceEnergyManagementManager * aInstance) { mInstance = aInstance; }
    DeviceEnergyManagementManager  * GetInstance() { return mInstance; }
    DeviceEnergyManagementDelegate * GetDelegate()
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
     * @brief   Allows a client application to send in power readings into the system
     *
     * @param[in]  aEndpointId     - Endpoint to send to EPM Cluster
     */
// TODO:??   CHIP_ERROR SendPowerReading(EndpointId aEndpointId, int64_t aActivePower_mW, int64_t aVoltage_mV, int64_t aCurrent_mA);


private:
    DeviceEnergyManagementManager * mInstance;

    int64_t mLastChargingEnergyMeter    = 0;
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
DeviceEnergyManagementManufacturer * GetDeviceEnergyManagementManufacturer();

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
