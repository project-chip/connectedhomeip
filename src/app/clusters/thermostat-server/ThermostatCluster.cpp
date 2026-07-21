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

#include "ThermostatCluster.h"
#include "SetpointAttributes.h"
#include "ThermostatClusterEvents.h"
#include "ThermostatClusterSetpoints.h"

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

// IMPORTANT NOTE:
// No Side effects are permitted in emberAfThermostatClusterServerPreAttributeChangedCallback
// If a setpoint changes is required as a result of setpoint limit change
// it does not happen here.  It is the responsibility of the device to adjust the setpoint(s)
// as required in emberAfThermostatClusterServerPostAttributeChangedCallback
// limit change validation assures that there is at least 1 setpoint that will be valid

static_assert(kThermostatEndpointCount <= kEmberInvalidEndpointIndex, "Thermostat Delegate table size error");

Delegate * gDelegateTable[kThermostatEndpointCount] = { nullptr };

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatAttrAccess gThermostatAttrAccess;

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= MATTER_ARRAY_SIZE(gDelegateTable) ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found, add the delegate in the delegate table
    if (ep < MATTER_ARRAY_SIZE(gDelegateTable))
    {
        gDelegateTable[ep] = delegate;
        delegate->SetEndpointId(endpoint);
    }
}

typedef Status (*SetpointGetter)(EndpointId endpoint, int16_t * value);
typedef Status (*SetpointSetter)(EndpointId endpoint, int16_t value);

void GenerateSetpointEvent(chip::EndpointId endpoint, SystemModeEnum systemMode, Optional<BitMask<OccupancyBitmap>> occupancy,
                           SetpointGetter getter)
{
    int16_t setpoint;
    auto status = getter(endpoint, &setpoint);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "GenerateSetpointEvent failed to queue event: could not get set point");
        return;
    }
    GenerateSetpointChangeEvent(endpoint, systemMode, occupancy, NullOptional, setpoint);
}

