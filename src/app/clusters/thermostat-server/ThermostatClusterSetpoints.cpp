/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ThermostatClusterSetpoints.h"
#include "ThermostatCluster.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

CHIP_ERROR Setpoints::LoadSetpointLimits(EndpointId endpoint, AttributePersistence & persistence)
{
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMinHeatSetpointLimit::Id }, mAbsMinHeatSetpointLimit,
                                      kDefaultAbsMinHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMaxHeatSetpointLimit::Id }, mAbsMaxHeatSetpointLimit,
                                      kDefaultAbsMaxHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMinCoolSetpointLimit::Id }, mAbsMinCoolSetpointLimit,
                                      kDefaultAbsMinCoolSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMaxCoolSetpointLimit::Id }, mAbsMaxCoolSetpointLimit,
                                      kDefaultAbsMaxCoolSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinHeatSetpointLimit::Id }, mMinHeatSetpointLimit,
                                      kDefaultMinHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MaxHeatSetpointLimit::Id }, mMaxHeatSetpointLimit,
                                      kDefaultMaxHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinCoolSetpointLimit::Id }, mMinCoolSetpointLimit,
                                      kDefaultMinCoolSetpointLimit);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MaxCoolSetpointLimit::Id }, mMaxCoolSetpointLimit,
                                      kDefaultMaxCoolSetpointLimit);
    int8_t deadBand;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinSetpointDeadBand::Id }, deadBand, kDefaultDeadBand);
    mDeadBand = static_cast<int16_t>(deadBand * 10);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, OccupiedCoolingSetpoint::Id }, mOccupiedCoolingSetpoint,
                                      kDefaultCoolingSetpoint);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, OccupiedHeatingSetpoint::Id }, mOccupiedHeatingSetpoint,
                                      kDefaultHeatingSetpoint);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, UnoccupiedCoolingSetpoint::Id }, mUnoccupiedCoolingSetpoint,
                                      kDefaultCoolingSetpoint);
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, UnoccupiedHeatingSetpoint::Id }, mUnoccupiedHeatingSetpoint,
                                      kDefaultHeatingSetpoint);

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus Setpoints::ChangeSetpoint(int16_t setpoint, bool isHeatingSetpoint, bool autoSupported, bool occupied,
                                                        bool & deadbandShift, AttributeValueDecoder & decoder,
                                                        AttributePersistence & persistence)
{
    auto minValue = isHeatingSetpoint ? mMinHeatSetpointLimit : mMinCoolSetpointLimit;
    auto maxValue = isHeatingSetpoint ? mMaxHeatSetpointLimit : mMaxCoolSetpointLimit;

    if (setpoint < minValue || setpoint > maxValue)
    {
        return Status::ConstraintError;
    }

    int16_t & existingSetpoint = isHeatingSetpoint ? (occupied ? mOccupiedHeatingSetpoint : mUnoccupiedHeatingSetpoint)
                                                   : (occupied ? mOccupiedCoolingSetpoint : mUnoccupiedCoolingSetpoint);
    auto attribute             = isHeatingSetpoint ? (occupied ? OccupiedHeatingSetpoint::Id : UnoccupiedHeatingSetpoint::Id)
                                                   : (occupied ? OccupiedCoolingSetpoint::Id : UnoccupiedCoolingSetpoint::Id);
    if (autoSupported)
    {
        int16_t & rangeSetpoint = isHeatingSetpoint ? (occupied ? mOccupiedCoolingSetpoint : mUnoccupiedCoolingSetpoint)
                                                    : (occupied ? mOccupiedHeatingSetpoint : mUnoccupiedHeatingSetpoint);
        auto rangeAttribute     = isHeatingSetpoint ? (occupied ? OccupiedCoolingSetpoint::Id : UnoccupiedCoolingSetpoint::Id)
                                                    : (occupied ? OccupiedHeatingSetpoint::Id : UnoccupiedHeatingSetpoint::Id);
        if (isHeatingSetpoint)
        {
            int16_t minValidCoolingSetpoint = static_cast<int16_t>(setpoint + mDeadBand);

            if (minValidCoolingSetpoint > mMaxCoolSetpointLimit)
            {
                // Adjusting the cooling setpoint to preserve the deadband would violate the max cooling setpoint
                return Status::ConstraintError;
            }
            if (minValidCoolingSetpoint > rangeSetpoint)
            {
                auto result = persistence.DecodeAndStoreNativeEndianValue({ mEndpoint, Thermostat::Id, rangeAttribute }, decoder,
                                                                          minValidCoolingSetpoint);
                if (!result.IsSuccess())
                {
                    return result;
                }
                rangeSetpoint = minValidCoolingSetpoint;
                deadbandShift = true;
            }
        }
    }
    auto result = persistence.DecodeAndStoreNativeEndianValue({ mEndpoint, Thermostat::Id, attribute }, decoder, setpoint);
    if (!result.IsSuccess())
    {
        return result;
    }
    existingSetpoint = setpoint;
    return result;
}

