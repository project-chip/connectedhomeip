/**
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

#include "identify-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;
using chip::Protocols::InteractionModel::Status;

// Legacy PluginServer callback stubs
void MatterIdentifyPluginServerInitCallback() {}
void MatterIdentifyPluginServerShutdownCallback() {}

static Identify * firstLegacyIdentify = nullptr;

static Identify * GetLegacyIdentifyInstance(EndpointId endpoint)
{
    Identify * current = firstLegacyIdentify;
    while (current != nullptr && current->mCluster.Cluster().GetPaths()[0].mEndpointId != endpoint)
    {
        current = current->nextIdentify;
    }
    return current;
}

static inline void RegisterLegacyIdentify(Identify * inst)
{
    inst->nextIdentify  = firstLegacyIdentify;
    firstLegacyIdentify = inst;
}

static inline void UnregisterLegacyIdentify(Identify * inst)
{
    if (firstLegacyIdentify == inst)
    {
        firstLegacyIdentify = firstLegacyIdentify->nextIdentify;
    }
    else
    {
        Identify * previous = firstLegacyIdentify;
        Identify * current  = firstLegacyIdentify->nextIdentify;

        while (current != nullptr && current != inst)
        {
            previous = current;
            current  = current->nextIdentify;
        }

        if (current != nullptr)
        {
            previous->nextIdentify = current->nextIdentify;
        }
    }
}

// Legacy Identify callback wrappers that translate new callback types into old callback types
// They also copy member variable state to maintain backwards compatibility
static void OnIdentifyStartLegacyWrapper(chip::app::Clusters::IdentifyCluster * cluster)
{
    Identify * identify = GetLegacyIdentifyInstance(cluster->GetPaths()[0].mEndpointId);
    if (identify != nullptr && identify->mOnIdentifyStart)
    {
        identify->mOnIdentifyStart(identify);
    }
}

static void OnIdentifyStopLegacyWrapper(chip::app::Clusters::IdentifyCluster * cluster)
{
    Identify * identify = GetLegacyIdentifyInstance(cluster->GetPaths()[0].mEndpointId);
    if (identify != nullptr && identify->mOnIdentifyStop)
    {
        identify->mOnIdentifyStop(identify);
    }
}

static void OnEffectIdentifierLegacyWrapper(chip::app::Clusters::IdentifyCluster * cluster)
{
    Identify * identify = GetLegacyIdentifyInstance(cluster->GetPaths()[0].mEndpointId);
    if (identify != nullptr)
    {
        identify->mCurrentEffectIdentifier = cluster->GetEffectIdentifier();
        identify->mEffectVariant           = cluster->GetEffectVariant();
        if (identify->mOnEffectIdentifier)
        {
            identify->mOnEffectIdentifier(identify);
        }
    }
}

Identify::Identify(EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
                   IdentifyTypeEnum identifyType, onEffectIdentifierCb onEffectIdentifier, EffectIdentifierEnum effectIdentifier,
                   EffectVariantEnum effectVariant, reporting::ReportScheduler::TimerDelegate * timerDelegate) :

    mOnIdentifyStart(onIdentifyStart),
    mOnIdentifyStop(onIdentifyStop), mIdentifyType(identifyType), mOnEffectIdentifier(onEffectIdentifier),
    mCurrentEffectIdentifier(effectIdentifier), mEffectVariant(effectVariant),
    mCluster(chip::app::Clusters::IdentifyCluster::Config(endpoint, identifyType)
                 .WithOnIdentifyStart(onIdentifyStart ? OnIdentifyStartLegacyWrapper : nullptr)
                 .WithOnIdentifyStop(onIdentifyStop ? OnIdentifyStopLegacyWrapper : nullptr)
                 .WithOnEffectIdentifier(onEffectIdentifier ? OnEffectIdentifierLegacyWrapper : nullptr)
                 .WithEffectIdentifier(effectIdentifier)
                 .WithEffectVariant(effectVariant)
                 .WithTimerDelegate(timerDelegate))
{
    RegisterLegacyIdentify(this);
};

Identify::~Identify()
{
    UnregisterLegacyIdentify(this);
}

namespace {

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        Identify * identify = GetLegacyIdentifyInstance(endpointId);
        VerifyOrDie(identify != nullptr);
        return identify->mCluster.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override { return nullptr; }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override {}
};

} // namespace

void MatterIdentifyClusterInitCallback(EndpointId endpointId)
{
    if (GetLegacyIdentifyInstance(endpointId) != nullptr)
    {
        IntegrationDelegate integrationDelegate;
        CodegenClusterIntegration::RegisterServer({ .endpointId                = endpointId,
                                                    .clusterId                 = Clusters::Identify::Id,
                                                    .fixedClusterInstanceCount = 1,
                                                    .maxClusterInstanceCount   = 1 },
                                                  integrationDelegate);
    }
}

void MatterIdentifyClusterShutdownCallback(EndpointId endpointId)
{
    if (GetLegacyIdentifyInstance(endpointId) != nullptr)
    {
        IntegrationDelegate integrationDelegate;
        CodegenClusterIntegration::UnregisterServer({ .endpointId                = endpointId,
                                                      .clusterId                 = Clusters::Identify::Id,
                                                      .fixedClusterInstanceCount = 1,
                                                      .maxClusterInstanceCount   = 1 },
                                                    integrationDelegate);
    }
}
