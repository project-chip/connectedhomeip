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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/clusters/audio-control-server/CodegenIntegration.h>
#include <app/static-cluster-config/AudioControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AudioControl;
using namespace chip::app::Clusters::AudioControl::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kAudioControlFixedClusterCount =
    AudioControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kAudioControlMaxClusterCount =
    kAudioControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<AudioControlCluster> gServers[kAudioControlMaxClusterCount];

// Delegate table: application may call SetDelegate(endpointId, delegate) before
// MatterAudioControlClusterInitCallback fires for each endpoint.
struct DelegateEntry
{
    EndpointId endpointId           = kInvalidEndpointId;
    AudioControlDelegate * delegate = nullptr;
};
DelegateEntry gDelegateTable[kAudioControlMaxClusterCount];

// Used when no application delegate is registered for an endpoint.
class NoOpAudioControlDelegate : public AudioControlDelegate
{
public:
    Status HandleVolumeAndMuteChange(uint16_t, bool) override { return Status::Success; }
};
NoOpAudioControlDelegate gNoOpDelegate;

AudioControlDelegate * GetDelegate(EndpointId endpointId)
{
    for (const auto & entry : gDelegateTable)
    {
        if (entry.endpointId == endpointId)
        {
            return entry.delegate;
        }
    }
    return nullptr;
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        AudioControlCluster::Config config;

        // Feature map
        config.WithFeatures(BitFlags<AudioControl::Feature>(featureMap));

        // Optional attribute enablement
        AudioControlCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        config.WithOptionalAttributes(optionalAttributeSet);

        // Fixed attributes - use Ember defaults; tolerate failure.
        {
            uint16_t value{};
            if (MinDeviceVolume::Get(endpointId, &value) == Status::Success)
            {
                config.WithMinDeviceVolume(value);
            }
        }
        {
            uint16_t value{};
            if (MaxDeviceVolume::Get(endpointId, &value) == Status::Success)
            {
                config.WithMaxDeviceVolume(value);
            }
        }
        const BitFlags<AudioControl::Feature> features(featureMap);
        if (!features.Has(Feature::kDecibel) && optionalAttributeSet.IsSet(MaxDeviceVolumeDB::Id))
        {
            uint16_t value{};
            if (MaxDeviceVolumeDB::Get(endpointId, &value) == Status::Success)
            {
                config.WithMaxDeviceVolumeDB(value);
            }
        }
        if (features.Has(Feature::kBasicEqualizer))
        {
            int16_t minCorr{};
            int16_t maxCorr{};
            if (MinCorrection::Get(endpointId, &minCorr) == Status::Success &&
                MaxCorrection::Get(endpointId, &maxCorr) == Status::Success)
            {
                config.WithCorrectionRange(minCorr, maxCorr);
            }
        }

        // Writable attribute defaults
        {
            bool value{};
            if (SoftMuted::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialSoftMuted(value);
            }
        }
        {
            uint16_t value{};
            if (Volume::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialVolume(value);
            }
        }
        {
            uint16_t value{};
            if (DefaultStepSize::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialDefaultStepSize(value);
            }
        }
        {
            UnmutePolicyEnum value{};
            if (SetVolumeUnmutePolicy::Get(endpointId, &value) == Status::Success)
            {
                config.WithSetVolumeUnmutePolicy(value);
            }
        }
        {
            UnmutePolicyEnum value{};
            if (IncreaseVolumeUnmutePolicy::Get(endpointId, &value) == Status::Success)
            {
                config.WithIncreaseVolumeUnmutePolicy(value);
            }
        }
        {
            UnmuteVolumeEnum value{};
            if (IncreaseVolumeUnmuteVolume::Get(endpointId, &value) == Status::Success)
            {
                config.WithIncreaseVolumeUnmuteVolume(value);
            }
        }
        {
            UnmutePolicyEnum value{};
            if (DecreaseVolumeUnmutePolicy::Get(endpointId, &value) == Status::Success)
            {
                config.WithDecreaseVolumeUnmutePolicy(value);
            }
        }

        if (optionalAttributeSet.IsSet(PhysicallyMuted::Id))
        {
            bool value{};
            if (PhysicallyMuted::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialPhysicallyMuted(value);
            }
        }
        if (optionalAttributeSet.IsSet(MaxUserVolume::Id))
        {
            uint16_t value{};
            if (MaxUserVolume::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialMaxUserVolume(value);
            }
        }
        if (optionalAttributeSet.IsSet(StartUpMuted::Id))
        {
            DataModel::Nullable<bool> value{};
            if (StartUpMuted::Get(endpointId, value) == Status::Success)
            {
                config.WithInitialStartUpMuted(value);
            }
        }
        if (optionalAttributeSet.IsSet(StartUpVolume::Id))
        {
            DataModel::Nullable<uint16_t> value{};
            if (StartUpVolume::Get(endpointId, value) == Status::Success)
            {
                config.WithInitialStartUpVolume(value);
            }
        }
        if (features.Has(Feature::kBasicEqualizer) && optionalAttributeSet.IsSet(Bass::Id))
        {
            int16_t value{};
            if (Bass::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialBass(value);
            }
        }
        if (features.Has(Feature::kBasicEqualizer) && optionalAttributeSet.IsSet(Mid::Id))
        {
            int16_t value{};
            if (Mid::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialMid(value);
            }
        }
        if (features.Has(Feature::kBasicEqualizer) && optionalAttributeSet.IsSet(Treble::Id))
        {
            int16_t value{};
            if (Treble::Get(endpointId, &value) == Status::Success)
            {
                config.WithInitialTreble(value);
            }
        }

        AudioControlDelegate * delegate = GetDelegate(endpointId);
        if (delegate == nullptr)
        {
            delegate = &gNoOpDelegate;
        }

        gServers[clusterInstanceIndex].Create(endpointId, *delegate, config);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterAudioControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AudioControl::Id,
            .fixedClusterInstanceCount = kAudioControlFixedClusterCount,
            .maxClusterInstanceCount   = kAudioControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterAudioControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AudioControl::Id,
            .fixedClusterInstanceCount = kAudioControlFixedClusterCount,
            .maxClusterInstanceCount   = kAudioControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::AudioControl {

AudioControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = AudioControl::Id,
            .fixedClusterInstanceCount = kAudioControlFixedClusterCount,
            .maxClusterInstanceCount   = kAudioControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<AudioControlCluster *>(cluster);
}

CHIP_ERROR SetVolume(EndpointId endpointId, uint16_t volume)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetVolume(volume);
}

CHIP_ERROR SetSoftMuted(EndpointId endpointId, bool softMuted)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetSoftMuted(softMuted);
}

CHIP_ERROR SetPhysicallyMuted(EndpointId endpointId, bool physicallyMuted)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetPhysicallyMuted(physicallyMuted);
}

CHIP_ERROR SetBass(EndpointId endpointId, int16_t bass)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetBass(bass);
}

CHIP_ERROR SetMid(EndpointId endpointId, int16_t mid)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetMid(mid);
}

CHIP_ERROR SetTreble(EndpointId endpointId, int16_t treble)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetTreble(treble);
}

void SetDelegate(EndpointId endpointId, AudioControlDelegate * delegate)
{
    for (auto & entry : gDelegateTable)
    {
        if (entry.endpointId == endpointId || entry.endpointId == kInvalidEndpointId)
        {
            entry.endpointId = endpointId;
            entry.delegate   = delegate;
            return;
        }
    }
    ChipLogError(Zcl, "AudioControl: SetDelegate table full, endpoint %u not registered", endpointId);
}

} // namespace chip::app::Clusters::AudioControl

// Stub callbacks for ZAP generated code
void MatterAudioControlPluginServerInitCallback() {}
void MatterAudioControlPluginServerShutdownCallback() {}