DataModel::ActionReturnStatus Setpoints::ChangeSetpointLimit(AttributeId attribute, int16_t limit, bool autoSupported,
                                                             AttributeValueDecoder & decoder, AttributePersistence & persistence)
{
    switch (attribute)
    {
    case MinHeatSetpointLimit::Id:
        if (limit < mAbsMinHeatSetpointLimit || limit > mMaxHeatSetpointLimit || limit > mAbsMaxCoolSetpointLimit)
        {
            return Status::ConstraintError;
        }
        if (autoSupported)
        {
            if (limit > mMinCoolSetpointLimit - mDeadBand)
            {
                return Status::ConstraintError;
            }
        }
        break;
    case MaxHeatSetpointLimit::Id:
        if (limit < mAbsMinHeatSetpointLimit || limit < mMinHeatSetpointLimit || limit > mAbsMaxHeatSetpointLimit)
        {
            return Status::ConstraintError;
        }
        if (autoSupported)
        {
            if (limit > mMaxCoolSetpointLimit - mDeadBand)
            {
                return Status::ConstraintError;
            }
        }
        break;
    case MinCoolSetpointLimit::Id:
        if (limit < mAbsMinCoolSetpointLimit || limit > mMaxCoolSetpointLimit || limit > mAbsMaxCoolSetpointLimit)
        {
            return Status::ConstraintError;
        }
        if (autoSupported)
        {
            if (limit < mMinHeatSetpointLimit + mDeadBand)
            {
                return Status::ConstraintError;
            }
        }
        break;
    case MaxCoolSetpointLimit::Id:
        if (limit < mAbsMinCoolSetpointLimit || limit < mMinCoolSetpointLimit || limit > mAbsMaxCoolSetpointLimit)
        {
            return Status::ConstraintError;
        }
        if (autoSupported)
        {
            if (limit < mMaxHeatSetpointLimit + mDeadBand)
            {
                return Status::ConstraintError;
            }
        }
        break;
    default:
        return Status::UnsupportedAttribute;
    }

    auto result = persistence.DecodeAndStoreNativeEndianValue({ mEndpoint, Thermostat::Id, attribute }, decoder, limit);
    if (!result.IsSuccess())
    {
        return result;
    }
    switch (attribute)
    {
    case MinHeatSetpointLimit::Id:
        mMinHeatSetpointLimit = limit;
        break;
    case MaxHeatSetpointLimit::Id:
        mMaxHeatSetpointLimit = limit;
        break;
    case MinCoolSetpointLimit::Id:
        mMinCoolSetpointLimit = limit;
        break;
    case MaxCoolSetpointLimit::Id:
        mMaxCoolSetpointLimit = limit;
        break;
    }
    return result;
}

