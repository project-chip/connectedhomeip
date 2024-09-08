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

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