CHIP_ERROR ThermostatAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == Status::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);

    switch (aPath.mAttributeId)
    {
    case LocalTemperature::Id:
        if (localTemperatureNotExposedSupported)
        {
            return aEncoder.EncodeNull();
        }
        break;
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing;
            Status status = RemoteSensing::Get(aPath.mEndpointId, &valueRemoteSensing);
            if (status != Status::Success)
            {
                StatusIB statusIB(status);
                return statusIB.ToChipError();
            }
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
            return aEncoder.Encode(valueRemoteSensing);
        }
        break;
    case PresetTypes::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetTypeStruct::Type presetType;
                auto err = delegate->GetPresetTypeAtIndex(i, presetType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(presetType));
            }
        });
    }
    break;
    case NumberOfPresets::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(delegate->GetNumberOfPresets()));
    }
    break;
    case Presets::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        auto & subjectDescriptor = aEncoder.GetSubjectDescriptor();
        if (InAtomicWrite(aPath.mEndpointId, subjectDescriptor, MakeOptional(aPath.mAttributeId)))
        {
            return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
                for (uint8_t i = 0; true; i++)
                {
                    PresetStructWithOwnedMembers preset;
                    auto err = delegate->GetPendingPresetAtIndex(i, preset);
                    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                    {
                        return CHIP_NO_ERROR;
                    }
                    ReturnErrorOnFailure(err);
                    ReturnErrorOnFailure(encoder.Encode(preset));
                }
            });
        }
        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetStructWithOwnedMembers preset;
                auto err = delegate->GetPresetAtIndex(i, preset);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(preset));
            }
        });
    }
    break;
    case ActivePresetHandle::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        uint8_t buffer[kPresetHandleSize];
        MutableByteSpan activePresetHandleSpan(buffer);
        auto activePresetHandle = DataModel::MakeNullable(activePresetHandleSpan);

        CHIP_ERROR err = delegate->GetActivePresetHandle(activePresetHandle);
        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(aEncoder.Encode(activePresetHandle));
    }
    break;
    case ScheduleTypes::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                ScheduleTypeStruct::Type scheduleType;
                auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(scheduleType));
            }
        });
    }
    break;
    case Schedules::Id: {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case MaxThermostatSuggestions::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(delegate->GetMaxThermostatSuggestions()));
    }
    break;
    case ThermostatSuggestions::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ThermostatSuggestionStructWithOwnedMembers thermostatSuggestion;
                auto err = delegate->GetThermostatSuggestionAtIndex(i, thermostatSuggestion);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(thermostatSuggestion));
            }
        });
    }
    break;
    case CurrentThermostatSuggestion::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> currentThermostatSuggestion;

        delegate->GetCurrentThermostatSuggestion(currentThermostatSuggestion);
        ReturnErrorOnFailure(aEncoder.Encode(currentThermostatSuggestion));
    }
    break;
    case ThermostatSuggestionNotFollowingReason::Id: {
        auto delegate = GetDelegate(aPath.mEndpointId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(delegate->GetThermostatSuggestionNotFollowingReason()));
    }
    break;
    case ClusterRevision::Id:
        return aEncoder.Encode(Thermostat::kRevision);
    default: // return CHIP_NO_ERROR and just read from the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    EndpointId endpoint      = aPath.mEndpointId;
    auto & subjectDescriptor = aDecoder.GetSubjectDescriptor();

    // Check atomic attributes first
    switch (aPath.mAttributeId)
    {
    case Presets::Id: {

        auto delegate = GetDelegate(endpoint);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        // Presets are not editable, return INVALID_IN_STATE.
        VerifyOrReturnError(InAtomicWrite(endpoint, MakeOptional(aPath.mAttributeId)), CHIP_IM_GLOBAL_STATUS(InvalidInState),
                            ChipLogError(Zcl, "Presets are not editable"));

        // OK, we're in an atomic write, make sure the requesting node is the same one that started the atomic write,
        // otherwise return BUSY.
        if (!InAtomicWrite(endpoint, subjectDescriptor, MakeOptional(aPath.mAttributeId)))
        {
            ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
            return CHIP_IM_GLOBAL_STATUS(Busy);
        }

        // If the list operation is replace all, clear the existing pending list, iterate over the new presets list
        // and add to the pending presets list.
        if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            // Clear the pending presets list
            delegate->ClearPendingPresetList();

            Presets::TypeInfo::DecodableType newPresetsList;
            ReturnErrorOnFailure(aDecoder.Decode(newPresetsList));

            // Iterate over the presets and call the delegate to append to the list of pending presets.
            auto iter = newPresetsList.begin();
            while (iter.Next())
            {
                const PresetStruct::Type & preset = iter.GetValue();
                ReturnErrorOnFailure(AppendPendingPreset(delegate, preset));
            }
            return iter.GetStatus();
        }

        // If the list operation is AppendItem, call the delegate to append the item to the list of pending presets.
        if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            PresetStruct::Type preset;
            ReturnErrorOnFailure(aDecoder.Decode(preset));
            return AppendPendingPreset(delegate, preset);
        }
    }
    break;
    case Schedules::Id: {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    break;
    }

    // This is not an atomic attribute, so check to make sure we don't have an atomic write going for this client
    if (InAtomicWrite(endpoint, subjectDescriptor))
    {
        ChipLogError(Zcl, "Can not write to non-atomic attributes during atomic write");
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == Status::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);

    switch (aPath.mAttributeId)
    {
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            uint8_t valueRemoteSensing;
            ReturnErrorOnFailure(aDecoder.Decode(valueRemoteSensing));
            if (valueRemoteSensing & 0x01) // If setting bit 1 (LocalTemperature RemoteSensing bit)
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
            Status status = RemoteSensing::Set(aPath.mEndpointId, valueRemoteSensing);
            StatusIB statusIB(status);
            return statusIB.ToChipError();
        }
        break;

    default: // return CHIP_NO_ERROR and just write to the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

void ThermostatAttrAccess::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(mAtomicWriteSessions); ++i)
    {
        auto & atomicWriteState = mAtomicWriteSessions[i];
        if (atomicWriteState.state == AtomicWriteState::Open && atomicWriteState.nodeId.GetFabricIndex() == fabricIndex)
        {
            ResetAtomicWrite(atomicWriteState.endpointId);
        }
    }
}

