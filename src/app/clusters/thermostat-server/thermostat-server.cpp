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

#include "thermostat-server.h"
#include "PresetStructWithOwnedMembers.h"
#include "thermostat-server-setpoints.h"

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
#include <lib/core/CHIPEncoding.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;

using imcode = Protocols::InteractionModel::Status;

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_DEFAULT FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

namespace {

ThermostatAttrAccess gThermostatAttrAccess;

static constexpr size_t kThermostatEndpointCount =
    MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kThermostatEndpointCount <= kEmberInvalidEndpointIndex, "Thermostat Delegate table size error");

Delegate * gDelegateTable[kThermostatEndpointCount]                     = { nullptr };
AtomicWriteManager * gAtomicWriteManagerTable[kThermostatEndpointCount] = { nullptr };

AtomicWriteManager * GetAtomicWriteManager(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= ArraySize(gAtomicWriteManagerTable) ? nullptr : gAtomicWriteManagerTable[ep]);
}

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found, add the delegate in the delegate table
    if (ep < ArraySize(gDelegateTable))
    {
        gDelegateTable[ep] = delegate;
    }
}

void SetDefaultAtomicWriteManager(EndpointId endpoint, AtomicWriteManager * atomicWriteManager)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found, add the delegate in the delegate table
    if (ep < ArraySize(gAtomicWriteManagerTable))
    {
        gAtomicWriteManagerTable[ep] = atomicWriteManager;

        atomicWriteManager->SetDelegate(&gThermostatAttrAccess);
    }
}

Delegate * ThermostatAttrAccess::GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= ArraySize(gDelegateTable) ? nullptr : gDelegateTable[ep]);
}

