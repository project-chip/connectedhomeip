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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

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

static_assert(kThermostatEndpointCount <= kEmberInvalidEndpointIndex, "Thermostat Delegate table size error");

Delegate * gDelegateTable[kThermostatEndpointCount] = { nullptr };

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatAttrAccess gThermostatAttrAccess;

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
    Status status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit);
    if (status != Status::Success)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit);
    if (status != Status::Success)
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
    Status status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit);
    if (status != Status::Success)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit);
    if (status != Status::Success)
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

struct SetpointLimits
{
    bool autoSupported      = false;
    bool heatSupported      = false;
    bool coolSupported      = false;
    bool occupancySupported = false;

    int16_t absMinHeatSetpointLimit;
    int16_t absMaxHeatSetpointLimit;
    int16_t minHeatSetpointLimit;
    int16_t maxHeatSetpointLimit;
    int16_t absMinCoolSetpointLimit;
    int16_t absMaxCoolSetpointLimit;
    int16_t minCoolSetpointLimit;
    int16_t maxCoolSetpointLimit;
    int16_t deadBand = 0;
    int16_t occupiedCoolingSetpoint;
    int16_t occupiedHeatingSetpoint;
    int16_t unoccupiedCoolingSetpoint;
    int16_t unoccupiedHeatingSetpoint;
};

/**
 * @brief Reads all the attributes for enforcing setpoint limits
 *
 * @param endpoint The endpoint for the server whose limits are being enforced
 * @param setpointLimits The SetpointLimits to populate
 * @return Success if the limits were read completely, otherwise an error code
 */
