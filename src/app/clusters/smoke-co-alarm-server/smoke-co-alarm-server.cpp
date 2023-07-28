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
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/error-mapping.h>
#include <cinttypes>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::SmokeCoAlarm;
using chip::Protocols::InteractionModel::Status;

SmokeCoAlarmServer SmokeCoAlarmServer::sInstance;

/**********************************************************
 * SmokeCoAlarmServer public methods
 *********************************************************/

SmokeCoAlarmServer & SmokeCoAlarmServer::Instance()
{
    return sInstance;
}

bool SmokeCoAlarmServer::SetExpressedState(EndpointId endpointId, ExpressedStateEnum newExpressedState)
{
    ExpressedStateEnum expressedState;
    bool success = GetAttribute(endpointId, Attributes::ExpressedState::Id, Attributes::ExpressedState::Get, expressedState);

    if (success && (expressedState != newExpressedState))
    {
        success = SetAttribute(endpointId, Attributes::ExpressedState::Id, Attributes::ExpressedState::Set, newExpressedState);

        if (success && (newExpressedState == ExpressedStateEnum::kNormal))
        {
            Events::AllClear::Type event{};
            SendEvent(endpointId, event);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetSmokeState(EndpointId endpointId, AlarmStateEnum newSmokeState)
{
    AlarmStateEnum smokeState;
    bool success = GetAttribute(endpointId, Attributes::SmokeState::Id, Attributes::SmokeState::Get, smokeState);

    if (success && (smokeState != newSmokeState))
    {
        success = SetAttribute(endpointId, Attributes::SmokeState::Id, Attributes::SmokeState::Set, newSmokeState);

        if (success && (newSmokeState == AlarmStateEnum::kWarning || newSmokeState == AlarmStateEnum::kCritical))
        {
            Events::SmokeAlarm::Type event{ newSmokeState };
            SendEvent(endpointId, event);
        }

        if (success && (newSmokeState == AlarmStateEnum::kCritical))
        {
            SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetCOState(EndpointId endpointId, AlarmStateEnum newCOState)
{
    AlarmStateEnum coState;
    bool success = GetAttribute(endpointId, Attributes::COState::Id, Attributes::COState::Get, coState);

    if (success && (coState != newCOState))
    {
        success = SetAttribute(endpointId, Attributes::COState::Id, Attributes::COState::Set, newCOState);

        if (success && (newCOState == AlarmStateEnum::kWarning || newCOState == AlarmStateEnum::kCritical))
        {
            Events::COAlarm::Type event{ newCOState };
            SendEvent(endpointId, event);
        }

        if (success && (newCOState == AlarmStateEnum::kCritical))
        {
            SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetBatteryAlert(EndpointId endpointId, AlarmStateEnum newBatteryAlert)
{
    AlarmStateEnum batteryAlert;
    bool success = GetAttribute(endpointId, Attributes::BatteryAlert::Id, Attributes::BatteryAlert::Get, batteryAlert);

    if (success && (batteryAlert != newBatteryAlert))
    {
        success = SetAttribute(endpointId, Attributes::BatteryAlert::Id, Attributes::BatteryAlert::Set, newBatteryAlert);

        if (success && (newBatteryAlert == AlarmStateEnum::kWarning || newBatteryAlert == AlarmStateEnum::kCritical))
        {
            Events::LowBattery::Type event{ newBatteryAlert };
            SendEvent(endpointId, event);
        }

        if (success && (newBatteryAlert == AlarmStateEnum::kCritical))
        {
            SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetDeviceMuted(EndpointId endpointId, MuteStateEnum newDeviceMuted)
{
    MuteStateEnum deviceMuted;
    bool success = GetAttribute(endpointId, Attributes::DeviceMuted::Id, Attributes::DeviceMuted::Get, deviceMuted);

    if (success && (deviceMuted != newDeviceMuted))
    {
        if (newDeviceMuted == MuteStateEnum::kMuted)
        {
            AlarmStateEnum alarmState;
            success = GetAttribute(endpointId, Attributes::SmokeState::Id, Attributes::SmokeState::Get, alarmState);
            if (success && (alarmState == AlarmStateEnum::kCritical))
            {
                success = false;
            }

            if (success)
            {
                success = GetAttribute(endpointId, Attributes::COState::Id, Attributes::COState::Get, alarmState);
                if (success && (alarmState == AlarmStateEnum::kCritical))
                {
                    success = false;
                }
            }

            if (success)
            {
                success = GetAttribute(endpointId, Attributes::BatteryAlert::Id, Attributes::BatteryAlert::Get, alarmState);
                if (success && (alarmState == AlarmStateEnum::kCritical))
                {
                    success = false;
                }
            }

            if (success)
            {
                success = GetAttribute(endpointId, Attributes::InterconnectSmokeAlarm::Id, Attributes::InterconnectSmokeAlarm::Get,
                                       alarmState);
                if (success && (alarmState == AlarmStateEnum::kCritical))
                {
                    success = false;
                }
            }

            if (success)
            {
                success =
                    GetAttribute(endpointId, Attributes::InterconnectCOAlarm::Id, Attributes::InterconnectCOAlarm::Get, alarmState);
                if (success && (alarmState == AlarmStateEnum::kCritical))
                {
                    success = false;
                }
            }
        }

        if (success)
        {
            success = SetAttribute(endpointId, Attributes::DeviceMuted::Id, Attributes::DeviceMuted::Set, newDeviceMuted);
        }

        if (success)
        {
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
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetTestInProgress(EndpointId endpointId, bool newTestInProgress)
{
    bool testInProgress;
    bool success = GetAttribute(endpointId, Attributes::TestInProgress::Id, Attributes::TestInProgress::Get, testInProgress);

    if (success && (testInProgress != newTestInProgress))
    {
        success = SetAttribute(endpointId, Attributes::TestInProgress::Id, Attributes::TestInProgress::Set, newTestInProgress);

        if (success && !newTestInProgress)
        {
            Events::SelfTestComplete::Type event{};
            SendEvent(endpointId, event);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetHardwareFaultAlert(EndpointId endpointId, bool newHardwareFaultAlert)
{
    bool hardwareFaultAlert;
    bool success =
        GetAttribute(endpointId, Attributes::HardwareFaultAlert::Id, Attributes::HardwareFaultAlert::Get, hardwareFaultAlert);

    if (success && (hardwareFaultAlert != newHardwareFaultAlert))
    {
        success = SetAttribute(endpointId, Attributes::HardwareFaultAlert::Id, Attributes::HardwareFaultAlert::Set,
                               newHardwareFaultAlert);

        if (success && newHardwareFaultAlert)
        {
            Events::HardwareFault::Type event{};
            SendEvent(endpointId, event);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetEndOfServiceAlert(EndpointId endpointId, EndOfServiceEnum newEndOfServiceAlert)
{
    EndOfServiceEnum endOfServiceAlert;
    bool success =
        GetAttribute(endpointId, Attributes::EndOfServiceAlert::Id, Attributes::EndOfServiceAlert::Get, endOfServiceAlert);

    if (success && (endOfServiceAlert != newEndOfServiceAlert))
    {
        success =
            SetAttribute(endpointId, Attributes::EndOfServiceAlert::Id, Attributes::EndOfServiceAlert::Set, newEndOfServiceAlert);

        if (success && (newEndOfServiceAlert == EndOfServiceEnum::kExpired))
        {
            Events::EndOfService::Type event{};
            SendEvent(endpointId, event);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetInterconnectSmokeAlarm(EndpointId endpointId, AlarmStateEnum newInterconnectSmokeAlarm)
{
    AlarmStateEnum interconnectSmokeAlarm;
    bool success = GetAttribute(endpointId, Attributes::InterconnectSmokeAlarm::Id, Attributes::InterconnectSmokeAlarm::Get,
                                interconnectSmokeAlarm);

    if (success && (interconnectSmokeAlarm != newInterconnectSmokeAlarm))
    {
        success = SetAttribute(endpointId, Attributes::InterconnectSmokeAlarm::Id, Attributes::InterconnectSmokeAlarm::Set,
                               newInterconnectSmokeAlarm);

        if (success &&
            (newInterconnectSmokeAlarm == AlarmStateEnum::kWarning || newInterconnectSmokeAlarm == AlarmStateEnum::kCritical))
        {
            Events::InterconnectSmokeAlarm::Type event{ newInterconnectSmokeAlarm };
            SendEvent(endpointId, event);
        }

        if (success && (newInterconnectSmokeAlarm == AlarmStateEnum::kCritical))
        {
            SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetInterconnectCOAlarm(EndpointId endpointId, AlarmStateEnum newInterconnectCOAlarm)
{
    AlarmStateEnum interconnectCOAlarm;
    bool success =
        GetAttribute(endpointId, Attributes::InterconnectCOAlarm::Id, Attributes::InterconnectCOAlarm::Get, interconnectCOAlarm);

    if (success && (interconnectCOAlarm != newInterconnectCOAlarm))
    {
        success = SetAttribute(endpointId, Attributes::InterconnectCOAlarm::Id, Attributes::InterconnectCOAlarm::Set,
                               newInterconnectCOAlarm);

        if (success && (newInterconnectCOAlarm == AlarmStateEnum::kWarning || newInterconnectCOAlarm == AlarmStateEnum::kCritical))
        {
            Events::InterconnectCOAlarm::Type event{ newInterconnectCOAlarm };
            SendEvent(endpointId, event);
        }

        if (success && (newInterconnectCOAlarm == AlarmStateEnum::kCritical))
        {
            SetDeviceMuted(endpointId, MuteStateEnum::kNotMuted);
        }
    }

    return success;
}

bool SmokeCoAlarmServer::SetContaminationState(EndpointId endpointId, ContaminationStateEnum newContaminationState)
{
    ContaminationStateEnum contaminationState;
    bool success =
        GetAttribute(endpointId, Attributes::ContaminationState::Id, Attributes::ContaminationState::Get, contaminationState);

    if (success && (contaminationState != newContaminationState))
    {
        success = SetAttribute(endpointId, Attributes::ContaminationState::Id, Attributes::ContaminationState::Set,
                               newContaminationState);
    }

    return success;
}

bool SmokeCoAlarmServer::SetSmokeSensitivityLevel(EndpointId endpointId, SensitivityEnum newSmokeSensitivityLevel)
{
    SensitivityEnum smokeSensitivityLevel;
    bool success = GetAttribute(endpointId, Attributes::SmokeSensitivityLevel::Id, Attributes::SmokeSensitivityLevel::Get,
                                smokeSensitivityLevel);

    if (success && (smokeSensitivityLevel != newSmokeSensitivityLevel))
    {
        success = SetAttribute(endpointId, Attributes::SmokeSensitivityLevel::Id, Attributes::SmokeSensitivityLevel::Set,
                               newSmokeSensitivityLevel);
    }

    return success;
}

bool SmokeCoAlarmServer::GetExpressedState(chip ::EndpointId endpointId, ExpressedStateEnum & expressedState)
{
    return GetAttribute(endpointId, Attributes::ExpressedState::Id, Attributes::ExpressedState::Get, expressedState);
}

bool SmokeCoAlarmServer::GetSmokeState(EndpointId endpointId, AlarmStateEnum & smokeState)
{
    return GetAttribute(endpointId, Attributes::SmokeState::Id, Attributes::SmokeState::Get, smokeState);
}

bool SmokeCoAlarmServer::GetCOState(EndpointId endpointId, AlarmStateEnum & coState)
{
    return GetAttribute(endpointId, Attributes::COState::Id, Attributes::COState::Get, coState);
}

bool SmokeCoAlarmServer::GetBatteryAlert(EndpointId endpointId, AlarmStateEnum & batteryAlert)
{
    return GetAttribute(endpointId, Attributes::BatteryAlert::Id, Attributes::BatteryAlert::Get, batteryAlert);
}

bool SmokeCoAlarmServer::GetDeviceMuted(EndpointId endpointId, MuteStateEnum & deviceMuted)
{
    return GetAttribute(endpointId, Attributes::DeviceMuted::Id, Attributes::DeviceMuted::Get, deviceMuted);
}

bool SmokeCoAlarmServer::GetTestInProgress(EndpointId endpointId, bool & testInProgress)
{
    return GetAttribute(endpointId, Attributes::TestInProgress::Id, Attributes::TestInProgress::Get, testInProgress);
}

bool SmokeCoAlarmServer::GetHardwareFaultAlert(EndpointId endpointId, bool & hardwareFaultAlert)
{
    return GetAttribute(endpointId, Attributes::HardwareFaultAlert::Id, Attributes::HardwareFaultAlert::Get, hardwareFaultAlert);
}

bool SmokeCoAlarmServer::GetEndOfServiceAlert(EndpointId endpointId, EndOfServiceEnum & endOfServiceAlert)
{
    return GetAttribute(endpointId, Attributes::EndOfServiceAlert::Id, Attributes::EndOfServiceAlert::Get, endOfServiceAlert);
}

bool SmokeCoAlarmServer::GetInterconnectSmokeAlarm(EndpointId endpointId, AlarmStateEnum & interconnectSmokeAlarm)
{
    return GetAttribute(endpointId, Attributes::InterconnectSmokeAlarm::Id, Attributes::InterconnectSmokeAlarm::Get,
                        interconnectSmokeAlarm);
}

bool SmokeCoAlarmServer::GetInterconnectCOAlarm(EndpointId endpointId, AlarmStateEnum & interconnectCOAlarm)
{
    return GetAttribute(endpointId, Attributes::InterconnectCOAlarm::Id, Attributes::InterconnectCOAlarm::Get, interconnectCOAlarm);
}

bool SmokeCoAlarmServer::GetContaminationState(EndpointId endpointId, ContaminationStateEnum & contaminationState)
{
    return GetAttribute(endpointId, Attributes::ContaminationState::Id, Attributes::ContaminationState::Get, contaminationState);
}

bool SmokeCoAlarmServer::GetSmokeSensitivityLevel(EndpointId endpointId, SensitivityEnum & smokeSensitivityLevel)
{
    return GetAttribute(endpointId, Attributes::SmokeSensitivityLevel::Id, Attributes::SmokeSensitivityLevel::Get,
                        smokeSensitivityLevel);
}

bool SmokeCoAlarmServer::GetExpiryDate(EndpointId endpointId, uint32_t & expiryDate)
{
    return GetAttribute(endpointId, Attributes::ExpiryDate::Id, Attributes::ExpiryDate::Get, expiryDate);
}

chip::BitFlags<Feature> SmokeCoAlarmServer::GetFeatures(EndpointId endpointId)
{
    chip::BitFlags<Feature> featureMap;
    if (!GetAttribute(endpointId, Attributes::FeatureMap::Id, Attributes::FeatureMap::Get, *featureMap.RawStorage()))
    {
        ChipLogError(Zcl, "Unable to get the Smoke CO Alarm feature map: attribute read error");
        featureMap.ClearAll();
    }
    return featureMap;
}

/**********************************************************
 * SmokeCoAlarmServer private methods
 *********************************************************/

bool SmokeCoAlarmServer::HandleRemoteSelfTestRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath)
{
    EndpointId endpointId = commandPath.mEndpointId;
    Status status         = Status::Success;

    ExpressedStateEnum expressedState;
    bool success = GetExpressedState(endpointId, expressedState);

    if (success)
    {
        // If the value is busy then return busy
        if (expressedState == ExpressedStateEnum::kSmokeAlarm || expressedState == ExpressedStateEnum::kCOAlarm ||
            expressedState == ExpressedStateEnum::kTesting || expressedState == ExpressedStateEnum::kInterconnectSmoke ||
            expressedState == ExpressedStateEnum::kInterconnectCO)
        {
            status = Status::Busy;
        }
        else
        {
            success = SetExpressedState(endpointId, ExpressedStateEnum::kTesting);

            if (success)
            {
                success = emberAfPluginSmokeCoAlarmSelfTestRequestCommand(endpointId);
            }
        }
    }

    if (!success)
    {
        status = Status::Failure;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
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
                                      EmberAfStatus (*getFn)(EndpointId endpointId, T * value), T & value) const
{
    EmberAfStatus status = getFn(endpointId, &value);
    bool success         = (EMBER_ZCL_STATUS_SUCCESS == status);

    if (!success)
    {
        ChipLogError(Zcl, "Failed to read SmokeCOAlarm attribute: attribute=" ChipLogFormatMEI ", status=0x%x",
                     ChipLogValueMEI(attributeId), to_underlying(status));
    }
    return success;
}

template <typename T>
bool SmokeCoAlarmServer::SetAttribute(EndpointId endpointId, AttributeId attributeId,
                                      EmberAfStatus (*setFn)(EndpointId endpointId, T value), T value)
{
    EmberAfStatus status = setFn(endpointId, value);
    bool success         = (EMBER_ZCL_STATUS_SUCCESS == status);

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
    return SmokeCoAlarmServer::Instance().HandleRemoteSelfTestRequest(commandObj, commandPath);
}

void MatterSmokeCoAlarmPluginServerInitCallback() {}
