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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/clusters/fan-control-server/CodegenIntegration.h>
#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/static-cluster-config/FanControl.h>
#include <app/util/attribute-storage.h>
#include <app/util/generic-callbacks.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kFanControlFixedClusterCount = FanControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kFanControlMaxClusterCount   = kFanControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kFanControlFixedClusterCount == MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT,
              "FanControl static cluster config must match ZAP server endpoint count");
static_assert(kFanControlMaxClusterCount <= kEmberInvalidEndpointIndex, "FanControl cluster table size error");

// Proxy delegate used only by the codegen integration layer.
//
// FanControlCluster is constructed with a mandatory FanControl::Delegate& (no nullptr). Ember/ZAP apps
// historically register a real application delegate later (or never) via SetDefaultDelegate, so the
// cluster cannot assume an application delegate exists at construction time.
//
// This wrapper is the single object passed into FanControlCluster::Config: it always lives for the
// endpoint slot and holds an optional pointer to the application delegate. Virtual calls forward when
// that pointer is non-null; otherwise HandleStep returns Failure and optional notifications are no-ops.
// OnFanDriveStateChanged also emits MatterPostAttributeChangeCallback so legacy apps keep working.
class FanControlIntegrationDelegateWrapper final : public FanControl::Delegate
{
public:
    FanControlIntegrationDelegateWrapper() : FanControl::Delegate(kInvalidEndpointId) {}

    void Init(EndpointId ep, FanControl::Delegate * wrapped)
    {
        mEndpoint = ep;
        mWrapped  = wrapped;
    }

    // --- pure-virtual requirement ---
    Status HandleStep(StepDirectionEnum dir, bool wrap, bool lowestOff) override
    {
        return mWrapped ? mWrapped->HandleStep(dir, wrap, lowestOff) : Status::Failure;
    }

    // --- forward all optional delegate callbacks, then emit legacy callbacks ---
    void OnFanDriveStateChanged(const FanDriveState & state) override
    {
        if (mWrapped)
            mWrapped->OnFanDriveStateChanged(state);
        EmitLegacyPostAttributeCallbacks(state);
    }

    void OnRockSettingChanged(BitMask<RockBitmap> v) override
    {
        if (mWrapped)
            mWrapped->OnRockSettingChanged(v);
    }

    void OnWindSettingChanged(BitMask<WindBitmap> v) override
    {
        if (mWrapped)
            mWrapped->OnWindSettingChanged(v);
    }

    void OnAirflowDirectionChanged(AirflowDirectionEnum v) override
    {
        if (mWrapped)
            mWrapped->OnAirflowDirectionChanged(v);
    }

private:
    FanControl::Delegate * mWrapped = nullptr;

    void EmitLegacyPostAttributeCallbacks(const FanDriveState & state)
    {
        ConcreteAttributePath path(mEndpoint, FanControl::Id, FanMode::Id);

        // FanMode (enum8, 1 byte)
        {
            uint8_t raw = to_underlying(state.mode);
            MatterPostAttributeChangeCallback(path, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, &raw);
        }

        // PercentSetting (nullable int8u, null sentinel = 0xFF)
        {
            path.mAttributeId = PercentSetting::Id;
            uint8_t raw       = state.percentSetting.IsNull() ? 0xFF : state.percentSetting.Value();
            MatterPostAttributeChangeCallback(path, ZCL_INT8U_ATTRIBUTE_TYPE, 1, &raw);
        }

        // SpeedSetting (nullable int8u) — only emit when non-null.
        // Emitting a null (0xFF) value here would cause SpeedSettingWriteCallback
        // to call FanMode::Set(kOff), corrupting state in kAuto mode.
        if (!state.speedSetting.IsNull())
        {
            path.mAttributeId = SpeedSetting::Id;
            uint8_t raw       = state.speedSetting.Value();
            MatterPostAttributeChangeCallback(path, ZCL_INT8U_ATTRIBUTE_TYPE, 1, &raw);
        }
    }
};

struct ClusterWithDelegate
{
    Delegate * userDelegate = nullptr;
    FanControlIntegrationDelegateWrapper integrationDelegateWrapper;
    LazyRegisteredServerCluster<FanControlCluster> server;
};

ClusterWithDelegate gClusters[kFanControlMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<FanControl::Feature> features(featureMap);

        gClusters[clusterInstanceIndex].integrationDelegateWrapper.Init(endpointId, gClusters[clusterInstanceIndex].userDelegate);
        FanControlCluster::Config config(endpointId, gClusters[clusterInstanceIndex].integrationDelegateWrapper);

        // Initialize FanModeSequence from attribute storage if available, otherwise use default.
        FanModeSequenceEnum defaultFanModeSequence =
            features.Has(FanControl::Feature::kAuto) ? FanModeSequenceEnum::kOffLowHighAuto : FanModeSequenceEnum::kOffLowHigh;

        FanModeSequenceEnum fanModeSequence = defaultFanModeSequence;
        if (FanModeSequence::GetDefault(endpointId, &fanModeSequence) != Status::Success)
        {
            fanModeSequence = defaultFanModeSequence;
        }

        if (EnsureKnownEnumValue(fanModeSequence) == FanModeSequenceEnum::kUnknownEnumValue)
        {
            fanModeSequence = defaultFanModeSequence;
        }

        config.WithFanModeSequence(fanModeSequence);

        if (features.Has(FanControl::Feature::kMultiSpeed))
        {
            uint8_t speedMax = 100;
            if (SpeedMax::GetDefault(endpointId, &speedMax) != Status::Success)
            {
                speedMax = 100;
            }
            config.WithSpeedMax(speedMax);
        }
        if (features.Has(FanControl::Feature::kRocking))
        {
            BitMask<RockBitmap> rockSupport;
            if (RockSupport::GetDefault(endpointId, &rockSupport) != Status::Success)
            {
                rockSupport = BitMask<RockBitmap>(RockBitmap::kRockLeftRight, RockBitmap::kRockUpDown, RockBitmap::kRockRound);
            }
            config.WithRockSupport(rockSupport);
        }
        if (features.Has(FanControl::Feature::kWind))
        {
            BitMask<WindBitmap> windSupport;
            if (WindSupport::GetDefault(endpointId, &windSupport) != Status::Success)
            {
                windSupport = BitMask<WindBitmap>(WindBitmap::kSleepWind, WindBitmap::kNaturalWind);
            }
            config.WithWindSupport(windSupport);
        }
        if (features.Has(FanControl::Feature::kAirflowDirection))
        {
            config.WithAirflowDirection();
        }
        if (features.Has(FanControl::Feature::kStep))
        {
            config.WithStep();
        }

        gClusters[clusterInstanceIndex].server.Create(config);
        return gClusters[clusterInstanceIndex].server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gClusters[clusterInstanceIndex].server.Destroy(); }
};

} // namespace

void MatterFanControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterFanControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::FanControl {

FanControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<FanControlCluster *>(cluster);
}

Delegate * GetDelegate(EndpointId aEndpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kFanControlMaxClusterCount ? nullptr : gClusters[ep].userDelegate);
}

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kFanControlMaxClusterCount)
    {
        gClusters[ep].userDelegate = aDelegate;
        gClusters[ep].integrationDelegateWrapper.Init(aEndpoint, aDelegate);
    }
}

namespace Attributes {

namespace FanMode {

Status Get(EndpointId endpoint, FanModeEnum * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetFanMode();
    return Status::Success;
}

Status Set(EndpointId endpoint, FanModeEnum value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetFanMode(value);
}

} // namespace FanMode

namespace FanModeSequence {

Status Get(EndpointId endpoint, FanModeSequenceEnum * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetFanModeSequence();
    return Status::Success;
}

Status Set(EndpointId endpoint, FanModeSequenceEnum value)
{
    (void) endpoint;
    (void) value;
    return Status::UnsupportedWrite;
}

} // namespace FanModeSequence

namespace PercentSetting {

Status Get(EndpointId endpoint, DataModel::Nullable<chip::Percent> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetPercentSetting();
    return Status::Success;
}

Status Set(EndpointId endpoint, chip::Percent value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetPercentSetting(DataModel::Nullable<chip::Percent>(value));
}

} // namespace PercentSetting

namespace PercentCurrent {

Status Get(EndpointId endpoint, chip::Percent * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetPercentCurrent();
    return Status::Success;
}

Status Set(EndpointId endpoint, chip::Percent value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetPercentCurrent(value) ? Status::Success : Status::Failure;
}

} // namespace PercentCurrent

namespace SpeedSetting {

Status Get(EndpointId endpoint, DataModel::Nullable<uint8_t> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetSpeedSetting();
    return Status::Success;
}

Status Set(EndpointId endpoint, const DataModel::Nullable<uint8_t> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    return cluster->SetSpeedSetting(value);
}

Status Set(EndpointId endpoint, uint8_t value)
{
    return Set(endpoint, DataModel::Nullable<uint8_t>(value));
}

} // namespace SpeedSetting

namespace SpeedCurrent {

Status Get(EndpointId endpoint, uint8_t * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetSpeedCurrent();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint8_t value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    return cluster->SetSpeedCurrent(value) ? Status::Success : Status::Failure;
}

} // namespace SpeedCurrent

namespace SpeedMax {

Status Get(EndpointId endpoint, uint8_t * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetSpeedMax();
    return Status::Success;
}

} // namespace SpeedMax

namespace FeatureMap {

Status Get(EndpointId endpoint, uint32_t * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetFeatureMap().Raw();
    return Status::Success;
}

} // namespace FeatureMap

namespace AirflowDirection {

Status Get(EndpointId endpoint, AirflowDirectionEnum * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kAirflowDirection))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetAirflowDirection();
    return Status::Success;
}

Status Set(EndpointId endpoint, AirflowDirectionEnum value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kAirflowDirection))
    {
        return Status::UnsupportedAttribute;
    }
    return cluster->SetAirflowDirection(value);
}

} // namespace AirflowDirection

namespace RockSupport {

Status Get(EndpointId endpoint, BitMask<RockBitmap> * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetRockSupport();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<RockBitmap> value)
{
    (void) endpoint;
    (void) value;
    return Status::UnsupportedWrite;
}

} // namespace RockSupport

namespace RockSetting {

Status Get(EndpointId endpoint, BitMask<RockBitmap> * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetRockSetting();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<RockBitmap> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    return cluster->SetRockSetting(value);
}

} // namespace RockSetting

namespace WindSupport {

Status Get(EndpointId endpoint, BitMask<WindBitmap> * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetWindSupport();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<WindBitmap> value)
{
    (void) endpoint;
    (void) value;
    return Status::UnsupportedWrite;
}

} // namespace WindSupport

namespace WindSetting {

Status Get(EndpointId endpoint, BitMask<WindBitmap> * value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    *value = cluster->GetWindSetting();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<WindBitmap> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    return cluster->SetWindSetting(value);
}

} // namespace WindSetting
} // namespace Attributes
} // namespace chip::app::Clusters::FanControl
