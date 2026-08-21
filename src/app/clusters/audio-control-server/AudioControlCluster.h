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

#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/AudioControl/Enums.h>
#include <clusters/AudioControl/Metadata.h>
#include <lib/support/BitFlags.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters {

class AudioControlCluster : public DefaultServerCluster
{
public:
    /// Bitset of optional attributes that can be individually enabled.
    /// Feature-gated attributes (MinCorrection, MaxCorrection) are controlled by the feature map,
    /// not this set.
    using OptionalAttributeSet =
        app::OptionalAttributeSet<AudioControl::Attributes::PhysicallyMuted::Id, AudioControl::Attributes::MaxDeviceVolumeDB::Id,
                                  AudioControl::Attributes::MaxUserVolume::Id, AudioControl::Attributes::StartUpMuted::Id,
                                  AudioControl::Attributes::StartUpVolume::Id, AudioControl::Attributes::Bass::Id,
                                  AudioControl::Attributes::Mid::Id, AudioControl::Attributes::Treble::Id>;

    class Config
    {
    public:
        Config & WithFeatures(BitFlags<AudioControl::Feature> features)
        {
            mFeatures = features;
            return *this;
        }
        Config & WithMinDeviceVolume(uint16_t value)
        {
            mMinDeviceVolume = value;
            return *this;
        }
        Config & WithMaxDeviceVolume(uint16_t value)
        {
            mMaxDeviceVolume = value;
            return *this;
        }
        Config & WithMaxDeviceVolumeDB(uint16_t value)
        {
            mMaxDeviceVolumeDB = value;
            return *this;
        }
        Config & WithCorrectionRange(int16_t minCorrection, int16_t maxCorrection)
        {
            mMinCorrection = minCorrection;
            mMaxCorrection = maxCorrection;
            return *this;
        }
        Config & WithOptionalAttributes(OptionalAttributeSet attrs)
        {
            mOptionalAttributeSet = attrs;
            return *this;
        }
        Config & WithInitialSoftMuted(bool value)
        {
            mSoftMuted = value;
            return *this;
        }
        Config & WithInitialPhysicallyMuted(bool value)
        {
            mPhysicallyMuted = value;
            return *this;
        }
        Config & WithInitialVolume(uint16_t value)
        {
            mVolume = value;
            return *this;
        }
        Config & WithInitialMaxUserVolume(uint16_t value)
        {
            mMaxUserVolume = value;
            return *this;
        }
        Config & WithInitialDefaultStepSize(uint16_t value)
        {
            mDefaultStepSize = value;
            return *this;
        }
        Config & WithSetVolumeUnmutePolicy(AudioControl::UnmutePolicyEnum value)
        {
            mSetVolumeUnmutePolicy = value;
            return *this;
        }
        Config & WithIncreaseVolumeUnmutePolicy(AudioControl::UnmutePolicyEnum value)
        {
            mIncreaseVolumeUnmutePolicy = value;
            return *this;
        }
        Config & WithIncreaseVolumeUnmuteVolume(AudioControl::UnmuteVolumeEnum value)
        {
            mIncreaseVolumeUnmuteVolume = value;
            return *this;
        }
        Config & WithDecreaseVolumeUnmutePolicy(AudioControl::UnmutePolicyEnum value)
        {
            mDecreaseVolumeUnmutePolicy = value;
            return *this;
        }
        Config & WithInitialStartUpMuted(DataModel::Nullable<bool> value)
        {
            mStartUpMuted = value;
            return *this;
        }
        Config & WithInitialStartUpVolume(DataModel::Nullable<uint16_t> value)
        {
            mStartUpVolume = value;
            return *this;
        }
        Config & WithInitialBass(int16_t value)
        {
            mBass = value;
            return *this;
        }
        Config & WithInitialMid(int16_t value)
        {
            mMid = value;
            return *this;
        }
        Config & WithInitialTreble(int16_t value)
        {
            mTreble = value;
            return *this;
        }

    private:
        friend class AudioControlCluster;

        BitFlags<AudioControl::Feature> mFeatures;
        OptionalAttributeSet mOptionalAttributeSet;

        uint16_t mMinDeviceVolume   = 1;
        uint16_t mMaxDeviceVolume   = 100;
        uint16_t mMaxDeviceVolumeDB = 0;
        int16_t mMinCorrection      = -10;
        int16_t mMaxCorrection      = 10;

        bool mSoftMuted           = false;
        bool mPhysicallyMuted     = false;
        uint16_t mVolume          = 1;
        uint16_t mMaxUserVolume   = 100;
        uint16_t mDefaultStepSize = 1;