CHIP_ERROR ThermostatAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
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
            imcode status = RemoteSensing::Get(aPath.mEndpointId, &valueRemoteSensing);
            if (status != imcode::Success)
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
            for (uint8_t i = 0; true; ++i)
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

        auto awm = GetAtomicWriteManager(aPath.mEndpointId);
        VerifyOrReturnError(awm != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Atomic Write Manager is null"));

        auto & subjectDescriptor = aEncoder.GetSubjectDescriptor();
        if (awm->InWrite(aPath.mAttributeId, subjectDescriptor, aPath.mEndpointId))
        {
            return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
                for (uint8_t i = 0; true; ++i)
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
            for (uint8_t i = 0; true; ++i)
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
        MutableByteSpan activePresetHandle(buffer);

        CHIP_ERROR err = delegate->GetActivePresetHandle(activePresetHandle);
        ReturnErrorOnFailure(err);

        if (activePresetHandle.empty())
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(activePresetHandle));
        }
    }
    break;
    case ScheduleTypes::Id: {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case Schedules::Id: {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
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

        auto awm = GetAtomicWriteManager(endpoint);
        VerifyOrReturnError(awm != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Atomic Write Manager is null"));

        // Presets are not editable, return INVALID_IN_STATE.
        VerifyOrReturnError(awm->InWrite(aPath.mAttributeId, endpoint), CHIP_IM_GLOBAL_STATUS(InvalidInState),
                            ChipLogError(Zcl, "Presets are not editable"));

        // OK, we're in an atomic write, make sure the requesting node is the same one that started the atomic write,
        // otherwise return BUSY.
        if (!awm->InWrite(aPath.mAttributeId, subjectDescriptor, endpoint))
        {
            ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
            return CHIP_IM_GLOBAL_STATUS(Busy);
        }

        auto delegate = GetDelegate(endpoint);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

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
    auto awm = GetAtomicWriteManager(endpoint);
    VerifyOrReturnError(awm != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Atomic Write Manager is null"));
    if (awm->InWrite(std::nullopt, subjectDescriptor, endpoint))
    {
        ChipLogError(Zcl, "Can not write to non-atomic attributes during atomic write");
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
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
            imcode status = RemoteSensing::Set(aPath.mEndpointId, valueRemoteSensing);
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
    for (size_t i = 0; i < ArraySize(gAtomicWriteManagerTable); ++i)
    {
        auto awm = gAtomicWriteManagerTable[i];
        if (awm != nullptr)
        {
            awm->ResetWrite(fabricIndex);
        }
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

// IMPORTANT NOTE:
// No Side effects are permitted in emberAfThermostatClusterServerPreAttributeChangedCallback
// If a setpoint changes is required as a result of setpoint limit change
// it does not happen here.  It is the responsibility of the device to adjust the setpoint(s)
// as required in emberAfThermostatClusterServerPostAttributeChangedCallback
// limit change validation assures that there is at least 1 setpoint that will be valid
Protocols::InteractionModel::Status
MatterThermostatClusterServerPreAttributeChangedCallback(const app::ConcreteAttributePath & attributePath,
                                                         EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    EndpointId endpoint = attributePath.mEndpointId;
    int16_t requested;

    // Limits will be needed for all checks
    // so we just get them all now
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

    if (FeatureMap::Get(endpoint, &OurFeatureMap) != imcode::Success)
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
        if (MinSetpointDeadBand::Get(endpoint, &DeadBand) != imcode::Success)
        {
            DeadBand = kDefaultDeadBand;
        }
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit) != imcode::Success)
        AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit) != imcode::Success)
        AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit) != imcode::Success)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit) != imcode::Success)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit) != imcode::Success)
        AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit) != imcode::Success)
        AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit) != imcode::Success)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit) != imcode::Success)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (CoolSupported)
        if (OccupiedCoolingSetpoint::Get(endpoint, &OccupiedCoolingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return imcode::Failure;
        }

    if (HeatSupported)
        if (OccupiedHeatingSetpoint::Get(endpoint, &OccupiedHeatingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return imcode::Failure;
        }

    if (CoolSupported && OccupancySupported)
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &UnoccupiedCoolingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return imcode::Failure;
        }

    if (HeatSupported && OccupancySupported)
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &UnoccupiedHeatingSetpoint) != imcode::Success)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return imcode::Failure;
        }

    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > OccupiedCoolingSetpoint - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case OccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < OccupiedHeatingSetpoint + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case UnoccupiedHeatingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(HeatSupported && OccupancySupported))
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > UnoccupiedCoolingSetpoint - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case UnoccupiedCoolingSetpoint::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(CoolSupported && OccupancySupported))
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < UnoccupiedHeatingSetpoint + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }

    case MinHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested > MaxHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MinCoolSetpointLimit - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MaxHeatSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested > MaxCoolSetpointLimit - DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MinCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested > MaxCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MinHeatSetpointLimit + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MaxCoolSetpointLimit::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return imcode::UnsupportedAttribute;
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return imcode::InvalidValue;
        if (AutoSupported)
        {
            if (requested < MaxHeatSetpointLimit + DeadBandTemp)
                return imcode::InvalidValue;
        }
        return imcode::Success;
    }
    case MinSetpointDeadBand::Id: {
        requested = *value;
        if (!AutoSupported)
            return imcode::UnsupportedAttribute;
        if (requested < 0 || requested > 25)
            return imcode::InvalidValue;
        return imcode::Success;
    }

    case ControlSequenceOfOperation::Id: {
        uint8_t requestedCSO;
        requestedCSO = *value;
        if (requestedCSO > to_underlying(ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat))
            return imcode::InvalidValue;
        return imcode::Success;
    }

    case SystemMode::Id: {
        ControlSequenceOfOperationEnum ControlSequenceOfOperation;
        imcode status = ControlSequenceOfOperation::Get(endpoint, &ControlSequenceOfOperation);
        if (status != imcode::Success)
        {
            return imcode::InvalidValue;
        }
        auto RequestedSystemMode = static_cast<SystemModeEnum>(*value);
        if (ControlSequenceOfOperation > ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat ||
            RequestedSystemMode > SystemModeEnum::kFanOnly)
        {
            return imcode::InvalidValue;
        }

        switch (ControlSequenceOfOperation)
        {
        case ControlSequenceOfOperationEnum::kCoolingOnly:
        case ControlSequenceOfOperationEnum::kCoolingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kHeat || RequestedSystemMode == SystemModeEnum::kEmergencyHeat)
                return imcode::InvalidValue;
            else
                return imcode::Success;

        case ControlSequenceOfOperationEnum::kHeatingOnly:
        case ControlSequenceOfOperationEnum::kHeatingWithReheat:
            if (RequestedSystemMode == SystemModeEnum::kCool || RequestedSystemMode == SystemModeEnum::kPrecooling)
                return imcode::InvalidValue;
            else
                return imcode::Success;
        default:
            return imcode::Success;
        }
    }
    default:
        return imcode::Success;
    }
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

bool validAtomicAttributes(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                           const Commands::AtomicRequest::DecodableType & commandData, bool requireBoth)
{
    auto attributeIdsIter = commandData.attributeRequests.begin();
    bool requestedPresets = false, requestedSchedules = false;
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();

        switch (attributeId)
        {
        case Presets::Id:
            if (requestedPresets) // Double-requesting an attribute is invalid
            {
                return false;
            }
            requestedPresets = true;
            break;
        case Schedules::Id:
            if (requestedSchedules) // Double-requesting an attribute is invalid
            {
                return false;
            }
            requestedSchedules = true;
            break;
        default:
            return false;
        }
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return false;
    }
    if (requireBoth)
    {
        return (requestedPresets && requestedSchedules);
    }
    // If the atomic request doesn't contain at least one of these attributes, it's invalid
    return (requestedPresets || requestedSchedules);
}

bool emberAfThermostatClusterSetActivePresetRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, gThermostatAttrAccess.SetActivePreset(commandPath.mEndpointId, commandData.presetHandle));
    return true;
}

