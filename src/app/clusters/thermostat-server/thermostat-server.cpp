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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPEncoding.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;

using imcode = Protocols::InteractionModel::Status;

constexpr int16_t kDefaultAbsMinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultAbsMaxHeatSetpointLimit = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultMinHeatSetpointLimit    = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultMaxHeatSetpointLimit    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultMinCoolSetpointLimit    = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultMaxCoolSetpointLimit    = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultHeatingSetpoint         = 2000;
constexpr int16_t kDefaultCoolingSetpoint         = 2600;
constexpr int8_t kDefaultDeadBand                 = 25; // 2.5C is the default

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

// ----------------------------------------------
// - Schedules and Presets Manager object       -
// ----------------------------------------------

// Object Tracking
static ThermostatMatterScheduleManager * gsMatterScheduleEditor = nullptr;

void 
ThermostatMatterScheduleManager::SetActiveInstance(ThermostatMatterScheduleManager * inManager)
{
    if (gsMatterScheduleEditor != nullptr)
    {
        if (gsMatterScheduleEditor->IsEditing())
        {
            ChipLogError(Zcl, "Warning: Active ThermostatMatterScheduleManager was editing when swapped out");
            gsMatterScheduleEditor->RollbackEdits();
        }
    }
    gsMatterScheduleEditor = inManager;
}

ThermostatMatterScheduleManager * ThermostatMatterScheduleManager::GetActiveInstance()
{
    return gsMatterScheduleEditor;
}

bool 
ThermostatMatterScheduleManager::areDescriptorsEqualAndValid(const Access::SubjectDescriptor &desc1, const Access::SubjectDescriptor &desc2)
{
    if (desc1.fabricIndex == kUndefinedFabricIndex || desc1.subject == kUndefinedNodeId)
        return false;
    if (desc2.fabricIndex == kUndefinedFabricIndex || desc2.subject == kUndefinedNodeId)
        return false;
    return ((desc1.fabricIndex == desc2.fabricIndex) && (desc1.subject == desc2.subject));
}

