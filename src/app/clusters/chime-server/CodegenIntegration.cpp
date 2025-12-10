/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/chime-server/ChimeCluster.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;
using chip::app::Clusters::ChimeServer;

ChimeServer::ChimeServer(EndpointId endpointId, ChimeDelegate & delegate) : mCluster(endpointId, delegate) {}

ChimeServer::~ChimeServer()
{
    RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
}

CHIP_ERROR ChimeServer::Init()
{
    // CodegenDataModelProvider::Instance() is a Meyer’s singleton so it's safe to call this here without worrying about
    // intialization order. It's also OK to Register() the cluster in the provider even if the endpoint is not yet started up. It
    // will be started up when the endpoint is started and a context is set.
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

Protocols::InteractionModel::Status ChimeServer::SetSelectedChime(uint8_t chimeSoundID)
{
    return mCluster.Cluster().SetSelectedChime(chimeSoundID);
}

Protocols::InteractionModel::Status ChimeServer::SetEnabled(bool Enabled)
{
    return mCluster.Cluster().SetEnabled(Enabled);
}

uint8_t ChimeServer::GetSelectedChime() const
{
    return mCluster.Cluster().GetSelectedChime();
}

bool ChimeServer::GetEnabled() const
{
    return mCluster.Cluster().GetEnabled();
}

void MatterChimeClusterInitCallback(EndpointId endpointId) {}
void MatterChimeClusterShutdownCallback(EndpointId endpointId) {}

// Stub callbacks for ZAP generated code
void MatterChimePluginServerInitCallback() {}
void MatterChimePluginServerShutdownCallback() {}
