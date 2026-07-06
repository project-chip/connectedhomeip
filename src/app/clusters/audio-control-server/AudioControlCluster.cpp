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

#include <app/clusters/audio-control-server/AudioControlCluster.h>

#include <algorithm>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/data-model/Decode.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/AudioControl/Commands.h>
#include <clusters/AudioControl/Metadata.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

using namespace AudioControl;
using namespace AudioControl::Attributes;
using namespace AudioControl::Commands;
using chip::Protocols::InteractionModel::Status;

AudioControlCluster::AudioControlCluster(EndpointId endpointId, AudioControlDelegate & delegate, const Config & config) :
    DefaultServerCluster({ endpointId, AudioControl::Id }), mDelegate(delegate), mFeatures(config.mFeatures),
    mOptionalAttributeSet(config.mOptionalAttributeSet), mMinDeviceVolume(config.mMinDeviceVolume),
    mMaxDeviceVolume(config.mMaxDeviceVolume), mMaxDeviceVolumeDB(config.mMaxDeviceVolumeDB), mMinCorrection(config.mMinCorrection),
    mMaxCorrection(config.mMaxCorrection)
{
    VerifyOrDie(mMinDeviceVolume >= 1);
    VerifyOrDie(mMaxDeviceVolume >= mMinDeviceVolume);
    VerifyOrDie(config.mVolume == 0 || (config.mVolume >= mMinDeviceVolume && config.mVolume <= mMaxDeviceVolume));

    if (mFeatures.Has(Feature::kBasicEqualizer))
    {
        VerifyOrDie(mMinCorrection <= 0);
        VerifyOrDie(mMaxCorrection >= 0);
        // BEQ feature requires at least one EQ band to be enabled.
        VerifyOrDie(mOptionalAttributeSet.IsSet(Bass::Id) || mOptionalAttributeSet.IsSet(Mid::Id) ||
                    mOptionalAttributeSet.IsSet(Treble::Id));
    }

    if (mFeatures.Has(Feature::kDecibel))
    {
        VerifyOrDie(!mOptionalAttributeSet.IsSet(MaxDeviceVolumeDB::Id));
    }

    mSoftMuted                  = config.mSoftMuted;
    mPhysicallyMuted            = config.mPhysicallyMuted;
    mVolume                     = config.mVolume;
    mDefaultStepSize            = config.mDefaultStepSize;
    mSetVolumeUnmutePolicy      = config.mSetVolumeUnmutePolicy;
    mIncreaseVolumeUnmutePolicy = config.mIncreaseVolumeUnmutePolicy;
    mIncreaseVolumeUnmuteVolume = config.mIncreaseVolumeUnmuteVolume;
    mDecreaseVolumeUnmutePolicy = config.mDecreaseVolumeUnmutePolicy;
    mStartUpMuted               = config.mStartUpMuted;
    mStartUpVolume              = config.mStartUpVolume;
    mBass                       = config.mBass;
    mMid                        = config.mMid;
    mTreble                     = config.mTreble;

    mMaxUserVolume = mOptionalAttributeSet.IsSet(MaxUserVolume::Id) ? config.mMaxUserVolume : mMaxDeviceVolume;
    VerifyOrDie(mMaxUserVolume >= mMinDeviceVolume && mMaxUserVolume <= mMaxDeviceVolume);
}

