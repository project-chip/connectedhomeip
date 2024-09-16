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

#include <chrono>

#include <app-common/zap-generated/ids/Attributes.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

void ClusterStateAttributes::Init(ClusterInitParameters state)
{
    mState.currentLevel = state.currentLevel;
    mState.currentState = state.currentState;
    mState.valveFault   = state.valveFault;
    mState.levelStep    = state.levelStep;

    // Overwrite default initial state with stored persisted values if set.
    uint32_t defaultOpenDuration;
    if (mMatterContext.GetDefaultOpenDuration(defaultOpenDuration) == CHIP_NO_ERROR)
    {
        mState.defaultOpenDuration.SetNonNull(defaultOpenDuration);
    }
    mMatterContext.GetDefaultOpenLevel(mState.defaultOpenLevel);

    QuieterReportingPolicyFlags & policy = mState.remainingDuration.policy();
    policy = QuieterReportingPolicyFlags(to_underlying(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero) |
                                         to_underlying(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement));
}

CHIP_ERROR ClusterStateAttributes::SetRemainingDuration(const DataModel::Nullable<ElapsedS> & remainingDuration)
{
    System::Clock::Milliseconds64 now = System::SystemClock().GetMonotonicMilliseconds64();
    AttributeDirtyState dirtyState    = mState.remainingDuration.SetValue(
        remainingDuration, now, mState.remainingDuration.GetPredicateForSufficientTimeSinceLastDirty(kRemainingDurationReportRate));
    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        mMatterContext.MarkDirty(Attributes::RemainingDuration::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterStateAttributes::SetOpenDuration(const DataModel::Nullable<ElapsedS> & openDuration)
{
    bool dirty          = openDuration != mState.openDuration;
    mState.openDuration = openDuration;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::OpenDuration::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetDefaultOpenDuration(const DataModel::Nullable<ElapsedS> & defaultOpenDuration)
{
    bool dirty = defaultOpenDuration != mState.defaultOpenDuration;
    ReturnErrorOnFailure(mMatterContext.StoreDefaultOpenDuration(defaultOpenDuration));
    mState.defaultOpenDuration = defaultOpenDuration;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::DefaultOpenDuration::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetAutoCloseTime(const DataModel::Nullable<EpochUs> & autoCloseTime)
{
    bool dirty           = autoCloseTime != mState.autoCloseTime;
    mState.autoCloseTime = autoCloseTime;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::AutoCloseTime::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetCurrentState(const DataModel::Nullable<ValveStateEnum> & currentState)
{
    bool dirty          = currentState != mState.currentState;
    mState.currentState = currentState;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::CurrentState::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetTargetState(const DataModel::Nullable<ValveStateEnum> & targetState)
{
    bool dirty         = targetState != mState.targetState;
    mState.targetState = targetState;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::TargetState::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetCurrentLevel(const DataModel::Nullable<Percent> & currentLevel)
{
    // TODO: Q quality
    bool dirty          = currentLevel != mState.currentLevel;
    mState.currentLevel = currentLevel;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::CurrentLevel::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetTargetLevel(const DataModel::Nullable<Percent> & targetLevel)
{
    bool dirty         = targetLevel != mState.targetLevel;
    mState.targetLevel = targetLevel;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::TargetLevel::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetDefaultOpenLevel(const Percent defaultOpenLevel)
{
    bool dirty = defaultOpenLevel != mState.defaultOpenLevel;
    ReturnErrorOnFailure(mMatterContext.StoreDefaultOpenLevel(defaultOpenLevel));
    mState.defaultOpenLevel = defaultOpenLevel;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::DefaultOpenLevel::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetValveFault(const BitMask<ValveFaultBitmap> & valveFault)
{
    bool dirty        = valveFault != mState.valveFault;
    mState.valveFault = valveFault;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::ValveFault::Id);
    }
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterStateAttributes::SetLevelStep(const uint8_t levelStep)
{
    bool dirty       = levelStep != mState.levelStep;
    mState.levelStep = levelStep;
    if (dirty)
    {
        mMatterContext.MarkDirty(Attributes::LevelStep::Id);
    }
    return CHIP_NO_ERROR;
}

System::Clock::Milliseconds64 ClusterStateAttributes::GetNextReportTimeForRemainingDuration()
{
    return std::chrono::duration_cast<System::Clock::Milliseconds64>(mState.remainingDuration.GetLastReportTime()) +
        kRemainingDurationReportRate;
}

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance, const ClusterInitParameters & initialState)
{
    if (!conformance.Valid())
    {
        return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
    }
    if (conformance.HasFeature(Feature::kLevel) && mClusterDriver.GetDelegateType() != kLevel)
    {
        return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
    }
    if (!conformance.HasFeature(Feature::kLevel) && mClusterDriver.GetDelegateType() != kNonLevel)
    {
        return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
    }
    mConformance = conformance;

    mState.Init(initialState);
    mDurationStarted = System::SystemClock().GetMonotonicMilliseconds64();

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
    openDuration = mState.GetState().openDuration;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetDefaultOpenDuration(DataModel::Nullable<uint32_t> & defaultOpenDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    defaultOpenDuration = mState.GetState().defaultOpenDuration;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetAutoCloseTime(DataModel::Nullable<uint64_t> & autoCloseTime)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kTimeSync), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    autoCloseTime = mState.GetState().autoCloseTime;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetRemainingDuration(DataModel::Nullable<uint32_t> & remainingDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    HandleUpdateRemainingDurationInternal();
    remainingDuration = mState.GetState().remainingDuration.value();
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetCurrentState(DataModel::Nullable<ValveStateEnum> & currentState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    currentState = mState.GetState().currentState;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetTargetState(DataModel::Nullable<ValveStateEnum> & targetState)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    targetState = mState.GetState().targetState;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetCurrentLevel(DataModel::Nullable<uint8_t> & currentLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    currentLevel = mState.GetState().currentLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetTargetLevel(DataModel::Nullable<uint8_t> & targetLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    targetLevel = mState.GetState().targetLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetDefaultOpenLevel(uint8_t & defaultOpenLevel)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsDefaultOpenLevel, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    defaultOpenLevel = mState.GetState().defaultOpenLevel;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetValveFault(BitMask<ValveFaultBitmap> & valveFault)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.supportsValveFault, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    valveFault = mState.GetState().valveFault;
    return CHIP_NO_ERROR;
}
CHIP_ERROR ClusterLogic::GetLevelStep(uint8_t & levelStep)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLevel), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mConformance.supportsLevelStep, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    levelStep = mState.GetState().levelStep;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetDefaultOpenDuration(const DataModel::Nullable<ElapsedS> & defaultOpenDuration)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    if (!defaultOpenDuration.IsNull() && defaultOpenDuration.Value() < 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return mState.SetDefaultOpenDuration(defaultOpenDuration);
}

bool ClusterLogic::ValueCompliesWithLevelStep(const uint8_t value)
{
    if (mConformance.supportsLevelStep)
    {
        if ((value != 100u) && ((value % mState.GetState().levelStep) != 0))
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
    return mState.SetDefaultOpenLevel(defaultOpenLevel);
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
            realTargetLevel = mState.GetState().defaultOpenLevel;
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
        mState.SetValveFault(returnedValveFault);
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (returnedCurrentLevel == realTargetLevel)
    {
        mState.SetTargetLevel(DataModel::NullNullable);
        mState.SetCurrentLevel(realTargetLevel);
        mState.SetTargetState(DataModel::NullNullable);
        mState.SetCurrentState(ValveStateEnum::kOpen);
    }
    else
    {
        mState.SetTargetLevel(realTargetLevel);
        mState.SetCurrentLevel(returnedCurrentLevel);
        mState.SetTargetState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kOpen));
        mState.SetCurrentState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning));
        // TODO: Need to start a timer to continue querying the device for updates. Or just let the delegate handle this?
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
        mState.SetValveFault(returnedValveFault);
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (returnedState == ValveStateEnum::kOpen)
    {
        mState.SetTargetLevel(DataModel::NullNullable);
        mState.SetCurrentState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kOpen));
    }
    else
    {
        mState.SetTargetState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kOpen));
        mState.SetCurrentState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning));
        // TODO: Need to start a timer to continue querying the device for updates. Or just let the delegate handle this?
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
        realOpenDuration = mState.GetState().defaultOpenDuration;
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

    mState.SetOpenDuration(realOpenDuration);
    mDurationStarted = System::SystemClock().GetMonotonicMilliseconds64();
    HandleUpdateRemainingDurationInternal();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::HandleCloseCommand()
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);
    return HandleCloseInternal();
}

CHIP_ERROR ClusterLogic::HandleCloseInternal()
{
    CHIP_ERROR err;
    BitMask<ValveFaultBitmap> faults;
    if (mConformance.HasFeature(Feature::kLevel))
    {
        Percent currentLevel;
        err = mClusterDriver.HandleCloseValve(currentLevel, faults);
        if (err == CHIP_NO_ERROR)
        {
            mState.SetCurrentLevel(DataModel::Nullable<Percent>(currentLevel));
            if (currentLevel == 0)
            {
                mState.SetCurrentState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kClosed));
            }
            else
            {
                mState.SetCurrentState(DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning));
            }
        }
    }
    else
    {
        ValveStateEnum state;
        err = mClusterDriver.HandleCloseValve(state, faults);
        if (err == CHIP_NO_ERROR)
        {
            mState.SetCurrentState(state);
        }
    }
    // If there was an error, we know nothing about the current state
    if (err != CHIP_NO_ERROR)
    {
        mState.SetCurrentLevel(DataModel::NullNullable);
        mState.SetCurrentState(DataModel::NullNullable);
    }
    mState.SetValveFault(faults);
    mState.SetOpenDuration(DataModel::NullNullable);
    mState.SetRemainingDuration(DataModel::NullNullable);
    mState.SetTargetLevel(DataModel::NullNullable);
    mState.SetTargetState(DataModel::NullNullable);
    mState.SetAutoCloseTime(DataModel::NullNullable);
    return err;
}

