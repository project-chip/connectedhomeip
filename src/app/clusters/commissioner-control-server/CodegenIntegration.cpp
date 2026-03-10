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

#include <app/clusters/commissioner-control-server/CodegenIntegration.h>
#include <app/static-cluster-config/CommissionerControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissionerControl;
using namespace chip::app::Clusters::CommissionerControl::Attributes;

namespace {

constexpr size_t kCommissionerControlFixedClusterCount = CommissionerControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kCommissionerControlMaxClusterCount =
    kCommissionerControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<CommissionerControlCluster> gServers[kCommissionerControlMaxClusterCount];

CommissionerControl::Delegate * gDelegate = nullptr;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId, gDelegate);
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

void MatterCommissionerControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = CommissionerControl::Id,
            .fixedClusterInstanceCount = kCommissionerControlFixedClusterCount,
            .maxClusterInstanceCount   = kCommissionerControlMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterCommissionerControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = CommissionerControl::Id,
            .fixedClusterInstanceCount = kCommissionerControlFixedClusterCount,
            .maxClusterInstanceCount   = kCommissionerControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterCommissionerControlPluginServerInitCallback() {}

namespace chip::app::Clusters::CommissionerControl {

CommissionerControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * commissionerControl = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = CommissionerControl::Id,
            .fixedClusterInstanceCount = kCommissionerControlFixedClusterCount,
            .maxClusterInstanceCount   = kCommissionerControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<CommissionerControlCluster *>(commissionerControl);
}

CHIP_ERROR SetSupportedDeviceCategories(EndpointId endpointId,
                                        const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories)
{
    auto commissionerControl = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(commissionerControl != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return commissionerControl->SetSupportedDeviceCategories(supportedDeviceCategories);
}

CHIP_ERROR
GenerateCommissioningRequestResultEvent(EndpointId endpointId,
                                        const CommissionerControl::Events::CommissioningRequestResult::Type & result)
{
    auto commissionerControl = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(commissionerControl != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return commissionerControl->GenerateCommissioningRequestResultEvent(result);
}

Delegate * GetDelegate(EndpointId endpointId)
{
    auto commissionerControl = FindClusterOnEndpoint(endpointId);
    if (commissionerControl != nullptr)
    {
        return commissionerControl->GetDelegate();
    }
    return gDelegate;
}

void SetDelegate(EndpointId endpointId, Delegate * delegate)
{
    VerifyOrReturn(delegate != nullptr);
    gDelegate                = delegate;
    auto commissionerControl = FindClusterOnEndpoint(endpointId);
    if (commissionerControl != nullptr)
    {
        commissionerControl->SetDelegate(gDelegate);
    }
}

} // namespace chip::app::Clusters::CommissionerControl
