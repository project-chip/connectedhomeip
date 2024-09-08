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
 */
/**
 * @file Cross-platform API to handle cluster-specific logic for the valve configuration and control cluster on a single endpoint.
 */

#include "valve-configuration-and-control-cluster-logic.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance, const ClusterState & initialState)
{
    if (!conformance.Valid())
    {
        return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
    }
    mConformance = conformance;
    mState       = initialState;

    // Overwrite default initial state with stored persisted values if set.
    uint32_t defaultOpenDuration;
    if (mMatterContext.GetDefaultOpenDuration(defaultOpenDuration) == CHIP_NO_ERROR)
    {
        mState.defaultOpenDuration.SetNonNull(defaultOpenDuration);
    }
    mMatterContext.GetDefaultOpenLevel(mState.defaultOpenLevel);

    mInitialized = true;
    return CHIP_NO_ERROR;
}

// All Get functions:
// Return CHIP_ERROR_INVALID_STATE if the class has not been initialized.
// Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
// Return CHIP_NO_ERROR and set the parameter value otherwise
CHIP_ERROR ClusterLogic::GetOpenDuration(DataModel::Nullable<uint32_t> & openDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    openDuration = mState.openDuration;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetDefaultOpenDuration(DataModel::Nullable<uint32_t> & defaultOpenDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    defaultOpenDuration = mState.defaultOpenDuration;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetAutoCloseTime(DataModel::Nullable<uint64_t> & autoCloseTime)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kTimeSync), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    autoCloseTime = mState.autoCloseTime;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetRemainingDuration(DataModel::Nullable<uint32_t> & remainingDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    remainingDuration = mState.remainingDuration;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetCurrentState(DataModel::Nullable<ValveStateEnum> & currentState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    currentState = mState.currentState;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetTargetState(DataModel::Nullable<ValveStateEnum> & targetState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    targetState = mState.targetState;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetCurrentLevel(DataModel::Nullable<uint8_t> & currentLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    currentLevel = mState.currentLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetTargetLevel(DataModel::Nullable<uint8_t> & targetLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    targetLevel = mState.targetLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetDefaultOpenLevel(uint8_t & defaultOpenLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsDefaultOpenLevel, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    defaultOpenLevel = mState.defaultOpenLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetValveFault(BitMask<ValveFaultBitmap> & valveFault)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.supportsValveFault, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    valveFault = mState.valveFault;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetLevelStep(uint8_t & levelStep)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsLevelStep, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    levelStep = mState.levelStep;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetDefaultOpenDuration(const DataModel::Nullable<ElapsedS> & defaultOpenDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    if (!defaultOpenDuration.IsNull() && defaultOpenDuration.Value() < 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ReturnErrorOnFailure(mMatterContext.StoreDefaultOpenDuration(defaultOpenDuration));
    mState.defaultOpenDuration = defaultOpenDuration;
    return CHIP_NO_ERROR;
}

bool ClusterLogic::ValueCompliesWithLevelStep(const uint8_t value)
{
    if (mConformance.supportsLevelStep)
    {
        if ((value != 100u) && ((value % mState.levelStep) != 0))
        {
            return false;
        }
    }
    return true;
}

CHIP_ERROR ClusterLogic::SetDefaultOpenLevel(const uint8_t defaultOpenLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.supportsDefaultOpenLevel, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    if (defaultOpenLevel < 1 || defaultOpenLevel > 100)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrReturnError(ValueCompliesWithLevelStep(defaultOpenLevel), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(mMatterContext.StoreDefaultOpenLevel(defaultOpenLevel));
    mState.defaultOpenLevel = defaultOpenLevel;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetValveFault(const ValveFaultBitmap valveFault)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR ClusterLogic::ClearValveFault(const ValveFaultBitmap valveFault)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ClusterLogic::GetRealTargetLevel(const std::optional<Percent> & targetLevel, Percent & realTargetLevel)
{
    if (!targetLevel.has_value())
    {
        if (mConformance.supportsDefaultOpenLevel)
        {
            realTargetLevel = mState.defaultOpenLevel;
            return CHIP_NO_ERROR;
        }
        realTargetLevel = 100u;
        return CHIP_NO_ERROR;
    }
    // targetLevel has a value
    VerifyOrReturnError(ValueCompliesWithLevelStep(targetLevel.value()), CHIP_ERROR_INVALID_ARGUMENT);
    realTargetLevel = targetLevel.value();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::HandleOpenLevel(const std::optional<Percent> & targetLevel)
{
    // This function should only be called for devices that support the level feature.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_INTERNAL);

    Percent realTargetLevel;
    Percent returnedCurrentLevel                 = 0;
    BitMask<ValveFaultBitmap> returnedValveFault = 0;
    ReturnErrorOnFailure(GetRealTargetLevel(targetLevel, realTargetLevel));

    CHIP_ERROR err = mClusterDriver.HandleOpenValve(realTargetLevel, returnedCurrentLevel, returnedValveFault);

    if (mConformance.supportsValveFault)
    {
        mState.valveFault = returnedValveFault;
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (returnedCurrentLevel == realTargetLevel)
    {
        mState.targetLevel  = DataModel::NullNullable;
        mState.currentLevel = realTargetLevel;
        mState.targetState  = DataModel::NullNullable;
        mState.currentState = ValveStateEnum::kOpen;
    }
    else
    {
        mState.targetLevel  = realTargetLevel;
        mState.currentLevel = returnedCurrentLevel;
        mState.targetState.SetNonNull(ValveStateEnum::kOpen);
        mState.currentState.SetNonNull(ValveStateEnum::kTransitioning);
        // TODO: Need to start a timer to continue querying the device for updates
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::HandleOpenNoLevel()
{
    // This function should only be called for devices that do not support the level feature.
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_INTERNAL);

    ValveStateEnum returnedState                 = ValveStateEnum::kUnknownEnumValue;
    BitMask<ValveFaultBitmap> returnedValveFault = 0;

    CHIP_ERROR err = mClusterDriver.HandleOpenValve(returnedState, returnedValveFault);
    if (mConformance.supportsValveFault)
    {
        mState.valveFault = returnedValveFault;
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (returnedState == ValveStateEnum::kOpen)
    {
        mState.targetLevel.SetNull();
        mState.currentState.SetNonNull(ValveStateEnum::kOpen);
    }
    else
    {
        mState.targetState.SetNonNull(ValveStateEnum::kOpen);
        mState.currentState.SetNonNull(ValveStateEnum::kTransitioning);
        // TODO: Need to start a timer to continue querying the device for updates
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::HandleOpenCommand(std::optional<DataModel::Nullable<ElapsedS>> openDuration,
                                           std::optional<Percent> targetLevel)
{
    // openDuration
    // - if this is omitted, fall back to defaultOpenDuration
    // - if this is NULL, remaining duration is NULL
    // - if this is a value, use that value
    // - if remaining duration is not null and TS is supported, set the autoCloseTime as appropriate
    // targetLevel
    // - if LVL is not supported
    //   - if this is omitted, that's correct
    //   - if this is supplied return error
    // - if LVL is supported
    //   - if this value is not supplied, use defaultOpenLevel if supported, otherwise 100
    //   - if this value is supplied, check against levelStep, error if not OK, otherwise set targetLevel
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    DataModel::Nullable<ElapsedS> realOpenDuration;
    if (openDuration.has_value())
    {
        realOpenDuration = openDuration.value();
    }
    else
    {
        realOpenDuration = mState.defaultOpenDuration;
    }

    if (!mConformance.HasFeature(Feature::kLevel) && targetLevel.has_value())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (mConformance.HasFeature(Feature::kLevel))
    {
        ReturnErrorOnFailure(HandleOpenLevel(targetLevel));
    }
    else
    {
        ReturnErrorOnFailure(HandleOpenNoLevel());
    }

    mState.openDuration = realOpenDuration;
    return CHIP_NO_ERROR;
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
