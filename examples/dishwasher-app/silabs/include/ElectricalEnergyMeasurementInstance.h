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
#include "AppEvent.h"
#include "ElectricalPowerMeasurementDelegate.h"
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <cmsis_os2.h>

namespace chip {
namespace app {
namespace Clusters {
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
    ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate * mEPMDelegate;
    EndpointId mEndpointId;
    osTimerId_t mTimer;

    CHIP_ERROR InitTimer();

    static void TimerEventHandler(void * timerCbArg);
    static void UpdateEnergyAttributesAndNotify(AppEvent * aEvent);
};

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