        AudioControl::UnmutePolicyEnum mSetVolumeUnmutePolicy      = AudioControl::UnmutePolicyEnum::kUnmuteOrChangeVolume;
        AudioControl::UnmutePolicyEnum mIncreaseVolumeUnmutePolicy = AudioControl::UnmutePolicyEnum::kUnmuteOrChangeVolume;
        AudioControl::UnmuteVolumeEnum mIncreaseVolumeUnmuteVolume = AudioControl::UnmuteVolumeEnum::kVolumePlusStepSize;
        AudioControl::UnmutePolicyEnum mDecreaseVolumeUnmutePolicy = AudioControl::UnmutePolicyEnum::kUnmuteOrChangeVolume;

        DataModel::Nullable<bool> mStartUpMuted;
        DataModel::Nullable<uint16_t> mStartUpVolume;
        int16_t mBass   = 0;
        int16_t mMid    = 0;
        int16_t mTreble = 0;
    };

    AudioControlCluster(EndpointId endpointId, AudioControlDelegate & delegate, const Config & config);

    // ServerClusterInterface overrides
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    // Application-facing API — call these from application code (e.g. hardware event handlers).
    // SetSoftMuted/SetVolume/SetPhysicallyMuted/SetBass/SetMid/SetTreble reflect hardware-initiated
    // changes and do NOT call the delegate. SetMaxUserVolume is app-initiated and DOES call
    // HandleVolumeAndMuteChange when Volume is clipped.
    CHIP_ERROR SetSoftMuted(bool softMuted);
    CHIP_ERROR SetPhysicallyMuted(bool physicallyMuted);
    CHIP_ERROR SetVolume(uint16_t volume);
    CHIP_ERROR SetMaxUserVolume(uint16_t maxUserVolume);
    CHIP_ERROR SetBass(int16_t bass);
    CHIP_ERROR SetMid(int16_t mid);
    CHIP_ERROR SetTreble(int16_t treble);

    bool GetSoftMuted() const { return mSoftMuted; }
    bool GetPhysicallyMuted() const { return mPhysicallyMuted; }
    uint16_t GetVolume() const { return mVolume; }
    uint16_t GetMaxUserVolume() const { return mMaxUserVolume; }
    uint16_t GetMinDeviceVolume() const { return mMinDeviceVolume; }
    uint16_t GetMaxDeviceVolume() const { return mMaxDeviceVolume; }
    int16_t GetBass() const { return mBass; }
    int16_t GetMid() const { return mMid; }
    int16_t GetTreble() const { return mTreble; }

private:
    AudioControlDelegate & mDelegate;

    const BitFlags<AudioControl::Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;

    // Fixed (construction-time) attributes
    const uint16_t mMinDeviceVolume;
    const uint16_t mMaxDeviceVolume;
    const uint16_t mMaxDeviceVolumeDB;
    const int16_t mMinCorrection;
    const int16_t mMaxCorrection;

    // Mutable attributes
    bool mSoftMuted       = false;
    bool mPhysicallyMuted = false;
    uint16_t mVolume;
    uint16_t mMaxUserVolume;
    uint16_t mDefaultStepSize;
    AudioControl::UnmutePolicyEnum mSetVolumeUnmutePolicy;
    AudioControl::UnmutePolicyEnum mIncreaseVolumeUnmutePolicy;
    AudioControl::UnmuteVolumeEnum mIncreaseVolumeUnmuteVolume;
    AudioControl::UnmutePolicyEnum mDecreaseVolumeUnmutePolicy;
    DataModel::Nullable<bool> mStartUpMuted;
    DataModel::Nullable<uint16_t> mStartUpVolume;
    int16_t mBass;
    int16_t mMid;
    int16_t mTreble;

    // Helpers
    uint16_t EffectiveMaxVolume() const;
    void StoreVolume();
    void StoreSoftMuted();
    void StoreStartUpMuted();
    void StoreStartUpVolume();
    void StoreDefaultStepSize();
    void StoreSetVolumeUnmutePolicy();
    void StoreIncreaseVolumeUnmutePolicy();
    void StoreIncreaseVolumeUnmuteVolume();
    void StoreDecreaseVolumeUnmutePolicy();
    void StoreMaxUserVolume();
    void StoreBass();
    void StoreMid();
    void StoreTreble();

    // Command handlers — one per accepted command.
    DataModel::ActionReturnStatus HandleMute();
    DataModel::ActionReturnStatus HandleUnmute();
    DataModel::ActionReturnStatus HandleToggleMuted();
    DataModel::ActionReturnStatus HandleSetVolume(chip::TLV::TLVReader & input_arguments);
    DataModel::ActionReturnStatus HandleIncreaseVolume(chip::TLV::TLVReader & input_arguments);
    DataModel::ActionReturnStatus HandleDecreaseVolume(chip::TLV::TLVReader & input_arguments);

    // Applies the UnmutePolicy table: calls delegate with computed (newVolume, newSoftMuted),
    // then updates cluster state only if the delegate succeeds.
    Protocols::InteractionModel::Status ApplyUnmutePolicy(AudioControl::UnmutePolicyEnum policy, uint16_t requestedVolume);
};

} // namespace chip::app::Clusters