CHIP_ERROR AudioControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    // MaxUserVolume must be restored before the volume clamp (EffectiveMaxVolume() depends on it).
    if (mOptionalAttributeSet.IsSet(MaxUserVolume::Id))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::MaxUserVolume::Id), mMaxUserVolume,
            mMaxUserVolume);
    }

    // Restore last-known Volume (always; serves as fallback when StartUpVolume is null or absent).
    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Volume::Id),
                                               mVolume, mVolume);

    // Restore behavioral configuration.
    // For enum attributes, capture the config default before loading so that a stale/corrupted
    // KVS entry that decodes to kUnknownEnumValue falls back to the config default rather than
    // leaving the enum in an invalid state that would break command handling.
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::DefaultStepSize::Id), mDefaultStepSize,
        mDefaultStepSize);

    const auto defaultSetVolumePolicy = mSetVolumeUnmutePolicy;
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::SetVolumeUnmutePolicy::Id), mSetVolumeUnmutePolicy,
        defaultSetVolumePolicy);
    if (mSetVolumeUnmutePolicy >= UnmutePolicyEnum::kUnknownEnumValue)
        mSetVolumeUnmutePolicy = defaultSetVolumePolicy;

    const auto defaultIncreaseVolumePolicy = mIncreaseVolumeUnmutePolicy;
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::IncreaseVolumeUnmutePolicy::Id),
        mIncreaseVolumeUnmutePolicy, defaultIncreaseVolumePolicy);
    if (mIncreaseVolumeUnmutePolicy >= UnmutePolicyEnum::kUnknownEnumValue)
        mIncreaseVolumeUnmutePolicy = defaultIncreaseVolumePolicy;

    const auto defaultIncreaseVolumeVolume = mIncreaseVolumeUnmuteVolume;
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::IncreaseVolumeUnmuteVolume::Id),
        mIncreaseVolumeUnmuteVolume, defaultIncreaseVolumeVolume);
    if (mIncreaseVolumeUnmuteVolume >= UnmuteVolumeEnum::kUnknownEnumValue)
        mIncreaseVolumeUnmuteVolume = defaultIncreaseVolumeVolume;

    const auto defaultDecreaseVolumePolicy = mDecreaseVolumeUnmutePolicy;
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::DecreaseVolumeUnmutePolicy::Id),
        mDecreaseVolumeUnmutePolicy, defaultDecreaseVolumePolicy);
    if (mDecreaseVolumeUnmutePolicy >= UnmutePolicyEnum::kUnknownEnumValue)
        mDecreaseVolumeUnmutePolicy = defaultDecreaseVolumePolicy;

    // Apply StartUpMuted:
    //   absent  → mSoftMuted stays at the config default (false); persisted state is not loaded.
    //   null    → load and retain the persisted SoftMuted state.
    //   non-null → load persisted state, then override with the StartUpMuted value.
    if (mOptionalAttributeSet.IsSet(StartUpMuted::Id))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::SoftMuted::Id), mSoftMuted, mSoftMuted);
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::StartUpMuted::Id), mStartUpMuted, mStartUpMuted);
        if (!mStartUpMuted.IsNull())
        {
            mSoftMuted = mStartUpMuted.Value();
        }
    }

    // Apply StartUpVolume: absent → mVolume stays at the persisted Volume (null-like behavior).
    if (mOptionalAttributeSet.IsSet(StartUpVolume::Id))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::StartUpVolume::Id), mStartUpVolume,
            mStartUpVolume);
        if (!mStartUpVolume.IsNull())
        {
            mVolume = mStartUpVolume.Value();
        }
    }
    mVolume = std::clamp(mVolume, mMinDeviceVolume, EffectiveMaxVolume());

    if (mFeatures.Has(Feature::kBasicEqualizer))
    {
        if (mOptionalAttributeSet.IsSet(Bass::Id))
        {
            attributePersistence.LoadNativeEndianValue(
                ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Bass::Id), mBass, mBass);
            mBass = std::clamp(mBass, mMinCorrection, mMaxCorrection);
        }
        if (mOptionalAttributeSet.IsSet(Mid::Id))
        {
            attributePersistence.LoadNativeEndianValue(
                ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Mid::Id), mMid, mMid);
            mMid = std::clamp(mMid, mMinCorrection, mMaxCorrection);
        }
        if (mOptionalAttributeSet.IsSet(Treble::Id))
        {
            attributePersistence.LoadNativeEndianValue(
                ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Treble::Id), mTreble, mTreble);
            mTreble = std::clamp(mTreble, mMinCorrection, mMaxCorrection);
        }
    }

    mDelegate.OnStartup({ mVolume, mSoftMuted, mBass, mMid, mTreble });

    return CHIP_NO_ERROR;
}

uint16_t AudioControlCluster::EffectiveMaxVolume() const
{
    return mOptionalAttributeSet.IsSet(MaxUserVolume::Id) ? mMaxUserVolume : mMaxDeviceVolume;
}

void AudioControlCluster::StoreVolume()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Volume::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mVolume), sizeof(mVolume))));
}

void AudioControlCluster::StoreSoftMuted()
{
    VerifyOrReturn(mContext != nullptr);
    // SoftMuted is only ever loaded back in Startup() when StartUpMuted is supported (it feeds the
    // null-retain behavior); skip the write otherwise to avoid pointless flash wear.
    VerifyOrReturn(mOptionalAttributeSet.IsSet(Attributes::StartUpMuted::Id));
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::SoftMuted::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mSoftMuted), sizeof(mSoftMuted))));
}

