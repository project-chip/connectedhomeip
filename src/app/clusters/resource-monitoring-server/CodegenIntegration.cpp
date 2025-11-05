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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/static-cluster-config/ActivatedCarbonFilterMonitoring.h>
#include <app/static-cluster-config/HepaFilterMonitoring.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

#include <array>
#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace Protocols::InteractionModel;

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ResourceMonitoring;

using chip::Protocols::InteractionModel::Status;

// Helper function to register a legacy instance with the codegen data model provider
inline void RegisterInstance(Instance * instance, EndpointId endpointId, ClusterId clusterId)
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(instance->mCluster.Registration());

    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, endpointId,
                     ChipLogValueMEI(instance->mCluster.Cluster().GetClusterId()), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

Instance::Instance(Delegate * delegate, EndpointId endpointId, ClusterId clusterId, uint32_t featureMap,
                   DegradationDirectionEnum degradationDirection, bool resetConditionCommandSupported) :
    mCluster(endpointId, clusterId, BitFlags<ResourceMonitoring::Feature>{ featureMap },
             OptionalAttributeSet{
                 ResourceMonitoring::Attributes::Condition::Id | ResourceMonitoring::Attributes::DegradationDirection::Id |
                 ResourceMonitoring::Attributes::InPlaceIndicator::Id | ResourceMonitoring::Attributes::LastChangedTime::Id },
             degradationDirection, resetConditionCommandSupported)
{
    mCluster.Cluster().SetDelegate(delegate);
    RegisterInstance(this, endpointId, clusterId);
}

Instance::~Instance()
{
    CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterActivatedCarbonFilterMonitoringClusterInitCallback(EndpointId endpointId) {}

void MatterHepaFilterMonitoringClusterInitCallback(EndpointId endpointId) {}

void MatterActivatedCarbonFilterMonitoringClusterShutdownCallback(EndpointId endpointId) {}

void MatterHepaFilterMonitoringClusterShutdownCallback(EndpointId endpointId) {}
