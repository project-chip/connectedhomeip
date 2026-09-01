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

#include "AppConfig.h"
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <system/SystemLayer.h>

namespace chip {
namespace app {
namespace Clusters {

// Prefer a forward declaration over including the full header when only a
// pointer/reference is needed (reduces coupling and compile-time dependencies).
// The complete type is included in the .cpp where methods are called.
namespace ElectricalPowerMeasurement {
class ElectricalPowerMeasurementDelegate;
} // namespace ElectricalPowerMeasurement

namespace ElectricalEnergyMeasurement {

class ElectricalEnergyMeasurementInstance : public ElectricalEnergyMeasurementAttrAccess
{
public:
    static constexpr uint32_t kTimerPeriodms = 1000; // Timer period 1000 ms
    static constexpr uint32_t kAttributeFrequency =
        CUMULATIVE_REPORT_INTERVAL_SECONDS; // Number of seconds between reports for cumulative energy

    ElectricalEnergyMeasurementInstance(EndpointId aEndpointId,
                                        ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate & aEPMDelegate,
                                        BitMask<Feature> aFeature, BitMask<OptionalAttributes> aOptionalAttrs) :
        ElectricalEnergyMeasurementAttrAccess::ElectricalEnergyMeasurementAttrAccess(aFeature, aOptionalAttrs),
        mEndpointId(aEndpointId)
    {
        mEPMDelegate = &aEPMDelegate;
    }

    // Delete copy constructor and assignment operator
    ElectricalEnergyMeasurementInstance(const ElectricalEnergyMeasurementInstance &)             = delete;
    ElectricalEnergyMeasurementInstance(const ElectricalEnergyMeasurementInstance &&)            = delete;
    ElectricalEnergyMeasurementInstance & operator=(const ElectricalEnergyMeasurementInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate * GetEPMDelegate() { return mEPMDelegate; }

    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer();

private:
    ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate * mEPMDelegate = nullptr;
    EndpointId mEndpointId                                                        = kInvalidEndpointId;
    bool mTimerActive                                                             = false;

    static void TimerEventHandler(System::Layer * systemLayer, void * appState);
    static void UpdateEnergyAttributesAndNotify(intptr_t arg);
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
