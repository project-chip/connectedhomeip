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

static constexpr size_t kDishwasherAlarmDelegateTableSize =
    EMBER_AF_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kDishwasherAlarmDelegateTableSize <= kEmberInvalidEndpointIndex, "Dishwasher Alarm Delegate table size error");

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

Delegate * gDelegateTable[kDishwasherAlarmDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DishwasherAlarm::Id,
                                                       EMBER_AF_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kDishwasherAlarmDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DishwasherAlarm::Id,
                                                       EMBER_AF_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kDishwasherAlarmDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

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
    return status;
}

EmberAfStatus DishwasherAlarmServer::GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch)
{
    if (!HasResetFeature(endpoint))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm feature: Unsupport Latch attribute");
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    EmberAfStatus status = Attributes::Latch::Get(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  latch, err:0x%x", status);
        return status;
    }
    return status;
}

EmberAfStatus DishwasherAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    EmberAfStatus status = Attributes::State::Get(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: get state, err:0x%x", status);
        return status;
    }
    return status;
}

EmberAfStatus DishwasherAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * supported)
{
    EmberAfStatus status = Attributes::Supported::Get(endpoint, supported);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  supported, err:0x%x", status);
    }
    return status;
}

EmberAfStatus DishwasherAlarmServer::SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    ;
    if ((status = Attributes::Supported::Set(endpoint, supported)) != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing supported, err:0x%x", status);
        return status;
    }
    // Whenever there is change in Supported attribute, Latch should change accordingly (if possible).
    BitMask<AlarmMap> latch;
    if (GetLatchValue(endpoint, &latch) == EMBER_ZCL_STATUS_SUCCESS && !supported.HasAll(latch))
    {
        latch  = latch & supported;
        status = SetLatchValue(endpoint, latch);
    }

    // Whenever there is change in Supported attribute, Mask, State should change accordingly.
    BitMask<AlarmMap> mask;
    if ((status = GetMaskValue(endpoint, &mask)) != EMBER_ZCL_STATUS_SUCCESS)
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

EmberAfStatus DishwasherAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    BitMask<AlarmMap> supported;
    if (GetSupportedValue(endpoint, &supported) || !supported.HasAll(mask))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: Mask is not supported");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    if ((status = Attributes::Mask::Set(endpoint, mask)) != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  mask, err:0x%x", status);
        return status;
    }

    // Whenever there is change in Mask, State should change accordingly.
    BitMask<AlarmMap> state;
    status = GetStateValue(endpoint, &state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }

    if (!mask.HasAll(state))
    {
        state  = mask & state;
        status = SetStateValue(endpoint, state, true);
    }
    return status;
}

EmberAfStatus DishwasherAlarmServer::SetLatchValue(EndpointId endpoint, const BitMask<AlarmMap> latch)
{
    if (!HasResetFeature(endpoint))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm feature: Unsupport Latch attribute");
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }

    BitMask<AlarmMap> supported;
    if (GetSupportedValue(endpoint, &supported) || !supported.HasAll(latch))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: Latch is not supported");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    EmberAfStatus status = Attributes::Latch::Set(endpoint, latch);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  latch, err:0x%x", status);
        return status;
    }

    return status;
}

EmberAfStatus DishwasherAlarmServer::SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState, bool ignoreLatchState)
{
    BitMask<AlarmMap> supported;
    BitMask<AlarmMap> finalNewState;
    finalNewState.Set(newState);

    if (GetSupportedValue(endpoint, &supported) || !supported.HasAll(finalNewState))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: Alarm is not supported");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    BitMask<AlarmMap> mask;
    if (GetMaskValue(endpoint, &mask) || !mask.HasAll(finalNewState))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: Alarm is suppressed");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    BitMask<AlarmMap> currentState;
    status = Attributes::State::Get(endpoint, &currentState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: reading  state, err:0x%x", status);
        return status;
    }

    BitMask<AlarmMap> latch;
    if (!ignoreLatchState && (GetLatchValue(endpoint, &latch) == EMBER_ZCL_STATUS_SUCCESS))
    {
        // Restore bits that have their Latch bit set.
        auto bitsToKeep = latch & currentState;
        finalNewState.Set(bitsToKeep);
    }

    // Store the new value of the State attribute.
    status = Attributes::State::Set(endpoint, finalNewState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: writing  state, err:0x%x", status);
        return status;
    }

    // Generate Notify event.
    BitMask<AlarmMap> becameActive;
    becameActive.Set(finalNewState).Clear(currentState);
    BitMask<AlarmMap> becameInactive;
    becameInactive.Set(currentState).Clear(finalNewState);

    SendNotifyEvent(endpoint, becameActive, becameInactive, finalNewState, mask);
    return status;
}

