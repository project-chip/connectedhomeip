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
#include "CodegenInstance.h"

#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(mCluster.Cluster().Init());
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

void Instance::Shutdown()
{
    CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    mCluster.Cluster().Shutdown();
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip

// unused bits ember wants
void MatterNetworkCommissioningPluginServerInitCallback() {}
void MatterNetworkCommissioningPluginServerShutdownCallback() {}
