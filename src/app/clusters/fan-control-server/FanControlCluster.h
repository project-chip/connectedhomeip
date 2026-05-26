/*
 *
 * Copyright (c) 2022-2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <app/clusters/fan-control-server/fan-control-delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/FanControl/Attributes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters {

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
            if (fanModeSequence == FanControl::FanModeSequenceEnum::kOffLowHighAuto ||
                fanModeSequence == FanControl::FanModeSequenceEnum::kOffLowMedHighAuto ||
                fanModeSequence == FanControl::FanModeSequenceEnum::kOffHighAuto)
            {
                mFeatureMap.Set(FanControl::Feature::kAuto);
            }
            else
            {
                mFeatureMap.Clear(FanControl::Feature::kAuto);
            }
            return *this;
        }
        Config & WithSpeedMax(uint8_t speedMax)
        {
            mSpeedMax = (speedMax == 0) ? 1 : (speedMax > 100 ? 100 : speedMax);
            mFeatureMap.Set(FanControl::Feature::kMultiSpeed);
            mOptionalAttributes.Set<FanControl::Attributes::SpeedMax::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::SpeedSetting::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::SpeedCurrent::Id>();
            return *this;
        }
        Config & WithRockSupport(BitMask<FanControl::RockBitmap> rockSupport)
        {
            mRockSupport = rockSupport;
            mFeatureMap.Set(FanControl::Feature::kRocking);
            mOptionalAttributes.Set<FanControl::Attributes::RockSupport::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::RockSetting::Id>();
            return *this;
        }
        Config & WithWindSupport(BitMask<FanControl::WindBitmap> windSupport)
        {
            mWindSupport = windSupport;
            mFeatureMap.Set(FanControl::Feature::kWind);
            mOptionalAttributes.Set<FanControl::Attributes::WindSupport::Id>();
            mOptionalAttributes.Set<FanControl::Attributes::WindSetting::Id>();
            return *this;
        }
        Config & WithAirflowDirection()
        {
            mFeatureMap.Set(FanControl::Feature::kAirflowDirection);
            mOptionalAttributes.Set<FanControl::Attributes::AirflowDirection::Id>();
            return *this;
        }
        Config & WithStep()
        {
            mFeatureMap.Set(FanControl::Feature::kStep);
            return *this;
        }

        EndpointId mEndpointId;
        FanControl::Delegate & mDelegate;
        FanControl::FanModeSequenceEnum mFanModeSequence = FanControl::FanModeSequenceEnum::kOffLowHigh;
        uint8_t mSpeedMax                                = 0;
        BitMask<FanControl::RockBitmap> mRockSupport;
        BitMask<FanControl::WindBitmap> mWindSupport;
        OptionalAttributes mOptionalAttributes;
        BitFlags<FanControl::Feature> mFeatureMap;
    };

    FanControlCluster(const Config & config);
    ~FanControlCluster() = default;

    /** Shared delegate when no application delegate is registered. HandleStep returns Failure. */
    static FanControl::Delegate & PlaceholderDelegate();

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

    // Getters
    FanControl::FanModeEnum GetFanMode() const { return mFanMode; }
    FanControl::FanModeSequenceEnum GetFanModeSequence() const { return mFanModeSequence; }
    DataModel::Nullable<chip::Percent> GetPercentSetting() const { return mPercentSetting; }
    chip::Percent GetPercentCurrent() const { return mPercentCurrent; }
    DataModel::Nullable<uint8_t> GetSpeedSetting() const { return mSpeedSetting; }
    uint8_t GetSpeedCurrent() const { return mSpeedCurrent; }
    uint8_t GetSpeedMax() const { return mSpeedMax; }
    BitFlags<FanControl::Feature> GetFeatureMap() const { return mFeatureMap; }
    BitMask<FanControl::RockBitmap> GetRockSupport() const { return mRockSupport; }
    BitMask<FanControl::RockBitmap> GetRockSetting() const { return mRockSetting; }
    BitMask<FanControl::WindBitmap> GetWindSupport() const { return mWindSupport; }
    BitMask<FanControl::WindBitmap> GetWindSetting() const { return mWindSetting; }
    FanControl::AirflowDirectionEnum GetAirflowDirection() const { return mAirflowDirection; }

    // Setters
    Protocols::InteractionModel::Status SetFanMode(FanControl::FanModeEnum value);
    Protocols::InteractionModel::Status SetPercentSetting(DataModel::Nullable<chip::Percent> value);
    Protocols::InteractionModel::Status SetSpeedSetting(DataModel::Nullable<uint8_t> value);
    Protocols::InteractionModel::Status SetRockSetting(BitMask<FanControl::RockBitmap> value);
    Protocols::InteractionModel::Status SetWindSetting(BitMask<FanControl::WindBitmap> value);
    Protocols::InteractionModel::Status SetAirflowDirection(FanControl::AirflowDirectionEnum value);
    bool SetPercentCurrent(chip::Percent value);
    bool SetSpeedCurrent(uint8_t value);

    void SetDelegate(FanControl::Delegate * delegate);