namespace {

class ThermostatAttrAccess : public AttributeAccessInterface
{
public:
    ThermostatAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Thermostat::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};

ThermostatAttrAccess gThermostatAttrAccess;

CHIP_ERROR ThermostatAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Thermostat::Id);

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);
    const bool presetsSupported           = ourFeatureMap & to_underlying(Feature::kPresets);
    const bool enhancedSchedulesSupported = ourFeatureMap & to_underlying(Feature::kMatterScheduleConfiguration);

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
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature); // clear bit 1 (LocalTemperature RemoteSensing bit)
            return aEncoder.Encode(valueRemoteSensing);
        }
        break;
    case PresetTypes::Id:
        if (presetsSupported)
        {
            ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
            if (manager == nullptr)
                return CHIP_ERROR_NOT_IMPLEMENTED;

            return aEncoder.EncodeList([manager, aPath](const auto & encoder) -> CHIP_ERROR {
                PresetTypeStruct::Type presetType;
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                while ((err = manager->GetPresetTypeAtIndex(aPath.mEndpointId, index, presetType)) == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(encoder.Encode(presetType));
                    index++;
                }
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        break;
    case Presets::Id:
        if (presetsSupported)
        {
            ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
            if (manager == nullptr)
                return CHIP_ERROR_NOT_IMPLEMENTED;

            return aEncoder.EncodeList([manager, aPath](const auto & encoder) -> CHIP_ERROR {
                PresetStruct::Type preset;
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                while ((err = manager->GetPresetAtIndex(aPath.mEndpointId, index, preset)) == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(encoder.Encode(preset));
                    index++;
                }
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        break;
    case ScheduleTypes::Id:
        if (enhancedSchedulesSupported)
        {
            ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
            if (manager == nullptr)
                return CHIP_ERROR_NOT_IMPLEMENTED;

            return aEncoder.EncodeList([manager, aPath](const auto & encoder) -> CHIP_ERROR {
                ScheduleTypeStruct::Type scheduleType;
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                while ((err = manager->GetScheduleTypeAtIndex(aPath.mEndpointId, index, scheduleType)) == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(encoder.Encode(scheduleType));
                    index++;
                }
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        break;
    case Schedules::Id:
        if (enhancedSchedulesSupported)
        {
            ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
            if (manager == nullptr)
                return CHIP_ERROR_NOT_IMPLEMENTED;

            return aEncoder.EncodeList([manager, aPath](const auto & encoder) -> CHIP_ERROR {
                ScheduleStruct::Type schedule;
                size_t index   = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                while ((err = manager->GetScheduleAtIndex(aPath.mEndpointId, index, schedule)) == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(encoder.Encode(schedule));
                    index++;
                }
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
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

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(aPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);
    const bool presetsSupported           = ourFeatureMap & to_underlying(Feature::kPresets);
    const bool enhancedSchedulesSupported = ourFeatureMap & to_underlying(Feature::kMatterScheduleConfiguration);

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
    case Presets::Id: {
        if (presetsSupported == false)
        {
            StatusIB statusIB(imcode::UnsupportedAttribute);
            return statusIB.ToChipError();
        }

        ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
        if (manager == nullptr)
            return CHIP_ERROR_NOT_IMPLEMENTED;

        bool currentlyEditing = manager->IsEditing();
        if (currentlyEditing == false)
        {
            StatusIB statusIB(imcode::InvalidInState);
            return statusIB.ToChipError();
        }

        if (!(manager->IsActiveSubjectDescriptor(aPath.mEndpointId, aDecoder.GetSubjectDescriptor())))
        {
            StatusIB statusIB(imcode::InvalidInState);
            return statusIB.ToChipError();
        }

        if (!aPath.IsListItemOperation())
        {
            // Replacing the entire list
            DataModel::DecodableList<PresetStruct::DecodableType> list;
            ReturnErrorOnFailure(aDecoder.Decode(list));
            ReturnErrorOnFailure(manager->ClearPresets(aPath.mEndpointId));
            auto iterator = list.begin();
            while (iterator.Next())
            {
                ReturnErrorOnFailure(StatusIB(manager->AppendPreset(aPath.mEndpointId, iterator.GetValue())).ToChipError());
            }
        }
        else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            PresetStruct::DecodableType decodableType;
            ReturnErrorOnFailure(aDecoder.Decode(decodableType));
            ReturnErrorOnFailure(StatusIB(manager->AppendPreset(aPath.mEndpointId, decodableType)).ToChipError());
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case Schedules::Id: {
        if (enhancedSchedulesSupported == false)
        {
            StatusIB statusIB(imcode::UnsupportedAttribute);
            return statusIB.ToChipError();
        }

        ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
        if (manager == nullptr)
            return CHIP_ERROR_NOT_IMPLEMENTED;

        bool currentlyEditing = manager->IsEditing();
        if (currentlyEditing == false)
        {
            StatusIB statusIB(imcode::InvalidInState);
            return statusIB.ToChipError();
        }

        if (!(manager->IsActiveSubjectDescriptor(aPath.mEndpointId, aDecoder.GetSubjectDescriptor())))
        {
            StatusIB statusIB(imcode::InvalidInState);
            return statusIB.ToChipError();
        }

        if (!aPath.IsListItemOperation())
        {
            // Replacing the entire list
            DataModel::DecodableList<ScheduleStruct::DecodableType> list;
            ReturnErrorOnFailure(aDecoder.Decode(list));
            ReturnErrorOnFailure(manager->ClearSchedules(aPath.mEndpointId));
            auto iterator = list.begin();
            while (iterator.Next())
            {
                ReturnErrorOnFailure(StatusIB(manager->AppendSchedule(aPath.mEndpointId, iterator.GetValue())).ToChipError());
            }
        }
        else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            ScheduleStruct::DecodableType decodableType;
            ReturnErrorOnFailure(aDecoder.Decode(decodableType));
            ReturnErrorOnFailure(StatusIB(manager->AppendSchedule(aPath.mEndpointId, decodableType)).ToChipError());
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;
    default: // return CHIP_NO_ERROR and just write to the attribute store in default
        break;
    }

    return CHIP_NO_ERROR;
}

} // anonymous namespace

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

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = kDefaultMinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit = kDefaultMaxHeatSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacturer imposed limits

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinHeatSetpointLimit < AbsMinHeatSetpointLimit)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit > AbsMaxHeatSetpointLimit)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (HeatingSetpoint < MinHeatSetpointLimit)
        HeatingSetpoint = MinHeatSetpointLimit;

    if (HeatingSetpoint > MaxHeatSetpointLimit)
        HeatingSetpoint = MaxHeatSetpointLimit;

    return HeatingSetpoint;
}

int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = kDefaultMinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit = kDefaultMaxCoolSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacture imposed limits
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinCoolSetpointLimit < AbsMinCoolSetpointLimit)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit > AbsMaxCoolSetpointLimit)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (CoolingSetpoint < MinCoolSetpointLimit)
        CoolingSetpoint = MinCoolSetpointLimit;

    if (CoolingSetpoint > MaxCoolSetpointLimit)
        CoolingSetpoint = MaxCoolSetpointLimit;

    return CoolingSetpoint;
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

static void onThermostatEditorTick(chip::System::Layer * systemLayer, void * appState)
{
    ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();

    // Compiler requires some shenanigans to get an Endpoint out of an void pointer.
    uintptr_t uintptr = reinterpret_cast<uintptr_t>(appState);
    chip::EndpointId endpoint = (uintptr & 0xFFFF);

    if (nullptr != manager)
    {
        if (manager->IsEditing(endpoint))
        {
            manager->RollbackEdits(endpoint);
        }
    }
}

// Edit/Cancel/Commit logic

static bool StartEditRequest(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             uint16_t timeout)
{
    imcode status                               = imcode::InvalidCommand;
    bool currentlyEditing                       = false;
    // compiler requires some shenanigans to point an endpoint in as the app state for a timer.
    uintptr_t endpointAsAppstate                = commandPath.mEndpointId;

    ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
    VerifyOrExit(manager != nullptr, status = imcode::InvalidCommand);

    currentlyEditing = manager->IsEditing(commandPath.mEndpointId);

    if (currentlyEditing && !(manager->IsActiveSubjectDescriptor(commandPath.mEndpointId, commandObj->GetSubjectDescriptor())))
    {
        DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Milliseconds16(timeout), onThermostatEditorTick, reinterpret_cast<void *>(endpointAsAppstate));
    }
    else
    {
        VerifyOrExit(currentlyEditing == false, status = imcode::Busy);

        manager->StartEditing(commandPath.mEndpointId, commandObj->GetSubjectDescriptor());
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(timeout), onThermostatEditorTick, reinterpret_cast<void *>(endpointAsAppstate));
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

static bool CancelEditRequest(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath)
{
    imcode status                               = imcode::InvalidCommand;
    bool currentlyEditing                       = false;
    // compiler requires some shenanigans to point an endpoint in as the app state for a timer.
    uintptr_t endpointAsAppstate                = commandPath.mEndpointId;

    ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
    VerifyOrExit(manager != nullptr, status = imcode::InvalidCommand);

    currentlyEditing = manager->IsEditing(commandPath.mEndpointId);

    if (currentlyEditing && !(manager->IsActiveSubjectDescriptor(commandPath.mEndpointId, commandObj->GetSubjectDescriptor())))
    {
        status = imcode::UnsupportedAccess;
    }
    else
    {
        VerifyOrExit(currentlyEditing == true, status = imcode::InvalidInState);

        (void) chip::DeviceLayer::SystemLayer().CancelTimer(onThermostatEditorTick, reinterpret_cast<void *>(endpointAsAppstate));
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

static bool CommitEditRequest(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath)
{
    imcode status = imcode::InvalidCommand;
    bool currentlyEditing = false;
    // compiler requires some shenanigans to point an endpoint in as the app state for a timer.
    uintptr_t endpointAsAppstate                = commandPath.mEndpointId;

    ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
    VerifyOrExit(manager != nullptr, status = imcode::InvalidCommand);

    currentlyEditing = manager->IsEditing(commandPath.mEndpointId);
    if (currentlyEditing && !(manager->IsActiveSubjectDescriptor(commandPath.mEndpointId, commandObj->GetSubjectDescriptor())))
    {
        status = imcode::UnsupportedAccess;
    }
    else
    {
        VerifyOrExit(currentlyEditing == true, status = imcode::InvalidInState);

        status = manager->CommitEdits(commandPath.mEndpointId);
        SuccessOrExit(StatusIB(status).ToChipError());

        (void) chip::DeviceLayer::SystemLayer().CancelTimer(onThermostatEditorTick, reinterpret_cast<void *>(endpointAsAppstate));
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

// Matter Commands

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    imcode status = imcode::InvalidCommand;
    uint32_t ourFeatureMap;
    bool enhancedSchedulesSupported = (FeatureMap::Get(commandPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kMatterScheduleConfiguration)) != 0);

    if (enhancedSchedulesSupported)
    {
        ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
        VerifyOrExit(manager != nullptr, status = imcode::InvalidCommand);

        const chip::ByteSpan & scheduleHandle = commandData.scheduleHandle;
        ScheduleStruct::Type schedule;
        bool found   = false;
        size_t index = 0;

        while (manager->GetScheduleAtIndex(commandPath.mEndpointId, index, schedule) != CHIP_ERROR_NOT_FOUND)
        {
            if ((schedule.scheduleHandle.IsNull() == false) && scheduleHandle.data_equal(schedule.scheduleHandle.Value()))
            {
                status = ActiveScheduleHandle::Set(commandPath.mEndpointId, scheduleHandle);
                SuccessOrExit(StatusIB(status).ToChipError());
                found = true;
                break;
            }
            index++;
        }

        VerifyOrExit(found == true, status = imcode::InvalidCommand);
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfThermostatClusterSetActivePresetRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData)
{
    imcode status = imcode::InvalidCommand;
    uint32_t ourFeatureMap;
    bool presetsSupported = (FeatureMap::Get(commandPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kPresets)) != 0);

    if (presetsSupported)
    {
        ThermostatMatterScheduleManager * manager = ThermostatMatterScheduleManager::GetActiveInstance();
        VerifyOrExit(manager != nullptr, status = imcode::InvalidCommand);

        const chip::ByteSpan & presetHandle = commandData.presetHandle;

        PresetStruct::Type preset;
        bool found   = false;
        size_t index = 0;

        while (manager->GetPresetAtIndex(commandPath.mEndpointId, index, preset) != CHIP_ERROR_NOT_FOUND)
        {
            VerifyOrDie(preset.presetHandle.IsNull() == false);
            if (presetHandle.data_equal(preset.presetHandle.Value()))
            {
                status = ActivePresetHandle::Set(commandPath.mEndpointId, presetHandle);
                SuccessOrExit(StatusIB(status).ToChipError());
                found = true;
                break;
            }
            index++;
        }

        VerifyOrExit(found == true, status = imcode::InvalidCommand);
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfThermostatClusterAtomicRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData)
{
    uint32_t ourFeatureMap;
    bool enhancedSchedulesSupported = (FeatureMap::Get(commandPath.mEndpointId, &ourFeatureMap) == imcode::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kMatterScheduleConfiguration)) != 0);
    bool presetsSupported = ((ourFeatureMap & to_underlying(Feature::kPresets)) != 0);

    if(enhancedSchedulesSupported || presetsSupported)
    {
        switch (commandData.requestType)
        {
            case AtomicRequestTypeEnum::kBeginWrite:
                if (commandData.timeout.HasValue() == true)
                    return StartEditRequest(commandObj, commandPath, commandData.timeout.Value());
                break;
            case AtomicRequestTypeEnum::kCommitWrite: 
                return CommitEditRequest(commandObj, commandPath);
                break;
            case AtomicRequestTypeEnum::kRollbackWrite: 
                return CancelEditRequest(commandObj, commandPath);
                break;
            default:
                commandObj->AddStatus(commandPath, imcode::InvalidInState);
                return true;
                break;
        }
    }

    commandObj->AddStatus(commandPath, imcode::InvalidCommand);
    return true;
}

void MatterThermostatPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gThermostatAttrAccess);
}
