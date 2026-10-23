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

#include <app/clusters/water-heater-management-server/CodegenIntegration.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

CHIP_ERROR Instance::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    if (err == CHIP_NO_ERROR)
    {
        mRegistered = true;
    }
    return err;
}

void Instance::Shutdown()
{
    if (!mRegistered)
    {
        return;
    }
    mRegistered    = false;
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister WaterHeaterManagement cluster on endpoint %u: %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, err.Format());
    }
}

Instance::~Instance()
{
    if (mRegistered)
    {
        Shutdown();
    }
}

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
