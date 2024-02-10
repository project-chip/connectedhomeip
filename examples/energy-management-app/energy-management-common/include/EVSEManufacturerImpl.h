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

    /**
     * @brief   Allows a client application to send in power readings into the system
     *
     * @param[in]  aEndpointId     - Endpoint to send to EPM Cluster
     * @param[in]  aActivePower_mW - Power measured in milli-watts
     * @param[in]  aVoltage_mV     - Voltage measured in milli-volts
     * @param[in]  aCurrent_mA     - Current measured in milli-amps
     */
    CHIP_ERROR SendPowerReading(EndpointId aEndpointId, int64_t aActivePower_mW, int64_t aVoltage_mV, int64_t aCurrent_mA);

    /**
     * @brief   Allows a client application to send in energy readings into the system
     *
     *          This is a helper function to add timestamps to the readings
     *
     * @param[in]  aCumulativeEnergyImported -total energy imported in milli-watthours
     * @param[in]  aCumulativeEnergyExported -total energy exported in milli-watthours
     */
    CHIP_ERROR SendEnergyReading(EndpointId aEndpointId, int64_t aCumulativeEnergyImported, int64_t aCumulativeEnergyExported);

    /**  Fake Meter data generation - used for testing EPM/EEM clusters */
    /**
     * @brief   Starts a fake load/generator to periodically callback the power and energy
     *          clusters.
     * @param[in]   aEndpointId  - which endpoint is the meter to be updated on
     * @param[in]   aPower_mW    - the mean power of the load
     *                             Positive power indicates Imported energy (e.g. a load)
     *                             Negative power indicated Exported energy (e.g. a generator)
     * @param[in]   aPowerRandomness_mW  This is used to scale random power fluctuations around the mean power of the load
     *
     * @param[in]   aInterval_s  - the callback interval in seconds
     * @param[in]   bReset       - boolean: true will reset the energy values to 0
     */
    void StartFakeReadings(EndpointId aEndpointId, int64_t aPower_mW, uint32_t aPowerRandomness_mW, uint8_t aInterval_s,
                           bool bReset);

    /**
     * @brief   Stops any active updates to the fake load data callbacks
     */
    void StopFakeReadings();

    /**
     * @brief   Sends fake meter data into the cluster and restarts the timer
     */
    void FakeReadingsUpdate();
    /**
     * @brief   Timer expiry callback to handle fake load
     */
    static void FakeReadingsTimerExpiry(System::Layer * systemLayer, void * manufacturer);

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