void ThermostatAttrAccess::GenerateEvents(const ConcreteAttributePath & attributePath)
{
    switch (attributePath.mAttributeId)
    {
    case SystemMode::Id: {
        SystemModeEnum systemMode = SystemModeEnum::kOff;
        if (SystemMode::Get(attributePath.mEndpointId, &systemMode) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to queue SystemModeChange event: could not get system mode");
        }
        else
        {
            GenerateSystemModeChangeEvent(attributePath.mEndpointId, NullOptional, systemMode);
        }
        break;
    }
    case OccupiedHeatingSetpoint::Id:
        GenerateSetpointEvent(attributePath.mEndpointId, SystemModeEnum::kHeat, MakeOptional(OccupancyBitmap::kOccupied),
                              OccupiedHeatingSetpoint::Get);
        break;
    case OccupiedCoolingSetpoint::Id:
        GenerateSetpointEvent(attributePath.mEndpointId, SystemModeEnum::kCool, MakeOptional(OccupancyBitmap::kOccupied),
                              OccupiedCoolingSetpoint::Get);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        GenerateSetpointEvent(attributePath.mEndpointId, SystemModeEnum::kHeat, MakeOptional(BitMask<OccupancyBitmap>(0)),
                              UnoccupiedHeatingSetpoint::Get);

        break;
    case UnoccupiedCoolingSetpoint::Id:
        GenerateSetpointEvent(attributePath.mEndpointId, SystemModeEnum::kCool, MakeOptional(BitMask<OccupancyBitmap>(0)),
                              UnoccupiedCoolingSetpoint::Get);

        break;
    case LocalTemperature::Id: {
        DataModel::Nullable<int16_t> local_temperature;
        if (LocalTemperature::Get(attributePath.mEndpointId, local_temperature) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to queue LocalTemperatureChange event: could not get local temperature");
        }
        else
        {
            GenerateLocalTemperatureChangeEvent(attributePath.mEndpointId, local_temperature);
        }
        break;
    }
    case Occupancy::Id: {
        BitMask<OccupancyBitmap, uint8_t> occupancy;
        if (Occupancy::Get(attributePath.mEndpointId, &occupancy) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to queue OccupancyChange event: could not get occupancy");
        }
        else
        {
            GenerateOccupancyChangeEvent(attributePath.mEndpointId, chip::Optional<chip::BitMask<OccupancyBitmap>>(), occupancy);
        }
        break;
    }
    case ThermostatRunningState::Id: {
        BitMask<RelayStateBitmap> running_state;
        if (ThermostatRunningState::Get(attributePath.mEndpointId, &running_state) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to queue RunningStateChange event: could not get running state");
        }
        else
        {
            GenerateRunningStateChangeEvent(attributePath.mEndpointId, chip::Optional<chip::BitMask<RelayStateBitmap>>(),
                                            running_state);
        }
        break;
    }
    case ThermostatRunningMode::Id: {
        ThermostatRunningModeEnum running_mode;
        if (ThermostatRunningMode::Get(attributePath.mEndpointId, &running_mode) != Status::Success)
        {
            ChipLogError(Zcl, "Failed to queue RunningModeChange event: could not get running mode");
        }
        else
        {
            GenerateRunningModeChangeEvent(attributePath.mEndpointId, Optional<ThermostatRunningModeEnum>(), running_mode);
        }
        break;
    }
    }
}

