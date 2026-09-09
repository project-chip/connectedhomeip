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

#include <app/icd/server/ICDServerConfig.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER

#include <array>

#include <app/clusters/icd-management-server/ICDManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <crypto/SessionKeystore.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/root-node/RootNode.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/// Feature policy that adds ICD (Intermittently Connected Device) support to
/// the root endpoint.
///
/// Owns the `ICDManagement` cluster on the root endpoint so the device
/// advertises as an ICD. The `ICDManager` itself lives on
/// `Server::GetInstance()` and is initialized automatically by the CHIP
/// application server when `CHIP_CONFIG_ENABLE_ICD_SERVER=1`.
class IcdFeature
{
public:
    struct Context
    {
        Crypto::SymmetricKeystore & symmetricKeystore;
    };

    explicit IcdFeature(const Context & context) : mContext(context) {}

    static constexpr std::array<DataModel::DeviceTypeEntry, 0> kExtraDeviceTypes{};
    static constexpr std::array<ClusterId, 0> kExtraClientClusters{};

    CHIP_ERROR RegisterFeatureClusters(EndpointId endpointId, CodeDrivenDataModelProvider & provider,
                                       RootNode::Context & rootContext,
                                       Clusters::GeneralCommissioningCluster & generalCommissioning);

    void UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider);

private:
    Context mContext;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    LazyRegisteredServerCluster<Clusters::ICDManagementClusterWithCIP> mIcdManagementCluster;
#else
    LazyRegisteredServerCluster<Clusters::ICDManagementCluster> mIcdManagementCluster;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
};

} // namespace app
} // namespace chip

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
