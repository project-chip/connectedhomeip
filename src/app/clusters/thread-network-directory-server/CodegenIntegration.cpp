/**
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

#include <app/clusters/thread-network-directory-server/CodegenIntegration.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR DefaultThreadNetworkDirectoryServer::Init()
{
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

DefaultThreadNetworkDirectoryServer::~DefaultThreadNetworkDirectoryServer()
{

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster on endpoint %u: %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, err.Format());
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThreadNetworkDirectoryPluginServerInitCallback() {}
void MatterThreadNetworkDirectoryPluginServerShutdownCallback() {}
