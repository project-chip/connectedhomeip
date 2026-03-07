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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/closure-control-server/ClosureControlCluster.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/static-cluster-config/ClosureControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DefaultTimerDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureControl::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kClosureControlFixedClusterCount = ClosureControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kClosureControlMaxClusterCount   = kClosureControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ClosureControlCluster> gServer[kClosureControlMaxClusterCount];
ClosureControlClusterDelegate * gDelegates[kClosureControlMaxClusterCount] = { nullptr };
ClusterConformance gConformances[kClosureControlMaxClusterCount];
ClusterInitParameters gInitParams[kClosureControlMaxClusterCount];
DefaultTimerDelegate gTimerDelegate;
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

ClosureControlCluster * GetInstance(EndpointId endpointId)
{
    if (gServer[endpointId].IsConstructed())
    {
        return &gServer[endpointId].Cluster();
    }
    ChipLogError(Zcl, "Closure Control Cluster not initialized.");
    return nullptr;
}

void MatterClosureControlSetDelegate(EndpointId endpointId, ClosureControlClusterDelegate & delegate)
{
    VerifyOrReturn(!gServer[endpointId].IsConstructed(),
                   ChipLogError(Zcl, "Closure Control Cluster already initialized. Cannot set delegate."));
    gDelegates[endpointId] = &delegate;
}

void MatterClosureControlSetConformance(EndpointId endpointId, const ClusterConformance & conformance)
{
    VerifyOrReturn(!gServer[endpointId].IsConstructed(),
                   ChipLogError(Zcl, "Closure Control Cluster already initialized. Cannot set conformance."));
    gConformances[endpointId] = conformance;
}

void MatterClosureControlSetInitParams(EndpointId endpointId, const ClusterInitParameters & initParams)
{
    VerifyOrReturn(!gServer[endpointId].IsConstructed(),
                   ChipLogError(Zcl, "Closure Control Cluster already initialized. Cannot set init params."));
    gInitParams[endpointId] = initParams;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterClosureControlClusterInitCallback(EndpointId endpointId)
{
    if (endpointId > kClosureControlMaxClusterCount)
    {
        ChipLogError(Zcl, "Closure Control Cluster cannot be initialized on endpoint %u. Endpoint ID is out of range.", endpointId);
        return;
    }

    if (gServer[endpointId].IsConstructed())
    {
        ChipLogError(Zcl, "Closure Control Cluster already initialized. Ignoring duplicate initialization.");
        return;
    }

    if (gDelegates[endpointId] == nullptr)
    {
        ChipLogError(Zcl,
                     "Closure Control Cluster cannot be initialized without a delegate. Call MatterClosureControlSetDelegate() "
                     "before ServerInit().");
        return;
    }

    ClosureControlCluster::Context context{ *gDelegates[endpointId], gTimerDelegate, gConformances[endpointId],
                                            gInitParams[endpointId] };
    gServer[endpointId].Create(endpointId, context);
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(gServer[endpointId].Registration()));
}

void MatterClosureControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    if (!gServer[endpointId].IsConstructed())
    {
        ChipLogError(Zcl, "Closure Control Cluster not initialized. Ignoring shutdown.");
        return;
    }

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&gServer[endpointId].Cluster()));
    gServer[endpointId].Destroy();
}
// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureControlPluginServerInitCallback() {}
void MatterClosureControlPluginServerShutdownCallback() {}
