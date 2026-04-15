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
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/chime-server/ChimeCluster.h>
#include <app/clusters/chime-server/MigrateChimeServerStorage.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::app::Clusters::ChimeServer;

CHIP_ERROR CodegenChimeCluster::Startup(ServerClusterContext & context)
{
    // Migrate attributes for this cluster from SafeAttribute to AttributePersistence.
    // This is done at Startup time when the persistence providers are guaranteed to be available.
    SafeAttributePersistenceProvider * srcProvider = GetSafeAttributePersistenceProvider();
    AttributePersistenceProvider & dstProvider     = context.attributeStorage;

    if (srcProvider != nullptr)
    {
        LogErrorOnFailure(Chime::MigrateChimeServerStorage(mPath.mEndpointId, *srcProvider, dstProvider));
    }

    return ChimeCluster::Startup(context);
}

ChimeServer::ChimeServer(EndpointId endpointId, ChimeDelegate & delegate) : mEndpointId(endpointId), mDelegate(&delegate) {}

ChimeServer::~ChimeServer()
{
    if (mCluster.IsConstructed())
    {
        RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
    }
}

CHIP_ERROR ChimeServer::Init()
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mCluster.Create(mEndpointId, *mDelegate);
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

Protocols::InteractionModel::Status ChimeServer::SetSelectedChime(uint8_t chimeSoundID)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SetSelectedChime(chimeSoundID);
}

Protocols::InteractionModel::Status ChimeServer::SetEnabled(bool Enabled)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().SetEnabled(Enabled);
}

uint8_t ChimeServer::GetSelectedChime() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetSelectedChime();
}

bool ChimeServer::GetEnabled() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetEnabled();
}

void MatterChimeClusterInitCallback(EndpointId) {}
void MatterChimeClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}

// Stub callbacks for ZAP generated code
void MatterChimePluginServerInitCallback() {}
void MatterChimePluginServerShutdownCallback() {}
