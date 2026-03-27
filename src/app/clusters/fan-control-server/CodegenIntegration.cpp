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

#include <app-common/zap-generated/callback.h>
#include <app/clusters/fan-control-server/CodegenIntegration.h>
#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/FanControl.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
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

constexpr size_t kFanControlDelegateTableSize =
    MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kFanControlDelegateTableSize <= kEmberInvalidEndpointIndex, "FanControl Delegate table size error");

FanControl::Delegate * gDelegateTable[kFanControlDelegateTableSize] = { nullptr };

LazyRegisteredServerCluster<FanControlCluster> gServers[kFanControlMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<FanControl::Feature> features(featureMap);

        FanControlCluster::Config config(endpointId, GetDelegate(endpointId));

        // Initialize FanModeSequence from attribute storage if available, otherwise use default.
        FanModeSequenceEnum defaultFanModeSequence =
            features.Has(FanControl::Feature::kAuto) ? FanModeSequenceEnum::kOffLowHighAuto : FanModeSequenceEnum::kOffLowHigh;

        FanModeSequenceEnum fanModeSequence = defaultFanModeSequence;

        (void) emberAfReadAttribute(endpointId, FanControl::Id, FanModeSequence::Id, reinterpret_cast<uint8_t *>(&fanModeSequence),
                                    sizeof(fanModeSequence));

        if (EnsureKnownEnumValue(fanModeSequence) == FanModeSequenceEnum::kUnknownEnumValue)
        {
            fanModeSequence = defaultFanModeSequence;
        }

        config.WithFanModeSequence(fanModeSequence);

        if (features.Has(FanControl::Feature::kMultiSpeed))
        {
            uint8_t speedMax = 100;
            if (emberAfReadAttribute(endpointId, FanControl::Id, SpeedMax::Id, &speedMax, sizeof(speedMax)) != Status::Success)
            {
                speedMax = 100;
            }
            config.WithSpeedMax(speedMax);
        }
        if (features.Has(FanControl::Feature::kRocking))
        {
            uint8_t rockSupportRaw =
                static_cast<uint8_t>(static_cast<uint8_t>(RockBitmap::kRockLeftRight) |
                                     static_cast<uint8_t>(RockBitmap::kRockUpDown) | static_cast<uint8_t>(RockBitmap::kRockRound));
            (void) emberAfReadAttribute(endpointId, FanControl::Id, RockSupport::Id, &rockSupportRaw, sizeof(rockSupportRaw));
            config.WithRockSupport(BitMask<RockBitmap>(rockSupportRaw));
        }
        if (features.Has(FanControl::Feature::kWind))
        {
            uint8_t windSupportRaw =
                static_cast<uint8_t>(static_cast<uint8_t>(WindBitmap::kSleepWind) | static_cast<uint8_t>(WindBitmap::kNaturalWind));
            (void) emberAfReadAttribute(endpointId, FanControl::Id, WindSupport::Id, &windSupportRaw, sizeof(windSupportRaw));
            config.WithWindSupport(BitMask<WindBitmap>(windSupportRaw));
        }
        if (features.Has(FanControl::Feature::kAirflowDirection))
            config.WithAirflowDirection();
        if (features.Has(FanControl::Feature::kStep))
            config.WithStep();

        gServers[clusterInstanceIndex].Create(config);
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

Status GetFanMode(EndpointId endpointId, FanModeEnum & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetFanMode();
    return Status::Success;
}

Status GetPercentSetting(EndpointId endpointId, DataModel::Nullable<chip::Percent> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetPercentSetting();
    return Status::Success;
}

Status GetSpeedSetting(EndpointId endpointId, DataModel::Nullable<uint8_t> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetSpeedSetting();
    return Status::Success;
}

Status GetSpeedMax(EndpointId endpointId, uint8_t & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetSpeedMax();
    return Status::Success;
}

Status GetFanModeSequence(EndpointId endpointId, FanModeSequenceEnum & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetFanModeSequence();
    return Status::Success;
}

Status GetPercentCurrent(EndpointId endpointId, chip::Percent & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetPercentCurrent();
    return Status::Success;
}

Status GetSpeedCurrent(EndpointId endpointId, uint8_t & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetSpeedCurrent();
    return Status::Success;
}

Status GetFeatureMap(EndpointId endpointId, uint32_t & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetFeatureMap().Raw();
    return Status::Success;
}

Status GetRockSupport(EndpointId endpointId, BitMask<RockBitmap> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetRockSupport();
    return Status::Success;
}

Status GetRockSetting(EndpointId endpointId, BitMask<RockBitmap> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetRockSetting();
    return Status::Success;
}

Status GetWindSupport(EndpointId endpointId, BitMask<WindBitmap> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetWindSupport();
    return Status::Success;
}

Status GetWindSetting(EndpointId endpointId, BitMask<WindBitmap> & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetWindSetting();
    return Status::Success;
}

Status GetAirflowDirection(EndpointId endpointId, AirflowDirectionEnum & value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!cluster->GetFeatureMap().Has(FanControl::Feature::kAirflowDirection))
    {
        return Status::UnsupportedAttribute;
    }
    value = cluster->GetAirflowDirection();
    return Status::Success;
}

Status SetSpeedSetting(EndpointId endpointId, DataModel::Nullable<uint8_t> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetSpeedSetting(value).GetStatusCode().GetStatus();
}

Status SetFanMode(EndpointId endpointId, FanModeEnum value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetFanMode(value).GetStatusCode().GetStatus();
}

Status SetPercentSetting(EndpointId endpointId, DataModel::Nullable<chip::Percent> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetPercentSetting(value).GetStatusCode().GetStatus();
}

Status SetRockSetting(EndpointId endpointId, BitMask<RockBitmap> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetRockSetting(value).GetStatusCode().GetStatus();
}

Status SetWindSetting(EndpointId endpointId, BitMask<WindBitmap> value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetWindSetting(value).GetStatusCode().GetStatus();
}

Status SetAirflowDirection(EndpointId endpointId, AirflowDirectionEnum value)
{
    FanControlCluster * cluster = FindClusterOnEndpoint(endpointId);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetAirflowDirection(value).GetStatusCode().GetStatus();
}

Delegate * GetDelegate(EndpointId aEndpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kFanControlDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kFanControlDelegateTableSize)
    {
        gDelegateTable[ep] = aDelegate;
    }

    // Update the cluster instance if it already exists (e.g. app sets delegate in emberAfFanControlClusterInitCallback)
    if (FanControlCluster * cluster = FindClusterOnEndpoint(aEndpoint); cluster != nullptr)
    {
        cluster->SetDelegate(aDelegate);
    }
}

} // namespace chip::app::Clusters::FanControl
