/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaFileManagement;
using Protocols::InteractionModel::Status;

namespace {

BitFlags<Feature> ReadFeatureMap(EndpointId endpointId)
{
    // MediaFileManagement is a code-driven cluster, so it does not use the Ember
    // RAM attribute store. Only the ZAP-configured default is available (GetDefault),
    // which is the correct source for the statically-configured feature map.
    uint32_t rawFeatureMap = 0;
    if (Attributes::FeatureMap::GetDefault(endpointId, &rawFeatureMap) != Status::Success)
    {
        rawFeatureMap = 0;
    }
    return BitFlags<Feature>(rawFeatureMap);
}

} // namespace

namespace chip::app::Clusters::MediaFileManagement {

MediaFileManagementServer::MediaFileManagementServer(EndpointId endpointId, Delegate & delegate) :
    mCluster(endpointId, delegate, ReadFeatureMap(endpointId))
{}

MediaFileManagementServer::~MediaFileManagementServer()
{
    if (mRegistered)
    {
        ChipLogError(AppServer, "MediaFileManagementServer destroyed without Shutdown() being called; shutting down now.");
        Shutdown();
    }
}

CHIP_ERROR MediaFileManagementServer::Init()
{
    VerifyOrReturnError(!mRegistered, CHIP_NO_ERROR);
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration()));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void MediaFileManagementServer::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered    = false;
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        [[maybe_unused]] const ConcreteClusterPath path = mCluster.Cluster().GetPaths()[0];
        ChipLogError(AppServer, "Failed to unregister MediaFileManagement cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     path.mEndpointId, ChipLogValueMEI(path.mClusterId), err.Format());
    }
}

} // namespace chip::app::Clusters::MediaFileManagement

// ZAP-generated plugin callbacks are left as stubs. Applications instantiate
// MediaFileManagementServer directly and register it via Init(), consistent with
// the code-driven cluster pattern where the application owns the cluster lifecycle.
void MatterMediaFileManagementClusterInitCallback(EndpointId endpointId) {}
void MatterMediaFileManagementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType type) {}
void MatterMediaFileManagementPluginServerInitCallback() {}
void MatterMediaFileManagementPluginServerShutdownCallback() {}
