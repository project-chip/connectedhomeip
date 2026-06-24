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

#include <app/FailSafeContext.h>
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ThreadBorderRouterManagement/Ids.h>
#include <lib/support/BitFlags.h>
#include <platform/PlatformManager.h>

namespace chip::app::Clusters {

class ThreadBorderRouterManagementCluster : public DefaultServerCluster,
                                            public ThreadBorderRouterManagementDelegate::ActivateDatasetCallback,
                                            public ThreadBorderRouterManagementDelegate::AttributeChangeCallback
{
public:
    class Config
    {
    public:
        Config(ThreadBorderRouterManagementDelegate & delegate, FailSafeContext & failSafeContext,
               BreadCrumbTracker & breadcrumbTracker, DeviceLayer::PlatformManager & platformManager) :
            mDelegate(delegate),
            mFailSafeContext(failSafeContext), mBreadcrumbTracker(breadcrumbTracker), mPlatformManager(platformManager)
        {}

    private:
        friend class ThreadBorderRouterManagementCluster;
        ThreadBorderRouterManagementDelegate & mDelegate;
        FailSafeContext & mFailSafeContext;
        BreadCrumbTracker & mBreadcrumbTracker;
        DeviceLayer::PlatformManager & mPlatformManager;
    };

    ThreadBorderRouterManagementCluster(EndpointId endpoint, const Config & config);
    ~ThreadBorderRouterManagementCluster();

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    void ReportAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType reason) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & payload,
                                                               CommandHandler * ctx) override;

    // ThreadBorderRouterManagementDelegate::ActivateDatasetCallback
    void OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error) override;

    // Platform event handler
    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

protected:
    ThreadBorderRouterManagementDelegate & mDelegate;
    BitFlags<ThreadBorderRouterManagement::Feature> mFeatureMap;
    FailSafeContext & mFailSafeContext;
    BreadCrumbTracker & mBreadcrumbTracker;
    DeviceLayer::PlatformManager & mPlatformManager;
    CommandHandler::Handle mAsyncCommandHandle;
    uint32_t mSetActiveDatasetSequenceNumber = 0;
    Optional<uint64_t> mBreadcrumb;
};

} // namespace chip::app::Clusters
