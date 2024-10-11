/**
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

/**
 * @file
 *   Routines for the Smoke CO Alarm Server plugin.
 *
 */

#include "smoke-co-alarm-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::app::Clusters::SmokeCoAlarm::Attributes;
using chip::Protocols::InteractionModel::Status;

SmokeCoAlarmServer SmokeCoAlarmServer::sInstance;

/**********************************************************
 * SmokeCoAlarmServer public methods
 *********************************************************/

SmokeCoAlarmServer & SmokeCoAlarmServer::Instance()
{
    return sInstance;
}

void SmokeCoAlarmServer::SetExpressedStateByPriority(EndpointId endpointId,
                                                     const std::array<ExpressedStateEnum, kPriorityOrderLength> & priorityOrder)
{
    for (ExpressedStateEnum priority : priorityOrder)
    {
        AlarmStateEnum alarmState          = AlarmStateEnum::kNormal;
        EndOfServiceEnum endOfServiceState = EndOfServiceEnum::kNormal;
        bool active                        = false;
        bool success                       = false;

        switch (priority)
        {
        case ExpressedStateEnum::kSmokeAlarm:
            success = GetSmokeState(endpointId, alarmState);
            break;
        case ExpressedStateEnum::kCOAlarm:
            success = GetCOState(endpointId, alarmState);
            break;
        case ExpressedStateEnum::kBatteryAlert:
            success = GetBatteryAlert(endpointId, alarmState);
            break;
        case ExpressedStateEnum::kTesting:
            success = GetTestInProgress(endpointId, active);
            break;
        case ExpressedStateEnum::kHardwareFault:
            success = GetHardwareFaultAlert(endpointId, active);
            break;
        case ExpressedStateEnum::kEndOfService:
            success = GetEndOfServiceAlert(endpointId, endOfServiceState);
            break;
        case ExpressedStateEnum::kInterconnectSmoke:
            success = GetInterconnectSmokeAlarm(endpointId, alarmState);
            break;
        case ExpressedStateEnum::kInterconnectCO:
            success = GetInterconnectCOAlarm(endpointId, alarmState);
            break;
        default:
            break;
        }

        if (success && ((alarmState != AlarmStateEnum::kNormal) || (endOfServiceState != EndOfServiceEnum::kNormal) || active))
        {
            SetExpressedState(endpointId, priority);
            return;
        }
    }

    SetExpressedState(endpointId, ExpressedStateEnum::kNormal);
}

bool SmokeCoAlarmServer::RequestSelfTest(EndpointId endpointId)
{
    ExpressedStateEnum expressedState;
    VerifyOrReturnValue(GetExpressedState(endpointId, expressedState), false);

    // If the value is busy then return busy
    if (expressedState == ExpressedStateEnum::kSmokeAlarm || expressedState == ExpressedStateEnum::kCOAlarm ||
        expressedState == ExpressedStateEnum::kTesting || expressedState == ExpressedStateEnum::kInterconnectSmoke ||
        expressedState == ExpressedStateEnum::kInterconnectCO)
    {
        return false;
    }

    VerifyOrReturnValue(SetTestInProgress(endpointId, true), false);
    SetExpressedState(endpointId, ExpressedStateEnum::kTesting);

    emberAfPluginSmokeCoAlarmSelfTestRequestCommand(endpointId);

    return true;
}

