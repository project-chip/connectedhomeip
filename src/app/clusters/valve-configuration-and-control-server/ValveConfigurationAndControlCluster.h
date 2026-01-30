/**
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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
 * @file API declarations for valve configuration and control cluster.
 */

#pragma once

#include "TimeSyncTracker.h"
#include "valve-configuration-and-control-delegate.h"

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ValveConfigurationAndControl/Metadata.h>
#include <system/SystemLayer.h>

namespace chip::app::Clusters {
class ValveConfigurationAndControlCluster : public DefaultServerCluster
{
public:
    static constexpr uint8_t kDefaultOpenLevel     = 100u;
    static constexpr uint8_t kDefaultLevelStep     = 1u;
    static constexpr uint8_t kMaxLevelValuePercent = 100u;

    using OptionalAttributeSet = chip::app::OptionalAttributeSet<ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id,
                                                                 ValveConfigurationAndControl::Attributes::ValveFault::Id,
                                                                 ValveConfigurationAndControl::Attributes::LevelStep::Id>;

    struct StartupConfiguration
    {
        const DataModel::Nullable<uint32_t> defaultOpenDuration;
        Percent defaultOpenLevel = kDefaultOpenLevel;
        uint8_t levelStep        = kDefaultLevelStep;
    };

    struct ValveContext
    {
        BitFlags<ValveConfigurationAndControl::Feature> features;
        OptionalAttributeSet optionalAttributeSet;
        StartupConfiguration config;
        ValveConfigurationAndControl::TimeSyncTracker * tsTracker;
        ValveConfigurationAndControl::Delegate * delegate;
    };

    ValveConfigurationAndControlCluster(EndpointId endpointId, ValveContext context);

    // Server cluster implementation
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

    void SetDelegate(ValveConfigurationAndControl::Delegate * delegate);
    static void HandleUpdateRemainingDuration(System::Layer * systemLayer, void * context);
    void Shutdown(ClusterShutdownType shutdownType) override;

    CHIP_ERROR CloseValve();
    CHIP_ERROR OpenValve(DataModel::Nullable<Percent> targetLevel, DataModel::Nullable<uint32_t> openDuration);
    void UpdateCurrentLevel(Percent currentLevel);
    void UpdateCurrentState(ValveConfigurationAndControl::ValveStateEnum currentState);
    // This methods set the ValveFault attribute of the cluster, it also emits an event for said fault.
    void SetValveFault(BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault);
    void UpdateAutoCloseTime(uint64_t epochTime);

private:
    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);
    std::optional<DataModel::ActionReturnStatus> HandleOpenCommand(const DataModel::InvokeRequest & request,
                                                                   TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleCloseCommand();
    bool ValueCompliesWithLevelStep(const uint8_t value) const;
    void HandleUpdateRemainingDurationInternal();
    void SetRemainingDuration(const DataModel::Nullable<ElapsedS> & remainingDuration);
    void SetCurrentState(const ValveConfigurationAndControl::ValveStateEnum & newState);
    CHIP_ERROR SetAutoCloseTime(DataModel::Nullable<uint32_t> openDuration);
    void EmitValveChangeEvent(ValveConfigurationAndControl::ValveStateEnum newState);

    // Attributes
    const BitFlags<ValveConfigurationAndControl::Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;
    DataModel::Nullable<uint32_t> mOpenDuration;
    DataModel::Nullable<uint32_t> mDefaultOpenDuration;
    DataModel::Nullable<uint64_t> mAutoCloseTime;
    QuieterReportingAttribute<uint32_t> mRemainingDuration;
    DataModel::Nullable<ValveConfigurationAndControl::ValveStateEnum> mCurrentState;
    DataModel::Nullable<ValveConfigurationAndControl::ValveStateEnum> mTargetState;
    DataModel::Nullable<Percent> mCurrentLevel;
    DataModel::Nullable<Percent> mTargetLevel;
    Percent mDefaultOpenLevel{ kDefaultOpenLevel };
    BitMask<ValveConfigurationAndControl::ValveFaultBitmap> mValveFault;
    uint8_t mLevelStep{ kDefaultLevelStep };
    ValveConfigurationAndControl::Delegate * mDelegate{ nullptr };
    ValveConfigurationAndControl::TimeSyncTracker * mTsTracker{ nullptr };
};
} // namespace chip::app::Clusters
