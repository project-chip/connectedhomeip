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

EmberAfStatus RefrigeratorAlarmServer::getMaskValue(chip::EndpointId endpoint, chip::BitMask<AlarmMap> * mask)
{
    EmberAfStatus status = Attributes::Mask::Get(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading  mask %x", status);
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask->Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::getLatchValue(chip::EndpointId endpoint, chip::BitMask<AlarmMap> * latch)
{
    EmberAfStatus status = Attributes::Latch::Get(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading latch %x", status);
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Latch ep%d value: %" PRIu32 "", endpoint, latch->Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::getStateValue(chip::EndpointId endpoint, chip::BitMask<AlarmMap> * state)
{
    EmberAfStatus status = Attributes::State::Get(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading state %x", status);
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: State ep%d value: %" PRIu32 "", endpoint, state->Raw());

    return status;
}

EmberAfStatus RefrigeratorAlarmServer::setMaskValue(chip::EndpointId endpoint, chip::BitMask<AlarmMap> mask)
{
    chip::BitMask<AlarmMap> * currentMask = nullptr;
    EmberAfStatus status                  = Attributes::Mask::Get(endpoint, currentMask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: reading  mask %x", status);
    }
    if (currentMask->Raw() == mask.Raw())
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Mask value already set to %" PRIu32 "", mask.Raw());
    }
    else
    {
        status = Attributes::Mask::Set(endpoint, mask);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: ERR: writing  mask %x", status);
        }
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Refrigerator Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask.Raw());

    return status;
}

bool RefrigeratorAlarmServer::SendNotifyEvent(chip::EndpointId endpointId, chip::BitMask<AlarmMap> active,
                                              chip::BitMask<AlarmMap> inActive, chip::BitMask<AlarmMap> state,
                                              chip::BitMask<AlarmMap> mask)
{
    Events::Notify::Type event{ active, inActive, state, mask };
    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }

    return true;
}

bool RefrigeratorAlarmServer::ResetCommand(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                           const Commands::Reset::DecodableType & commandData)
{
    auto & alarms = commandData.alarms;
    auto & mask   = commandData.mask;

    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status;

    chip::BitMask<AlarmMap> eActive   = 0;
    chip::BitMask<AlarmMap> eInActive = alarms;
    chip::BitMask<AlarmMap> eMask     = 0;
    chip::BitMask<AlarmMap> eState    = 0;

    chip::BitMask<AlarmMap> state = 0;
    status                        = State::Get(endpoint, &state);

    eActive.Clear(alarms);
    eInActive.Clear(state);
    Mask::Get(endpoint, &eMask);
    eActive = state;
    eState  = state;

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
        return false;
    }

    state.Clear(alarms);
    status = State::Set(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
        return false;
    }

    if (mask.HasValue())
    {
        status = Mask::Set(endpoint, mask.Value());
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
            return false;
        }
    }

    SendNotifyEvent(endpoint, eActive, eInActive, eMask, eState);
    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}
/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfRefrigeratorAlarmClusterResetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::Reset::DecodableType & commandData)
{
    return RefrigeratorAlarmServer::Instance().ResetCommand(commandObj, commandPath, commandData);
}

void emberAfRefrigeratorAlarmClusterServerInitCallback(chip::EndpointId endpoint) {}

void MatterRefrigeratorAlarmPluginServerInitCallback() {}