void AudioControlCluster::StoreStartUpMuted()
{
    VerifyOrReturn(mContext != nullptr);
    NumericAttributeTraits<bool>::StorageType storageValue;
    DataModel::NullableToStorage(mStartUpMuted, storageValue);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::StartUpMuted::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
}

void AudioControlCluster::StoreStartUpVolume()
{
    VerifyOrReturn(mContext != nullptr);
    NumericAttributeTraits<uint16_t>::StorageType storageValue;
    DataModel::NullableToStorage(mStartUpVolume, storageValue);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::StartUpVolume::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
}

void AudioControlCluster::StoreDefaultStepSize()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::DefaultStepSize::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mDefaultStepSize), sizeof(mDefaultStepSize))));
}

void AudioControlCluster::StoreSetVolumeUnmutePolicy()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::SetVolumeUnmutePolicy::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mSetVolumeUnmutePolicy), sizeof(mSetVolumeUnmutePolicy))));
}

void AudioControlCluster::StoreIncreaseVolumeUnmutePolicy()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::IncreaseVolumeUnmutePolicy::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mIncreaseVolumeUnmutePolicy), sizeof(mIncreaseVolumeUnmutePolicy))));
}

void AudioControlCluster::StoreIncreaseVolumeUnmuteVolume()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::IncreaseVolumeUnmuteVolume::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mIncreaseVolumeUnmuteVolume), sizeof(mIncreaseVolumeUnmuteVolume))));
}

void AudioControlCluster::StoreDecreaseVolumeUnmutePolicy()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::DecreaseVolumeUnmutePolicy::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mDecreaseVolumeUnmutePolicy), sizeof(mDecreaseVolumeUnmutePolicy))));
}

void AudioControlCluster::StoreMaxUserVolume()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::MaxUserVolume::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mMaxUserVolume), sizeof(mMaxUserVolume))));
}

void AudioControlCluster::StoreBass()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Bass::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mBass), sizeof(mBass))));
}

void AudioControlCluster::StoreMid()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Mid::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mMid), sizeof(mMid))));
}

void AudioControlCluster::StoreTreble()
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, AudioControl::Id, Attributes::Treble::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mTreble), sizeof(mTreble))));
}

