/*
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

#include <app/clusters/actions-server/ActionsCluster.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/core/CHIPConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;

namespace {

// Actions cluster is code-driven, so we don't have static cluster config
// Support only dynamic endpoints for now
static constexpr uint16_t kActionsFixedClusterCount = 0;
static constexpr uint16_t kActionsMaxClusterCount   = CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ActionsCluster> gServers[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];

// Null delegate implementation for placeholder use
// TODO: Applications should provide their own delegate implementation
class NullDelegate : public Delegate
{
public:
    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    bool HaveActionWithId(uint16_t aActionId, uint16_t & aActionIndex) override { return false; }

    Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                          Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                       Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                        Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }
};

static NullDelegate gNullDelegate;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // Note: Applications should provide their own delegate implementation
        // For now, we use a null delegate that returns failure for all operations
        // TODO: Implement proper delegate retrieval mechanism
        gServers[clusterInstanceIndex].Create(endpointId, gNullDelegate);

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

void MatterActionsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Actions::Id,
            .fixedClusterInstanceCount = kActionsFixedClusterCount,
            .maxClusterInstanceCount   = kActionsMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterActionsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Actions::Id,
            .fixedClusterInstanceCount = kActionsFixedClusterCount,
            .maxClusterInstanceCount   = kActionsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterActionsPluginServerInitCallback() {}
void MatterActionsPluginServerShutdownCallback() {}
