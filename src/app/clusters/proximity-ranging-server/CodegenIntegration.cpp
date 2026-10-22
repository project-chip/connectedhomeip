/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/proximity-ranging-server/CodegenIntegration.h>

#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

ProximityRangingServer::~ProximityRangingServer()
{
    Deinit();
}

CHIP_ERROR ProximityRangingServer::Init(BitMask<Feature> features)
{
    VerifyOrReturnError(!mCluster.IsConstructed(), CHIP_ERROR_ALREADY_INITIALIZED);

    mCluster.Create(mEndpointId, ProximityRangingCluster::Config(mTimerDelegate).WithFeatures(features).WithAdapters(mAdapters));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        mCluster.Destroy();
        return err;
    }
    return CHIP_NO_ERROR;
}

void ProximityRangingServer::Deinit()
{
    if (!mCluster.IsConstructed())
    {
        return;
    }

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster()));
    mCluster.Destroy();
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip

// The Proximity Ranging cluster requires a TimerDelegate and an adapter set
// at construction. The auto-generated init callbacks fire before the
// application has supplied either, so cluster construction and registration
// are deferred to the application via ProximityRangingServer (declared above).
// These callbacks are intentional no-ops.
void MatterProximityRangingClusterInitCallback(chip::EndpointId) {}
void MatterProximityRangingClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}

void MatterProximityRangingPluginServerInitCallback() {}
void MatterProximityRangingPluginServerShutdownCallback() {}
