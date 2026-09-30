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

#include "speaker/SpeakerEndpoint.h"

#include "speaker/SpeakerAudioCoordinator.h"

#include <cstdint>
#include <optional>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/AudioControl/Enums.h>
#include <clusters/AudioControl/Ids.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AudioControl;
using chip::Protocols::InteractionModel::Status;

namespace {

// MA-speaker (tv-app.zap) is endpoint 2.
constexpr EndpointId kSpeakerEndpointId = 2;

// Initial Volume reported at first boot (before anything is persisted). Within
// [MinDeviceVolume, MaxDeviceVolume] = [1, 100] from the hardware delegate's defaults.
constexpr uint16_t kInitialVolume = 100;

DefaultTimerDelegate gTimerDelegate;

// No hardware speaker is attached to the tv-app; this delegate logs the audio notifications
// the coordinator forwards and reports success. Fixed limits are the AudioControlDelegate
// defaults (MinDeviceVolume 1, MaxDeviceVolume 100, MinCorrection -10, MaxCorrection +10),
// except MaxDeviceVolumeDB which is exposed here and gets a plausible value.
class SpeakerHardwareDelegate : public AudioControlDelegate
{
public:
    // 100.00 dB SPL, encoded as SoundPressure (dB SPL x 100).
    uint16_t GetMaxDeviceVolumeDB() const override { return 10000; }

    Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override
    {
        ChipLogProgress(Zcl, "TV Speaker: volume=%u softMuted=%u", newVolume, static_cast<unsigned>(newSoftMuted));
        return Status::Success;
    }

    Status HandleBassChanged(int16_t bass) override
    {
        ChipLogProgress(Zcl, "TV Speaker: bass=%d", bass);
        return Status::Success;
    }

    Status HandleMidChanged(int16_t mid) override
    {
        ChipLogProgress(Zcl, "TV Speaker: mid=%d", mid);
        return Status::Success;
    }

    Status HandleTrebleChanged(int16_t treble) override
    {
        ChipLogProgress(Zcl, "TV Speaker: treble=%d", treble);
        return Status::Success;
    }
};

SpeakerHardwareDelegate gHardwareDelegate;

// Constructed in InitSpeaker() (it needs the ember-backed ZAP reads, which are only valid
// after Server::Init()), torn down in ShutdownSpeaker(). Owns the three code-driven clusters.
std::optional<chip::app::Clusters::Speaker::SpeakerAudioCoordinator> gCoordinator;

} // namespace

namespace chip::app::Clusters::Speaker {

void InitSpeaker()
{
    VerifyOrReturn(!gCoordinator.has_value());

    // Seed first-boot state from tv-app.zap's declared defaults instead of literals; each
    // cluster then reloads its own persisted value in Startup() and only falls back to this.
    // The ember-backed accessors are tv-app-specific -- a dynamic-endpoint owner passes
    // literals instead.
    SpeakerAudioCoordinator::Config config{ .endpoint         = kSpeakerEndpointId,
                                            .timerDelegate    = gTimerDelegate,
                                            .hardwareDelegate = gHardwareDelegate };

    if (OnOff::Attributes::OnOff::Get(kSpeakerEndpointId, &config.onOffStartValue) != Status::Success)
    {
        config.onOffStartValue = false;
    }
    if (LevelControl::Attributes::OnOffTransitionTime::Get(kSpeakerEndpointId, &config.levelControlOnOffTransitionTime) !=
        Status::Success)
    {
        config.levelControlOnOffTransitionTime = 0;
    }
    if (LevelControl::Attributes::OnLevel::Get(kSpeakerEndpointId, config.levelControlOnLevel) != Status::Success)
    {
        config.levelControlOnLevel.SetNull();
    }
    if (LevelControl::Attributes::OnTransitionTime::Get(kSpeakerEndpointId, config.levelControlOnTransitionTime) != Status::Success)
    {
        config.levelControlOnTransitionTime.SetNull();
    }
    if (LevelControl::Attributes::OffTransitionTime::Get(kSpeakerEndpointId, config.levelControlOffTransitionTime) !=
        Status::Success)
    {
        config.levelControlOffTransitionTime.SetNull();
    }
    if (LevelControl::Attributes::DefaultMoveRate::Get(kSpeakerEndpointId, config.levelControlDefaultMoveRate) != Status::Success)
    {
        config.levelControlDefaultMoveRate.SetNull();
    }
    if (LevelControl::Attributes::StartUpCurrentLevel::Get(kSpeakerEndpointId, config.levelControlStartUpCurrentLevel) !=
        Status::Success)
    {
        config.levelControlStartUpCurrentLevel.SetNull();
    }
    // Initial CurrentLevel. Audio Control's OnStartup() reconcile then moves it to match the
    // resolved Volume, so this only holds until that runs.
    DataModel::Nullable<uint8_t> currentLevel;
    if (LevelControl::Attributes::CurrentLevel::Get(kSpeakerEndpointId, currentLevel) != Status::Success || currentLevel.IsNull())
    {
        currentLevel.SetNonNull(LevelControlCluster::kLightingMinLevel);
    }
    config.levelControlInitialCurrentLevel = currentLevel.Value();

    // Audio Control: BasicEqualizer feature (FeatureMap 0x02) with every optional attribute
    // enabled. No delegate in this config, so it is built here.
    AudioControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<Attributes::PhysicallyMuted::Id>()
        .Set<Attributes::MaxDeviceVolumeDB::Id>()
        .Set<Attributes::MaxUserVolume::Id>()
        .Set<Attributes::StartUpMuted::Id>()
        .Set<Attributes::StartUpVolume::Id>()
        .Set<Attributes::Bass::Id>()
        .Set<Attributes::Mid::Id>()
        .Set<Attributes::Treble::Id>();

    AudioControlCluster::Config audioConfig;
    audioConfig.WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer));
    audioConfig.WithOptionalAttributes(optionalAttributeSet);
    audioConfig.WithInitialVolume(kInitialVolume);
    audioConfig.WithInitialPhysicallyMuted(false);
    audioConfig.WithInitialMaxUserVolume(100);
    audioConfig.WithInitialStartUpMuted(DataModel::MakeNullable(false));
    audioConfig.WithInitialStartUpVolume(DataModel::NullNullable);
    config.audioControl = audioConfig;

    gCoordinator.emplace(config);

    // Register. Registry::Register() starts each cluster immediately (Server::Init() has run);
    // the coordinator's constructor has already wired all three. Audio Control is registered
    // last so its Startup() -> OnStartup() reconcile sees the other two started.
    SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();
    LogErrorOnFailure(registry.Register(gCoordinator->OnOffRegistration()));
    LogErrorOnFailure(registry.Register(gCoordinator->LevelControlRegistration()));
    LogErrorOnFailure(registry.Register(gCoordinator->AudioControlRegistration()));
}

void ShutdownSpeaker()
{
    VerifyOrReturn(gCoordinator.has_value());

    // Unregister in reverse order while the clusters are still alive, then destroy the
    // coordinator (its destructor detaches the On/Off delegates and tears down the clusters).
    SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();
    LogErrorOnFailure(registry.Unregister(&gCoordinator->GetAudioControl()));
    LogErrorOnFailure(registry.Unregister(&gCoordinator->GetLevelControl()));
    LogErrorOnFailure(registry.Unregister(&gCoordinator->GetOnOff()));

    gCoordinator.reset();
}

} // namespace chip::app::Clusters::Speaker
