/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "smoke-co-alarm-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <app/util/error-mapping.h>
#include <app/util/util.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;
using chip::Protocols::InteractionModel::Status;

template <typename Enumeration>
auto as_integer(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

/**********************************************************
 * Attributes Definition
 *********************************************************/

SmokeCoAlarmServer SmokeCoAlarmServer::instance;

/**********************************************************
 * SmokeCoAlarmServer Implementation
 *********************************************************/

SmokeCoAlarmServer & SmokeCoAlarmServer::Instance()
{
    return instance;
}

EmberAfStatus SmokeCoAlarmServer::selfTestRequestCommand(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath)
{
    ExpressedStateEnum currentValue;

    EmberAfStatus status = getExpressedState(commandPath.mEndpointId, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is busy then do nothing
        if (currentValue == ExpressedStateEnum::kSmokeAlarm || currentValue == ExpressedStateEnum::kCOAlarm ||
            currentValue == ExpressedStateEnum::kTesting || currentValue == ExpressedStateEnum::kInterconnectSmoke ||
            currentValue == ExpressedStateEnum::kInterconnectCO)
        {
            emberAfAlarmClusterPrintln("Endpoint %x is busy", commandPath.mEndpointId);
            status = EMBER_ZCL_STATUS_BUSY;
        }
    }

    commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
    return status;
}

void SmokeCoAlarmServer::initSmokeCoAlarmServer(EndpointId endpoint)
{
    emberAfPluginSmokeCoAlarmClusterServerPostInitCallback(endpoint);
}

EmberAfStatus SmokeCoAlarmServer::getExpressedState(EndpointId endpoint, ExpressedStateEnum * currentValue)
{
    // read current ExpressedState value
    EmberAfStatus status = Attributes::ExpressedState::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading ExpressedState %x", status);
    }

    emberAfAlarmClusterPrintln("ExpressedState ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setExpressedState(chip::EndpointId endpoint, ExpressedStateEnum newValue)
{
    ExpressedStateEnum currentValue;

    EmberAfStatus status = getExpressedState(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new ExpressedState value
            status = Attributes::ExpressedState::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing ExpressedState %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("ExpressedState ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getSmokeState(EndpointId endpoint, AlarmStateEnum * currentValue)
{
    // read current SmokeState value
    EmberAfStatus status = Attributes::SmokeState::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading SmokeState %x", status);
    }

    emberAfAlarmClusterPrintln("SmokeState ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setSmokeState(chip::EndpointId endpoint, AlarmStateEnum newValue)
{
    AlarmStateEnum currentValue;

    EmberAfStatus status = getSmokeState(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new SmokeState value
            status = Attributes::SmokeState::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing SmokeState %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("SmokeState ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getCOState(EndpointId endpoint, AlarmStateEnum * currentValue)
{
    // read current COState value
    EmberAfStatus status = Attributes::COState::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading COState %x", status);
    }

    emberAfAlarmClusterPrintln("COState ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setCOState(chip::EndpointId endpoint, AlarmStateEnum newValue)
{
    AlarmStateEnum currentValue;

    EmberAfStatus status = getCOState(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new COState value
            status = Attributes::COState::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing COState %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("COState ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getBatteryAlert(EndpointId endpoint, AlarmStateEnum * currentValue)
{
    // read current BatteryAlert value
    EmberAfStatus status = Attributes::BatteryAlert::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading BatteryAlert %x", status);
    }

    emberAfAlarmClusterPrintln("BatteryAlert ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setBatteryAlert(chip::EndpointId endpoint, AlarmStateEnum newValue)
{
    AlarmStateEnum currentValue;

    EmberAfStatus status = getBatteryAlert(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new BatteryAlert value
            status = Attributes::BatteryAlert::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing BatteryAlert %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("BatteryAlert ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getDeviceMuted(EndpointId endpoint, MuteStateEnum * currentValue)
{
    // read current DeviceMuted value
    EmberAfStatus status = Attributes::DeviceMuted::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading DeviceMuted %x", status);
    }

    emberAfAlarmClusterPrintln("DeviceMuted ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setDeviceMuted(chip::EndpointId endpoint, MuteStateEnum newValue)
{
    MuteStateEnum currentValue;

    EmberAfStatus status = getDeviceMuted(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new DeviceMuted value
            status = Attributes::DeviceMuted::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing DeviceMuted %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("DeviceMuted ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getTestInProgress(EndpointId endpoint, bool * currentValue)
{
    // read current TestInProgress value
    EmberAfStatus status = Attributes::TestInProgress::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading TestInProgress %x", status);
    }

    emberAfAlarmClusterPrintln("TestInProgress ep%d value: %d", endpoint, *currentValue);

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setTestInProgress(chip::EndpointId endpoint, bool newValue)
{
    bool currentValue;

    EmberAfStatus status = getTestInProgress(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new TestInProgress value
            status = Attributes::TestInProgress::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing TestInProgress %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("TestInProgress ep%d value: %d", endpoint, newValue);
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getHardwareFaultAlert(EndpointId endpoint, bool * currentValue)
{
    // read current HardwareFaultAlert value
    EmberAfStatus status = Attributes::HardwareFaultAlert::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading HardwareFaultAlert %x", status);
    }

    emberAfAlarmClusterPrintln("HardwareFaultAlert ep%d value: %d", endpoint, *currentValue);

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setHardwareFaultAlert(chip::EndpointId endpoint, bool newValue)
{
    bool currentValue;

    EmberAfStatus status = getHardwareFaultAlert(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new HardwareFaultAlert value
            status = Attributes::HardwareFaultAlert::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing HardwareFaultAlert %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("HardwareFaultAlert ep%d value: %d", endpoint, newValue);
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getEndOfServiceAlert(EndpointId endpoint, EndOfServiceEnum * currentValue)
{
    // read current EndOfServiceAlert value
    EmberAfStatus status = Attributes::EndOfServiceAlert::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading EndOfServiceAlert %x", status);
    }

    emberAfAlarmClusterPrintln("EndOfServiceAlert ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setEndOfServiceAlert(chip::EndpointId endpoint, EndOfServiceEnum newValue)
{
    EndOfServiceEnum currentValue;

    EmberAfStatus status = getEndOfServiceAlert(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new EndOfServiceAlert value
            status = Attributes::EndOfServiceAlert::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing EndOfServiceAlert %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("EndOfServiceAlert ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getInterconnectSmokeAlarm(EndpointId endpoint, AlarmStateEnum * currentValue)
{
    // read current InterconnectSmokeAlarm value
    EmberAfStatus status = Attributes::InterconnectSmokeAlarm::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading InterconnectSmokeAlarm %x", status);
    }

    emberAfAlarmClusterPrintln("InterconnectSmokeAlarm ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setInterconnectSmokeAlarm(chip::EndpointId endpoint, AlarmStateEnum newValue)
{
    AlarmStateEnum currentValue;

    EmberAfStatus status = getInterconnectSmokeAlarm(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new InterconnectSmokeAlarm value
            status = Attributes::InterconnectSmokeAlarm::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing InterconnectSmokeAlarm %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("InterconnectSmokeAlarm ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getInterconnectCOAlarm(EndpointId endpoint, AlarmStateEnum * currentValue)
{
    // read current InterconnectCOAlarm value
    EmberAfStatus status = Attributes::InterconnectCOAlarm::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading InterconnectCOAlarm %x", status);
    }

    emberAfAlarmClusterPrintln("InterconnectCOAlarm ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setInterconnectCOAlarm(chip::EndpointId endpoint, AlarmStateEnum newValue)
{
    AlarmStateEnum currentValue;

    EmberAfStatus status = getInterconnectCOAlarm(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new InterconnectCOAlarm value
            status = Attributes::InterconnectCOAlarm::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing InterconnectCOAlarm %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("InterconnectCOAlarm ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getContaminationState(EndpointId endpoint, ContaminationStateEnum * currentValue)
{
    // read current ContaminationState value
    EmberAfStatus status = Attributes::ContaminationState::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading ContaminationState %x", status);
    }

    emberAfAlarmClusterPrintln("ContaminationState ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setContaminationState(chip::EndpointId endpoint, ContaminationStateEnum newValue)
{
    ContaminationStateEnum currentValue;

    EmberAfStatus status = getContaminationState(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new ContaminationState value
            status = Attributes::ContaminationState::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing ContaminationState %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("ContaminationState ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getSensitivityLevel(EndpointId endpoint, SensitivityEnum * currentValue)
{
    // read current SensitivityLevel value
    EmberAfStatus status = Attributes::SensitivityLevel::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading SensitivityLevel %x", status);
    }

    emberAfAlarmClusterPrintln("SensitivityLevel ep%d value: %d", endpoint, as_integer(*currentValue));

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setSensitivityLevel(chip::EndpointId endpoint, SensitivityEnum newValue)
{
    SensitivityEnum currentValue;

    EmberAfStatus status = getSensitivityLevel(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new SensitivityLevel value
            status = Attributes::SensitivityLevel::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing SensitivityLevel %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("SensitivityLevel ep%d value: %d", endpoint, as_integer(newValue));
            }
        }
    }

    return status;
}

EmberAfStatus SmokeCoAlarmServer::getExpiryDate(EndpointId endpoint, uint32_t * currentValue)
{
    // read current ExpiryDate value
    EmberAfStatus status = Attributes::ExpiryDate::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading ExpiryDate %x", status);
    }

    emberAfAlarmClusterPrintln("ExpiryDate ep%d value: %ld", endpoint, *currentValue);

    return status;
}

EmberAfStatus SmokeCoAlarmServer::setExpiryDate(chip::EndpointId endpoint, uint32_t newValue)
{
    uint32_t currentValue;

    EmberAfStatus status = getExpiryDate(endpoint, &currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new ExpiryDate value
            status = Attributes::ExpiryDate::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing ExpiryDate %x", status);
            }
            else
            {
                emberAfAlarmClusterPrintln("ExpiryDate ep%d value: %ld", endpoint, newValue);
            }
        }
    }

    return status;
}

bool SmokeCoAlarmServer::sendSmokeAlarmEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::SmokeAlarm::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record SmokeAlarm event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendCOAlarmEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::COAlarm::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record COAlarm event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendLowBatteryEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::LowBattery::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record LowBattery event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendHardwareFaultEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::HardwareFault::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record HardwareFault event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendEndOfServiceEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::EndOfService::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record EndOfService event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendSelfTestCompleteEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::SelfTestComplete::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record SelfTestComplete event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendAlarmMutedEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::AlarmMuted::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record AlarmMuted event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendMuteEndedEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::MuteEnded::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record MuteEnded event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendInterconnectSmokeAlarmEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::InterconnectSmokeAlarm::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record InterconnectSmokeAlarm event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendInterconnectCOAlarmEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::InterconnectCOAlarm::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record InterconnectCOAlarm event");
    }

    return true;
}

bool SmokeCoAlarmServer::sendAllClearEvent(EndpointId endpoint)
{
    EventNumber eventNumber;
    Events::AllClear::Type event{};

    if (CHIP_NO_ERROR != app::LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SmokeCoAlarmServer: Failed to record AllClear event");
    }

    return true;
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfSmokeCoAlarmClusterSelfTestRequestCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::SelfTestRequest::DecodableType & commandData)
{
    return SmokeCoAlarmServer::Instance().selfTestRequestCommand(commandObj, commandPath);
}

void emberAfSmokeCoAlarmClusterServerInitCallback(chip::EndpointId endpoint)
{
    SmokeCoAlarmServer::Instance().initSmokeCoAlarmServer(endpoint);
}

void emberAfPluginSmokeCoAlarmClusterServerPostInitCallback(EndpointId endpoint) {}

void MatterSmokeCoAlarmPluginServerInitCallback() {}