DataModel::ActionReturnStatus AudioControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(AudioControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case SoftMuted::Id:
        return encoder.Encode(mSoftMuted);
    case PhysicallyMuted::Id:
        return encoder.Encode(mPhysicallyMuted);
    case Volume::Id:
        return encoder.Encode(mVolume);
    case MinDeviceVolume::Id:
        return encoder.Encode(mMinDeviceVolume);
    case MaxDeviceVolume::Id:
        return encoder.Encode(mMaxDeviceVolume);
    case MaxDeviceVolumeDB::Id:
        return encoder.Encode(mMaxDeviceVolumeDB);
    case MaxUserVolume::Id:
        return encoder.Encode(mMaxUserVolume);
    case DefaultStepSize::Id:
        return encoder.Encode(mDefaultStepSize);
    case SetVolumeUnmutePolicy::Id:
        return encoder.Encode(mSetVolumeUnmutePolicy);
    case IncreaseVolumeUnmutePolicy::Id:
        return encoder.Encode(mIncreaseVolumeUnmutePolicy);
    case IncreaseVolumeUnmuteVolume::Id:
        return encoder.Encode(mIncreaseVolumeUnmuteVolume);
    case DecreaseVolumeUnmutePolicy::Id:
        return encoder.Encode(mDecreaseVolumeUnmutePolicy);
    case StartUpMuted::Id:
        return encoder.Encode(mStartUpMuted);
    case StartUpVolume::Id:
        return encoder.Encode(mStartUpVolume);
    case Bass::Id:
        return encoder.Encode(mBass);
    case Mid::Id:
        return encoder.Encode(mMid);
    case Treble::Id:
        return encoder.Encode(mTreble);
    case MinCorrection::Id:
        return encoder.Encode(mMinCorrection);
    case MaxCorrection::Id:
        return encoder.Encode(mMaxCorrection);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus AudioControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case MaxUserVolume::Id: {
        uint16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        if (value == mMaxUserVolume)
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        return SetMaxUserVolume(value);
    }
    case DefaultStepSize::Id: {
        uint16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        const uint16_t effectiveMax = EffectiveMaxVolume();
        VerifyOrReturnError(value >= 1 && effectiveMax > mMinDeviceVolume && value <= effectiveMax - mMinDeviceVolume,
                            Status::ConstraintError);
        if (!SetAttributeValue(mDefaultStepSize, value, DefaultStepSize::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreDefaultStepSize();
        return CHIP_NO_ERROR;
    }
    case SetVolumeUnmutePolicy::Id: {
        UnmutePolicyEnum value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value != UnmutePolicyEnum::kUnknownEnumValue, Status::ConstraintError);
        if (!SetAttributeValue(mSetVolumeUnmutePolicy, value, SetVolumeUnmutePolicy::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreSetVolumeUnmutePolicy();
        return CHIP_NO_ERROR;
    }
    case IncreaseVolumeUnmutePolicy::Id: {
        UnmutePolicyEnum value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value != UnmutePolicyEnum::kUnknownEnumValue, Status::ConstraintError);
        if (!SetAttributeValue(mIncreaseVolumeUnmutePolicy, value, IncreaseVolumeUnmutePolicy::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreIncreaseVolumeUnmutePolicy();
        return CHIP_NO_ERROR;
    }
    case IncreaseVolumeUnmuteVolume::Id: {
        UnmuteVolumeEnum value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value != UnmuteVolumeEnum::kUnknownEnumValue, Status::ConstraintError);
        if (!SetAttributeValue(mIncreaseVolumeUnmuteVolume, value, IncreaseVolumeUnmuteVolume::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreIncreaseVolumeUnmuteVolume();
        return CHIP_NO_ERROR;
    }
    case DecreaseVolumeUnmutePolicy::Id: {
        UnmutePolicyEnum value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value != UnmutePolicyEnum::kUnknownEnumValue, Status::ConstraintError);
        if (!SetAttributeValue(mDecreaseVolumeUnmutePolicy, value, DecreaseVolumeUnmutePolicy::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreDecreaseVolumeUnmutePolicy();
        return CHIP_NO_ERROR;
    }
    case StartUpMuted::Id: {
        DataModel::Nullable<bool> value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        if (!SetAttributeValue(mStartUpMuted, value, StartUpMuted::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreStartUpMuted();
        return CHIP_NO_ERROR;
    }
    case StartUpVolume::Id: {
        DataModel::Nullable<uint16_t> value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        if (!value.IsNull())
        {
            const uint16_t effectiveMax = EffectiveMaxVolume();
            VerifyOrReturnError(value.Value() >= mMinDeviceVolume && value.Value() <= effectiveMax, Status::ConstraintError);
        }
        if (!SetAttributeValue(mStartUpVolume, value, StartUpVolume::Id))
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        StoreStartUpVolume();
        return CHIP_NO_ERROR;
    }
    case Bass::Id: {
        int16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= mMinCorrection && value <= mMaxCorrection, Status::ConstraintError);
        if (value == mBass)
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        {
            Status s = mDelegate.HandleBassChanged(value);
            VerifyOrReturnError(s == Status::Success, s);
        }
        SetAttributeValue(mBass, value, Bass::Id);
        StoreBass();
        return CHIP_NO_ERROR;
    }
    case Mid::Id: {
        int16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= mMinCorrection && value <= mMaxCorrection, Status::ConstraintError);
        if (value == mMid)
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        {
            Status s = mDelegate.HandleMidChanged(value);
            VerifyOrReturnError(s == Status::Success, s);
        }
        SetAttributeValue(mMid, value, Mid::Id);
        StoreMid();
        return CHIP_NO_ERROR;
    }
    case Treble::Id: {
        int16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnError(value >= mMinCorrection && value <= mMaxCorrection, Status::ConstraintError);
        if (value == mTreble)
            return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
        {
            Status s = mDelegate.HandleTrebleChanged(value);
            VerifyOrReturnError(s == Status::Success, s);
        }
        SetAttributeValue(mTreble, value, Treble::Id);
        StoreTreble();
        return CHIP_NO_ERROR;
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR AudioControlCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(PhysicallyMuted::Id), PhysicallyMuted::kMetadataEntry },
        { !mFeatures.Has(Feature::kDecibel), MaxDeviceVolumeDB::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(MaxUserVolume::Id), MaxUserVolume::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(StartUpMuted::Id), StartUpMuted::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(StartUpVolume::Id), StartUpVolume::kMetadataEntry },
        { mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Bass::Id), Bass::kMetadataEntry },
        { mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Mid::Id), Mid::kMetadataEntry },
        { mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Treble::Id), Treble::kMetadataEntry },
        { mFeatures.Has(Feature::kBasicEqualizer), MinCorrection::kMetadataEntry },
        { mFeatures.Has(Feature::kBasicEqualizer), MaxCorrection::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR AudioControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        Mute::kMetadataEntry,      Unmute::kMetadataEntry,         ToggleMuted::kMetadataEntry,
        SetVolume::kMetadataEntry, IncreaseVolume::kMetadataEntry, DecreaseVolume::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

// Applies the UnmutePolicy table (spec §ref_UnmutePolicyTable).
// Computes final (volume, softMuted), notifies the delegate, then updates cluster state.
// Only called when at least one of SoftMuted or PhysicallyMuted is TRUE.
Status AudioControlCluster::ApplyUnmutePolicy(UnmutePolicyEnum policy, uint16_t requestedVolume)
{
    uint16_t newVolume = mVolume;
    bool newSoftMuted  = mSoftMuted;

    switch (policy)
    {
    case UnmutePolicyEnum::kUnmuteOrChangeVolume:
        // Always set Volume and clear SoftMuted, regardless of PhysicallyMuted.
        newVolume    = requestedVolume;
        newSoftMuted = false;
        break;

    case UnmutePolicyEnum::kUnmuteOrDoNotChangeVolume:
        if (mPhysicallyMuted)
        {
            // Cannot unmute a physically-muted device — leave state unchanged.
            return Status::InvalidInState;
        }
        // PhysicallyMuted is FALSE; SoftMuted must have been TRUE (we're in muted state).
        newVolume    = requestedVolume;
        newSoftMuted = false;
        break;

    case UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume:
        // Change volume but leave SoftMuted unchanged.
        newVolume = requestedVolume;
        break;

    case UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume:
        // Leave all state unchanged.
        return Status::InvalidInState;

    default:
        // Not a valid UnmutePolicyEnum value — malformed command field, not a state conflict.
        return Status::ConstraintError;
    }

    Status s = mDelegate.HandleVolumeAndMuteChange(newVolume, newSoftMuted);
    VerifyOrReturnError(s == Status::Success, s);

    if (SetAttributeValue(mVolume, newVolume, Volume::Id))
        StoreVolume();
    if (SetAttributeValue(mSoftMuted, newSoftMuted, SoftMuted::Id))
        StoreSoftMuted();
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AudioControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Mute::Id:
        return HandleMute();
    case Unmute::Id:
        return HandleUnmute();
    case ToggleMuted::Id:
        return HandleToggleMuted();
    case SetVolume::Id:
        return HandleSetVolume(input_arguments);
    case IncreaseVolume::Id:
        return HandleIncreaseVolume(input_arguments);
    case DecreaseVolume::Id:
        return HandleDecreaseVolume(input_arguments);
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus AudioControlCluster::HandleMute()
{
    Status s = mDelegate.HandleVolumeAndMuteChange(mVolume, true);
    VerifyOrReturnError(s == Status::Success, s);
    if (SetAttributeValue(mSoftMuted, true, SoftMuted::Id))
        StoreSoftMuted();
    return Status::Success;
}

DataModel::ActionReturnStatus AudioControlCluster::HandleUnmute()
{
    Status s = mDelegate.HandleVolumeAndMuteChange(mVolume, false);
    VerifyOrReturnError(s == Status::Success, s);
    if (SetAttributeValue(mSoftMuted, false, SoftMuted::Id))
        StoreSoftMuted();
    return Status::Success;
}

DataModel::ActionReturnStatus AudioControlCluster::HandleToggleMuted()
{
    const bool newSoftMuted = !mSoftMuted;
    Status s                = mDelegate.HandleVolumeAndMuteChange(mVolume, newSoftMuted);
    VerifyOrReturnError(s == Status::Success, s);
    if (SetAttributeValue(mSoftMuted, newSoftMuted, SoftMuted::Id))
        StoreSoftMuted();
    return Status::Success;
}

DataModel::ActionReturnStatus AudioControlCluster::HandleSetVolume(chip::TLV::TLVReader & input_arguments)
{
    SetVolume::DecodableType req;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));

    const uint16_t effectiveMax = EffectiveMaxVolume();

    // Constraint: 0 OR [MinDeviceVolume, EffectiveMax]
    if (req.newVolume != 0 && (req.newVolume < mMinDeviceVolume || req.newVolume > effectiveMax))
    {
        return Status::ConstraintError;
    }

    // NewVolume == 0: mute and succeed.
    if (req.newVolume == 0)
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(mVolume, true);
        VerifyOrReturnError(s == Status::Success, s);
        if (SetAttributeValue(mSoftMuted, true, SoftMuted::Id))
            StoreSoftMuted();
        return Status::Success;
    }

    // Fast path: device is not muted.
    if (!mSoftMuted && !mPhysicallyMuted)
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(req.newVolume, false);
        VerifyOrReturnError(s == Status::Success, s);
        if (SetAttributeValue(mVolume, req.newVolume, Volume::Id))
            StoreVolume();
        return Status::Success;
    }

    return ApplyUnmutePolicy(req.unmutePolicy.ValueOr(mSetVolumeUnmutePolicy), req.newVolume);
}

DataModel::ActionReturnStatus AudioControlCluster::HandleIncreaseVolume(chip::TLV::TLVReader & input_arguments)
{
    IncreaseVolume::DecodableType req;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));

    const uint16_t effectiveMax = EffectiveMaxVolume();
    const uint16_t stepSize     = req.stepSize.ValueOr(mDefaultStepSize);

    // Constraint: 1 to (EffectiveMax - MinDeviceVolume). When EffectiveMax <= MinDeviceVolume the
    // valid range is empty, so any StepSize is out of range.
    const uint16_t maxStepSize = (effectiveMax > mMinDeviceVolume) ? static_cast<uint16_t>(effectiveMax - mMinDeviceVolume) : 0;
    VerifyOrReturnError(stepSize >= 1 && stepSize <= maxStepSize, Status::ConstraintError);

    const UnmutePolicyEnum policy       = req.unmutePolicy.ValueOr(mIncreaseVolumeUnmutePolicy);
    const UnmuteVolumeEnum unmuteVolume = req.unmuteVolume.ValueOr(mIncreaseVolumeUnmuteVolume);

    uint16_t requestedVolume;

    // Step 2.a: special case for soft-muted-only device with unmute policy
    if (mSoftMuted && !mPhysicallyMuted &&
        (policy == UnmutePolicyEnum::kUnmuteOrChangeVolume || policy == UnmutePolicyEnum::kUnmuteOrDoNotChangeVolume))
    {
        switch (unmuteVolume)
        {
        case UnmuteVolumeEnum::kMinDeviceVolume:
            requestedVolume = mMinDeviceVolume;
            break;
        case UnmuteVolumeEnum::kVolume:
            requestedVolume = mVolume;
            break;
        case UnmuteVolumeEnum::kVolumePlusStepSize:
            requestedVolume = static_cast<uint16_t>(
                std::min<uint32_t>(static_cast<uint32_t>(mVolume) + static_cast<uint32_t>(stepSize), effectiveMax));
            break;
        default:
            return Status::ConstraintError;
        }
    }
    else
    {
        // Step 2.b: Volume + StepSize, capped at EffectiveMax
        requestedVolume = static_cast<uint16_t>(
            std::min<uint32_t>(static_cast<uint32_t>(mVolume) + static_cast<uint32_t>(stepSize), effectiveMax));
    }

    // Step 3: fast path — device is not muted
    if (!mSoftMuted && !mPhysicallyMuted)
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(requestedVolume, false);
        VerifyOrReturnError(s == Status::Success, s);
        if (SetAttributeValue(mVolume, requestedVolume, Volume::Id))
            StoreVolume();
        return Status::Success;
    }

    // Step 4: apply UnmutePolicy table
    return ApplyUnmutePolicy(policy, requestedVolume);
}

