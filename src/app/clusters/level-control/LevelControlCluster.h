/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Enums.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters {

/**
 * @brief Level Control Cluster Implementation
 *
 * Supports:
 * - Mandatory Attributes (CurrentLevel, Options, OnLevel, etc.)
 * - OnOff Feature (optional)
 * - Lighting Feature (optional)
 * - Optional Attributes: MinLevel, MaxLevel, DefaultMoveRate, StartUpCurrentLevel.
 * - Transitions (MoveToLevel, Move, Step with transition time).
 * - Scenes.
 * - RemainingTime reporting.
 */
class LevelControlCluster : public DefaultServerCluster, public TimerContext, public scenes::DefaultSceneHandlerImpl
{
public:
    // Helper set for managing optional attributes availability based on configuration.
    using OptionalAttributes =
        app::OptionalAttributeSet<LevelControl::Attributes::MinLevel::Id, LevelControl::Attributes::MaxLevel::Id,
                                  LevelControl::Attributes::DefaultMoveRate::Id, LevelControl::Attributes::OnTransitionTime::Id,
                                  LevelControl::Attributes::OffTransitionTime::Id,
                                  LevelControl::Attributes::OnOffTransitionTime::Id>;

    struct Config
    {
        Config(EndpointId endpoint, TimerDelegate & timerDelegate, LevelControlDelegate & delegate) :
            mEndpointId(endpoint), mDelegate(delegate), mTimerDelegate(timerDelegate)
        {}

        Config & WithOnOff()
        {
            mFeatureMap.Set(LevelControl::Feature::kOnOff);
            return *this;
        }
        Config & WithLighting(DataModel::Nullable<uint8_t> startUpCurrentLevel)
        {
            mFeatureMap.Set(LevelControl::Feature::kLighting);
            WithMinLevel(1);   // Spec mandates MinLevel=1 for Lighting feature
            WithMaxLevel(254); // Spec mandates MaxLevel=254 for Lighting feature
            mStartUpCurrentLevel = startUpCurrentLevel;
            return *this;
        }
        Config & WithMinLevel(uint8_t min)
        {
            mMinLevel = mFeatureMap.Has(LevelControl::Feature::kLighting) ? 1 : min;
            mOptionalAttributes.Set<LevelControl::Attributes::MinLevel::Id>();
            return *this;
        }
        Config & WithMaxLevel(uint8_t max)
        {
            mMaxLevel = mFeatureMap.Has(LevelControl::Feature::kLighting) ? 254 : max;
            mOptionalAttributes.Set<LevelControl::Attributes::MaxLevel::Id>();
            return *this;
        }
        Config & WithDefaultMoveRate(DataModel::Nullable<uint8_t> rate)
        {
            mDefaultMoveRate = rate;
            mOptionalAttributes.Set<LevelControl::Attributes::DefaultMoveRate::Id>();
            return *this;
        }
        Config & WithOnOffTransitionTime(uint16_t time)
        {
            mOnOffTransitionTime = time;
            mOptionalAttributes.Set<LevelControl::Attributes::OnOffTransitionTime::Id>();
            return *this;
        }
        Config & WithOnTransitionTime(DataModel::Nullable<uint16_t> time)
        {
            mOnTransitionTime = time;
            mOptionalAttributes.Set<LevelControl::Attributes::OnTransitionTime::Id>();
            return *this;
        }
        Config & WithOffTransitionTime(DataModel::Nullable<uint16_t> time)
        {
            mOffTransitionTime = time;
            mOptionalAttributes.Set<LevelControl::Attributes::OffTransitionTime::Id>();
            return *this;
        }
        Config & WithInitialCurrentLevel(uint8_t level)
        {
            mInitialCurrentLevel.SetNonNull(level);
            return *this;
        }

        EndpointId mEndpointId;
        LevelControlDelegate & mDelegate;
        TimerDelegate & mTimerDelegate;

        uint8_t mMinLevel = 0;
        uint8_t mMaxLevel = 0;
        DataModel::Nullable<uint8_t> mDefaultMoveRate;
        DataModel::Nullable<uint8_t> mStartUpCurrentLevel;
        DataModel::Nullable<uint8_t> mInitialCurrentLevel;
        OptionalAttributes mOptionalAttributes;
        BitMask<LevelControl::Feature> mFeatureMap;

        uint16_t mOnOffTransitionTime = 0;
        DataModel::Nullable<uint16_t> mOnTransitionTime;
        DataModel::Nullable<uint16_t> mOffTransitionTime;
    };

    LevelControlCluster(const Config & config);
    ~LevelControlCluster() override;

    // ServerClusterInterface Implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    // Cluster Public API
    DataModel::ActionReturnStatus SetOptions(BitMask<LevelControl::OptionsBitmap> newOptions);
    DataModel::ActionReturnStatus SetOnLevel(DataModel::Nullable<uint8_t> newOnLevel);
    DataModel::ActionReturnStatus SetDefaultMoveRate(DataModel::Nullable<uint8_t> newDefaultMoveRate);
    DataModel::ActionReturnStatus SetCurrentLevel(uint8_t level);
    DataModel::ActionReturnStatus SetStartUpCurrentLevel(DataModel::Nullable<uint8_t> startupLevel);
    DataModel::ActionReturnStatus SetOnTransitionTime(DataModel::Nullable<uint16_t> onTransitionTime);
    DataModel::ActionReturnStatus SetOffTransitionTime(DataModel::Nullable<uint16_t> offTransitionTime);
    DataModel::ActionReturnStatus SetOnOffTransitionTime(uint16_t onOffTransitionTime);

    // Getters
    DataModel::Nullable<uint8_t> GetCurrentLevel() const { return mCurrentLevel; }
    uint8_t GetMinLevel() const { return mMinLevel; }
    uint8_t GetMaxLevel() const { return mMaxLevel; }
    DataModel::Nullable<uint8_t> GetDefaultMoveRate() const { return mDefaultMoveRate; }
    DataModel::Nullable<uint8_t> GetOnLevel() const { return mOnLevel; }
    BitMask<LevelControl::OptionsBitmap> GetOptions() const { return mOptions; }
    DataModel::Nullable<uint8_t> GetStartUpCurrentLevel() const { return mStartUpCurrentLevel; }
    BitMask<LevelControl::Feature> GetFeatureMap() const { return mFeatureMap; }

    // TimerContext
    void TimerFired() override;

    // SceneHandler implementation
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override;
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override;
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override;

    // Updates the Level Control state in response to an On/Off cluster state change.
    // Spec: "Effect of On/Off Commands on the CurrentLevel attribute".
    // isOn=true: Move to OnLevel (or stored/previous) starting from MinLevel.
    // isOn=false: Move to MinLevel and store current level.
    void OnOffChanged(bool isOn);

private:
    // Attributes
    DataModel::Nullable<uint8_t> mCurrentLevel;
    BitMask<LevelControl::OptionsBitmap> mOptions;
    DataModel::Nullable<uint8_t> mOnLevel;
    uint8_t mMinLevel;
    uint8_t mMaxLevel;
    DataModel::Nullable<uint8_t> mDefaultMoveRate;
    DataModel::Nullable<uint8_t> mStartUpCurrentLevel;

    // Extended Attributes (Lighting/Transitions)
    uint16_t mRemainingTime = 0;
    DataModel::Nullable<uint16_t> mOnTransitionTime;
    DataModel::Nullable<uint16_t> mOffTransitionTime;
    uint16_t mOnOffTransitionTime = 0;

    OptionalAttributes mOptionalAttributes;
    BitMask<LevelControl::Feature> mFeatureMap;
    LevelControlDelegate & mDelegate;
    TimerDelegate & mTimerDelegate;

    // Transition Logic State
    uint8_t mTargetLevel        = 0;
    uint32_t mTransitionTimeMs  = 0;
    uint32_t mElapsedTimeMs     = 0;
    uint32_t mEventDurationMs   = 0;
    bool mIncreasing            = false;
    CommandId mCurrentCommandId = kInvalidCommandId;
    DataModel::Nullable<uint8_t> mStoredLevel; // Stores the level before turning Off, to restore on On if OnLevel is null.

    // Used to ignore/prevent reentrance of OnOffChanged callbacks when we are programmatically setting On/Off state
    // during a Level Control command (like MoveToLevelWithOnOff).
    bool mTemporarilyIgnoreOnOffCallbacks = false;
    uint64_t mLastReportTimeMs            = 0; // Used for Quieter Reporting (Spec mandates at most 1 report per second).

    // Helpers
    void UpdateOnOff(bool on, bool temporarilyIgnoreOnOffCallbacks = false);
    // Checks "Options" attribute ExecuteIfOff bit and command-specific overrides to determine if command should run.
    bool ShouldExecuteIfOff(CommandId commandId, BitMask<LevelControl::OptionsBitmap> optionsMask,
                            BitMask<LevelControl::OptionsBitmap> optionsOverride);

    // Handles the "With On/Off" command logic: Turns device On if needed and resets level to MinLevel.
    void HandleOnOffTask(CommandId commandId, bool isMoveUp);

    // Setup and start the transition timer
    void ScheduleTimer(uint32_t durationMs, uint32_t transitionTimeMs);

    // Helper to set CurrentLevel with specific reporting rules (e.g. forced at end of transition).
    // Spec 6.2. CurrentLevel Attribute:
    // "Changes to this attribute SHALL only be marked as reportable in the following cases:
    //  At most once per second, or
    //  At the end of the movement/transition, or
    //  When it changes from null to any other value and vice versa."
    DataModel::ActionReturnStatus SetCurrentLevelQuietReport(DataModel::Nullable<uint8_t> newValue, bool isEndOfTransition);
    // Helper to update RemainingTime attribute with quiet reporting rules (delta check).
    void UpdateRemainingTime(uint32_t remainingTimeMs, bool isNewTransition);

    void StartTimer(uint32_t delayMs);
    void CancelTimer();
    void HandleTick();

    // Handlers
    DataModel::ActionReturnStatus MoveToLevelHandler(CommandId commandId, uint8_t level,
                                                     DataModel::Nullable<uint16_t> transitionTimeDS,
                                                     BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                     BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus MoveHandler(CommandId commandId, LevelControl::MoveModeEnum moveMode,
                                              DataModel::Nullable<uint8_t> rate, BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StepHandler(CommandId commandId, LevelControl::StepModeEnum stepMode, uint8_t stepSize,
                                              DataModel::Nullable<uint16_t> transitionTime,
                                              BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StopHandler(CommandId commandId, BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
};

} // namespace chip::app::Clusters