void MatterThermostatClusterServerAttributeChangedCallback(const ConcreteAttributePath & attributePath)
{

    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id: {
        HandleSetpointWrite(attributePath);
        break;
    }
    }
    uint32_t flags;
    if (FeatureMap::Get(attributePath.mEndpointId, &flags) != Status::Success)
    {
        ChipLogError(Zcl, "MatterThermostatClusterServerAttributeChangedCallback: could not get feature flags");
        return;
    }
    auto featureMap      = BitMask<Feature, uint32_t>(flags);
    bool supportsPresets = featureMap.Has(Feature::kPresets);
    bool occupied        = true;
    if (featureMap.Has(Feature::kOccupancy))
    {
        BitMask<OccupancyBitmap, uint8_t> occupancy;
        if (Occupancy::Get(attributePath.mEndpointId, &occupancy) == Status::Success)
        {
            occupied = occupancy.Has(OccupancyBitmap::kOccupied);
        }
    }

    bool clearActivePreset = false;
    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case OccupiedCoolingSetpoint::Id:
        clearActivePreset = supportsPresets && occupied;
        break;
    case UnoccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
        clearActivePreset = supportsPresets && !occupied;
        break;
    }
    if (featureMap.Has(Feature::kEvents))
    {
        gThermostatAttrAccess.GenerateEvents(attributePath);
    }
    if (clearActivePreset)
    {
        ChipLogProgress(Zcl, "Setting active preset to null");
        gThermostatAttrAccess.SetActivePreset(attributePath.mEndpointId, std::nullopt);
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void emberAfThermostatClusterServerInitCallback(chip::EndpointId endpoint)
{
    // TODO
    // Get from the "real thermostat"
    // current mode
    // current occupied heating setpoint
    // current unoccupied heating setpoint
    // current occupied cooling setpoint
    // current unoccupied cooling setpoint
    // and update the zcl cluster values
    // This should be a callback defined function
    // with weak binding so that real thermostat
    // can get the values.
    // or should this just be the responsibility of the thermostat application?
}

/*
This callback is invoked before the Ember framework updates an attribute to a new value.

If the client is modifying a setpoint attribute, we must validate the new value in the context of the
other setpoints to which the attribute is related. Depending on the value, we may need to either adjust the
other setpoints to maintain the setpoint rules, adjust the provided value, or return a constraint error.

The order of operations is to first check to see if it's possible to take this new value, even if it means adjusting
other setpoint attributes. If not, we will return a constraint error.

This method works around a limitation of the ember framework. When a client changes a setpoint attribute,
other attributes may need to be updated as well. For example, if a client changes the occupied heating setpoint,
the occupied cooling setpoint may also need to be updated to maintain the setpoint rules. However, it's possible
that a setpoint value cannot be set without violating one of the other rules, and we'll need to return a constraint
error.

That error needs to be returned in MatterThermostatClusterServerPreAttributeChangedCallback, otherwise the new
setpoint value will be committed to the Matter Data Storage even when it should not have been.
*/
Status MatterThermostatClusterServerPreAttributeChangedCallback(const app::ConcreteAttributePath & attributePath,
                                                                EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{

    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id: {
        int16_t temperature = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        Setpoints setpoints;
        auto status = LoadSetpoints(attributePath.mEndpointId, setpoints);
        if (status != Status::Success)
        {
            return status;
        }
        SetpointAttributes changedAttributes;
        return ValidateSetpointChange(setpoints, attributePath.mAttributeId, temperature, changedAttributes);
    }
    case MinSetpointDeadBand::Id: {
        Setpoints setpoints;
        auto status = LoadSetpoints(attributePath.mEndpointId, setpoints);
        if (status != Status::Success)
        {
            return status;
        }
        if (!setpoints.autoSupported)
        {
            return Status::UnsupportedAttribute;
        }
        int8_t requested = static_cast<int8_t>(chip::Encoding::Get8(value));
        if (requested < 0)
        {
            return Status::InvalidValue;
        }
        return Status::Success;
    }
    default:
        return Status::Success;
    }
}

void MatterThermostatClusterServerAttributeChangedCallback(const ConcreteAttributePath & attributePath)
{
    Thermostat::MatterThermostatClusterServerAttributeChangedCallback(attributePath);
}

bool emberAfThermostatClusterClearWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::ClearWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterGetWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::GetWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetWeeklyScheduleCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::SetWeeklySchedule::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Clusters::Thermostat::Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetpointRaiseLowerCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Thermostat::SetpointRaiseLower(commandPath.mEndpointId, commandData));
    return true;
}

void MatterThermostatPluginServerInitCallback()
{
    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().AddFabricDelegate(&gThermostatAttrAccess);
    AttributeAccessInterfaceRegistry::Instance().Register(&gThermostatAttrAccess);
}

void MatterThermostatPluginServerShutdownCallback()
{
    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().RemoveFabricDelegate(&gThermostatAttrAccess);
    AttributeAccessInterfaceRegistry::Instance().Unregister(&gThermostatAttrAccess);
}