DataModel::ActionReturnStatus AudioControlCluster::HandleDecreaseVolume(chip::TLV::TLVReader & input_arguments)
{
    DecreaseVolume::DecodableType req;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));

    const uint16_t effectiveMax = EffectiveMaxVolume();
    const uint16_t stepSize     = req.stepSize.ValueOr(mDefaultStepSize);

    // Constraint: 1 to (EffectiveMax - MinDeviceVolume). When EffectiveMax <= MinDeviceVolume the
    // valid range is empty, so any StepSize is out of range.
    const uint16_t maxStepSize = (effectiveMax > mMinDeviceVolume) ? static_cast<uint16_t>(effectiveMax - mMinDeviceVolume) : 0;
    VerifyOrReturnError(stepSize >= 1 && stepSize <= maxStepSize, Status::ConstraintError);

    // RequestedVolume = Volume - StepSize (with underflow protection)
    const uint32_t requestedVolume32 = static_cast<uint32_t>(mVolume) > static_cast<uint32_t>(stepSize)
        ? static_cast<uint32_t>(mVolume) - static_cast<uint32_t>(stepSize)
        : 0;

    // Step 3: floor at MinDeviceVolume → mute and succeed
    if (requestedVolume32 < static_cast<uint32_t>(mMinDeviceVolume))
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(mMinDeviceVolume, true);
        VerifyOrReturnError(s == Status::Success, s);
        if (SetAttributeValue(mVolume, mMinDeviceVolume, Volume::Id))
            StoreVolume();
        if (SetAttributeValue(mSoftMuted, true, SoftMuted::Id))
            StoreSoftMuted();
        return Status::Success;
    }

    const uint16_t requestedVolume = static_cast<uint16_t>(requestedVolume32);

    // Step 4: fast path — device is not muted
    if (!mSoftMuted && !mPhysicallyMuted)
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(requestedVolume, false);
        VerifyOrReturnError(s == Status::Success, s);
        if (SetAttributeValue(mVolume, requestedVolume, Volume::Id))
            StoreVolume();
        return Status::Success;
    }

    // Step 5: apply UnmutePolicy table
    return ApplyUnmutePolicy(req.unmutePolicy.ValueOr(mDecreaseVolumeUnmutePolicy), requestedVolume);
}

