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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/core/ClusterEnums.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

#include "AppConfig.h"
#include "ElectricalEnergyMeasurementInstance.h"

#include <ElectricalPowerMeasurementDelegateImpl.h>

#define mWms_TO_mWh(power) ((power) / 3600'000)

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
using namespace chip::app::DataModel;
using namespace chip::DeviceLayer;

// Example of setting CumulativeEnergyReset structure - for now set these to 0
// but the manufacturer may want to store these in non volatile storage for timestamp (based on epoch_s)
// Create an accuracy entry which is between +/-0.5 and +/- 5% across the range of all possible energy readings
static const MeasurementAccuracyRangeStruct::Type kEnergyAccuracyRanges[] = {
    { .rangeMin   = 0,
      .rangeMax   = 1'000'000'000'000'000, // 1 million Mwh
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(500)),
      .percentMin = MakeOptional(static_cast<chip::Percent100ths>(50)) }
};

static const MeasurementAccuracyStruct::Type kAccuracy = {
    .measurementType  = MeasurementTypeEnum::kElectricalEnergy,
    .measured         = false, // this should be made true in an implementation where a real metering device is used.
    .minMeasuredValue = 0,
    .maxMeasuredValue = 1'000'000'000'000'000, // 1 million Mwh
    .accuracyRanges = List<const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type>(kEnergyAccuracyRanges)
};

static const CumulativeEnergyResetStruct::Type kResetStruct = {
    .importedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
    .exportedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
    .importedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
    .exportedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
};

static EnergyMeasurementStruct::Type sCumulativeImported = {
    .energy = static_cast<int64_t>(0),
};

static const EnergyMeasurementStruct::Type sCumulativeExported = {
    .energy = static_cast<int64_t>(0),
};

namespace {
ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate * gEPMDelegate = nullptr;
EndpointId gEndpointId                                                        = kInvalidEndpointId;
int64_t sCumulativeActivePower                                                = 0;
uint8_t sSecondsSinceUpdate                                                   = 0;
} // namespace

CHIP_ERROR ElectricalEnergyMeasurementInstance::Init()
{
    // Initialize attributes
    TEMPORARY_RETURN_IGNORED SetMeasurementAccuracy(mEndpointId, kAccuracy);
    TEMPORARY_RETURN_IGNORED SetCumulativeReset(mEndpointId, MakeOptional(kResetStruct));

    // Assign class attributes to instantiated global variables
    // for the access to TimerEventHandler
    gEPMDelegate = GetEPMDelegate();
    gEndpointId  = mEndpointId;

    uint32_t currentTimestamp;
    ReturnErrorOnFailure(System::Clock::GetClock_MatterEpochS(currentTimestamp));

    sCumulativeImported.startTimestamp.SetValue(currentTimestamp);
    sCumulativeImported.startSystime.SetValue(System::SystemClock().GetMonotonicTimestamp().count());

    // Start Matter SystemLayer timer (CMSIS-free; works on FreeRTOS and Zephyr)
    StartTimer(kTimerPeriodms);

    return ElectricalEnergyMeasurementAttrAccess::Init();
}

void ElectricalEnergyMeasurementInstance::Shutdown()
{
    CancelTimer();
    ElectricalEnergyMeasurementAttrAccess::Shutdown();
}

void ElectricalEnergyMeasurementInstance::StartTimer(uint32_t aTimeoutMs)
{
    mTimerActive   = true;
    CHIP_ERROR err = SystemLayer().StartTimer(System::Clock::Milliseconds32(aTimeoutMs), TimerEventHandler, this);
    if (err != CHIP_NO_ERROR)
    {
        mTimerActive = false;
        ChipLogError(AppServer, "EEM SystemLayer StartTimer failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ElectricalEnergyMeasurementInstance::CancelTimer()
{
    mTimerActive = false;
    SystemLayer().CancelTimer(TimerEventHandler, this);
}

void ElectricalEnergyMeasurementInstance::TimerEventHandler(System::Layer * systemLayer, void * appState)
{
    auto * self = static_cast<ElectricalEnergyMeasurementInstance *>(appState);
    VerifyOrReturn(self != nullptr && self->mTimerActive);

    // Get different EPM Active Power values according to operational mode change
    Nullable<int64_t> EPMActivePower = gEPMDelegate ? gEPMDelegate->GetActivePower() : Nullable<int64_t>(0);
    int64_t activePower              = (EPMActivePower.IsNull()) ? 0 : EPMActivePower.Value();

    // Calculate cumulative imported active power
    sCumulativeActivePower += activePower;
    sSecondsSinceUpdate += 1;

    if (sSecondsSinceUpdate >= ElectricalEnergyMeasurementInstance::kAttributeFrequency)
    {
        sSecondsSinceUpdate = 0;
        // Hop to the Matter/CHIP thread for attribute updates (no FreeRTOS AppTask queue).
        RETURN_SAFELY_IGNORED PlatformMgr().ScheduleWork(UpdateEnergyAttributesAndNotify, 0);
    }

    // Re-arm periodic timer
    self->StartTimer(kTimerPeriodms);
}

void ElectricalEnergyMeasurementInstance::UpdateEnergyAttributesAndNotify(intptr_t arg)
{
    (void) arg;

    // cumulativeImported update code - To update energy (mWs to mWh), startSystime and endSystime
    // Convert the unit : mW * ms -> mWh
    sCumulativeImported.energy = mWms_TO_mWh(sCumulativeActivePower * kTimerPeriodms);

    uint32_t currentTimestamp;
    if (System::Clock::GetClock_MatterEpochS(currentTimestamp) == CHIP_NO_ERROR)
    {
        // Use EpochTS
        sCumulativeImported.endTimestamp.SetValue(currentTimestamp);
    }

    sCumulativeImported.endSystime.SetValue(System::SystemClock().GetMonotonicTimestamp().count());

    // Call the SDK to update attributes and generate an event
    NotifyCumulativeEnergyMeasured(gEndpointId, MakeOptional(sCumulativeImported), MakeOptional(sCumulativeExported));
    MatterReportingAttributeChangeCallback(gEndpointId, ElectricalEnergyMeasurement::Id, CumulativeEnergyImported::Id);
}
