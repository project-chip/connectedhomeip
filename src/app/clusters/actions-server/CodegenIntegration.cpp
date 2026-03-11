/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;

ActionsServer::ActionsServer(EndpointId endpointId, Delegate & delegate) :
    mCluster(endpointId, delegate, [this, endpointId]() {
        ActionsCluster::ClusterConfig config;
        // Make sure <app/util/attribute-storage.h> is included at the top of the file!
        if (emberAfContainsServerAttribute(endpointId, Actions::Id, Attributes::SetupURL::Id))
        {
            // ADDED 'template' KEYWORD HERE:
            config.optionalAttributes.template ForceSet<Attributes::SetupURL::Id>();

            MutableCharSpan urlSpan(mSetupURLBuffer);
            if (Attributes::SetupURL::Get(endpointId, urlSpan) == Protocols::InteractionModel::Status::Success)
            {
                config.setupURL = CharSpan(urlSpan.data(), urlSpan.size());
            }
        }
        return config;
    }())
{}

ActionsServer::~ActionsServer()
{
    Shutdown();
}

CHIP_ERROR ActionsServer::Init()
{
    // Registers the code-driven cluster with the new framework
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

void ActionsServer::Shutdown()
{
    // Unregisters the code-driven cluster
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(DeviceEnergyManagement::Id), err.Format());
    }
}

void ActionsServer::ActionListModified(EndpointId aEndpoint)
{
    // Proxy the legacy call to the new cluster's centralized notification helper
    mCluster.Cluster().ActionListModified();
}

void ActionsServer::EndpointListModified(EndpointId aEndpoint)
{
    // Proxy the legacy call to the new cluster's centralized notification helper
    mCluster.Cluster().EndpointListsModified();
}

// Stub callbacks: Since we are using the RegisteredServerCluster instantiation pattern
// in the applications themselves, these ZAP-generated callbacks can remain empty stubs.
void MatterActionsClusterInitCallback(EndpointId endpointId) {}
void MatterActionsClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType type) {}
void MatterActionsPluginServerInitCallback() {}
void MatterActionsPluginServerShutdownCallback() {}
