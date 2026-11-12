/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <array>

#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/root-node/RootNode.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {

/// Feature policy that adds Thread support to the root endpoint.
///
/// Owns the Thread flavored `NetworkCommissioning` cluster, wired up to the
/// root-node breadcrumb tracker via `GeneralCommissioningCluster`. Thread
/// network diagnostics live elsewhere in the SDK and are not part of this
/// feature.
class ThreadFeature
{
public:
    struct Context
    {
        DeviceLayer::NetworkCommissioning::ThreadDriver & threadDriver;
    };

    explicit ThreadFeature(const Context & context) : mContext(context) {}

    static constexpr std::array<DataModel::DeviceTypeEntry, 0> kExtraDeviceTypes{};
    static constexpr std::array<ClusterId, 0> kExtraClientClusters{};

    CHIP_ERROR RegisterFeatureClusters(EndpointId endpointId, CodeDrivenDataModelProvider & provider,
                                       RootNode::Context & rootContext,
                                       Clusters::GeneralCommissioningCluster & generalCommissioning);

    void UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider);

private:
    Context mContext;
    LazyRegisteredServerCluster<Clusters::NetworkCommissioningCluster> mNetworkCommissioningCluster;
};

} // namespace app
} // namespace chip