void ClusterLogic::HandleUpdateRemainingDuration(System::Layer * systemLayer, void * context)
{
    auto * logic = static_cast<ClusterLogic *>(context);
    logic->HandleUpdateRemainingDurationInternal();
}

void ClusterLogic::HandleUpdateRemainingDurationInternal()
{
    // Start by cancelling the timer in case this was called from a command handler
    // We will start a new timer if required.
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);

    if (mState.GetState().openDuration.IsNull())
    {
        // I think this might be an error state - if openDuration is NULL, this timer shouldn't be on.
        mState.SetRemainingDuration(DataModel::NullNullable);
        return;
    }

    // Setup a new timer to either send the next report or handle the close operation
    System::Clock::Milliseconds64 now      = System::SystemClock().GetMonotonicMilliseconds64();
    System::Clock::Seconds64 openDurationS = System::Clock::Seconds64(mState.GetState().openDuration.ValueOr(0));
    System::Clock::Milliseconds64 closeTimeMs =
        mDurationStarted + std::chrono::duration_cast<System::Clock::Milliseconds64>(openDurationS);
    if (now >= closeTimeMs)
    {
        // Time's up, close the valve. Close handles setting the open and remaining duration
        HandleCloseInternal();
        return;
    }
    System::Clock::Milliseconds64 remainingMs     = closeTimeMs - now;
    System::Clock::Milliseconds64 nextReportTimer = mState.GetNextReportTimeForRemainingDuration() - now;

    System::Clock::Milliseconds64 nextTimerTime = std::min(nextReportTimer, remainingMs);
    DeviceLayer::SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(nextTimerTime),
                                          HandleUpdateRemainingDuration, this);

    auto remainingS = std::chrono::round<System::Clock::Seconds32>(remainingMs);
    mState.SetRemainingDuration(DataModel::Nullable<ElapsedS>(remainingS.count()));
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
