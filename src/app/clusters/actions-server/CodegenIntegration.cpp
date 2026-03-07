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

#include "CodegenIntegration.h"
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
// The constructor wires the application's real delegate directly into the new ActionsCluster
ActionsServer::ActionsServer(EndpointId endpointId, Actions::Delegate & delegate) : mCluster(endpointId, delegate) {}

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
    RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
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
