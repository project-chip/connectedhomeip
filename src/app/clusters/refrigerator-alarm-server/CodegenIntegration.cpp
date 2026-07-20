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
#include <app/clusters/refrigerator-alarm-server/CodegenIntegration.h>
#include <app/static-cluster-config/RefrigeratorAlarm.h>
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

constexpr size_t kRefrigeratorAlarmFixedClusterCount = RefrigeratorAlarm::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kRefrigeratorAlarmMaxClusterCount =
    kRefrigeratorAlarmFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kRefrigeratorAlarmFixedClusterCount == MATTER_DM_REFRIGERATOR_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT,
              "RefrigeratorAlarm static cluster config must match ZAP server endpoint count");
static_assert(kRefrigeratorAlarmMaxClusterCount <= kEmberInvalidEndpointIndex, "RefrigeratorAlarm cluster table size error");

LazyRegisteredServerCluster<AlarmBaseCluster> gRefrigeratorAlarmClusters[kRefrigeratorAlarmMaxClusterCount];

class RefrigeratorAlarmIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        (void) optionalAttributeBits;
        (void) featureMap;

        BitMask<RefrigeratorAlarm::AlarmBitmap> supported{};

        AlarmBaseCluster::Config config{
            .feature                     = BitFlags<AlarmBase::Feature>(),
            .clusterRevision             = GetClusterRevision(RefrigeratorAlarm::Id),
            .supported                   = AlarmMap(supported.Raw()),
            .latch                       = {},
            .supportsModifyEnabledAlarms = false,
            .delegate                    = nullptr,
        };

        gRefrigeratorAlarmClusters[clusterInstanceIndex].Create(endpointId, RefrigeratorAlarm::Id, config);
        return gRefrigeratorAlarmClusters[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gRefrigeratorAlarmClusters[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gRefrigeratorAlarmClusters[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gRefrigeratorAlarmClusters[clusterInstanceIndex].Destroy();
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

AlarmBaseCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    RefrigeratorAlarmIntegrationDelegate integrationDelegate;
    return static_cast<AlarmBaseCluster *>(CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = RefrigeratorAlarm::Id,
            .fixedClusterInstanceCount = kRefrigeratorAlarmFixedClusterCount,
            .maxClusterInstanceCount   = kRefrigeratorAlarmMaxClusterCount,
        },
        integrationDelegate));
}

} // namespace chip::app::Clusters::RefrigeratorAlarm
