/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/FailSafeContext.h>
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <app/reporting/reporting.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

class ServerInstance : public CommandHandlerInterface,
                       public AttributeAccessInterface,
                       public Delegate::ActivateDatasetCallback,
                       public Delegate::AttributeChangeCallback
{
public:
    using Status = Protocols::InteractionModel::Status;
    ServerInstance(EndpointId endpointId, Delegate * delegate, FailSafeContext & failSafeContext) :
        CommandHandlerInterface(Optional<EndpointId>(endpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(endpointId), Id), mDelegate(delegate), mServerEndpointId(endpointId),
        mFailsafeContext(failSafeContext)
    {}
    virtual ~ServerInstance() = default;

    CHIP_ERROR Init();

    // CommandHanlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // ActivateDatasetCallback
    void OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error) override;

    // AttributeChangeCallback
    void ReportAttributeChanged(AttributeId attributeId) override;

private:
    // TODO: Split the business logic from the unit test class
    friend class TestThreadBorderRouterManagementCluster;
    // Command Handlers
    Status HandleGetActiveDatasetRequest(HandlerContext & ctx, Thread::OperationalDataset & dataset)
    {
        return HandleGetDatasetRequest(ctx, Delegate::DatasetType::kActive, dataset);
    }
    Status HandleGetPendingDatasetRequest(HandlerContext & ctx, Thread::OperationalDataset & dataset)
    {
        return HandleGetDatasetRequest(ctx, Delegate::DatasetType::kPending, dataset);
    }
    Status HandleSetActiveDatasetRequest(HandlerContext & ctx, const Commands::SetActiveDatasetRequest::DecodableType & req);
    Status HandleSetPendingDatasetRequest(HandlerContext & ctx, const Commands::SetPendingDatasetRequest::DecodableType & req);
    Status HandleGetDatasetRequest(HandlerContext & ctx, Delegate::DatasetType type, Thread::OperationalDataset & dataset);

    // Attribute Read handlers
    void ReadFeatureMap(BitFlags<Feature> & feature);
    std::optional<uint64_t> ReadActiveDatasetTimestamp();
    std::optional<uint64_t> ReadPendingDatasetTimestamp();
    CHIP_ERROR ReadBorderRouterName(MutableCharSpan & borderRouterName);
    CHIP_ERROR ReadBorderAgentID(MutableByteSpan & borderAgentId);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SetSkipCASESessionCheck(bool skipCheck) { mSkipCASESessionCheck = skipCheck; }
    bool mSkipCASESessionCheck;
#endif
    bool IsCommandOverCASESession(CommandHandlerInterface::HandlerContext & ctx);
    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void OnFailSafeTimerExpired();
    void CommitSavedBreadcrumb();

    Delegate * mDelegate;
    app::CommandHandler::Handle mAsyncCommandHandle;
    ConcreteCommandPath mPath = ConcreteCommandPath(0, 0, 0);
    Optional<uint64_t> mBreadcrumb;
    uint32_t mSetActiveDatasetSequenceNumber = 0;
    EndpointId mServerEndpointId;
    FailSafeContext & mFailsafeContext;
};

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
