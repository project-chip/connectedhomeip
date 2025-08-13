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

#include <app/clusters/fixed-label-server/fixed-label-cluster.h>
#include <app/static-cluster-config/FixedLabel.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FixedLabel;
using namespace chip::app::Clusters::FixedLabel::Attributes;

namespace {

static constexpr size_t kFixedLabelFixedClusterCount = FixedLabel::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kFixedLabelMaxClusterCount   = kFixedLabelFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<FixedLabelCluster> gServers[kFixedLabelMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool FindEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, FixedLabel::Id, kFixedLabelFixedClusterCount);

    if (arrayIndex >= kFixedLabelMaxClusterCount)
    {
        ChipLogError(AppServer, "Could not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    outArrayIndex = arrayIndex;

    return true;
}

} // namespace

void emberAfFixedLabelClusterServerInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Create(endpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register FixedLabel cluster on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfFixedLabelClusterServerShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister FixedLabel cluster on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[arrayIndex].Destroy();
}

void MatterFixedLabelClusterServerInitCallback(EndpointId endpointId) {}

void MatterFixedLabelClusterServerShutdownCallback(EndpointId endpointId) {}

void MatterFixedLabelPluginServerInitCallback() {}

void MatterFixedLabelPluginServerShutdownCallback() {}
