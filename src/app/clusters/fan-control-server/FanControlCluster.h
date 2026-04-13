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
        Config(EndpointId endpointId, FanControl::Delegate * delegate) : mEndpointId(endpointId), mDelegate(delegate) {}

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
        FanControl::Delegate * mDelegate;
        FanControl::FanModeSequenceEnum mFanModeSequence = FanControl::FanModeSequenceEnum::kOffLowHigh;
        uint8_t mSpeedMax                                = 0;
        BitMask<FanControl::RockBitmap> mRockSupport;
        BitMask<FanControl::WindBitmap> mWindSupport;
        OptionalAttributes mOptionalAttributes;
        BitFlags<FanControl::Feature> mFeatureMap;
    };

    FanControlCluster(const Config & config);
    ~FanControlCluster() = default;

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
    DataModel::ActionReturnStatus SetFanMode(FanControl::FanModeEnum value, bool syncOnOffDelegate = true);
    DataModel::ActionReturnStatus SetPercentSetting(DataModel::Nullable<chip::Percent> value);
    DataModel::ActionReturnStatus SetSpeedSetting(DataModel::Nullable<uint8_t> value);
    DataModel::ActionReturnStatus SetRockSetting(BitMask<FanControl::RockBitmap> value);
    DataModel::ActionReturnStatus SetWindSetting(BitMask<FanControl::WindBitmap> value);
    DataModel::ActionReturnStatus SetAirflowDirection(FanControl::AirflowDirectionEnum value);

    /**
     * @brief Synchronizes the FanControl cluster with the OnOff cluster state.
     * The application must call this method whenever the OnOff cluster state changes.
     * When turned off, this sets PercentCurrent and SpeedCurrent to 0.
     * When turned on, it restores them based on the current settings.
    */
    void SetOnOffState(bool on);

    void SetDelegate(FanControl::Delegate * delegate);

private:
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

    void SetFanModeToOff();
    void ApplyFanModeOffSideEffects();
    void ApplyFanModeAutoSideEffects();

    // Mandatory attribute side effects when FanMode becomes kLow, kMedium, or kHigh.
    // PercentSetting is set to 33%, 66%, or 100% respectively; when MultiSpeed is supported,
    // SpeedSetting is set to 1, the rounded midpoint of 1..SpeedMax, or SpeedMax.
    // PercentCurrent and SpeedCurrent are updated to match only while On/Off is on (fan energized);
    // otherwise only the *Setting attributes change.
    void ApplyFanModeLowSideEffects();
    void ApplyFanModeMediumSideEffects();
    void ApplyFanModeHighSideEffects();

    void ApplyPercentSettingChanged();
    void ApplySpeedSettingChanged();

    void StoreFanModePersistence();

    void NotifyDelegateFanDriveState();

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

    bool mIsOnOffOn = false;
    uint8_t mSpeedMax;
    BitMask<FanControl::RockBitmap> mRockSupport;
    BitMask<FanControl::WindBitmap> mWindSupport;
    OptionalAttributes mOptionalAttributes;
    BitFlags<FanControl::Feature> mFeatureMap;
    FanControl::Delegate * mDelegate;
};

} // namespace chip::app::Clusters
