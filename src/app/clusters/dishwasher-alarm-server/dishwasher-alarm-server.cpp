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
#include "dishwasher-alarm-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>
#include <lib/support/BitFlags.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DishwasherAlarm;
using namespace chip::app::Clusters::DishwasherAlarm::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

using namespace std;


DishwasherAlarmServer DishwasherAlarmServer::instance;

DishwasherAlarmServer & DishwasherAlarmServer::Instance()
{
    return instance;
}

EmberAfStatus DishwasherAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask)
{
    EmberAfStatus status = Attributes::Mask::Get(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  mask, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask->Raw());

    return status;
}

EmberAfStatus DishwasherAlarmServer::GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch)
{
    EmberAfStatus status = Attributes::Latch::Get(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  latch, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: latch ep%d value: %" PRIu32 "", endpoint, latch->Raw());

    return status;
}

EmberAfStatus DishwasherAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    EmberAfStatus status = Attributes::State::Get(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading state, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: State ep%d value: %" PRIu32 "", endpoint, state->Raw());

    return status;
}

EmberAfStatus DishwasherAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    status               = Attributes::Mask::Set(endpoint, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  mask, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: Mask ep%d value: %" PRIu32 "", endpoint, mask.Raw());

    // Whenever there is change in Mask, State should change accordingly.
    BitMask<AlarmMap> state;
    status = GetStateValue(endpoint, &state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    if (state != (mask & state))
    {
        state  = mask & state;
        status = SetStateValue(endpoint, state);
    }

    return status;
}


EmberAfStatus DishwasherAlarmServer::SetLatchValue(EndpointId endpoint, const BitMask<AlarmMap> latch)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    status               = Attributes::Latch::Set(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  latch, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: latch ep%d value: %" PRIu32 "", endpoint, latch.Raw());
    return status;
}


EmberAfStatus DishwasherAlarmServer::SetStateValue(EndpointId endpoint, BitMask<AlarmMap> newState)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    BitMask<AlarmMap> currentState;

    status = Attributes::State::Get(endpoint, &currentState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  state, err:0x%x", status);
        return status;
    }

    status = Attributes::State::Set(endpoint, newState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  state, err:0x%x", status);
        return status;
    }

    ChipLogProgress(Zcl, "Dishwasher Alarm: State ep%d value: %" PRIu32 "", endpoint, newState.Raw());

	// Maintain consistency between latch and state states
    BitMask<AlarmMap> latch;
    status = GetLatchValue(endpoint, &latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    if (newState != latch)
    {
		latch.SetField(latch,newState.Raw());
        status = SetLatchValue(endpoint, latch);
    }

    // Generate Notify event.
    BitMask<AlarmMap> becameActive;
    becameActive.Set(newState).Clear(currentState);
    BitMask<AlarmMap> becameInactive;
    becameInactive.Set(currentState).Clear(newState);
    // This field SHALL be a copy of the Mask attribute when this event was generated.
    BitMask<AlarmMap> mask;

    status = GetMaskValue(endpoint, &mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
    SendNotifyEvent(endpoint, becameActive, becameInactive, newState, mask);

    return status;
}

void DishwasherAlarmServer::SendNotifyEvent(EndpointId endpointId, BitMask<AlarmMap> becameActive,
                                              BitMask<AlarmMap> becameInactive, BitMask<AlarmMap> newState, BitMask<AlarmMap> mask)
{
    Events::Notify::Type event{ .active = becameActive, .inactive = becameInactive, .state = newState, .mask = mask };
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", error.AsString(), endpointId);
    }
}

static Status resetHandler(const app::ConcreteCommandPath & commandPath, const chip::BitMask<AlarmMap> alarms)
{
    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status;
    chip::BitMask<AlarmMap> state = 0;
    status = State::Get(endpoint, &state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

	chip::BitMask<AlarmMap> latch = 0;
    status = Latch::Get(endpoint, &latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

    uint32_t alarmValue = alarms.Raw();
    uint32_t stateValue = state.Raw();
	uint32_t latchValue = latch.Raw();

    // Flip the bits of alarms (i.e. ~alarms)
    alarmValue = 0xFFFF ^ alarmValue;

    // Reset state from active to inactive
    stateValue = stateValue & alarmValue;

    state.SetRaw(stateValue);
    status = State::Set(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

	// Reset latch from active to inactive
    latchValue = latchValue & alarmValue;

	latch.SetRaw(latchValue);
    status = Latch::Set(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

    return Status::Success;

}

static Status modifyEnabledHandler(const app::ConcreteCommandPath & commandPath, const chip::BitMask<AlarmMap> mask)
{
	EndpointId endpoint = commandPath.mEndpointId;
	chip::BitMask<AlarmMap> support;

	EmberAfStatus status = Attributes::Supported::Get(endpoint, &support);
	
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  supported, err:0x%x", status);
		return Status::Failure;
    }
    else
    {
        ChipLogDetail(Zcl, "Dishwasher Alarm: Supported ep%d value: %" PRIu32 "", endpoint, support.Raw());
    }

	if((support & mask) != mask)
	{
		return Status::Failure;
	}
	status = Mask::Set(endpoint, mask);        
	if (status != EMBER_ZCL_STATUS_SUCCESS)
	{
		return Status::Failure;
	}
	return Status::Success;
}

bool emberAfDishwasherAlarmClusterResetCallback(app::CommandHandler * commandObj,const app::ConcreteCommandPath & commandPath, const Commands::Reset::DecodableType & commandData)
{
    auto & alarms = commandData.alarms;
    Status status = resetHandler(commandPath, alarms);
    commandObj->AddStatus(commandPath, status);

    return true;
}

bool emberAfDishwasherAlarmClusterModifyEnabledAlarmsCallback(app::CommandHandler * commandObj,const app::ConcreteCommandPath & commandPath,const Commands::ModifyEnabledAlarms::DecodableType & commandData)
{
	auto & mask = commandData.mask;
	Status status = modifyEnabledHandler(commandPath, mask);
    commandObj->AddStatus(commandPath, status);

    return true;
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

void MatterDishwasherAlarmPluginServerInitCallback() {}
