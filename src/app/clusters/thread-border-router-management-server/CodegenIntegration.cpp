/*
 *
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

#include <app/clusters/general-commissioning-server/CodegenIntegration.h>
#include <app/clusters/general-commissioning-server/GeneralCommissioningCluster.h>
#include <app/clusters/thread-border-router-management-server/CodegenIntegration.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

namespace {

class CodegenGeneralCommissioningBreadcrumbTracker : public BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t v) override
    {
        auto cluster = chip::app::Clusters::GeneralCommissioning::Instance();
        if (cluster != nullptr)
        {
            cluster->SetBreadCrumb(v);
        }
    }
};

static CodegenGeneralCommissioningBreadcrumbTracker sBreadcrumbTracker;

} // namespace

ServerInstance::ServerInstance(EndpointId endpointId, Delegate * delegate, FailSafeContext & failSafeContext) :
    mCluster(
        endpointId,
        ThreadBorderRouterManagementCluster::Config(*delegate, failSafeContext, sBreadcrumbTracker, DeviceLayer::PlatformMgr()))
{}

ServerInstance::~ServerInstance()
{
    mCluster.Cluster().Shutdown(ClusterShutdownType::kClusterShutdown);
    // Note: Unregister always errors out if Init() was never called.
    // We expect Init() to essentially be always called if the instance is used.
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(Id), err.Format());
    }
}

CHIP_ERROR ServerInstance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mCluster.Cluster().GetPaths()[0].mEndpointId, Id) == true);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(Id), err.Format());
    }
    return err;
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThreadBorderRouterManagementPluginServerInitCallback() {}
void MatterThreadBorderRouterManagementPluginServerShutdownCallback() {}

void MatterThreadBorderRouterManagementClusterInitCallback(chip::EndpointId endpointId) {}
void MatterThreadBorderRouterManagementClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType shutdownType)
{}
