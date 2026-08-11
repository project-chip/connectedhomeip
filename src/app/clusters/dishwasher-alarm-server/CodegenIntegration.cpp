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
#include <app/clusters/dishwasher-alarm-server/CodegenIntegration.h>
#include <app/clusters/dishwasher-alarm-server/DishwasherAlarmCluster.h>
#include <app/static-cluster-config/DishwasherAlarm.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/generic-callbacks.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AlarmBase;
using namespace chip::app::Clusters::DishwasherAlarm;
using namespace chip::app::Clusters::DishwasherAlarm::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kDishwasherAlarmFixedClusterCount = DishwasherAlarm::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kDishwasherAlarmMaxClusterCount   = kDishwasherAlarmFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kDishwasherAlarmFixedClusterCount == MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT,
              "DishwasherAlarm static cluster config must match ZAP server endpoint count");
static_assert(kDishwasherAlarmMaxClusterCount <= kEmberInvalidEndpointIndex, "DishwasherAlarm cluster table size error");

BitMask<DishwasherAlarm::AlarmMap> ToAlarmMap(AlarmBase::AlarmMap map)
{
    return BitMask<DishwasherAlarm::AlarmMap>(map.Raw());
}

AlarmBase::AlarmMap FromAlarmMap(const BitMask<DishwasherAlarm::AlarmMap> map)
{
    return AlarmBase::AlarmMap(map.Raw());
}

bool EndpointHasModifyEnabledAlarmsCommand(EndpointId endpointId)
{
    constexpr CommandId kCommandId = DishwasherAlarm::Commands::ModifyEnabledAlarms::Id;
    const EmberAfCluster * cluster = emberAfFindServerCluster(endpointId, DishwasherAlarm::Id);
    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->acceptedCommandList != nullptr, false);

    for (const CommandId * cmd = cluster->acceptedCommandList; *cmd != kInvalidCommandId; cmd++)
    {
        if (*cmd == kCommandId)
        {
            return true;
        }
    }
    return false;
}

class DishwasherAlarmIntegrationDelegateWrapper final : public AlarmBase::Delegate
{
public:
    void Init(DishwasherAlarm::Delegate * wrapped) { mWrapped = wrapped; }

    bool ModifyEnabledAlarms(AlarmBase::AlarmMap mask) override
    {
        if (mWrapped == nullptr)
        {
            return true;
        }
        return mWrapped->ModifyEnabledAlarmsCallback(BitMask<DishwasherAlarm::AlarmBitmap>(mask.Raw()));
    }

    bool ResetAlarms(AlarmBase::AlarmMap alarms) override
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
    LazyRegisteredServerCluster<DishwasherAlarmCluster> cluster;
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

        AlarmBase::AlarmMap supported{};
        BitMask<DishwasherAlarm::AlarmBitmap> supportedDefault{};
        if (Supported::GetDefault(endpointId, &supportedDefault) == Status::Success)
        {
            supported = AlarmBase::AlarmMap(supportedDefault.Raw());
        }

        AlarmBase::AlarmMap latch{};
        BitMask<DishwasherAlarm::AlarmBitmap> latchDefault{};
        if (Latch::GetDefault(endpointId, &latchDefault) == Status::Success)
        {
            latch = AlarmBase::AlarmMap(latchDefault.Raw());
        }

        DishwasherAlarmCluster::Config config(gDishwasherAlarmClusters[clusterInstanceIndex].integrationDelegateWrapper);
        config.feature                     = features;
        config.clusterRevision             = DishwasherAlarm::kRevision;
        config.supported                   = supported;
        config.latch                       = latch;
        config.supportsModifyEnabledAlarms = EndpointHasModifyEnabledAlarmsCommand(endpointId);

        gDishwasherAlarmClusters[clusterInstanceIndex].integrationDelegateWrapper.Init(
            gDishwasherAlarmClusters[clusterInstanceIndex].userDelegate);
        gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Create(endpointId, config);

        DishwasherAlarmCluster & cluster = gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Cluster();

        BitMask<DishwasherAlarm::AlarmBitmap> maskDefault{};
        if (Mask::GetDefault(endpointId, &maskDefault) == Status::Success)
        {
            cluster.SetMaskValue(AlarmBase::AlarmMap(maskDefault.Raw()));
        }

        BitMask<DishwasherAlarm::AlarmBitmap> stateDefault{};
        if (State::GetDefault(endpointId, &stateDefault) == Status::Success)
        {
            cluster.SetStateValue(AlarmBase::AlarmMap(stateDefault.Raw()), true);
        }

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

DishwasherAlarmCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    DishwasherAlarmIntegrationDelegate integrationDelegate;
    return static_cast<DishwasherAlarmCluster *>(CodegenClusterIntegration::FindClusterOnEndpoint(
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
    }
}

DishwasherAlarmServer DishwasherAlarmServer::instance;

DishwasherAlarmServer & DishwasherAlarmServer::Instance()
{
    return instance;
}

Status DishwasherAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetMaskValue(&value);
    if (status == Status::Success && mask != nullptr)
    {
        *mask = ToAlarmMap(value);
    }
    return status;
}

Status DishwasherAlarmServer::GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetLatchValue(&value);
    if (status == Status::Success && latch != nullptr)
    {
        *latch = ToAlarmMap(value);
    }
    return status;
}

Status DishwasherAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetStateValue(&value);
    if (status == Status::Success && state != nullptr)
    {
        *state = ToAlarmMap(value);
    }
    return status;
}

Status DishwasherAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * supported)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetSupportedValue(&value);
    if (status == Status::Success && supported != nullptr)
    {
        *supported = ToAlarmMap(value);
    }
    return status;
}

Status DishwasherAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetMaskValue(FromAlarmMap(mask));
}

Status DishwasherAlarmServer::SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState, bool ignoreLatchState)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetStateValue(FromAlarmMap(newState), ignoreLatchState);
}

Status DishwasherAlarmServer::ResetLatchedAlarms(EndpointId endpoint, const BitMask<AlarmMap> alarms)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->ResetLatchedAlarms(FromAlarmMap(alarms));
}

bool DishwasherAlarmServer::HasResetFeature(EndpointId endpoint)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, false);
    return cluster->HasResetFeature();
}

} // namespace chip::app::Clusters::DishwasherAlarm