DataModel::ActionReturnStatus Setpoints::ChangeSetpointDeadBand(int16_t deadBand, bool autoSupported,
                                                                AttributeValueDecoder & decoder, AttributePersistence & persistence)
{
    if (deadBand < 0 || deadBand > 127)
    {
        return Status::ConstraintError;
    }
    int8_t shortDeadBand = static_cast<int8_t>(deadBand);
    auto result =
        persistence.DecodeAndStoreNativeEndianValue({ mEndpoint, Thermostat::Id, MinSetpointDeadBand::Id }, decoder, shortDeadBand);
    if (!result.IsSuccess())
    {
        return result;
    }
    mDeadBand = static_cast<int16_t>(deadBand * 10);
    return result;
}

DataModel::ActionReturnStatus Setpoints::RaiseLowerSetpoint(int16_t amount, SetpointRaiseLowerModeEnum mode,
                                                            const BitFlags<Thermostat::Feature> features, bool occupied,
                                                            AttributePersistenceProvider & persistence)
{
    auto supportsHeat = features.Has(Feature::kHeating);
    auto supportsCool = features.Has(Feature::kCooling);

    auto & coolSetpoint        = occupied ? mOccupiedCoolingSetpoint : mUnoccupiedCoolingSetpoint;
    auto & heatSetpoint        = occupied ? mOccupiedHeatingSetpoint : mUnoccupiedHeatingSetpoint;
    auto coolSetpointAttribute = occupied ? OccupiedCoolingSetpoint::Id : UnoccupiedCoolingSetpoint::Id;
    auto heatSetpointAttribute = occupied ? OccupiedHeatingSetpoint::Id : UnoccupiedHeatingSetpoint::Id;

    int16_t targetCoolSetpoint = coolSetpoint;
    int16_t targetHeatSetpoint = heatSetpoint;

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth: {
        if (supportsHeat && supportsCool)
        {
            targetCoolSetpoint = static_cast<int16_t>(targetCoolSetpoint + amount * 10);
            targetHeatSetpoint = static_cast<int16_t>(targetHeatSetpoint + amount * 10);
            if (targetCoolSetpoint < mMinCoolSetpointLimit)
            {
                targetCoolSetpoint = mMinCoolSetpointLimit;
                int16_t diff       = targetCoolSetpoint - targetHeatSetpoint;
                if (diff > mDeadBand)
                {
                    targetHeatSetpoint = targetCoolSetpoint - mDeadBand;
                }
            }
            if (targetHeatSetpoint < mMinHeatSetpointLimit)
            {
                targetHeatSetpoint = mMinHeatSetpointLimit;
                int16_t diff       = targetCoolSetpoint - targetHeatSetpoint;
                if (diff > mDeadBand)
                {
                    targetCoolSetpoint = targetHeatSetpoint + mDeadBand;
                }
            }
        }
        else if (supportsHeat)
        {
            targetHeatSetpoint = static_cast<int16_t>(targetHeatSetpoint + amount * 10);
            if (targetHeatSetpoint > mMaxHeatSetpointLimit)
            {
                targetHeatSetpoint = mMaxHeatSetpointLimit;
            }
            else if (targetHeatSetpoint < mMinHeatSetpointLimit)
            {
                targetHeatSetpoint = mMinHeatSetpointLimit;
            }
        }
        else if (supportsCool)
        {
            targetCoolSetpoint = static_cast<int16_t>(targetCoolSetpoint + amount * 10);
            if (targetCoolSetpoint > mMaxCoolSetpointLimit)
            {
                targetCoolSetpoint = mMaxCoolSetpointLimit;
            }
            else if (targetCoolSetpoint < mMinCoolSetpointLimit)
            {
                targetCoolSetpoint = mMinCoolSetpointLimit;
            }
        }

        break;
    }
    case SetpointRaiseLowerModeEnum::kCool: {
        if (!supportsCool)
        {
            ChipLogError(Zcl, "Setpoints::RaiseLowerSetpoint tried to change cool without Cooling feature");
            return Status::InvalidCommand;
        }
        targetCoolSetpoint = static_cast<int16_t>(targetCoolSetpoint + amount * 10);
        if (targetCoolSetpoint > mMaxCoolSetpointLimit)
        {
            targetCoolSetpoint = mMaxCoolSetpointLimit;
        }
        else if (targetCoolSetpoint < mMinCoolSetpointLimit)
        {
            targetCoolSetpoint = mMinCoolSetpointLimit;
        }
        break;
    }

    case SetpointRaiseLowerModeEnum::kHeat: {
        if (!supportsHeat)
        {
            ChipLogError(Zcl, "Setpoints::RaiseLowerSetpoint tried to change heat without Heating feature");
            return Status::InvalidCommand;
        }
        targetHeatSetpoint = static_cast<int16_t>(targetHeatSetpoint + amount * 10);
        if (targetHeatSetpoint > mMaxHeatSetpointLimit)
        {
            targetHeatSetpoint = mMaxHeatSetpointLimit;
        }
        else if (targetHeatSetpoint < mMinHeatSetpointLimit)
        {
            targetHeatSetpoint = mMinHeatSetpointLimit;
        }
        break;
    }
    case SetpointRaiseLowerModeEnum::kUnknownEnumValue: {
        return Status::InvalidCommand;
    }
    }
    if (targetCoolSetpoint != coolSetpoint)
    {
        DataModel::ActionReturnStatus result =
            persistence.WriteValue({ mEndpoint, Thermostat::Id, coolSetpointAttribute },
                                   { reinterpret_cast<const uint8_t *>(&targetCoolSetpoint), sizeof(targetCoolSetpoint) });
        if (!result.IsSuccess())
        {
            return result;
        }
        coolSetpoint = targetCoolSetpoint;
    }
    if (targetHeatSetpoint != heatSetpoint)
    {
        DataModel::ActionReturnStatus result =
            persistence.WriteValue({ mEndpoint, Thermostat::Id, heatSetpointAttribute },
                                   { reinterpret_cast<const uint8_t *>(&targetHeatSetpoint), sizeof(targetHeatSetpoint) });
        if (!result.IsSuccess())
        {
            return result;
        }
        heatSetpoint = targetHeatSetpoint;
    }
    return Status::Success;
}

