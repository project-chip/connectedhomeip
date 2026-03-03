/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include "fan-control-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/FanControl/Attributes.h>
#include <clusters/FanControl/Enums.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters {

/**
 * @brief Fan Control Cluster Implementation (Code-Driven Architecture)
 *
 * Supports:
 * - Mandatory Attributes: FanMode, FanModeSequence, PercentSetting, PercentCurrent
 * - Optional Attributes (feature-gated): SpeedMax, SpeedSetting, SpeedCurrent,
 *   RockSupport, RockSetting, WindSupport, WindSetting, AirflowDirection
 * - Step command (when Step feature is enabled)
 * - Attribute change cascades (e.g., FanMode Off -> zero out PercentSetting, SpeedSetting)
 */
class FanControlCluster : public DefaultServerCluster
{
public:
    using OptionalAttributes =
        app::OptionalAttributeSet<FanControl::Attributes::SpeedMax::Id, FanControl::Attributes::SpeedSetting::Id,
                                  FanControl::Attributes::SpeedCurrent::Id, FanControl::Attributes::RockSupport::Id,
                                  FanControl::Attributes::RockSetting::Id, FanControl::Attributes::WindSupport::Id,
                                  FanControl::Attributes::WindSetting::Id, FanControl::Attributes::AirflowDirection::Id>;

    struct Config
    {
        Config(EndpointId endpointId, FanControl::Delegate & delegate) : mEndpointId(endpointId), mDelegate(delegate) {}

        Config & WithFanModeSequence(FanControl::FanModeSequenceEnum fanModeSequence)
        {
            mFanModeSequence = fanModeSequence;
            return *this;
        }
        Config & WithSpeedMax(uint8_t speedMax)
        {
            mSpeedMax = speedMax;
            mOptionalAttributes.Set<FanControl::Attributes::SpeedMax::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::SpeedSetting::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::SpeedCurrent::Id>();
            return *this;
        }
        Config & WithRockSupport(BitMask<FanControl::RockBitmap> rockSupport)
        {
            mRockSupport = rockSupport;
            mOptionalAttributes.Set<FanControl::Attributes::RockSupport::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::RockSetting::Id>();
            return *this;
        }
        Config & WithWindSupport(BitMask<FanControl::WindBitmap> windSupport)
        {
            mWindSupport = windSupport;
            mOptionalAttributes.Set<FanControl::Attributes::WindSupport::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::WindSetting::Id>();
            return *this;
        }
        Config & WithAirflowDirection()
        {
            mOptionalAttributes.Set<FanControl::Attributes::AirflowDirection::Id>();
            return *this;
        }
        Config & WithStep()
        {
            mSupportsStep = true;
            return *this;
        }

        EndpointId mEndpointId;
        FanControl::Delegate & mDelegate;
        bool mSupportsStep                               = false;
        FanControl::FanModeSequenceEnum mFanModeSequence = FanControl::FanModeSequenceEnum::kOffLowHigh;
        uint8_t mSpeedMax                                = 0;
        BitMask<FanControl::RockBitmap> mRockSupport;
        BitMask<FanControl::WindBitmap> mWindSupport;
        OptionalAttributes mOptionalAttributes;
    };

    FanControlCluster(const Config & config);
    ~FanControlCluster() = default;

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

    // Getters
    FanControl::FanModeEnum GetFanMode() const { return mFanMode; }
    DataModel::Nullable<chip::Percent> GetPercentSetting() const { return mPercentSetting; }
    DataModel::Nullable<uint8_t> GetSpeedSetting() const { return mSpeedSetting; }

private:
    bool SupportsMultiSpeed() const { return mOptionalAttributes.Has(FanControl::Attributes::SpeedMax::Id); }
    bool SupportsAuto() const
    {
        return mFanModeSequence == FanModeSequenceEnum::kOffLowHighAuto ||
            mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto || mFanModeSequence == FanModeSequenceEnum::kOffHighAuto;
    }
    bool SupportsRocking() const { return mOptionalAttributes.Has(FanControl::Attributes::RockSupport::Id); }
    bool SupportsWind() const { return mOptionalAttributes.Has(FanControl::Attributes::WindSupport::Id); }
    bool SupportsStep() const { return mSupportsStep; }
    bool SupportsAirflowDirection() const { return mOptionalAttributes.Has(FanControl::Attributes::AirflowDirection::Id); }

    Protocols::InteractionModel::Status SetFanModeToOff();
    void ApplyFanModeOffSideEffects();
    void ApplyFanModeAutoSideEffects();
    void ApplyPercentSettingChanged();
    void ApplySpeedSettingChanged();

    // Attributes
    FanControl::FanModeEnum mFanMode = FanControl::FanModeEnum::kOff;
    FanControl::FanModeSequenceEnum mFanModeSequence;
    DataModel::Nullable<chip::Percent> mPercentSetting;
    DataModel::Nullable<uint8_t> mSpeedSetting;
    BitMask<FanControl::RockBitmap> mRockSetting;
    BitMask<FanControl::WindBitmap> mWindSetting;
    FanControl::AirflowDirectionEnum mAirflowDirection = FanControl::AirflowDirectionEnum::kForward;

    bool mSupportsStep = false;
    uint8_t mSpeedMax;
    BitMask<FanControl::RockBitmap> mRockSupport;
    BitMask<FanControl::WindBitmap> mWindSupport;
    OptionalAttributes mOptionalAttributes;
    FanControl::Delegate & mDelegate;

    // Avoid circular callback when adjusting SpeedSetting and PercentSetting together
    bool mSpeedWriteInProgress   = false;
    bool mPercentWriteInProgress = false;
    bool mFanModeWriteInProgress = false;
};

} // namespace chip::app::Clusters
