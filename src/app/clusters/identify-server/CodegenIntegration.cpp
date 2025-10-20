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
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Identify;

using chip::app::Clusters::IdentifyCluster;
using chip::app::Clusters::IdentifyDelegate;
using chip::Protocols::InteractionModel::Status;

Identify * firstLegacyIdentify = nullptr;
DefaultTimerDelegate sDefaultTimerDelegate;

Identify * GetLegacyIdentifyInstance(EndpointId endpoint)
{
    Identify * current = firstLegacyIdentify;
    while (current != nullptr && current->mCluster.Cluster().GetPaths()[0].mEndpointId != endpoint)
    {
        current = current->nextIdentify;
    }
    return current;
}

class IdentifyLegacyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override
    {
        Identify * identify = GetLegacyIdentifyInstance(cluster.GetPaths()[0].mEndpointId);
        if (identify != nullptr)
        {
            identify->mActive = true;
            if (identify->mOnIdentifyStart)
            {
                identify->mOnIdentifyStart(identify);
            }
        }
    }
    void OnIdentifyStop(IdentifyCluster & cluster) override
    {
        Identify * identify = GetLegacyIdentifyInstance(cluster.GetPaths()[0].mEndpointId);
        if (identify != nullptr)
        {
            identify->mActive = false;
            if (identify->mOnIdentifyStop)
            {
                identify->mOnIdentifyStop(identify);
            }
        }
    }
    void OnTriggerEffect(IdentifyCluster & cluster) override
    {
        Identify * identify = GetLegacyIdentifyInstance(cluster.GetPaths()[0].mEndpointId);
        if (identify != nullptr)
        {
            identify->mCurrentEffectIdentifier = cluster.GetEffectIdentifier();
            identify->mEffectVariant           = cluster.GetEffectVariant();
            if (identify->mOnEffectIdentifier)
            {
                identify->mOnEffectIdentifier(identify);
            }
        }
    }
    bool IsTriggerEffectEnabled() const override { return true; }
};

IdentifyLegacyDelegate gLegacyDelegate;

inline void RegisterLegacyIdentify(Identify * inst)
{
    inst->nextIdentify  = firstLegacyIdentify;
    firstLegacyIdentify = inst;
}

inline void UnregisterLegacyIdentify(Identify * inst)
{
    if (firstLegacyIdentify == inst)
    {
        firstLegacyIdentify = firstLegacyIdentify->nextIdentify;
    }
    else if (firstLegacyIdentify != nullptr)
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

} // namespace

IdentifyCluster * FindIdentifyClusterOnEndpoint(EndpointId endpointId)
{
    Identify * legacyInstance = GetLegacyIdentifyInstance(endpointId);
    if (legacyInstance != nullptr)
    {
        return &legacyInstance->mCluster.Cluster();
    }
    return nullptr;
}

Identify::Identify(EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
                   IdentifyTypeEnum identifyType, onEffectIdentifierCb onEffectIdentifier, EffectIdentifierEnum effectIdentifier,
                   EffectVariantEnum effectVariant, reporting::ReportScheduler::TimerDelegate * timerDelegate) :

    mOnIdentifyStart(onIdentifyStart),
    mOnIdentifyStop(onIdentifyStop), mIdentifyType(identifyType), mOnEffectIdentifier(onEffectIdentifier),
    mCurrentEffectIdentifier(effectIdentifier), mEffectVariant(effectVariant),
    mCluster(chip::app::Clusters::IdentifyCluster::Config(endpoint, timerDelegate ? *timerDelegate : sDefaultTimerDelegate)
                 .WithIdentifyType(identifyType)
                 .WithDelegate(&gLegacyDelegate)
                 .WithEffectIdentifier(effectIdentifier)
                 .WithEffectVariant(effectVariant))
{
    RegisterLegacyIdentify(this);
};

Identify::~Identify()
{
    UnregisterLegacyIdentify(this);
}

void MatterIdentifyClusterInitCallback(EndpointId endpointId)
{
    Identify * identify = GetLegacyIdentifyInstance(endpointId);
    if (identify != nullptr)
    {
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(identify->mCluster.Registration());

        if (err != CHIP_NO_ERROR)
        {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
            ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, endpointId,
                         ChipLogValueMEI(Clusters::Identify::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        }
    }
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    else
    {
        ChipLogError(AppServer, "Failed to find Instance of Identify cluster for endpoint %u", endpointId);
    }
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
}

void MatterIdentifyClusterShutdownCallback(EndpointId endpointId)
{
    Identify * identify = GetLegacyIdentifyInstance(endpointId);
    if (identify != nullptr)
    {
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&(identify->mCluster.Cluster()));

        if (err != CHIP_NO_ERROR)
        {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
            ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT, endpointId,
                         ChipLogValueMEI(Clusters::Identify::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        }
    }
}

// Legacy PluginServer callback stubs
void MatterIdentifyPluginServerInitCallback() {}
void MatterIdentifyPluginServerShutdownCallback() {}
