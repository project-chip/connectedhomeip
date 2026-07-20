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

#include <app/clusters/alarm-base-server/AlarmBaseCluster.h>
#include <app/clusters/alarm-base-server/CodegenIntegrationHelpers.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>
#include <app/clusters/dishwasher-alarm-server/CodegenIntegration.h>
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-delegate.h>
#include <app/static-cluster-config/DishwasherAlarm.h>
#include <app/util/attribute-storage.h>
#include <app/util/generic-callbacks.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AlarmBase;

namespace {

constexpr size_t kDishwasherAlarmFixedClusterCount = DishwasherAlarm::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kDishwasherAlarmMaxClusterCount   = kDishwasherAlarmFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kDishwasherAlarmFixedClusterCount == MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT,
              "DishwasherAlarm static cluster config must match ZAP server endpoint count");
static_assert(kDishwasherAlarmMaxClusterCount <= kEmberInvalidEndpointIndex, "DishwasherAlarm cluster table size error");

class DishwasherAlarmIntegrationDelegateWrapper final : public AlarmBase::Delegate
{
public:
    void Init(DishwasherAlarm::Delegate * wrapped) { mWrapped = wrapped; }

    bool ModifyEnabledAlarms(AlarmMap mask) override
    {
        if (mWrapped == nullptr)
        {
            return true;
        }
        return mWrapped->ModifyEnabledAlarmsCallback(BitMask<DishwasherAlarm::AlarmBitmap>(mask.Raw()));
    }

    bool ResetAlarms(AlarmMap alarms) override
    {
        if (mWrapped == nullptr)
        {
            return true;
        }
        return mWrapped->ResetAlarmsCallback(BitMask<DishwasherAlarm::AlarmBitmap>(alarms.Raw()));
    }

private:
    DishwasherAlarm::Delegate * mWrapped = nullptr;
};

struct DishwasherAlarmClusterSlot
{
    LazyRegisteredServerCluster<AlarmBaseCluster> cluster;
    DishwasherAlarmIntegrationDelegateWrapper integrationDelegateWrapper;
    DishwasherAlarm::Delegate * userDelegate = nullptr;
};

DishwasherAlarmClusterSlot gDishwasherAlarmClusters[kDishwasherAlarmMaxClusterCount];

class DishwasherAlarmIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        (void) optionalAttributeBits;

        BitFlags<DishwasherAlarm::Feature> features(featureMap);

        BitMask<DishwasherAlarm::AlarmBitmap> supported{};
        BitMask<DishwasherAlarm::AlarmBitmap> latch{};

        AlarmBaseCluster::Config config{
            .feature         = features,
            .clusterRevision = GetClusterRevision(DishwasherAlarm::Id),
            .supported       = AlarmMap(supported.Raw()),
            .latch           = AlarmMap(latch.Raw()),
            .supportsModifyEnabledAlarms =
                EndpointHasCommand(endpointId, DishwasherAlarm::Id, DishwasherAlarm::Commands::ModifyEnabledAlarms::Id),
            .delegate = &gDishwasherAlarmClusters[clusterInstanceIndex].integrationDelegateWrapper,
        };

        gDishwasherAlarmClusters[clusterInstanceIndex].integrationDelegateWrapper.Init(
            gDishwasherAlarmClusters[clusterInstanceIndex].userDelegate);
        gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Create(endpointId, DishwasherAlarm::Id, config);
        return gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gDishwasherAlarmClusters[clusterInstanceIndex].cluster.IsConstructed(), nullptr);
        return &gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Destroy();
    }
};

} // namespace

void MatterDishwasherAlarmClusterInitCallback(EndpointId endpointId)
{
    DishwasherAlarmIntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = DishwasherAlarm::Id,
            .fixedClusterInstanceCount = kDishwasherAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kDishwasherAlarmMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterDishwasherAlarmClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    DishwasherAlarmIntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = DishwasherAlarm::Id,
            .fixedClusterInstanceCount = kDishwasherAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kDishwasherAlarmMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

__attribute__((weak)) void MatterDishwasherAlarmPluginServerInitCallback() {}
__attribute__((weak)) void MatterDishwasherAlarmPluginServerShutdownCallback() {}

namespace chip::app::Clusters::DishwasherAlarm {

AlarmBaseCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    DishwasherAlarmIntegrationDelegate integrationDelegate;
    return static_cast<AlarmBaseCluster *>(CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = DishwasherAlarm::Id,
            .fixedClusterInstanceCount = kDishwasherAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kDishwasherAlarmMaxClusterCount,
        },
        integrationDelegate));
}

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DishwasherAlarm::Id,
                                                       MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kDishwasherAlarmMaxClusterCount ? nullptr : gDishwasherAlarmClusters[ep].userDelegate);
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DishwasherAlarm::Id,
                                                       MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kDishwasherAlarmMaxClusterCount)
    {
        gDishwasherAlarmClusters[ep].userDelegate = delegate;
        gDishwasherAlarmClusters[ep].integrationDelegateWrapper.Init(delegate);

        AlarmBaseCluster * cluster = FindClusterOnEndpoint(endpoint);
        if (cluster != nullptr)
        {
            cluster->SetDelegate(&gDishwasherAlarmClusters[ep].integrationDelegateWrapper);
        }
    }
}

} // namespace chip::app::Clusters::DishwasherAlarm
