/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LoggingTemperatureControlledCabinetPart.h"

#include <device/api/PlatformIdentifyIntegration.h>
#include <lib/support/CodeUtils.h>

namespace chip::app {

LoggingTemperatureControlledCabinetPart::LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, const char * name) :
    LoggingTemperatureControlledCabinetPart(timerDelegate, Config{}, name)
{}

LoggingTemperatureControlledCabinetPart::LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, Config config,
                                                                                 const char * name) :
    TemperatureControlledCabinetPart(timerDelegate, config, static_cast<Clusters::OperationalState::Delegate &>(*this),
                                     static_cast<Clusters::IdentifyDelegate &>(*this)),
    mName(name), mTimerDelegate(timerDelegate)
{}

LoggingTemperatureControlledCabinetPart::~LoggingTemperatureControlledCabinetPart()
{
    CancelTimer();
}

void LoggingTemperatureControlledCabinetPart::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    mCountdownTime.SetNull();
    TemperatureControlledCabinetPart::Unregister(provider);
}

void LoggingTemperatureControlledCabinetPart::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void LoggingTemperatureControlledCabinetPart::StartEmulatedOperationTimer()
{
    CancelTimer();
    mCountdownTime = DataModel::MakeNullable<uint32_t>(kEmulatedOperationDurationSec);
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kEmulatedOperationDurationSec)));
}

void LoggingTemperatureControlledCabinetPart::TimerFired()
{
    ChipLogProgress(Zcl, "LoggingTemperatureControlledCabinetPart (%s): Emulated operation timer finished.", mName);
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
}

CHIP_ERROR LoggingTemperatureControlledCabinetPart::GetOperationalStateAtIndex(
    size_t index, Clusters::OperationalState::GenericOperationalState & operationalState)
{
    static const Clusters::OperationalState::GenericOperationalState kSupportedStates[] = {
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        Clusters::OperationalState::GenericOperationalState(
            to_underlying(Clusters::OperationalState::OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        Clusters::OperationalState::GenericOperationalState(to_underlying(Clusters::OperationalState::OperationalStateEnum::kError),
                                                            MakeOptional("Error"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingTemperatureControlledCabinetPart::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    static constexpr CharSpan kSupportedPhases[] = { "Starting"_span, "Operating"_span, "Finishing"_span };

    if (index >= MATTER_ARRAY_SIZE(kSupportedPhases))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(kSupportedPhases[index], operationalPhase);
}

void LoggingTemperatureControlledCabinetPart::HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingTemperatureControlledCabinetPart (%s): Pause command received.", mName);
    CancelTimer();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kPaused;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kPaused));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void LoggingTemperatureControlledCabinetPart::HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingTemperatureControlledCabinetPart (%s): Resume command received.", mName);
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void LoggingTemperatureControlledCabinetPart::HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingTemperatureControlledCabinetPart (%s): Start command received.", mName);
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kRunning;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kRunning));
    StartEmulatedOperationTimer();
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void LoggingTemperatureControlledCabinetPart::HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingTemperatureControlledCabinetPart (%s): Stop command received.", mName);
    CancelTimer();
    mCountdownTime.SetNull();
    mOperationalState = Clusters::OperationalState::OperationalStateEnum::kStopped;
    LogErrorOnFailure(OperationalState().SetOperationalState(Clusters::OperationalState::OperationalStateEnum::kStopped));
    err.Set(to_underlying(Clusters::OperationalState::ErrorStateEnum::kNoError));
}

void LoggingTemperatureControlledCabinetPart::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnIdentifyStart", mName);
    PlatformIdentifyIntegration::GetInstance().NotifyIdentifyStart(cluster);
}

void LoggingTemperatureControlledCabinetPart::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnIdentifyStop", mName);
    PlatformIdentifyIntegration::GetInstance().NotifyIdentifyStop(cluster);
}

void LoggingTemperatureControlledCabinetPart::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnTriggerEffect", mName);
    PlatformIdentifyIntegration::GetInstance().NotifyTriggerEffect(cluster);
}

} // namespace chip::app