EmberAfStatus DishwasherAlarmServer::ResetLatchedAlarms(EndpointId endpoint, const BitMask<AlarmMap> alarms)
{
    BitMask<AlarmMap> supported;
    if (GetSupportedValue(endpoint, &supported) || !supported.HasAll(alarms))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm: ERR: Alarm is not supported");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    BitMask<AlarmMap> state;
    if (GetStateValue(endpoint, &state) != EMBER_ZCL_STATUS_SUCCESS)
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    state.Clear(alarms);
    return SetStateValue(endpoint, state, true);
}

bool DishwasherAlarmServer::HasResetFeature(EndpointId endpoint)
{
    uint32_t featureMap = 0;
    if (Attributes::FeatureMap::Get(endpoint, &featureMap) != EMBER_ZCL_STATUS_SUCCESS)
    {
        return false;
    }

    if (featureMap & to_underlying(Feature::kReset))
    {
        return true;
    }
    return false;
}

void DishwasherAlarmServer::SendNotifyEvent(EndpointId endpointId, BitMask<AlarmMap> becameActive, BitMask<AlarmMap> becameInactive,
                                            BitMask<AlarmMap> newState, BitMask<AlarmMap> mask)
{
    Events::Notify::Type event{ .active = becameActive, .inactive = becameInactive, .state = newState, .mask = mask };
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", error.AsString(), endpointId);
    }
}

static Status ModifyEnabledHandler(const app::ConcreteCommandPath & commandPath, const BitMask<AlarmMap> mask)
{
    EndpointId endpoint = commandPath.mEndpointId;
    BitMask<AlarmMap> supported;

    if (DishwasherAlarmServer::Instance().GetSupportedValue(endpoint, &supported) != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

    // receives this command with a Mask that includes bits that are set for alarms which are not supported
    if (!supported.HasAll(mask))
    {
        return Status::InvalidCommand;
    }

    // A server that is unable to enable a currently suppressed alarm,
    // or is unable to suppress a currently enabled alarm SHALL respond
    // with a status code of FAILURE
    Delegate * delegate = DishwasherAlarm::GetDelegate(endpoint);
    if (delegate && !(delegate->ModifyEnabledAlarmsCallback(mask)))
    {
        ChipLogProgress(Zcl, "Unable to modify enabled alarms");
        return Status::Failure;
    }
    // The cluster will do this update if delegate.ModifyEnabledAlarmsCallback() returns true.
    if (DishwasherAlarmServer::Instance().SetMaskValue(endpoint, mask) != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }
    return Status::Success;
}

static Status ResetHandler(const app::ConcreteCommandPath & commandPath, const BitMask<AlarmMap> alarms)
{
    EndpointId endpoint = commandPath.mEndpointId;

    if (!DishwasherAlarmServer::Instance().HasResetFeature(endpoint))
    {
        ChipLogProgress(Zcl, "Dishwasher Alarm feature: Unsupport Reset Command");
        return app::ToInteractionModelStatus(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
    }

    // A server that is unable to reset alarms SHALL respond with a status code of FAILURE
    Delegate * delegate = DishwasherAlarm::GetDelegate(endpoint);
    if (delegate && !(delegate->ResetAlarmsCallback(alarms)))
    {
        ChipLogProgress(Zcl, "Unable to reset alarms");
        return Status::Failure;
    }

    // The cluster will do this update if delegate.ResetAlarmsCallback() returns true.
    if (DishwasherAlarmServer::Instance().ResetLatchedAlarms(endpoint, alarms) != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl, "reset alarms fail");
        return Status::Failure;
    }
    return Status::Success;
}

bool emberAfDishwasherAlarmClusterResetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::Reset::DecodableType & commandData)
{
    auto & alarms = commandData.alarms;

    Status status = ResetHandler(commandPath, alarms);
    commandObj->AddStatus(commandPath, status);

    return true;
}

bool emberAfDishwasherAlarmClusterModifyEnabledAlarmsCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::ModifyEnabledAlarms::DecodableType & commandData)
{
    auto & mask   = commandData.mask;
    Status status = ModifyEnabledHandler(commandPath, mask);
    commandObj->AddStatus(commandPath, status);

    return true;
}