bool emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData)
{

    // If we've gotten this far, then the client has manage permission to call AtomicRequest, which is also the
    // privilege necessary to write to the atomic attributes, so no need to check

    auto * awm = GetAtomicWriteManager(commandPath.mEndpointId);
    if (awm == nullptr)
    {
        commandObj->AddStatus(commandPath, imcode::InvalidInState);
        return true;
    }
    auto & requestType = commandData.requestType;
    switch (requestType)
    {
    case Globals::AtomicRequestTypeEnum::kBeginWrite:
        return awm->BeginWrite(commandObj, commandPath, commandData);
    case Globals::AtomicRequestTypeEnum::kCommitWrite:
        return awm->CommitWrite(commandObj, commandPath, commandData);
    case Globals::AtomicRequestTypeEnum::kRollbackWrite:
        return awm->RollbackWrite(commandObj, commandPath, commandData);
    case Globals::AtomicRequestTypeEnum::kUnknownEnumValue:
        commandObj->AddStatus(commandPath, imcode::InvalidCommand);
        return true;
    }

    return false;
}

bool emberAfThermostatClusterSetpointRaiseLowerCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    auto & mode   = commandData.mode;
    auto & amount = commandData.amount;

    EndpointId aEndpointId = commandPath.mEndpointId;

    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    imcode status                     = imcode::Failure;
    imcode WriteCoolingSetpointStatus = imcode::Failure;
    imcode WriteHeatingSetpointStatus = imcode::Failure;
    int16_t DeadBandTemp              = 0;
    int8_t DeadBand                   = 0;
    uint32_t OurFeatureMap;
    bool AutoSupported = false;
    bool HeatSupported = false;
    bool CoolSupported = false;

    if (FeatureMap::Get(aEndpointId, &OurFeatureMap) != imcode::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(aEndpointId, &DeadBand) != imcode::Success)
            DeadBand = kDefaultDeadBand;
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (HeatSupported && CoolSupported)
        {
            int16_t DesiredCoolingSetpoint, CoolLimit, DesiredHeatingSetpoint, HeatLimit;
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                DesiredCoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolLimit              = static_cast<int16_t>(DesiredCoolingSetpoint -
                                                              EnforceCoolingSetpointLimits(DesiredCoolingSetpoint, aEndpointId));
                {
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
                    {
                        DesiredHeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                        HeatLimit              = static_cast<int16_t>(DesiredHeatingSetpoint -
                                                                      EnforceHeatingSetpointLimits(DesiredHeatingSetpoint, aEndpointId));
                        {
                            if (CoolLimit != 0 || HeatLimit != 0)
                            {
                                if (abs(CoolLimit) <= abs(HeatLimit))
                                {
                                    // We are limited by the Heating Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - HeatLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - HeatLimit);
                                }
                                else
                                {
                                    // We are limited by Cooling Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - CoolLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - CoolLimit);
                                }
                            }
                            WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, DesiredCoolingSetpoint);
                            if (WriteCoolingSetpointStatus != imcode::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, DesiredHeatingSetpoint);
                            if (WriteHeatingSetpointStatus != imcode::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                        }
                    }
                }
            }
        }

        if (CoolSupported && !HeatSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                if (WriteCoolingSetpointStatus != imcode::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                }
            }
        }

        if (HeatSupported && !CoolSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
            {
                HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                if (WriteHeatingSetpointStatus != imcode::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                }
            }
        }

        if ((!HeatSupported || WriteHeatingSetpointStatus == imcode::Success) &&
            (!CoolSupported || WriteCoolingSetpointStatus == imcode::Success))
            status = imcode::Success;
        break;

    case SetpointRaiseLowerModeEnum::kCool:
        if (CoolSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
            {
                CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            HeatingSetpoint = static_cast<int16_t>(CoolingSetpoint - DeadBandTemp);
                            if (HeatingSetpoint == EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == imcode::Success)
                                {
                                    if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == imcode::Success)
                                        status = imcode::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust heating setpoint to maintain dead band!");
                                status = imcode::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
                }
                else
                {
                    status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
        }
        else
            status = imcode::InvalidCommand;
        break;

    case SetpointRaiseLowerModeEnum::kHeat:
        if (HeatSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == imcode::Success)
            {
                HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == imcode::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            CoolingSetpoint = static_cast<int16_t>(HeatingSetpoint + DeadBandTemp);
                            if (CoolingSetpoint == EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == imcode::Success)
                                {
                                    if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == imcode::Success)
                                        status = imcode::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust cooling setpoint to maintain dead band!");
                                status = imcode::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
                }
                else
                {
                    status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
        }
        else
            status = imcode::InvalidCommand;
        break;

    default:
        status = imcode::InvalidCommand;
        break;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterThermostatPluginServerInitCallback()
{
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gThermostatAttrAccess);
    AttributeAccessInterfaceRegistry::Instance().Register(&gThermostatAttrAccess);
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", endpoint);
    auto awm = GetAtomicWriteManager(endpoint);
    if (awm != nullptr)
    {
        awm->ResetWrite(endpoint);
    }
}
