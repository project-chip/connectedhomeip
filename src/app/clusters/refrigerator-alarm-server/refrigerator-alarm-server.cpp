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
 *
 */
#include "refrigerator-alarm-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAlarm::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

RefrigeratorAlarmServer RefrigeratorAlarmServer::instance;

RefrigeratorAlarmServer & RefrigeratorAlarmServer::Instance()
{
    return instance;
}

EmberAfStatus RefrigeratorAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask)
{
    EmberAfStatus status = Attributes::Mask::Get(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading  mask %x", status);
        return status;
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask->Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    EmberAfStatus status = Attributes::State::Get(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading state %x", status);
        return status;
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: State ep%d value: %" PRIu32 "", endpoint, state->Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> & mask)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    status               = Attributes::Mask::Set(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: writing  mask %x", status);
        return status;
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask.Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::SetStateValue(EndpointId endpoint, BitMask<AlarmMap> state)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    status               = Attributes::State::Set(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: writing  state %x", status);
        return status;
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: State ep%d value: %" PRIu32 "", endpoint, state.Raw());

    // Generate Notify evenet.
    BitMask<AlarmMap> becameActive   = state;
    BitMask<AlarmMap> becameInActive = ~state.Raw();
    // This field SHALL be a copy of the Mask attribute when this event was generated.
    BitMask<AlarmMap> eMask;
    BitMask<AlarmMap> newState = becameActive;

    status = GetMaskValue(endpoint, &eMask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
    SendNotifyEvent(endpoint, becameActive, becameInActive, eMask, newState);

    return status;
}

bool RefrigeratorAlarmServer::SendNotifyEvent(EndpointId endpointId, BitMask<AlarmMap> becameActive,
                                              BitMask<AlarmMap> becameInactive, BitMask<AlarmMap> state, BitMask<AlarmMap> mask)
{
    Events::Notify::Type event{ .active = becameInactive, .inactive = becameInactive, .state = state, .mask = mask };
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }

    return true;
}

void RefrigeratorAlarmServer::ModifyEnabledAlarmsCommand(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                         const Commands::ModifyEnabledAlarms::DecodableType & commandData)
{
    auto & mask         = commandData.mask;
    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status;
    BitMask<AlarmMap> state;

    status = SetMaskValue(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        commandObj->AddStatus(commandPath, ToInteractionModelStatus(status));
        return;
    }

    status = GetStateValue(endpoint, &state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        commandObj->AddStatus(commandPath, ToInteractionModelStatus(status));
        return;
    }

    state  = state & mask;
    status = SetStateValue(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        commandObj->AddStatus(commandPath, ToInteractionModelStatus(status));
        return;
    }

    commandObj->AddStatus(commandPath, Status::Success);
}
/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfRefrigeratorAlarmClusterModifyEnabledAlarmsCallback(app::CommandHandler * commandObj,
                                                                const app::ConcreteCommandPath & commandPath,
                                                                const Commands::ModifyEnabledAlarms::DecodableType & commandData)
{
    RefrigeratorAlarmServer::Instance().ModifyEnabledAlarmsCommand(commandObj, commandPath, commandData);
    return true;
}

void emberAfRefrigeratorAlarmClusterServerInitCallback(EndpointId endpoint) {}

void MatterRefrigeratorAlarmPluginServerInitCallback() {}