Status GetSetpointLimits(EndpointId endpoint, SetpointLimits & setpointLimits)
{
    uint32_t flags;
    if (FeatureMap::Get(endpoint, &flags) != Status::Success)
    {
        ChipLogError(Zcl, "GetSetpointLimits: could not get feature flags");
        flags = FEATURE_MAP_DEFAULT;
    }

    auto featureMap                   = BitMask<Feature, uint32_t>(flags);
    setpointLimits.autoSupported      = featureMap.Has(Feature::kAutoMode);
    setpointLimits.heatSupported      = featureMap.Has(Feature::kHeating);
    setpointLimits.coolSupported      = featureMap.Has(Feature::kCooling);
    setpointLimits.occupancySupported = featureMap.Has(Feature::kOccupancy);

    if (setpointLimits.autoSupported)
    {
        int8_t deadBand;
        if (MinSetpointDeadBand::Get(endpoint, &deadBand) != Status::Success)
        {
            deadBand = kDefaultDeadBand;
        }
        setpointLimits.deadBand = static_cast<int16_t>(deadBand * 10);
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &setpointLimits.absMinCoolSetpointLimit) != Status::Success)
        setpointLimits.absMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &setpointLimits.absMaxCoolSetpointLimit) != Status::Success)
        setpointLimits.absMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (MinCoolSetpointLimit::Get(endpoint, &setpointLimits.minCoolSetpointLimit) != Status::Success)
        setpointLimits.minCoolSetpointLimit = setpointLimits.absMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit::Get(endpoint, &setpointLimits.maxCoolSetpointLimit) != Status::Success)
        setpointLimits.maxCoolSetpointLimit = setpointLimits.absMaxCoolSetpointLimit;

    if (AbsMinHeatSetpointLimit::Get(endpoint, &setpointLimits.absMinHeatSetpointLimit) != Status::Success)
        setpointLimits.absMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &setpointLimits.absMaxHeatSetpointLimit) != Status::Success)
        setpointLimits.absMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (MinHeatSetpointLimit::Get(endpoint, &setpointLimits.minHeatSetpointLimit) != Status::Success)
        setpointLimits.minHeatSetpointLimit = setpointLimits.absMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit::Get(endpoint, &setpointLimits.maxHeatSetpointLimit) != Status::Success)
        setpointLimits.maxHeatSetpointLimit = setpointLimits.absMaxHeatSetpointLimit;

    if (setpointLimits.coolSupported)
    {
        if (OccupiedCoolingSetpoint::Get(endpoint, &setpointLimits.occupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.heatSupported)
    {
        if (OccupiedHeatingSetpoint::Get(endpoint, &setpointLimits.occupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.coolSupported && setpointLimits.occupancySupported)
    {
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &setpointLimits.unoccupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.heatSupported && setpointLimits.occupancySupported)
    {
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &setpointLimits.unoccupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return Status::Failure;
        }
    }
    return Status::Success;
}

/**
 * @brief Checks to see if it's possible to adjust the heating setpoint to preserve a given deadband
 * if the cooling setpoint is changed
 *
 * @param autoSupported Whether or not the thermostat supports Auto mode
 * @param newCoolingSetpoing The desired cooling setpoint
 * @param minHeatingSetpoint The minimum allowed heating setpoint
 * @param deadband The deadband to preserve
 * @return Success if the deadband can be preserved, InvalidValue if it cannot
 */
Status CheckHeatingSetpointDeadband(bool autoSupported, int16_t newCoolingSetpoint, int16_t minHeatingSetpoint, int16_t deadband)
{
    if (!autoSupported)
    {
        return Status::Success;
    }
    int16_t maxValidHeatingSetpoint = static_cast<int16_t>(newCoolingSetpoint - deadband);
    if (maxValidHeatingSetpoint < minHeatingSetpoint)
    {
        // If we need to adjust the heating setpoint to preserve the deadband, it will go below the min heat setpoint
        return Status::InvalidValue;
    }
    // It's possible to adjust the heating setpoint, if needed
    return Status::Success;
}

/**
 * @brief Checks to see if it's possible to adjust the cooling setpoint to preserve a given deadband
 * if the heating setpoint is changed
 *
 * @param autoSupported Whether or not the thermostat supports Auto mode
 * @param newHeatingSetpoint The desired heating setpoint
 * @param maxCoolingSetpoint The maximum allowed cooling setpoint
 * @param deadband The deadband to preserve
 * @return Success if the deadband can be preserved, InvalidValue if it cannot
 */
Status CheckCoolingSetpointDeadband(bool autoSupported, int16_t newHeatingSetpoint, int16_t maxCoolingSetpoint, int16_t deadband)
{
    if (!autoSupported)
    {
        return Status::Success;
    }
    int16_t minValidCoolingSetpoint = static_cast<int16_t>(newHeatingSetpoint + deadband);
    if (minValidCoolingSetpoint > maxCoolingSetpoint)
    {
        // If we need to adjust the cooling setpoint to preserve the deadband, it will go above the max cool setpoint
        return Status::InvalidValue;
    }
    // It's possible to adjust the cooling setpoint, if needed
    return Status::Success;
}

typedef Status (*SetpointSetter)(EndpointId endpoint, int16_t value);

/**
 * @brief Attempts to ensure that a change to the heating setpoint maintains the deadband with the cooling setpoint
 * by adjusting the cooling setpoint
 *
 * @param endpoint The endpoint on which the heating setpoint has been changed
 * @param currentCoolingSetpoint The current cooling setpoint
 * @param newHeatingSetpoint The newly adjusted heating setpoint
 * @param maxCoolingSetpoint The maximum allowed cooling setpoint
 * @param deadband The deadband to preserve
 * @param setter A function for setting the cooling setpoint
 */
void EnsureCoolingSetpointDeadband(EndpointId endpoint, int16_t currentCoolingSetpoint, int16_t newHeatingSetpoint,
                                   int16_t maxCoolingSetpoint, int16_t deadband, SetpointSetter setter)
{
    int16_t minValidCoolingSetpoint = static_cast<int16_t>(newHeatingSetpoint + deadband);
    if (currentCoolingSetpoint >= minValidCoolingSetpoint)
    {
        // The current cooling setpoint doesn't violate the deadband
        return;
    }
    if (minValidCoolingSetpoint > maxCoolingSetpoint)
    {
        // Adjusting the cool setpoint to preserve the deadband would violate the max cool setpoint
        // This should have been caught in CheckCoolingSetpointDeadband, so log and exit
        ChipLogError(Zcl, "Failed ensuring cooling setpoint deadband");
        return;
    }
    // Adjust the cool setpoint to preserve deadband
    auto status = setter(endpoint, minValidCoolingSetpoint);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Error: EnsureCoolingSetpointDeadband failed!");
    }
}

/**
 * @brief Attempts to ensure that a change to the cooling setpoint maintains the deadband with the heating setpoint
 * by adjusting the heating setpoint
 *
 * @param endpoint The endpoint on which the cooling setpoint has been changed
 * @param currentHeatingSetpointThe current heating setpoint
 * @param newCoolingSetpoint The newly adjusted cooling setpoint
 * @param minHeatingSetpoint The minimum allowed cooling setpoint
 * @param deadband The deadband to preserve
 * @param setter A function for setting the heating setpoint
 */
void EnsureHeatingSetpointDeadband(EndpointId endpoint, int16_t currentHeatingSetpoint, int16_t newCoolingSetpoint,
                                   int16_t minHeatingSetpoint, int16_t deadband, SetpointSetter setter)
{
    int16_t maxValidHeatingSetpoint = static_cast<int16_t>(newCoolingSetpoint - deadband);
    if (currentHeatingSetpoint <= maxValidHeatingSetpoint)
    {
        // The current heating setpoint doesn't violate the deadband
        return;
    }
    if (maxValidHeatingSetpoint < minHeatingSetpoint)
    {
        // Adjusting the heating setpoint to preserve the deadband would violate the min heating setpoint
        // This should have been caught in CheckHeatingSetpointDeadband, so log and exit
        ChipLogError(Zcl, "Failed ensuring heating setpoint deadband");
        return;
    }
    // Adjust the heating setpoint to preserve deadband
    auto status = setter(endpoint, maxValidHeatingSetpoint);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Error: EnsureHeatingSetpointDeadband failed!");
    }
}

