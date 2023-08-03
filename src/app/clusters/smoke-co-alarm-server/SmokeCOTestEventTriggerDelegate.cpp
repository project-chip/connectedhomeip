/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SmokeCOTestEventTriggerDelegate.h"

#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>

#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters::SmokeCoAlarm;

namespace chip {

bool SmokeCOTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR SmokeCOTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    Trigger trigger = static_cast<Trigger>(eventTrigger);

    switch (trigger)
    {
    case Trigger::kForceSmokeCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(mEndpointId, AlarmStateEnum::kCritical);
        SetExpressedState();
        break;
    case Trigger::kForceSmokeWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(mEndpointId, AlarmStateEnum::kWarning);
        SetExpressedState();
        break;
    case Trigger::kForceSmokeInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke interconnect (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(mEndpointId, AlarmStateEnum::kWarning);
        SetExpressedState();
        break;
    case Trigger::kForceCOCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force CO (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetCOState(mEndpointId, AlarmStateEnum::kCritical);
        SetExpressedState();
        break;
    case Trigger::kForceCOWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force CO (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetCOState(mEndpointId, AlarmStateEnum::kWarning);
        SetExpressedState();
        break;
    case Trigger::kForceCOInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force CO (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(mEndpointId, AlarmStateEnum::kWarning);
        SetExpressedState();
        break;
    case Trigger::kForceSmokeContaminationHigh:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke contamination (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(mEndpointId, ContaminationStateEnum::kCritical);
        break;
    case Trigger::kForceSmokeContaminationLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke contamination (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(mEndpointId, ContaminationStateEnum::kLow);
        break;
    case Trigger::kForceSmokeSensitivityHigh:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke sensistivity (high) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(mEndpointId, SensitivityEnum::kHigh);
        break;
    case Trigger::kForceSmokeSensitivityLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force smoke sensitivity (low) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(mEndpointId, SensitivityEnum::kLow);
        break;
    case Trigger::kForceMalfunction:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force malfunction\033[0;37m");
        SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(mEndpointId, true);
        SetExpressedState();
        break;
    case Trigger::kForceLowBatteryWarning:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force low battery (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(mEndpointId, AlarmStateEnum::kWarning);
        SetExpressedState();
        break;
    case Trigger::kForceLowBatteryCritical:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force low battery (critical) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(mEndpointId, AlarmStateEnum::kCritical);
        SetExpressedState();
        break;
    case Trigger::kForceEndOfLife:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force end-of-life\033[0;37m");
        SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(mEndpointId, EndOfServiceEnum::kExpired);
        SetExpressedState();
        break;
    case Trigger::kForceSilence:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force silence\033[0;37m");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(mEndpointId, MuteStateEnum::kMuted);
        break;
    case Trigger::kClearSmoke:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear smoke\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeState(mEndpointId, AlarmStateEnum::kNormal);
        SetExpressedState();
        break;
    case Trigger::kClearCO:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear CO\033[0;37m");
        SetExpressedState();
        SmokeCoAlarmServer::Instance().SetCOState(mEndpointId, AlarmStateEnum::kNormal);
        break;
    case Trigger::kClearSmokeInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear smoke interconnect\033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(mEndpointId, AlarmStateEnum::kNormal);
        SetExpressedState();
        break;
    case Trigger::kClearCOInterconnect:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear CO interconnect\033[0;37m");
        SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(mEndpointId, AlarmStateEnum::kNormal);
        SetExpressedState();
        break;
    case Trigger::kClearMalfunction:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear malfunction\033[0;37m");
        SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(mEndpointId, false);
        SetExpressedState();
        break;
    case Trigger::kClearEndOfLife:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear end-of-life\033[0;37m");
        SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(mEndpointId, EndOfServiceEnum::kNormal);
        SetExpressedState();
        break;
    case Trigger::kClearSilence:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear silence\033[0;37m");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(mEndpointId, MuteStateEnum::kNotMuted);
        break;
    case Trigger::kClearBatteryLevelLow:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear low battery\033[0;37m");
        SmokeCoAlarmServer::Instance().SetBatteryAlert(mEndpointId, AlarmStateEnum::kNormal);
        SetExpressedState();
        break;
    case Trigger::kClearContamination:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Force SmokeContamination (warning) \033[0;37m");
        SmokeCoAlarmServer::Instance().SetContaminationState(mEndpointId, ContaminationStateEnum::kNormal);
        break;
    case Trigger::kClearSensitivity:
        ChipLogProgress(Support, "  \033[0;96m[Smoke-CO-Alarm-Test-Event] => Clear Smoke Sensitivity\033[0;37m");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(mEndpointId, SensitivityEnum::kStandard);
        break;
    default:
        ChipLogProgress(Support, " Unknown Smoke CO Event");

        return (mOtherDelegate != nullptr) ? mOtherDelegate->HandleEventTrigger(eventTrigger) : CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void SmokeCOTestEventTriggerDelegate::SetExpressedState(void)
{
    ExpressedStateEnum currentExpressedState = ExpressedStateEnum::kNormal;

    for (ExpressedStateEnum priority : priorityOrder)
    {
        switch (priority)
        {
        case ExpressedStateEnum::kSmokeAlarm: {
            SmokeCoAlarmServer::Instance().GetSmokeState(mEndpointId, mCurrentSmokeAlarm);
            if (mCurrentSmokeAlarm != AlarmStateEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kSmokeAlarm;
            }
        }
        break;
        case ExpressedStateEnum::kCOAlarm: {
            SmokeCoAlarmServer::Instance().GetCOState(mEndpointId, mCurrentCoAlarm);
            if (mCurrentCoAlarm != AlarmStateEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kCOAlarm;
            }
        }
        break;
        case ExpressedStateEnum::kBatteryAlert: {
            SmokeCoAlarmServer::Instance().GetBatteryAlert(mEndpointId, mCurrentBatteryState);
            if (mCurrentBatteryState != AlarmStateEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kBatteryAlert;
            }
        }
        break;
        case ExpressedStateEnum::kTesting: {
            SmokeCoAlarmServer::Instance().GetTestInProgress(mEndpointId, mCurrentTestInProgress);
            if (mCurrentTestInProgress != false)
            {
                currentExpressedState = ExpressedStateEnum::kTesting;
            }
        }
        break;
        case ExpressedStateEnum::kHardwareFault: {
            SmokeCoAlarmServer::Instance().GetHardwareFaultAlert(mEndpointId, mCurrentHardwareFault);
            if (mCurrentHardwareFault != false)
            {
                currentExpressedState = ExpressedStateEnum::kHardwareFault;
            }
        }
        break;
        case ExpressedStateEnum::kEndOfService: {
            SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(mEndpointId, mCurrentEndOfService);
            if (mCurrentEndOfService != EndOfServiceEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kEndOfService;
            }
        }
        break;
        case ExpressedStateEnum::kInterconnectSmoke: {
            SmokeCoAlarmServer::Instance().GetInterconnectSmokeAlarm(mEndpointId, mCurrentInterconnectSmokeAlarm);
            if (mCurrentInterconnectSmokeAlarm != AlarmStateEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kInterconnectSmoke;
            }
        }
        break;
        case ExpressedStateEnum::kInterconnectCO: {
            SmokeCoAlarmServer::Instance().GetInterconnectCOAlarm(mEndpointId, mCurrentInterconnectCoAlarm);
            if (mCurrentInterconnectCoAlarm != AlarmStateEnum::kNormal)
            {
                currentExpressedState = ExpressedStateEnum::kInterconnectCO;
            }
        }
        break;

        default:
            break;
        }

        if (currentExpressedState != ExpressedStateEnum::kNormal)
        {
            break;
        }
    }

    SmokeCoAlarmServer::Instance().SetExpressedState(mEndpointId, currentExpressedState);
}

} // namespace chip
