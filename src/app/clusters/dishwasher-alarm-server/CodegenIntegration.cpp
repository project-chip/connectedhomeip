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

// Proxy delegate used only by the codegen integration layer.
//
// AlarmBaseCluster is constructed with a mandatory AlarmBase::Delegate& (no nullptr). Apps may register
// a DishwasherAlarm::Delegate later (or never) via SetDefaultDelegate, so the cluster cannot assume an
// application delegate exists at construction time.
//
// This wrapper is the object passed into AlarmBaseCluster::Config: it lives for the endpoint slot and
// holds an optional pointer to the application delegate. When registered, command callbacks are forwarded
// to the app. When mWrapped is null, ModifyEnabledAlarms and ResetAlarms return true so the cluster still
// updates Matter attributes. That matches master Ember behavior (GetDelegate() == nullptr skipped the
// callback and allowed the attribute update) and lets example/simulator apps work without product logic.
class DishwasherAlarmIntegrationDelegateWrapper final : public AlarmBase::Delegate
{
public:
    void SetWrapped(DishwasherAlarm::Delegate * wrapped) { mWrapped = wrapped; }

    DishwasherAlarm::Delegate * GetWrapped() const { return mWrapped; }

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
};

DishwasherAlarmClusterSlot gDishwasherAlarmClusters[kDishwasherAlarmMaxClusterCount];

class DishwasherAlarmIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex, uint32_t,
                                                   uint32_t featureMap) override
    {
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

        DishwasherAlarmCluster::Config config{
            .delegate                    = gDishwasherAlarmClusters[clusterInstanceIndex].integrationDelegateWrapper,
            .feature                     = features,
            .supported                   = supported,
            .latch                       = latch,
            .supportsModifyEnabledAlarms = EndpointHasModifyEnabledAlarmsCommand(endpointId),
        };

        gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Create(endpointId, config);

        DishwasherAlarmCluster & cluster = gDishwasherAlarmClusters[clusterInstanceIndex].cluster.Cluster();

        BitMask<DishwasherAlarm::AlarmBitmap> maskDefault{};
        if (Mask::GetDefault(endpointId, &maskDefault) == Status::Success)
        {
            cluster.SetMask(AlarmBase::AlarmMap(maskDefault.Raw()));
        }

        BitMask<DishwasherAlarm::AlarmBitmap> stateDefault{};
        if (State::GetDefault(endpointId, &stateDefault) == Status::Success)
        {
            cluster.SetState(AlarmBase::AlarmMap(stateDefault.Raw()), true);
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
    return (ep >= kDishwasherAlarmMaxClusterCount ? nullptr : gDishwasherAlarmClusters[ep].integrationDelegateWrapper.GetWrapped());
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, DishwasherAlarm::Id,
                                                       MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kDishwasherAlarmMaxClusterCount)
    {
        gDishwasherAlarmClusters[ep].integrationDelegateWrapper.SetWrapped(delegate);
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

    if (mask != nullptr)
    {
        *mask = ToAlarmMap(cluster->GetMask());
    }
    return Status::Success;
}

Status DishwasherAlarmServer::GetLatchValue(EndpointId endpoint, BitMask<AlarmMap> * latch)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    VerifyOrReturnError(cluster->HasResetFeature(), Status::UnsupportedAttribute);

    if (latch != nullptr)
    {
        *latch = ToAlarmMap(cluster->GetLatch());
    }
    return Status::Success;
}

Status DishwasherAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    if (state != nullptr)
    {
        *state = ToAlarmMap(cluster->GetState());
    }
    return Status::Success;
}

Status DishwasherAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * supported)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    if (supported != nullptr)
    {
        *supported = ToAlarmMap(cluster->GetSupported());
    }
    return Status::Success;
}

Status DishwasherAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetMask(FromAlarmMap(mask));
}

Status DishwasherAlarmServer::SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState, bool ignoreLatchState)
{
    DishwasherAlarmCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetState(FromAlarmMap(newState), ignoreLatchState);
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

namespace Attributes {

namespace Mask {

Status Get(EndpointId endpoint, BitMask<AlarmMap> * value)
{
    return DishwasherAlarmServer::Instance().GetMaskValue(endpoint, value);
}

Status Set(EndpointId endpoint, BitMask<AlarmMap> value)
{
    return DishwasherAlarmServer::Instance().SetMaskValue(endpoint, value);
}

} // namespace Mask

namespace Latch {

Status Get(EndpointId endpoint, BitMask<AlarmMap> * value)
{
    return DishwasherAlarmServer::Instance().GetLatchValue(endpoint, value);
}

} // namespace Latch

namespace State {

Status Get(EndpointId endpoint, BitMask<AlarmMap> * value)
{
    return DishwasherAlarmServer::Instance().GetStateValue(endpoint, value);
}

Status Set(EndpointId endpoint, BitMask<AlarmMap> value)
{
    return DishwasherAlarmServer::Instance().SetStateValue(endpoint, value);
}

} // namespace State

namespace Supported {

Status Get(EndpointId endpoint, BitMask<AlarmMap> * value)
{
    return DishwasherAlarmServer::Instance().GetSupportedValue(endpoint, value);
}

} // namespace Supported

} // namespace Attributes

} // namespace chip::app::Clusters::DishwasherAlarm
