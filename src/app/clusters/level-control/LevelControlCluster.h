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

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/data-model/Nullable.h>
#include <app/persistence/AttributePersistenceProvider.h>
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
class LevelControlCluster : public DefaultServerCluster, public scenes::DefaultSceneHandlerImpl, public OnOffDelegate
{
public:
    // Helper set for managing optional attributes availability based on configuration.
    using OptionalAttributes = app::OptionalAttributeSet<  //
        LevelControl::Attributes::MinLevel::Id,            //
        LevelControl::Attributes::MaxLevel::Id,            //
        LevelControl::Attributes::DefaultMoveRate::Id,     //
        LevelControl::Attributes::OnOffTransitionTime::Id, //
        LevelControl::Attributes::OnTransitionTime::Id,    //
        LevelControl::Attributes::OffTransitionTime::Id    //
        >;

    constexpr static uint8_t kLightingMinLevel = 1;
    constexpr static uint8_t kMaxLevel         = 254;

    struct Config
    {
        Config(EndpointId endpoint, TimerDelegate & timerDelegate, LevelControlDelegate & delegate) :
            mEndpointId(endpoint), mDelegate(delegate), mTimerDelegate(timerDelegate)
        {}

        Config & WithOnOff(OnOffCluster & onOffCluster)
        {
            mFeatureMap.Set(LevelControl::Feature::kOnOff);
            mOnOffCluster = &onOffCluster;
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
            mMinLevel = min;
            mOptionalAttributes.Set<LevelControl::Attributes::MinLevel::Id>();
            VerifyOrDie(!mFeatureMap.Has(LevelControl::Feature::kLighting) ||
                        (mFeatureMap.Has(LevelControl::Feature::kLighting) && mMinLevel == kLightingMinLevel));
            return *this;
        }
        Config & WithMaxLevel(uint8_t max)
        {
            mMaxLevel = max;
            mOptionalAttributes.Set<LevelControl::Attributes::MaxLevel::Id>();
            VerifyOrDie(!mFeatureMap.Has(LevelControl::Feature::kLighting) ||
                        (mFeatureMap.Has(LevelControl::Feature::kLighting) && mMaxLevel == kMaxLevel));
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
        OnOffCluster * mOnOffCluster = nullptr;

        uint8_t mMinLevel = 0;
        uint8_t mMaxLevel = kMaxLevel;
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
    ~LevelControlCluster() = default;

    // ServerClusterInterface Implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    // OnOffDelegate Implementation
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // Cluster Public API
    void SetOptions(BitMask<LevelControl::OptionsBitmap> newOptions);
    void SetOnLevel(DataModel::Nullable<uint8_t> newOnLevel);
    CHIP_ERROR SetDefaultMoveRate(DataModel::Nullable<uint8_t> newDefaultMoveRate);
    CHIP_ERROR SetStartUpCurrentLevel(DataModel::Nullable<uint8_t> startupLevel);
    void SetOnTransitionTime(DataModel::Nullable<uint16_t> onTransitionTime);
    void SetOffTransitionTime(DataModel::Nullable<uint16_t> offTransitionTime);
    void SetOnOffTransitionTime(uint16_t onOffTransitionTime);

    // Command APIs
    DataModel::ActionReturnStatus MoveToLevel(uint8_t level, DataModel::Nullable<uint16_t> transitionTime,
                                              BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus MoveToLevelWithOnOff(uint8_t level, DataModel::Nullable<uint16_t> transitionTime,
                                                       BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                       BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus Move(LevelControl::MoveModeEnum moveMode, DataModel::Nullable<uint8_t> rate,
                                       BitMask<LevelControl::OptionsBitmap> optionsMask,
                                       BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus MoveWithOnOff(LevelControl::MoveModeEnum moveMode, DataModel::Nullable<uint8_t> rate,
                                                BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus Step(LevelControl::StepModeEnum stepMode, uint8_t stepSize,
                                       DataModel::Nullable<uint16_t> transitionTime,
                                       BitMask<LevelControl::OptionsBitmap> optionsMask,
                                       BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StepWithOnOff(LevelControl::StepModeEnum stepMode, uint8_t stepSize,
                                                DataModel::Nullable<uint16_t> transitionTime,
                                                BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus Stop(BitMask<LevelControl::OptionsBitmap> optionsMask,
                                       BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StopWithOnOff(BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                BitMask<LevelControl::OptionsBitmap> optionsOverride);

    // Getters
    DataModel::Nullable<uint8_t> GetCurrentLevel() const { return mCurrentLevel.value(); }
    uint8_t GetMinLevel() const { return mMinLevel; }
    uint8_t GetMaxLevel() const { return mMaxLevel; }
    DataModel::Nullable<uint8_t> GetDefaultMoveRate() const { return mDefaultMoveRate; }
    DataModel::Nullable<uint8_t> GetOnLevel() const { return mOnLevel; }
    BitMask<LevelControl::OptionsBitmap> GetOptions() const { return mOptions; }
    DataModel::Nullable<uint8_t> GetStartUpCurrentLevel() const { return mStartUpCurrentLevel; }
    BitMask<LevelControl::Feature> GetFeatureMap() const { return mFeatureMap; }
    uint16_t GetRemainingTime() const { return mRemainingTime.value().ValueOr(0); }
    DataModel::Nullable<uint16_t> GetOnTransitionTime() const { return mOnTransitionTime; }
    DataModel::Nullable<uint16_t> GetOffTransitionTime() const { return mOffTransitionTime; }
    uint16_t GetOnOffTransitionTime() const { return mOnOffTransitionTime; }

    // SceneHandler implementation
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override;
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override;
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override;

private:
    enum class ReportingMode
    {
        kForceReport,
        kQuietReport
    };

    CHIP_ERROR SetCurrentLevel(uint8_t level, ReportingMode reportingMode);

    // Attributes
    app::QuieterReportingAttribute<uint8_t> mCurrentLevel;
    BitMask<LevelControl::OptionsBitmap> mOptions;
    DataModel::Nullable<uint8_t> mOnLevel;
    uint8_t mMinLevel;
    uint8_t mMaxLevel;
    DataModel::Nullable<uint8_t> mDefaultMoveRate;
    DataModel::Nullable<uint8_t> mStartUpCurrentLevel;

    // Extended Attributes (Lighting/Transitions)
    app::QuieterReportingAttribute<uint16_t> mRemainingTime;
    DataModel::Nullable<uint16_t> mOnTransitionTime;
    DataModel::Nullable<uint16_t> mOffTransitionTime;
    uint16_t mOnOffTransitionTime = 0;

    OptionalAttributes mOptionalAttributes;
    BitMask<LevelControl::Feature> mFeatureMap;
    LevelControlDelegate & mDelegate;
    TimerDelegate & mTimerDelegate;
    OnOffCluster * mOnOffCluster = nullptr;

    DataModel::Nullable<uint8_t> mLevelBeforeTurnedOff; // Stores the level before turning Off, to restore on On if OnLevel is null.

    // Private helper class to aggregate state and timing of level transitions. This helps keep the main
    // cluster class cleaner.
    class TransitionHandler : public TimerContext
    {
    public:
        TransitionHandler(LevelControlCluster & cluster) : mCluster(cluster) {}
        ~TransitionHandler();

        void StartTransition(CommandId commandId, uint8_t initialLevel, uint8_t targetLevel, uint32_t transitionTimeMs,
                             uint32_t stepDurationMs);
        void StopTransition();

        uint32_t GetTransitionTimeMs() const { return mTransitionTimeMs; }

        // TimerContext
        void TimerFired() override;

    private:
        LevelControlCluster & mCluster;

        // Transition Logic State
        uint8_t mInitialLevel           = 0;
        uint8_t mTargetLevel            = 0;
        uint32_t mTransitionTimeMs      = 0;
        uint32_t mElapsedTimeMs         = 0;
        uint32_t mTickDurationMs        = 0;
        uint64_t mTransitionStartTimeMs = 0;
        CommandId mCurrentCommandId     = kInvalidCommandId;
    };

    TransitionHandler mTransitionHandler;

    // Jitter Compensation (Moved to TransitionHandler)

    // Used to ignore/prevent reentrance of OnOffChanged callbacks when we are programmatically setting On/Off state
    // during a Level Control command (like MoveToLevelWithOnOff).
    bool mTemporarilyIgnoreOnOffCallbacks = false;

    // Helpers
    bool IsValidLevel(uint8_t level);
    CHIP_ERROR SetOnOff(bool on);
    bool GetOnOff();
    bool ShouldExecuteIfOff(BitMask<LevelControl::OptionsBitmap> optionsMask, BitMask<LevelControl::OptionsBitmap> optionsOverride);

    // Helper to write CurrentLevel to NVM.
    void StoreCurrentLevel(DataModel::Nullable<uint8_t> value);

    void UpdateRemainingTime(uint32_t remainingTimeMs, ReportingMode mode);

    DataModel::ActionReturnStatus MoveToLevelCommand(CommandId commandId, uint8_t level,
                                                     DataModel::Nullable<uint16_t> transitionTimeDS,
                                                     BitMask<LevelControl::OptionsBitmap> optionsMask,
                                                     BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus MoveCommand(CommandId commandId, LevelControl::MoveModeEnum moveMode,
                                              DataModel::Nullable<uint8_t> rate, BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StepCommand(CommandId commandId, LevelControl::StepModeEnum stepMode, uint8_t stepSize,
                                              DataModel::Nullable<uint16_t> transitionTime,
                                              BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
    DataModel::ActionReturnStatus StopCommand(CommandId commandId, BitMask<LevelControl::OptionsBitmap> optionsMask,
                                              BitMask<LevelControl::OptionsBitmap> optionsOverride);
};

} // namespace chip::app::Clusters
