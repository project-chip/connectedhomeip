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
#include "PresetStructWithOwnedMembers.h"
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

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_DEFAULT FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

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

void MatterThermostatClusterServerAttributeChangedCallback(const ConcreteAttributePath & attributePath)
{
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
        EnsureDeadband(attributePath);
        break;
    case UnoccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
        clearActivePreset = supportsPresets && !occupied;
        EnsureDeadband(attributePath);
        break;
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

Status MatterThermostatClusterServerPreAttributeChangedCallback(const app::ConcreteAttributePath & attributePath,
                                                                EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    EndpointId endpoint = attributePath.mEndpointId;
    int16_t requested;

    // Limits will be needed for all checks
    // so we just get them all now
    // TODO: use GetSetpointLimits to fetch this information
    int16_t AbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit;
    int16_t MinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit;
    int16_t AbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit;
    int16_t MinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit;
    int8_t DeadBand      = 0;
    int16_t DeadBandTemp = 0;
    int16_t OccupiedCoolingSetpoint;
    int16_t OccupiedHeatingSetpoint;
    int16_t UnoccupiedCoolingSetpoint;
    int16_t UnoccupiedHeatingSetpoint;
    uint32_t OurFeatureMap;
    bool AutoSupported      = false;
    bool HeatSupported      = false;
    bool CoolSupported      = false;
    bool OccupancySupported = false;

    if (FeatureMap::Get(endpoint, &OurFeatureMap) != Status::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (OurFeatureMap & 1 << 2)
        OccupancySupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(endpoint, &DeadBand) != Status::Success)
        {
            DeadBand = kDefaultDeadBand;
        }
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit) != Status::Success)
        AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit) != Status::Success)
        AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit) != Status::Success)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit) != Status::Success)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit) != Status::Success)
        AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit) != Status::Success)
        AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit) != Status::Success)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit) != Status::Success)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (CoolSupported)
        if (OccupiedCoolingSetpoint::Get(endpoint, &OccupiedCoolingSetpoint) != Status::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }

    if (HeatSupported)
        if (OccupiedHeatingSetpoint::Get(endpoint, &OccupiedHeatingSetpoint) != Status::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }

    if (CoolSupported && OccupancySupported)
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &UnoccupiedCoolingSetpoint) != Status::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return Status::Failure;
        }

    if (HeatSupported && OccupancySupported)
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &UnoccupiedHeatingSetpoint) != Status::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return Status::Failure;
        }

    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return Status::InvalidValue;
        return CheckCoolingSetpointDeadband(AutoSupported, requested, std::min(MaxCoolSetpointLimit, AbsMaxCoolSetpointLimit),
                                            DeadBandTemp);
    }

    case OccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return Status::InvalidValue;
        return CheckHeatingSetpointDeadband(AutoSupported, requested, std::max(MinHeatSetpointLimit, AbsMinHeatSetpointLimit),
                                            DeadBandTemp);
    }

    case UnoccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(HeatSupported && OccupancySupported))
            return Status::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return Status::InvalidValue;
        return CheckCoolingSetpointDeadband(AutoSupported, requested, std::min(MaxCoolSetpointLimit, AbsMaxCoolSetpointLimit),
                                            DeadBandTemp);
    }
    case UnoccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(CoolSupported && OccupancySupported))
            return Status::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return Status::InvalidValue;
        return CheckHeatingSetpointDeadband(AutoSupported, requested, std::max(MinHeatSetpointLimit, AbsMinHeatSetpointLimit),
                                            DeadBandTemp);
    }

    case MinHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested > MaxHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return Status::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MinCoolSetpointLimit - DeadBandTemp)
                return Status::InvalidValue;
        }
        return Status::Success;
    }
    case MaxHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return Status::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MaxCoolSetpointLimit - DeadBandTemp)
                return Status::InvalidValue;
        }
        return Status::Success;
    }
    case MinCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested > MaxCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return Status::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MinHeatSetpointLimit + DeadBandTemp)
                return Status::InvalidValue;
        }
        return Status::Success;
    }
    case MaxCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return Status::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return Status::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MaxHeatSetpointLimit + DeadBandTemp)
                return Status::InvalidValue;
        }
        return Status::Success;
    }
    case MinSetpointDeadBand::Id: {
        requested = *value;
        if (!AutoSupported)
            return Status::UnsupportedAttribute;
        if (requested < 0 || requested > 127)
            return Status::InvalidValue;
        return Status::Success;
    }

    case ControlSequenceOfOperation::Id: {
        uint8_t requestedCSO;
        requestedCSO = *value;
        if (requestedCSO > to_underlying(ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat))
            return Status::InvalidValue;
        return Status::Success;
    }

    case SystemMode::Id: {
        ControlSequenceOfOperationEnum ControlSequenceOfOperation;
        Status status = ControlSequenceOfOperation::Get(endpoint, &ControlSequenceOfOperation);
        if (status != Status::Success)
        {
            return Status::InvalidValue;
        }
        auto RequestedSystemMode = static_cast<SystemModeEnum>(*value);
        if (EnsureKnownEnumValue(ControlSequenceOfOperation) == ControlSequenceOfOperationEnum::kUnknownEnumValue ||
            EnsureKnownEnumValue(RequestedSystemMode) == SystemModeEnum::kUnknownEnumValue)
        {
            return Status::InvalidValue;
        }

        switch (ControlSequenceOfOperation)
        {
        case ControlSequenceOfOperationEnum::kCoolingOnly:
        case ControlSequenceOfOperationEnum::kCoolingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kHeat || RequestedSystemMode == SystemModeEnum::kEmergencyHeat)
                return Status::InvalidValue;
            else
                return Status::Success;

        case ControlSequenceOfOperationEnum::kHeatingOnly:
        case ControlSequenceOfOperationEnum::kHeatingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kCool || RequestedSystemMode == SystemModeEnum::kPrecooling)
                return Status::InvalidValue;
            else
                return Status::Success;
        default:
            return Status::Success;
        }
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