/**
 * @brief For thermostats that support auto, shift setpoints to maintain the current deadband
 * Note: this assumes that the shift is possible; setpoint changes which prevent the deadband
 * from being maintained due to the min/max limits for setpoints should be rejected by
 * MatterThermostatClusterServerPreAttributeChangedCallback
 *
 * @param attributePath
 */
void EnsureDeadband(const ConcreteAttributePath & attributePath)
{
    auto endpoint = attributePath.mEndpointId;
    SetpointLimits setpointLimits;
    auto status = GetSetpointLimits(endpoint, setpointLimits);
    if (status != Status::Success)
    {
        return;
    }
    if (!setpointLimits.autoSupported)
    {
        return;
    }
    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        EnsureCoolingSetpointDeadband(endpoint, setpointLimits.occupiedCoolingSetpoint, setpointLimits.occupiedHeatingSetpoint,
                                      setpointLimits.maxCoolSetpointLimit, setpointLimits.deadBand, OccupiedCoolingSetpoint::Set);
        break;
    case OccupiedCoolingSetpoint::Id:
        EnsureHeatingSetpointDeadband(endpoint, setpointLimits.occupiedHeatingSetpoint, setpointLimits.occupiedCoolingSetpoint,
                                      setpointLimits.minHeatSetpointLimit, setpointLimits.deadBand, OccupiedHeatingSetpoint::Set);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        EnsureCoolingSetpointDeadband(endpoint, setpointLimits.unoccupiedCoolingSetpoint, setpointLimits.unoccupiedHeatingSetpoint,
                                      setpointLimits.maxCoolSetpointLimit, setpointLimits.deadBand, UnoccupiedCoolingSetpoint::Set);
        break;
    case UnoccupiedCoolingSetpoint::Id:
        EnsureHeatingSetpointDeadband(endpoint, setpointLimits.unoccupiedHeatingSetpoint, setpointLimits.unoccupiedCoolingSetpoint,
                                      setpointLimits.minHeatSetpointLimit, setpointLimits.deadBand, UnoccupiedHeatingSetpoint::Set);
        break;
    }
}

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
        if (requested < 0 || requested > 25)
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

bool emberAfThermostatClusterSetpointRaiseLowerCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    auto & mode   = commandData.mode;
    auto & amount = commandData.amount;

    EndpointId aEndpointId = commandPath.mEndpointId;

    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    Status status                     = Status::Failure;
    Status WriteCoolingSetpointStatus = Status::Failure;
    Status WriteHeatingSetpointStatus = Status::Failure;
    int16_t DeadBandTemp              = 0;
    int8_t DeadBand                   = 0;
    uint32_t OurFeatureMap;
    bool AutoSupported = false;
    bool HeatSupported = false;
    bool CoolSupported = false;

    if (FeatureMap::Get(aEndpointId, &OurFeatureMap) != Status::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(aEndpointId, &DeadBand) != Status::Success)
            DeadBand = kDefaultDeadBand;
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (HeatSupported && CoolSupported)
        {
            int16_t DesiredCoolingSetpoint, CoolLimit, DesiredHeatingSetpoint, HeatLimit;
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                DesiredCoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolLimit              = static_cast<int16_t>(DesiredCoolingSetpoint -
                                                 EnforceCoolingSetpointLimits(DesiredCoolingSetpoint, aEndpointId));
                {
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
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
                            if (WriteCoolingSetpointStatus != Status::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, DesiredHeatingSetpoint);
                            if (WriteHeatingSetpointStatus != Status::Success)
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
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                if (WriteCoolingSetpointStatus != Status::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                }
            }
        }

        if (HeatSupported && !CoolSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
            {
                HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                if (WriteHeatingSetpointStatus != Status::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                }
            }
        }

        if ((!HeatSupported || WriteHeatingSetpointStatus == Status::Success) &&
            (!CoolSupported || WriteCoolingSetpointStatus == Status::Success))
            status = Status::Success;
        break;

    case SetpointRaiseLowerModeEnum::kCool:
        if (CoolSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
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
                                if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == Status::Success)
                                {
                                    if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == Status::Success)
                                        status = Status::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust heating setpoint to maintain dead band!");
                                status = Status::InvalidCommand;
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
            status = Status::InvalidCommand;
        break;

    case SetpointRaiseLowerModeEnum::kHeat:
        if (HeatSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
            {
                HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
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
                                if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == Status::Success)
                                {
                                    if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == Status::Success)
                                        status = Status::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust cooling setpoint to maintain dead band!");
                                status = Status::InvalidCommand;
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
            status = Status::InvalidCommand;
        break;

    default:
        status = Status::InvalidCommand;
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
