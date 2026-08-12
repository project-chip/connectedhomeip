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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>
#include <app/clusters/refrigerator-alarm-server/CodegenIntegration.h>
#include <app/clusters/refrigerator-alarm-server/RefrigeratorAlarmCluster.h>
#include <app/static-cluster-config/RefrigeratorAlarm.h>
#include <app/util/attribute-storage.h>
#include <app/util/generic-callbacks.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AlarmBase;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAlarm::Attributes;
using chip::Protocols::InteractionModel::Status;

BitMask<RefrigeratorAlarm::AlarmMap> ToAlarmMap(AlarmBase::AlarmMap map)
{
    return BitMask<RefrigeratorAlarm::AlarmMap>(map.Raw());
}

AlarmBase::AlarmMap FromAlarmMap(const BitMask<RefrigeratorAlarm::AlarmMap> map)
{
    return AlarmBase::AlarmMap(map.Raw());
}

namespace {

constexpr size_t kRefrigeratorAlarmFixedClusterCount = RefrigeratorAlarm::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kRefrigeratorAlarmMaxClusterCount =
    kRefrigeratorAlarmFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kRefrigeratorAlarmFixedClusterCount == MATTER_DM_REFRIGERATOR_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT,
              "RefrigeratorAlarm static cluster config must match ZAP server endpoint count");
static_assert(kRefrigeratorAlarmMaxClusterCount <= kEmberInvalidEndpointIndex, "RefrigeratorAlarm cluster table size error");

struct RefrigeratorAlarmClusterSlot
{
    LazyRegisteredServerCluster<RefrigeratorAlarmCluster> cluster;
    AlarmBase::Delegate integrationDelegate;
};

RefrigeratorAlarmClusterSlot gRefrigeratorAlarmClusters[kRefrigeratorAlarmMaxClusterCount];

class RefrigeratorAlarmIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        (void) optionalAttributeBits;
        (void) featureMap;

        AlarmBase::AlarmMap supported{};
        BitMask<RefrigeratorAlarm::AlarmBitmap> supportedDefault{};
        if (Supported::GetDefault(endpointId, &supportedDefault) == Status::Success)
        {
            supported = AlarmBase::AlarmMap(supportedDefault.Raw());
        }

        RefrigeratorAlarmCluster::Config config{
            .delegate                    = gRefrigeratorAlarmClusters[clusterInstanceIndex].integrationDelegate,
            .feature                     = BitFlags<AlarmBase::Feature>(),
            .clusterRevision             = RefrigeratorAlarm::kRevision,
            .supported                   = supported,
            .supportsModifyEnabledAlarms = false,
        };

        gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.Create(endpointId, config);

        RefrigeratorAlarmCluster & cluster = gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.Cluster();

        BitMask<RefrigeratorAlarm::AlarmBitmap> maskDefault{};
        if (Mask::GetDefault(endpointId, &maskDefault) == Status::Success)
        {
            cluster.SetMaskValue(AlarmBase::AlarmMap(maskDefault.Raw()));
        }

        BitMask<RefrigeratorAlarm::AlarmBitmap> stateDefault{};
        if (State::GetDefault(endpointId, &stateDefault) == Status::Success)
        {
            cluster.SetStateValue(AlarmBase::AlarmMap(stateDefault.Raw()), true);
        }

        return gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.IsConstructed(), nullptr);
        return &gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gRefrigeratorAlarmClusters[clusterInstanceIndex].cluster.Destroy();
    }
};

} // namespace

void MatterRefrigeratorAlarmClusterInitCallback(EndpointId endpointId)
{
    RefrigeratorAlarmIntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = RefrigeratorAlarm::Id,
            .fixedClusterInstanceCount = kRefrigeratorAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kRefrigeratorAlarmMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterRefrigeratorAlarmClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    RefrigeratorAlarmIntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = RefrigeratorAlarm::Id,
            .fixedClusterInstanceCount = kRefrigeratorAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kRefrigeratorAlarmMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

__attribute__((weak)) void MatterRefrigeratorAlarmPluginServerInitCallback() {}
__attribute__((weak)) void MatterRefrigeratorAlarmPluginServerShutdownCallback() {}

namespace chip::app::Clusters::RefrigeratorAlarm {

RefrigeratorAlarmCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    RefrigeratorAlarmIntegrationDelegate integrationDelegate;
    return static_cast<RefrigeratorAlarmCluster *>(CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = RefrigeratorAlarm::Id,
            .fixedClusterInstanceCount = kRefrigeratorAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kRefrigeratorAlarmMaxClusterCount,
        },
        integrationDelegate));
}

} // namespace chip::app::Clusters::RefrigeratorAlarm

RefrigeratorAlarmServer RefrigeratorAlarmServer::instance;

RefrigeratorAlarmServer & RefrigeratorAlarmServer::Instance()
{
    return instance;
}

Status RefrigeratorAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<RefrigeratorAlarm::AlarmMap> * mask)
{
    RefrigeratorAlarmCluster * cluster = chip::app::Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetMaskValue(&value);
    if (status == Status::Success && mask != nullptr)
    {
        *mask = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::GetStateValue(EndpointId endpoint, BitMask<RefrigeratorAlarm::AlarmMap> * state)
{
    RefrigeratorAlarmCluster * cluster = chip::app::Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetStateValue(&value);
    if (status == Status::Success && state != nullptr)
    {
        *state = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<RefrigeratorAlarm::AlarmMap> * supported)
{
    RefrigeratorAlarmCluster * cluster = chip::app::Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetSupportedValue(&value);
    if (status == Status::Success && supported != nullptr)
    {
        *supported = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<RefrigeratorAlarm::AlarmMap> mask)
{
    RefrigeratorAlarmCluster * cluster = chip::app::Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetMaskValue(FromAlarmMap(mask));
}

Status RefrigeratorAlarmServer::SetStateValue(EndpointId endpoint, const BitMask<RefrigeratorAlarm::AlarmMap> newState)
{
    RefrigeratorAlarmCluster * cluster = chip::app::Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetStateValue(FromAlarmMap(newState));
}