private:
    bool IsFanModeSupportedBySequence(FanControl::FanModeEnum value) const;

    bool SupportsMultiSpeed() const { return mOptionalAttributes.IsSet(FanControl::Attributes::SpeedMax::Id); }
    bool SupportsAuto() const
    {
        return mFanModeSequence == FanControl::FanModeSequenceEnum::kOffLowHighAuto ||
            mFanModeSequence == FanControl::FanModeSequenceEnum::kOffLowMedHighAuto ||
            mFanModeSequence == FanControl::FanModeSequenceEnum::kOffHighAuto;
    }
    bool SupportsRocking() const { return mOptionalAttributes.IsSet(FanControl::Attributes::RockSupport::Id); }
    bool SupportsWind() const { return mOptionalAttributes.IsSet(FanControl::Attributes::WindSupport::Id); }
    bool SupportsStep() const { return mFeatureMap.Has(FanControl::Feature::kStep); }
    bool SupportsAirflowDirection() const { return mOptionalAttributes.IsSet(FanControl::Attributes::AirflowDirection::Id); }

    void CommitFanModeOffState();

    // Updates PercentSetting (and SpeedSetting when MultiSpeed is supported) for the given
    // FanMode: Off clears settings to 0 and zeros PercentCurrent / SpeedCurrent. Low/Medium/High
    // set fixed percentages and matching speeds without changing current attributes. Auto nulls
    // settings while leaving current values unchanged.
    void ApplyFanModeSideEffects(FanControl::FanModeEnum fanMode);

    void ApplyPercentSettingChanged();
    void ApplySpeedSettingChanged();
    void ApplyNonZeroFanDrive(chip::Percent percent);

    void StoreFanModePersistence();

    void NotifyDelegateFanDriveState();

    // Used to ignore/prevent reentrance of OnFanDriveStateChanged when notifying the delegate while nested cluster updates
    // run synchronously from that callback (SetPercentSetting / SetFanMode / SetSpeedSetting).
    bool mTemporarilyIgnoreFanDriveDelegateCallbacks = false;

    // Attributes
    FanControl::FanModeEnum mFanMode = FanControl::FanModeEnum::kOff;
    FanControl::FanModeSequenceEnum mFanModeSequence;
    DataModel::Nullable<chip::Percent> mPercentSetting = DataModel::Nullable<chip::Percent>(0);
    DataModel::Nullable<uint8_t> mSpeedSetting         = DataModel::Nullable<uint8_t>(0);
    chip::Percent mPercentCurrent                      = 0;
    uint8_t mSpeedCurrent                              = 0;
    BitMask<FanControl::RockBitmap> mRockSetting;
    BitMask<FanControl::WindBitmap> mWindSetting;
    FanControl::AirflowDirectionEnum mAirflowDirection = FanControl::AirflowDirectionEnum::kForward;

    uint8_t mSpeedMax;
    BitMask<FanControl::RockBitmap> mRockSupport;
    BitMask<FanControl::WindBitmap> mWindSupport;
    OptionalAttributes mOptionalAttributes;
    BitFlags<FanControl::Feature> mFeatureMap;
    FanControl::Delegate * mDelegate;
};

} // namespace chip::app::Clusters
