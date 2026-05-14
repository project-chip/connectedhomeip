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

struct ClusterWithDelegate
{
    Delegate * delegate = nullptr;
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

        FanControlCluster::Config config(endpointId, gClusters[clusterInstanceIndex].delegate);

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
    return (ep >= kFanControlMaxClusterCount ? nullptr : gClusters[ep].delegate);
}

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kFanControlMaxClusterCount)
    {
        gClusters[ep].delegate = aDelegate;

        // Update the cluster instance if it already exists (e.g. app sets delegate in emberAfFanControlClusterInitCallback)
        if (gClusters[ep].server.IsConstructed())
        {
            gClusters[ep].server.Cluster().SetDelegate(aDelegate);
        }
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
    return cluster->SetFanMode(value).GetStatusCode().GetStatus();
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
    return cluster->SetPercentSetting(DataModel::Nullable<chip::Percent>(value)).GetStatusCode().GetStatus();
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
    return cluster->SetSpeedSetting(value).GetStatusCode().GetStatus();
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
    return cluster->SetAirflowDirection(value).GetStatusCode().GetStatus();
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
    return cluster->SetRockSetting(value).GetStatusCode().GetStatus();
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
    return cluster->SetWindSetting(value).GetStatusCode().GetStatus();
}

} // namespace WindSetting

} // namespace Attributes

} // namespace chip::app::Clusters::FanControl
