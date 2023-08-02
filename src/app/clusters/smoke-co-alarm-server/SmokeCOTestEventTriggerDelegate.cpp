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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include "SmokeCOTestEventTriggerDelegate.h"
#include "smoke-co-alarm-server.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/core/DataModelTypes.h>

using namespace chip::app::Clusters;

namespace chip {

bool SmokeCOTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR SmokeCOTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    switch (eventTrigger)
    {
    case trigger::ForceSmokeCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force smoke (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(1, SmokeCoAlarm::AlarmStateEnum::kCritical);
        ExampleSetExpressedState();
        break;
    case trigger::ForceSmokeWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force smoke (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        ExampleSetExpressedState();
        break;
    case trigger::ForceSmokeInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force smoke interconnect (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        ExampleSetExpressedState();
        break;
    case trigger::ForceCOCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force co (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetCOState(1, SmokeCoAlarm::AlarmStateEnum::kCritical);
        ExampleSetExpressedState();
        break;
    case trigger::ForceCOWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force co (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetCOState(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        ExampleSetExpressedState();
        break;
    case trigger::ForceCOInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force co (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        ExampleSetExpressedState();
        break;
    case trigger::ForceSmokeContaminationHigh:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force SmokeContamination (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kCritical);
        break;
    case trigger::ForceSmokeContaminationLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force SmokeContamination (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kLow);
        break;
    case trigger::ForceSmokeSensitivityHigh:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force Smoke Sensistivity High\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kHigh);
        break;
    case trigger::ForceSmokeSensitivityLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force  smoke Sensitivity low\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kLow);
        break;
    case trigger::ForceMalfunction:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force malfunction\033[0;37m");
        SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, true);
        ExampleSetExpressedState();
        break;
    case trigger::ForceLowBatteryWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force low battery (warning)\033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        ExampleSetExpressedState();
        break;
    case trigger::ForceLowBatteryCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force low battery (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(1, SmokeCoAlarm::AlarmStateEnum::kCritical);
        ExampleSetExpressedState();
        break;
    case trigger::ForceCOEndOfLife:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force co end-of-life\033[0;37m");
        SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, SmokeCoAlarm::EndOfServiceEnum::kExpired);
        ExampleSetExpressedState();
        break;
    case trigger::ForceSilence:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force silence\033[0;37m");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, SmokeCoAlarm::MuteStateEnum::kMuted);
        break;
    case trigger::ClearSmoke:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear smoke\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        ExampleSetExpressedState();
        break;
    case trigger::ClearCO:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear co\033[0;37m");
        ExampleSetExpressedState();
        SmokeCoAlarmServer::Instance().SetCOState(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        break;
    case trigger::ClearSmokeInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear smoke interconnect\033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        ExampleSetExpressedState();
        break;
    case trigger::ClearCOInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear co interconnect\033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        ExampleSetExpressedState();
        break;
    case trigger::ClearMalfunction:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear malfunction\033[0;37m");
        SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, false);
        ExampleSetExpressedState();
        break;
    case trigger::ClearCOEndOfLife:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear co end-of-life\033[0;37m");
        SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, SmokeCoAlarm::EndOfServiceEnum::kNormal);
        ExampleSetExpressedState();
        break;
    case trigger::ClearSilence:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear silence\033[0;37m");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, SmokeCoAlarm::MuteStateEnum::kNotMuted);
        break;
    case trigger::ClearBatteryLevelLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => clear low battery\033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        ExampleSetExpressedState();
        break;
    case trigger::ClearContamination:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => force SmokeContamination (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kNormal);
        break;
    case trigger::ClearSensitivity:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear Smoke Sensitivity\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kStandard);
        break;
    default:
        ChipLogProgress(Support, " Unknown  Smoke CO Event");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

// For the Smoke CO example the following is the priority order for expressed alarms from Highest to lowest.
// Smoke Alarm
// Interconnect Smoke Alarm
// CO Alarm
// Interconneect CO alarm
// Hardware Fault
// Test In Progress
// End of Service
// Battery alert

void SmokeCOTestEventTriggerDelegate::ExampleSetExpressedState(void)
{
    SmokeCoAlarm::ExpressedStateEnum currentExpressedState     = SmokeCoAlarm::ExpressedStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum currentBatteryState           = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum currentInterconnectSmokeAlarm = SmokeCoAlarm::AlarmStateEnum::kNormal;
    ;
    SmokeCoAlarm::AlarmStateEnum currentInterconnectCoAlarm = SmokeCoAlarm::AlarmStateEnum::kNormal;
    ;
    SmokeCoAlarm::AlarmStateEnum currentCoAlarm = SmokeCoAlarm::AlarmStateEnum::kNormal;
    ;
    SmokeCoAlarm::AlarmStateEnum currentSmokeAlarm = SmokeCoAlarm::AlarmStateEnum::kNormal;
    ;
    bool currentTestInProgress = false;
    ;
    SmokeCoAlarm::AlarmStateEnum currentHardwareFault = SmokeCoAlarm::AlarmStateEnum::kNormal;
    ;
    SmokeCoAlarm::EndOfServiceEnum currentEndOfService = SmokeCoAlarm::EndOfServiceEnum::kNormal;

    SmokeCoAlarmServer::Instance().GetBatteryAlert(1, currentBatteryState);
        SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(1, currentEndOfService);

    SmokeCoAlarmServer::Instance().GetInterconnectSmokeAlarm(1, currentInterconnectSmokeAlarm);
    SmokeCoAlarmServer::Instance().GetInterconnectCOAlarm(1, currentInterconnectCoAlarm);
    SmokeCoAlarmServer::Instance().GetCOState(1, currentCoAlarm);
    SmokeCoAlarmServer::Instance().GetSmokeState(1, currentSmokeAlarm);
    SmokeCoAlarmServer::Instance().GetTestInProgress(1, currentTestInProgress);

    // This is written going lowest to highest priority
    // If someone wanted to change the order it makes it easier
    if (currentBatteryState != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kBatteryAlert;
    if (currentEndOfService != SmokeCoAlarm::EndOfServiceEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kEndOfService;
    if (currentTestInProgress != false)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kTesting;
    if (currentHardwareFault != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kHardwareFault;
    if (currentInterconnectCoAlarm != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kInterconnectCO;
    if (currentCoAlarm != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kCOAlarm;
    if (currentInterconnectSmokeAlarm != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kInterconnectSmoke;
    if (currentSmokeAlarm != SmokeCoAlarm::AlarmStateEnum::kNormal)
        currentExpressedState = SmokeCoAlarm::ExpressedStateEnum::kSmokeAlarm;

    SmokeCoAlarmServer::Instance().SetExpressedState(1, currentExpressedState);
}

} // namespace chip