int16_t Setpoints::EnforceHeatingSetpointLimits(int16_t heatingSetpoint)
{

    // Optional User supplied limits
    int16_t minHeatSetpointLimit = mMinHeatSetpointLimit;
    int16_t maxHeatSetpointLimit = mMaxHeatSetpointLimit;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    if (minHeatSetpointLimit < mAbsMinHeatSetpointLimit)
        minHeatSetpointLimit = mAbsMinHeatSetpointLimit;

    if (maxHeatSetpointLimit > mAbsMaxHeatSetpointLimit)
        maxHeatSetpointLimit = mAbsMaxHeatSetpointLimit;

    if (heatingSetpoint < minHeatSetpointLimit)
        heatingSetpoint = minHeatSetpointLimit;

    if (heatingSetpoint > maxHeatSetpointLimit)
        heatingSetpoint = maxHeatSetpointLimit;

    return heatingSetpoint;
}

int16_t Setpoints::EnforceCoolingSetpointLimits(int16_t coolingSetpoint)
{

    // Optional User supplied limits
    int16_t minCoolSetpointLimit = mMinCoolSetpointLimit;
    int16_t maxCoolSetpointLimit = mMaxCoolSetpointLimit;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    if (minCoolSetpointLimit < mAbsMinCoolSetpointLimit)
        minCoolSetpointLimit = mAbsMinCoolSetpointLimit;

    if (maxCoolSetpointLimit > mAbsMaxCoolSetpointLimit)
        maxCoolSetpointLimit = mAbsMaxCoolSetpointLimit;

    if (coolingSetpoint < minCoolSetpointLimit)
        coolingSetpoint = minCoolSetpointLimit;

    if (coolingSetpoint > maxCoolSetpointLimit)
        coolingSetpoint = maxCoolSetpointLimit;

    return coolingSetpoint;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