bool SmokeCoAlarmServer::SetSmokeState(EndpointId endpointId, AlarmStateEnum newSmokeState)
{
    AlarmStateEnum alarmState;
    VerifyOrReturnValue(GetAttribute(endpointId, SmokeState::Id, SmokeState::Get, alarmState), false);
    VerifyOrReturnValue(alarmState != newSmokeState, true);

    VerifyOrReturnValue(SetAttribute(endpointId, SmokeState::Id, SmokeState::Set, newSmokeState), false);
    if (newSmokeState == AlarmStateEnum::kWarning || newSmokeState == AlarmStateEnum::kCritical)
    {
        Events::SmokeAlarm::Type event{ newSmokeState };
        SendEvent(endpointId, event);
    }

    if (newSmokeState == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmServer::SetCOState(EndpointId endpointId, AlarmStateEnum newCOState)
{
    AlarmStateEnum alarmState;
    VerifyOrReturnValue(GetAttribute(endpointId, COState::Id, COState::Get, alarmState), false);
    VerifyOrReturnValue(alarmState != newCOState, true);

    VerifyOrReturnValue(SetAttribute(endpointId, COState::Id, COState::Set, newCOState), false);
    if (newCOState == AlarmStateEnum::kWarning || newCOState == AlarmStateEnum::kCritical)
    {
        Events::COAlarm::Type event{ newCOState };
        SendEvent(endpointId, event);
    }

    if (newCOState == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmServer::SetBatteryAlert(EndpointId endpointId, AlarmStateEnum newBatteryAlert)
{
    AlarmStateEnum alarmState;
    VerifyOrReturnValue(GetAttribute(endpointId, BatteryAlert::Id, BatteryAlert::Get, alarmState), false);
    VerifyOrReturnValue(alarmState != newBatteryAlert, true);

    VerifyOrReturnValue(SetAttribute(endpointId, BatteryAlert::Id, BatteryAlert::Set, newBatteryAlert), false);
    if (newBatteryAlert == AlarmStateEnum::kWarning || newBatteryAlert == AlarmStateEnum::kCritical)
    {
        Events::LowBattery::Type event{ newBatteryAlert };
        SendEvent(endpointId, event);
    }

    if (newBatteryAlert == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmServer::SetDeviceMuted(EndpointId endpointId, MuteStateEnum newDeviceMuted)
{
    MuteStateEnum deviceMuted;
    VerifyOrReturnValue(GetAttribute(endpointId, DeviceMuted::Id, DeviceMuted::Get, deviceMuted), false);
    VerifyOrReturnValue(deviceMuted != newDeviceMuted, true);

    if (newDeviceMuted == MuteStateEnum::kMuted)
    {
        AlarmStateEnum alarmState;

        // If the attribute has been read and the attribute is Critical, return false

        bool success = GetSmokeState(endpointId, alarmState);
        VerifyOrReturnValue(!success || alarmState != AlarmStateEnum::kCritical, false);

        success = GetCOState(endpointId, alarmState);
        VerifyOrReturnValue(!success || alarmState != AlarmStateEnum::kCritical, false);

        success = GetBatteryAlert(endpointId, alarmState);
        VerifyOrReturnValue(!success || alarmState != AlarmStateEnum::kCritical, false);

        success = GetInterconnectSmokeAlarm(endpointId, alarmState);
        VerifyOrReturnValue(!success || alarmState != AlarmStateEnum::kCritical, false);

        success = GetInterconnectCOAlarm(endpointId, alarmState);
        VerifyOrReturnValue(!success || alarmState != AlarmStateEnum::kCritical, false);
    }

    VerifyOrReturnValue(SetAttribute(endpointId, DeviceMuted::Id, DeviceMuted::Set, newDeviceMuted), false);
    if (newDeviceMuted == MuteStateEnum::kMuted)
    {
        Events::AlarmMuted::Type event{};
        SendEvent(endpointId, event);
    }
    else if (newDeviceMuted == MuteStateEnum::kNotMuted)
    {
        Events::MuteEnded::Type event{};
        SendEvent(endpointId, event);
    }

    return true;
}

bool SmokeCoAlarmServer::SetTestInProgress(EndpointId endpointId, bool newTestInProgress)
{
    bool active;
    VerifyOrReturnValue(GetAttribute(endpointId, TestInProgress::Id, TestInProgress::Get, active), false);
    VerifyOrReturnValue(active != newTestInProgress, true);

    VerifyOrReturnValue(SetAttribute(endpointId, TestInProgress::Id, TestInProgress::Set, newTestInProgress), false);
    if (!newTestInProgress)
    {
        Events::SelfTestComplete::Type event{};
        SendEvent(endpointId, event);
    }

    return true;
}

bool SmokeCoAlarmServer::SetHardwareFaultAlert(EndpointId endpointId, bool newHardwareFaultAlert)
{
    bool active;
    VerifyOrReturnValue(GetAttribute(endpointId, HardwareFaultAlert::Id, HardwareFaultAlert::Get, active), false);
    VerifyOrReturnValue(active != newHardwareFaultAlert, true);

    VerifyOrReturnValue(SetAttribute(endpointId, HardwareFaultAlert::Id, HardwareFaultAlert::Set, newHardwareFaultAlert), false);
    if (newHardwareFaultAlert)
    {
        Events::HardwareFault::Type event{};
        SendEvent(endpointId, event);
    }

    return true;
}

bool SmokeCoAlarmServer::SetEndOfServiceAlert(EndpointId endpointId, EndOfServiceEnum newEndOfServiceAlert)
{
    EndOfServiceEnum endOfServiceState;
    VerifyOrReturnValue(GetAttribute(endpointId, EndOfServiceAlert::Id, EndOfServiceAlert::Get, endOfServiceState), false);
    VerifyOrReturnValue(endOfServiceState != newEndOfServiceAlert, true);

    VerifyOrReturnValue(SetAttribute(endpointId, EndOfServiceAlert::Id, EndOfServiceAlert::Set, newEndOfServiceAlert), false);
    if (newEndOfServiceAlert == EndOfServiceEnum::kExpired)
    {
        Events::EndOfService::Type event{};
        SendEvent(endpointId, event);
    }

    return true;
}

bool SmokeCoAlarmServer::SetInterconnectSmokeAlarm(EndpointId endpointId, AlarmStateEnum newInterconnectSmokeAlarm)
{
    AlarmStateEnum alarmState;
    VerifyOrReturnValue(GetAttribute(endpointId, InterconnectSmokeAlarm::Id, InterconnectSmokeAlarm::Get, alarmState), false);
    VerifyOrReturnValue(alarmState != newInterconnectSmokeAlarm, true);

    VerifyOrReturnValue(
        SetAttribute(endpointId, InterconnectSmokeAlarm::Id, InterconnectSmokeAlarm::Set, newInterconnectSmokeAlarm), false);
    if (newInterconnectSmokeAlarm == AlarmStateEnum::kWarning || newInterconnectSmokeAlarm == AlarmStateEnum::kCritical)
    {
        Events::InterconnectSmokeAlarm::Type event{ newInterconnectSmokeAlarm };
        SendEvent(endpointId, event);
    }

    if (newInterconnectSmokeAlarm == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmServer::SetInterconnectCOAlarm(EndpointId endpointId, AlarmStateEnum newInterconnectCOAlarm)
{
    AlarmStateEnum alarmState;
    VerifyOrReturnValue(GetAttribute(endpointId, InterconnectCOAlarm::Id, InterconnectCOAlarm::Get, alarmState), false);
    VerifyOrReturnValue(alarmState != newInterconnectCOAlarm, true);

    VerifyOrReturnValue(SetAttribute(endpointId, InterconnectCOAlarm::Id, InterconnectCOAlarm::Set, newInterconnectCOAlarm), false);
    if (newInterconnectCOAlarm == AlarmStateEnum::kWarning || newInterconnectCOAlarm == AlarmStateEnum::kCritical)
    {
        Events::InterconnectCOAlarm::Type event{ newInterconnectCOAlarm };
        SendEvent(endpointId, event);
    }

    if (newInterconnectCOAlarm == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmServer::SetContaminationState(EndpointId endpointId, ContaminationStateEnum newContaminationState)
{
    ContaminationStateEnum contaminationState;
    VerifyOrReturnValue(GetAttribute(endpointId, ContaminationState::Id, ContaminationState::Get, contaminationState), false);
    VerifyOrReturnValue(contaminationState != newContaminationState, true);

    VerifyOrReturnValue(SetAttribute(endpointId, ContaminationState::Id, ContaminationState::Set, newContaminationState), false);

    return true;
}

bool SmokeCoAlarmServer::SetSmokeSensitivityLevel(EndpointId endpointId, SensitivityEnum newSmokeSensitivityLevel)
{
    SensitivityEnum sensitivity;
    VerifyOrReturnValue(GetAttribute(endpointId, SmokeSensitivityLevel::Id, SmokeSensitivityLevel::Get, sensitivity), false);
    VerifyOrReturnValue(sensitivity != newSmokeSensitivityLevel, true);

    VerifyOrReturnValue(SetAttribute(endpointId, SmokeSensitivityLevel::Id, SmokeSensitivityLevel::Set, newSmokeSensitivityLevel),
                        false);

    return true;
}

bool SmokeCoAlarmServer::GetExpressedState(chip ::EndpointId endpointId, ExpressedStateEnum & expressedState)
{
    return GetAttribute(endpointId, ExpressedState::Id, ExpressedState::Get, expressedState);
}

bool SmokeCoAlarmServer::GetSmokeState(EndpointId endpointId, AlarmStateEnum & smokeState)
{
    return GetAttribute(endpointId, SmokeState::Id, SmokeState::Get, smokeState);
}

bool SmokeCoAlarmServer::GetCOState(EndpointId endpointId, AlarmStateEnum & coState)
{
    return GetAttribute(endpointId, COState::Id, COState::Get, coState);
}

bool SmokeCoAlarmServer::GetBatteryAlert(EndpointId endpointId, AlarmStateEnum & batteryAlert)
{
    return GetAttribute(endpointId, BatteryAlert::Id, BatteryAlert::Get, batteryAlert);
}

bool SmokeCoAlarmServer::GetDeviceMuted(EndpointId endpointId, MuteStateEnum & deviceMuted)
{
    return GetAttribute(endpointId, DeviceMuted::Id, DeviceMuted::Get, deviceMuted);
}

bool SmokeCoAlarmServer::GetTestInProgress(EndpointId endpointId, bool & testInProgress)
{
    return GetAttribute(endpointId, TestInProgress::Id, TestInProgress::Get, testInProgress);
}

bool SmokeCoAlarmServer::GetHardwareFaultAlert(EndpointId endpointId, bool & hardwareFaultAlert)
{
    return GetAttribute(endpointId, HardwareFaultAlert::Id, HardwareFaultAlert::Get, hardwareFaultAlert);
}

bool SmokeCoAlarmServer::GetEndOfServiceAlert(EndpointId endpointId, EndOfServiceEnum & endOfServiceAlert)
{
    return GetAttribute(endpointId, EndOfServiceAlert::Id, EndOfServiceAlert::Get, endOfServiceAlert);
}

bool SmokeCoAlarmServer::GetInterconnectSmokeAlarm(EndpointId endpointId, AlarmStateEnum & interconnectSmokeAlarm)
{
    return GetAttribute(endpointId, InterconnectSmokeAlarm::Id, InterconnectSmokeAlarm::Get, interconnectSmokeAlarm);
}

bool SmokeCoAlarmServer::GetInterconnectCOAlarm(EndpointId endpointId, AlarmStateEnum & interconnectCOAlarm)
{
    return GetAttribute(endpointId, InterconnectCOAlarm::Id, InterconnectCOAlarm::Get, interconnectCOAlarm);
}

bool SmokeCoAlarmServer::GetContaminationState(EndpointId endpointId, ContaminationStateEnum & contaminationState)
{
    return GetAttribute(endpointId, ContaminationState::Id, ContaminationState::Get, contaminationState);
}

bool SmokeCoAlarmServer::GetSmokeSensitivityLevel(EndpointId endpointId, SensitivityEnum & smokeSensitivityLevel)
{
    return GetAttribute(endpointId, SmokeSensitivityLevel::Id, SmokeSensitivityLevel::Get, smokeSensitivityLevel);
}

bool SmokeCoAlarmServer::GetExpiryDate(EndpointId endpointId, uint32_t & expiryDate)
{
    return GetAttribute(endpointId, ExpiryDate::Id, ExpiryDate::Get, expiryDate);
}

chip::BitFlags<Feature> SmokeCoAlarmServer::GetFeatures(EndpointId endpointId)
{
    chip::BitFlags<Feature> featureMap;
    if (!GetAttribute(endpointId, FeatureMap::Id, FeatureMap::Get, *featureMap.RawStorage()))
    {
        ChipLogError(Zcl, "Unable to get the Smoke CO Alarm feature map: attribute read error");
        featureMap.ClearAll();
    }
    return featureMap;
}

/**********************************************************
 * SmokeCoAlarmServer private methods
 *********************************************************/

void SmokeCoAlarmServer::SetExpressedState(EndpointId endpointId, ExpressedStateEnum newExpressedState)
{
    ExpressedStateEnum expressedState;
    VerifyOrReturn(GetAttribute(endpointId, ExpressedState::Id, ExpressedState::Get, expressedState));
    VerifyOrReturn(expressedState != newExpressedState);

    VerifyOrReturn(SetAttribute(endpointId, ExpressedState::Id, ExpressedState::Set, newExpressedState));
    if (newExpressedState == ExpressedStateEnum::kNormal)
    {
        Events::AllClear::Type event{};
        SendEvent(endpointId, event);
    }
}

void SmokeCoAlarmServer::HandleRemoteSelfTestRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath)
{
    EndpointId endpointId = commandPath.mEndpointId;

    ExpressedStateEnum expressedState;
    VerifyOrReturn(GetExpressedState(endpointId, expressedState), commandObj->AddStatus(commandPath, Status::Failure));

    // If the value is busy then return busy
    if (expressedState == ExpressedStateEnum::kSmokeAlarm || expressedState == ExpressedStateEnum::kCOAlarm ||
        expressedState == ExpressedStateEnum::kTesting || expressedState == ExpressedStateEnum::kInterconnectSmoke ||
        expressedState == ExpressedStateEnum::kInterconnectCO)
    {
        commandObj->AddStatus(commandPath, Status::Busy);
        return;
    }

    VerifyOrReturn(SetTestInProgress(endpointId, true), commandObj->AddStatus(commandPath, Status::Failure));
    SetExpressedState(endpointId, ExpressedStateEnum::kTesting);

    emberAfPluginSmokeCoAlarmSelfTestRequestCommand(endpointId);

    commandObj->AddStatus(commandPath, Status::Success);
}

template <typename T>
void SmokeCoAlarmServer::SendEvent(EndpointId endpointId, T & event)
{
    EventNumber eventNumber;
    auto err = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to log event: err=%" CHIP_ERROR_FORMAT ", event_id=" ChipLogFormatMEI, err.Format(),
                     ChipLogValueMEI(event.GetEventId()));
    }
}

template <typename T>
bool SmokeCoAlarmServer::GetAttribute(EndpointId endpointId, AttributeId attributeId,
                                      Status (*getFn)(EndpointId endpointId, T * value), T & value) const
{
    Status status          = getFn(endpointId, &value);
    bool success           = (Status::Success == status);
    bool unsupportedStatus = (Status::UnsupportedAttribute == status);

    if (unsupportedStatus)
    {
        ChipLogProgress(Zcl, "Read unsupported SmokeCOAlarm attribute: attribute=" ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
    else if (!success)
    {
        ChipLogError(Zcl, "Failed to read SmokeCOAlarm attribute: attribute=" ChipLogFormatMEI ", status=0x%x",
                     ChipLogValueMEI(attributeId), to_underlying(status));
    }
    return success;
}

template <typename T>
bool SmokeCoAlarmServer::SetAttribute(EndpointId endpointId, AttributeId attributeId,
                                      Status (*setFn)(EndpointId endpointId, T value), T value)
{
    Status status = setFn(endpointId, value);
    bool success  = (Status::Success == status);

    if (!success)
    {
        ChipLogError(Zcl, "Failed to write SmokeCOAlarm attribute: attribute=" ChipLogFormatMEI ", status=0x%x",
                     ChipLogValueMEI(attributeId), to_underlying(status));
    }
    return success;
}

// =============================================================================
// Cluster commands callbacks
// =============================================================================

bool emberAfSmokeCoAlarmClusterSelfTestRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                       const Commands::SelfTestRequest::DecodableType & commandData)
{
    SmokeCoAlarmServer::Instance().HandleRemoteSelfTestRequest(commandObj, commandPath);
    return true;
}

void MatterSmokeCoAlarmPluginServerInitCallback() {}
