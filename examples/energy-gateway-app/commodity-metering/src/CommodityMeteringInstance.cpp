/*
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

#include <CommodityMeteringInstance.h>
#include <app/util/af-types.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityMetering;

namespace {
static std::unique_ptr<Instance> gCommodityMeteringCluster;
} // namespace

Instance * CommodityMetering::GetInstance()
{
    return gCommodityMeteringCluster.get();
}

void emberAfCommodityMeteringClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gCommodityMeteringCluster == nullptr);
    gCommodityMeteringCluster =
        std::make_unique<Instance>(endpointId);
    gCommodityMeteringCluster->Init();
}

void emberAfCommodityMeteringClusterShutdownCallback(chip::EndpointId endpointId)
{
    if (gCommodityMeteringCluster)
    {
        gCommodityMeteringCluster->Shutdown();
        gCommodityMeteringCluster.reset(nullptr);
    }
}
