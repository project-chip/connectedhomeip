/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "CodegenIntegration.h"
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/static-cluster-config/ValveConfigurationAndControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
#include <app/clusters/time-synchronization-server/CodegenIntegration.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
using namespace chip::app::Clusters::ValveConfigurationAndControl::Attributes;

namespace {

constexpr size_t kValveConfigurationAndControlFixedClusterCount =
    ValveConfigurationAndControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kValveConfigurationAndControlMaxClusterCount =
    kValveConfigurationAndControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ValveConfigurationAndControlCluster> gServers[kValveConfigurationAndControlMaxClusterCount];

class CodegenTimeSyncTracker : public TimeSyncTracker
{
public:
    bool IsTimeSyncClusterSupported() override
    {
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
        return (nullptr != TimeSynchronization::GetClusterInstance());
#else
        return false;
#endif
    }

    bool IsValidUTCTime() override
    {
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
        if (IsTimeSyncClusterSupported())
        {
            return TimeSynchronization::GetClusterInstance()->GetGranularity() !=
                TimeSynchronization::GranularityEnum::kNoTimeGranularity;
        }
#endif
        return false;
    }
};

CodegenTimeSyncTracker codegenTracker;
class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId, BitFlags<ValveConfigurationAndControl::Feature>(featureMap),
                                                          ValveConfigurationAndControlCluster::OptionalAttributeSet(optionalAttributeBits),
                                                          ValveConfigurationAndControlCluster::StartupConfiguration{
                                                            .defaultOpenDuration = DataModel::NullNullable,
                                                            .defaultOpenLevel = ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                            .levelStep = ValveConfigurationAndControlCluster::kDefaultLevelStep
                                                          }, 
                                                          &codegenTracker);
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

void MatterValveConfigurationAndControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterValveConfigurationAndControlClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);
}

void MatterValveConfigurationAndControlPluginServerInitCallback() {}

void MatterValveConfigurationAndControlPluginServerShutdownCallback() {}

namespace chip::app::Clusters::ValveConfigurationAndControl {

ValveConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * valveConfigurationAndControl = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ValveConfigurationAndControlCluster *>(valveConfigurationAndControl);
}

void SetDefaultDelegate(EndpointId endpointId, Delegate * delegate)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(interface != nullptr);
    interface->SetDelegate(delegate);
}

CHIP_ERROR CloseValve(chip::EndpointId ep)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_UNINITIALIZED);
    return interface->CloseValve();
}

CHIP_ERROR SetValveLevel(EndpointId ep, DataModel::Nullable<Percent> level, DataModel::Nullable<uint32_t> openDuration)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_UNINITIALIZED);
    return interface->SetValveLevel(level, openDuration);
}

CHIP_ERROR UpdateCurrentLevel(chip::EndpointId ep, chip::Percent currentLevel)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_UNINITIALIZED);
    interface->UpdateCurrentLevel(currentLevel);
    return CHIP_NO_ERROR;
}

CHIP_ERROR UpdateCurrentState(chip::EndpointId ep, ValveConfigurationAndControl::ValveStateEnum currentState)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_UNINITIALIZED);
    interface->UpdateCurrentState(currentState);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmitValveFault(chip::EndpointId ep, chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ValveConfigurationAndControlCluster * interface = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_UNINITIALIZED);
    interface->EmitValveFault(fault);
    return CHIP_NO_ERROR;
}

void UpdateAutoCloseTime(uint64_t time)
{
    for (size_t serverIndex = 0; serverIndex < kValveConfigurationAndControlMaxClusterCount; serverIndex++)
    {
        if (gServers[serverIndex].IsConstructed())
        {
            gServers[serverIndex].Cluster().UpdateAutoCloseTime(time);
        }
    }
}

} // namespace chip::app::Clusters::ValveConfigurationAndControl
