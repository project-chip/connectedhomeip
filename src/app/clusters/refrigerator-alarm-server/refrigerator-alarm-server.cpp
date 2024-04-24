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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAlarm::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

using namespace std;
RefrigeratorAlarmServer RefrigeratorAlarmServer::instance;

RefrigeratorAlarmServer & RefrigeratorAlarmServer::Instance()
{
    return instance;
}

Status RefrigeratorAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask)
{
    Status status = Attributes::Mask::Get(endpoint, mask);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: reading  mask, err:0x%x", to_underlying(status));
    }
    else
    {
        ChipLogDetail(Zcl, "Refrigerator Alarm: Mask ep%d value: %" PRIx32 "", endpoint, mask->Raw());
    }

    return status;
}

Status RefrigeratorAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    Status status = Attributes::State::Get(endpoint, state);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: reading state, err:0x%x", to_underlying(status));
    }
    else
    {
        ChipLogDetail(Zcl, "Refrigerator Alarm: State ep%d value: %" PRIx32 "", endpoint, state->Raw());
    }

    return status;
}

Status RefrigeratorAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * supported)
{
    Status status = Attributes::Supported::Get(endpoint, supported);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: reading  supported, err:0x%x", to_underlying(status));
    }
    else
    {
        ChipLogDetail(Zcl, "Refrigerator Alarm: Supported ep%d value: %" PRIx32 "", endpoint, supported->Raw());
    }

    return status;
}

Status RefrigeratorAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    Status status = Status::Success;
    status        = Attributes::Mask::Set(endpoint, mask);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: writing  mask, err:0x%x", to_underlying(status));
        return status;
    }

    ChipLogProgress(Zcl, "Refrigerator Alarm: Mask ep%d value: %" PRIx32 "", endpoint, mask.Raw());

    // Whenever there is change in Mask, State should change accordingly.
    BitMask<AlarmMap> state;
    status = GetStateValue(endpoint, &state);
    if (status != Status::Success)
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

Status RefrigeratorAlarmServer::SetStateValue(EndpointId endpoint, BitMask<AlarmMap> newState)
{
    Status status = Status::Success;
    BitMask<AlarmMap> currentState;

    status = Attributes::State::Get(endpoint, &currentState);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: reading  state, err:0x%x", to_underlying(status));
        return status;
    }

    status = Attributes::State::Set(endpoint, newState);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: writing  state, err:0x%x", to_underlying(status));
        return status;
    }

    ChipLogProgress(Zcl, "Refrigerator Alarm: State ep%d value: %" PRIx32 "", endpoint, newState.Raw());

    // Generate Notify event.
    BitMask<AlarmMap> becameActive;
    becameActive.Set(newState).Clear(currentState);
    BitMask<AlarmMap> becameInactive;
    becameInactive.Set(currentState).Clear(newState);
    // This field SHALL be a copy of the Mask attribute when this event was generated.
    BitMask<AlarmMap> mask;

    status = GetMaskValue(endpoint, &mask);
    if (status != Status::Success)
    {
        return status;
    }
    SendNotifyEvent(endpoint, becameActive, becameInactive, newState, mask);

    return status;
}

Status RefrigeratorAlarmServer::SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported)
{
    Status status = Status::Success;
    status        = Attributes::Supported::Set(endpoint, supported);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "Refrigerator Alarm: ERR: writing supported, err:0x%x", to_underlying(status));
        return status;
    }

    ChipLogProgress(Zcl, "Refrigerator Alarm: Supported ep%d value: %" PRIx32 "", endpoint, supported.Raw());

    // Whenever there is change in Supported attribute, Mask, State should change accordingly.
    BitMask<AlarmMap> mask;
    status = GetMaskValue(endpoint, &mask);
    if (status != Status::Success)
    {
        return status;
    }

    if (!supported.HasAll(mask))
    {
        mask   = supported & mask;
        status = SetMaskValue(endpoint, mask);
    }
    return status;
}

void RefrigeratorAlarmServer::SendNotifyEvent(EndpointId endpointId, BitMask<AlarmMap> becameActive,
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

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

void MatterRefrigeratorAlarmPluginServerInitCallback() {}
