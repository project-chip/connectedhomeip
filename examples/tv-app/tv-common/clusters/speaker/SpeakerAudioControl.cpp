/*
 *
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

#include "speaker/SpeakerAudioControl.h"

#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/AudioControl/Enums.h>
#include <clusters/AudioControl/Ids.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AudioControl;
using chip::Protocols::InteractionModel::Status;

namespace {

// MA-speaker (tv-app.zap) is endpoint 2.
constexpr EndpointId kSpeakerEndpointId = 2;

// Initial Volume reported at first boot. Kept in step with the Speaker endpoint's
// LevelControl CurrentLevel default (tv-app.zap declares 100) even though the two
// clusters are not wired together on this endpoint. Within [MinDeviceVolume,
// MaxDeviceVolume] = [1, 100] from the delegate's default hardware limits below.
constexpr uint16_t kInitialVolume = 100;

// No hardware speaker is attached to the tv-app; this delegate just logs the changes
// the cluster asks for and reports success. Fixed hardware limits are the
// AudioControlDelegate defaults (MinDeviceVolume 1, MaxDeviceVolume 100,
// MinCorrection -10, MaxCorrection +10), except MaxDeviceVolumeDB which is given a
// plausible value since that attribute is exposed here.
class SpeakerAudioControlDelegate : public AudioControlDelegate
{
public:
    // 100.00 dB SPL, encoded as SoundPressure (dB SPL x 100). Reported by the
    // MaxDeviceVolumeDB attribute (present because the Decibel feature is not set).
    uint16_t GetMaxDeviceVolumeDB() const override { return 10000; }

    Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override
    {
        ChipLogProgress(Zcl, "TV Speaker AudioControl: volume=%u softMuted=%u", newVolume, static_cast<unsigned>(newSoftMuted));
        return Status::Success;
    }

    Status HandleBassChanged(int16_t bass) override
    {
        ChipLogProgress(Zcl, "TV Speaker AudioControl: bass=%d", bass);
        return Status::Success;
    }

    Status HandleMidChanged(int16_t mid) override
    {
        ChipLogProgress(Zcl, "TV Speaker AudioControl: mid=%d", mid);
        return Status::Success;
    }

    Status HandleTrebleChanged(int16_t treble) override
    {
        ChipLogProgress(Zcl, "TV Speaker AudioControl: treble=%d", treble);
        return Status::Success;
    }
};

SpeakerAudioControlDelegate gDelegate;
LazyRegisteredServerCluster<AudioControlCluster> gAudioControlCluster;

} // namespace

namespace chip::app::Clusters::Speaker {

void InitAudioControl()
{
    // BasicEqualizer feature: FeatureMap 0x02. Every optional attribute the cluster supports
    // is enabled: the three EQ bands (Bass/Mid/Treble), plus PhysicallyMuted, MaxUserVolume,
    // MaxDeviceVolumeDB (allowed because the Decibel feature is off), StartUpMuted and
    // StartUpVolume. MinCorrection/MaxCorrection are exposed by the feature bit alone and are
    // not part of the optional attribute set.
    AudioControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<Attributes::PhysicallyMuted::Id>()
        .Set<Attributes::MaxDeviceVolumeDB::Id>()
        .Set<Attributes::MaxUserVolume::Id>()
        .Set<Attributes::StartUpMuted::Id>()
        .Set<Attributes::StartUpVolume::Id>()
        .Set<Attributes::Bass::Id>()
        .Set<Attributes::Mid::Id>()
        .Set<Attributes::Treble::Id>();

    AudioControlCluster::Config config;
    config.WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer));
    config.WithOptionalAttributes(optionalAttributeSet);
    config.WithInitialVolume(kInitialVolume);
    config.WithInitialPhysicallyMuted(false);
    // Spec fallback for MaxUserVolume is MaxDeviceVolume, i.e. no additional user-facing cap.
    config.WithInitialMaxUserVolume(100);
    // Spec fallbacks: StartUpMuted FALSE, StartUpVolume null (retain the pre-reboot volume).
    config.WithInitialStartUpMuted(DataModel::MakeNullable(false));
    config.WithInitialStartUpVolume(DataModel::NullNullable);

    gAudioControlCluster.Create(kSpeakerEndpointId, gDelegate, config);

    SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();
    CHIP_ERROR err                                 = registry.Register(gAudioControlCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Linux App: Speaker AudioControl cluster registration failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ShutdownAudioControl()
{
    if (gAudioControlCluster.IsConstructed())
    {
        SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();
        LogErrorOnFailure(registry.Unregister(&gAudioControlCluster.Cluster()));
        gAudioControlCluster.Destroy();
    }
}

} // namespace chip::app::Clusters::Speaker
