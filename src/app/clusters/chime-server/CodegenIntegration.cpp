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

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;
using chip::app::Clusters::ChimeServer;

ChimeServer * firstChimeServer = nullptr;

inline void RegisterChimeServer(ChimeServer * inst)
{
    inst->nextChimeServer = firstChimeServer;
    firstChimeServer      = inst;
}

inline void UnregisterChimeServer(ChimeServer * inst)
{
    if (firstChimeServer == inst)
    {
        firstChimeServer = firstChimeServer->nextChimeServer;
    }
    else if (firstChimeServer != nullptr)
    {
        ChimeServer * previous = firstChimeServer;
        ChimeServer * current  = firstChimeServer->nextChimeServer;

        while (current != nullptr && current != inst)
        {
            previous = current;
            current  = current->nextChimeServer;
        }

        if (current != nullptr)
        {
            previous->nextChimeServer = current->nextChimeServer;
        }
    }
}

} // namespace

ChimeServer::ChimeServer(EndpointId endpointId, ChimeDelegate & delegate) : mCluster(endpointId, delegate)
{
    RegisterChimeServer(this);
    // CodegenDataModelProvider::Instance() is a Meyer’s singleton so it's safe to call this here without worrying about
    // intialization order. It's also OK to Register() the cluster in the provider even if the endpoint is not yet started up. It
    // will be started up when the endpoint is started and a context is set.
    RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

ChimeServer::~ChimeServer()
{
    RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
    UnregisterChimeServer(this);
}

CHIP_ERROR ChimeServer::Init()
{
    // Registration is done in constructor
    return CHIP_NO_ERROR;
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

void ChimeServer::ReportInstalledChimeSoundsChange()
{
    mCluster.Cluster().ReportInstalledChimeSoundsChange();
}

void MatterChimeClusterInitCallback(EndpointId endpointId){}
void MatterChimeClusterShutdownCallback(EndpointId endpointId) {}

// Stub callbacks for ZAP generated code
void MatterChimePluginServerInitCallback() {}
void MatterChimePluginServerShutdownCallback() {}