// Application-facing setters. SetSoftMuted/SetVolume/SetPhysicallyMuted reflect hardware-initiated
// changes and do NOT call the delegate. SetMaxUserVolume is app-initiated and DOES call
// HandleVolumeAndMuteChange when Volume must be clipped to the new ceiling.

CHIP_ERROR AudioControlCluster::SetSoftMuted(bool softMuted)
{
    if (SetAttributeValue(mSoftMuted, softMuted, SoftMuted::Id))
        StoreSoftMuted();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetPhysicallyMuted(bool physicallyMuted)
{
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(PhysicallyMuted::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    SetAttributeValue(mPhysicallyMuted, physicallyMuted, PhysicallyMuted::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetVolume(uint16_t volume)
{
    VerifyOrReturnError(volume >= mMinDeviceVolume && volume <= EffectiveMaxVolume(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mVolume, volume, Volume::Id))
        StoreVolume();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetMaxUserVolume(uint16_t maxUserVolume)
{
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(MaxUserVolume::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(maxUserVolume >= mMinDeviceVolume && maxUserVolume <= mMaxDeviceVolume,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // If Volume must be clipped, inform hardware before committing any state change
    // so that a delegate failure leaves the cluster in a consistent state.
    if (mVolume > maxUserVolume)
    {
        Status s = mDelegate.HandleVolumeAndMuteChange(maxUserVolume, mSoftMuted);
        ReturnErrorOnFailure(StatusIB(s).ToChipError());
    }

    if (SetAttributeValue(mMaxUserVolume, maxUserVolume, MaxUserVolume::Id))
        StoreMaxUserVolume();

    if (mVolume > maxUserVolume)
    {
        SetAttributeValue(mVolume, maxUserVolume, Volume::Id);
        StoreVolume();
    }

    if (mOptionalAttributeSet.IsSet(StartUpVolume::Id) && !mStartUpVolume.IsNull() && mStartUpVolume.Value() > maxUserVolume)
    {
        SetAttributeValue(mStartUpVolume, DataModel::MakeNullable<uint16_t>(maxUserVolume), StartUpVolume::Id);
        StoreStartUpVolume();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetBass(int16_t bass)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Bass::Id),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(bass >= mMinCorrection && bass <= mMaxCorrection, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mBass, bass, Bass::Id))
        StoreBass();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetMid(int16_t mid)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Mid::Id),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mid >= mMinCorrection && mid <= mMaxCorrection, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mMid, mid, Mid::Id))
        StoreMid();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioControlCluster::SetTreble(int16_t treble)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kBasicEqualizer) && mOptionalAttributeSet.IsSet(Treble::Id),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(treble >= mMinCorrection && treble <= mMaxCorrection, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mTreble, treble, Treble::Id))
        StoreTreble();
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
