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

#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementCluster.h>

#include <app/CommandHandler.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <clusters/ThreadBorderRouterManagement/Commands.h>
#include <clusters/ThreadBorderRouterManagement/Metadata.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters {

using namespace ThreadBorderRouterManagement;

DataModel::ActionReturnStatus ThreadBorderRouterManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(ThreadBorderRouterManagement::kRevision);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case ThreadBorderRouterManagement::Attributes::BorderRouterName::Id: {
        char buffer[ThreadBorderRouterManagementDelegate::kBorderRouterNameMaxLength];
        MutableCharSpan name(buffer, sizeof(buffer));
        mDelegate.GetBorderRouterName(name);
        return encoder.Encode(name);
    }
    case ThreadBorderRouterManagement::Attributes::BorderAgentID::Id: {
        uint8_t buffer[ThreadBorderRouterManagementDelegate::kBorderAgentIdLength];
        MutableByteSpan agentId(buffer, sizeof(buffer));
        ReturnErrorOnFailure(mDelegate.GetBorderAgentId(agentId));
        if (agentId.size() != ThreadBorderRouterManagementDelegate::kBorderAgentIdLength)
        {
            return CHIP_ERROR_READ_FAILED;
        }
        return encoder.Encode(agentId);
    }
    case ThreadBorderRouterManagement::Attributes::ThreadVersion::Id:
        return encoder.Encode(mDelegate.GetThreadVersion());
    case ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id:
        return encoder.Encode(mDelegate.GetInterfaceEnabled());
    case ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id: {
        uint64_t timestamp = 0;
        Thread::OperationalDataset dataset;
        if ((mDelegate.GetDataset(dataset, ThreadBorderRouterManagementDelegate::DatasetType::kActive) == CHIP_NO_ERROR) &&
            (dataset.GetActiveTimestamp(timestamp) == CHIP_NO_ERROR))
        {
            return encoder.Encode(DataModel::Nullable<uint64_t>(timestamp));
        }
        return encoder.Encode(DataModel::Nullable<uint64_t>());
    }
    case ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id: {
        uint64_t timestamp = 0;
        Thread::OperationalDataset dataset;
        if ((mDelegate.GetDataset(dataset, ThreadBorderRouterManagementDelegate::DatasetType::kPending) == CHIP_NO_ERROR) &&
            (dataset.GetActiveTimestamp(timestamp) == CHIP_NO_ERROR))
        {
            return encoder.Encode(DataModel::Nullable<uint64_t>(timestamp));
        }
        return encoder.Encode(DataModel::Nullable<uint64_t>());
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ThreadBorderRouterManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ThreadBorderRouterManagement::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR ThreadBorderRouterManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(4));
    ReturnErrorOnFailure(builder.Append(ThreadBorderRouterManagement::Commands::GetActiveDatasetRequest::kMetadataEntry));
    ReturnErrorOnFailure(builder.Append(ThreadBorderRouterManagement::Commands::GetPendingDatasetRequest::kMetadataEntry));
    ReturnErrorOnFailure(builder.Append(ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::kMetadataEntry));

    if (mDelegate.GetPanChangeSupported())
    {
        ReturnErrorOnFailure(builder.Append(ThreadBorderRouterManagement::Commands::SetPendingDatasetRequest::kMetadataEntry));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadBorderRouterManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                                  ReadOnlyBufferBuilder<CommandId> & builder)
{
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
    ReturnErrorOnFailure(builder.Append(ThreadBorderRouterManagement::Commands::DatasetResponse::Id));
    return CHIP_NO_ERROR;
}

ThreadBorderRouterManagementCluster::ThreadBorderRouterManagementCluster(EndpointId endpoint, const Config & config) :
    DefaultServerCluster({ endpoint, ThreadBorderRouterManagement::Id }), mDelegate(config.mDelegate),
    mFailSafeContext(config.mFailSafeContext), mBreadcrumbTracker(config.mBreadcrumbTracker),
    mPlatformManager(config.mPlatformManager)
{
    if (mDelegate.GetPanChangeSupported())
    {
        mFeatureMap.Set(ThreadBorderRouterManagement::Feature::kPANChange);
    }
}

ThreadBorderRouterManagementCluster::~ThreadBorderRouterManagementCluster() {}

CHIP_ERROR ThreadBorderRouterManagementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(app::DefaultServerCluster::Startup(context));
    ReturnErrorOnFailure(mDelegate.Init(static_cast<ThreadBorderRouterManagementDelegate::AttributeChangeCallback *>(this)));
    ReturnErrorOnFailure(mPlatformManager.AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

void ThreadBorderRouterManagementCluster::Shutdown(ClusterShutdownType reason)
{
    mPlatformManager.RemoveEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this));
    // clearing the delegate MUST always succeed.
    RETURN_SAFELY_IGNORED mDelegate.Init(nullptr);
    mAsyncCommandHandle = CommandHandler::Handle();
    app::DefaultServerCluster::Shutdown(reason);
}

std::optional<DataModel::ActionReturnStatus>
ThreadBorderRouterManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & payload,
                                                   CommandHandler * ctx)
{
    if (ctx->GetSubjectDescriptor().authMode != Access::AuthMode::kCase)
    {
        return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedAccess));
    }

    switch (request.path.mCommandId)
    {
    case ThreadBorderRouterManagement::Commands::GetActiveDatasetRequest::Id: {
        ThreadBorderRouterManagement::Commands::GetActiveDatasetRequest::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(payload, req));

        Thread::OperationalDataset dataset;
        CHIP_ERROR err = mDelegate.GetDataset(dataset, ThreadBorderRouterManagementDelegate::DatasetType::kActive);
        if (err != CHIP_NO_ERROR)
        {
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                ReturnErrorOnFailure(dataset.Init(ByteSpan()));
            }
            else
            {
                ReturnErrorOnFailure(err);
            }
        }

        ThreadBorderRouterManagement::Commands::DatasetResponse::Type response;
        response.dataset = dataset.AsByteSpan();
        ctx->AddResponse(request.path, response);
        // Return nullopt because we already added the response via ctx->AddResponse.
        // Returning a status here would cause a duplicate response.
        return std::nullopt;
    }
    case ThreadBorderRouterManagement::Commands::GetPendingDatasetRequest::Id: {
        ThreadBorderRouterManagement::Commands::GetPendingDatasetRequest::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(payload, req));

        Thread::OperationalDataset dataset;
        CHIP_ERROR err = mDelegate.GetDataset(dataset, ThreadBorderRouterManagementDelegate::DatasetType::kPending);
        if (err != CHIP_NO_ERROR)
        {
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                ReturnErrorOnFailure(dataset.Init(ByteSpan()));
            }
            else
            {
                ReturnErrorOnFailure(err);
            }
        }

        ThreadBorderRouterManagement::Commands::DatasetResponse::Type response;
        response.dataset = dataset.AsByteSpan();
        ctx->AddResponse(request.path, response);
        // Return nullopt because we already added the response via ctx->AddResponse.
        // Returning a status here would cause a duplicate response.
        return std::nullopt;
    }
    case ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::Id: {
        ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(payload, req));

        if (!mFailSafeContext.IsFailSafeArmed(ctx->GetAccessingFabricIndex()))
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::FailsafeRequired));
        }

        Thread::OperationalDataset activeDataset;
        if (activeDataset.Init(req.activeDataset) != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand));
        }

        Thread::OperationalDataset currentActiveDataset;
        uint64_t currentActiveDatasetTimestamp = 0;
        if ((mDelegate.GetDataset(currentActiveDataset, ThreadBorderRouterManagementDelegate::DatasetType::kActive) ==
             CHIP_NO_ERROR) &&
            (currentActiveDataset.GetActiveTimestamp(currentActiveDatasetTimestamp) == CHIP_NO_ERROR))
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidInState));
        }

        if (mAsyncCommandHandle.Get())
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::Busy));
        }

        mAsyncCommandHandle = CommandHandler::Handle(ctx);
        mBreadcrumb         = req.breadcrumb;
        mSetActiveDatasetSequenceNumber++;

        ctx->FlushAcksRightAwayOnSlowCommand();
        mDelegate.SetActiveDataset(activeDataset, mSetActiveDatasetSequenceNumber,
                                   static_cast<ThreadBorderRouterManagementDelegate::ActivateDatasetCallback *>(this));

        // Return nullopt because this is an async operation. The response will be sent
        // later in the OnActivateDatasetComplete callback.
        return std::nullopt;
    }
    case ThreadBorderRouterManagement::Commands::SetPendingDatasetRequest::Id: {
        ThreadBorderRouterManagement::Commands::SetPendingDatasetRequest::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(payload, req));

        if (!mDelegate.GetPanChangeSupported())
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedCommand));
        }

        Thread::OperationalDataset pendingDataset;
        if (pendingDataset.Init(req.pendingDataset) != CHIP_NO_ERROR)
        {
            return std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand));
        }

        CHIP_ERROR err = mDelegate.SetPendingDataset(pendingDataset);
        return std::make_optional(DataModel::ActionReturnStatus(app::StatusIB(err).mStatus));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

void ThreadBorderRouterManagementCluster::OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error)
{
    if (mSetActiveDatasetSequenceNumber != sequenceNum)
    {
        return;
    }

    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }

    if (error == CHIP_NO_ERROR && mBreadcrumb.HasValue())
    {
        mBreadcrumbTracker.SetBreadCrumb(mBreadcrumb.Value());
    }
    mBreadcrumb.ClearValue();

    commandHandle->AddStatus(ConcreteCommandPath(mPath.mEndpointId, mPath.mClusterId,
                                                 ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::Id),
                             app::StatusIB(error).mStatus);
}

void ThreadBorderRouterManagementCluster::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    auto * cluster = reinterpret_cast<ThreadBorderRouterManagementCluster *>(arg);

    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        (void) cluster->mDelegate.RevertActiveDataset();

        auto commandHandleRef = std::move(cluster->mAsyncCommandHandle);
        auto commandHandle    = commandHandleRef.Get();
        if (commandHandle != nullptr)
        {
            commandHandle->AddStatus(ConcreteCommandPath(cluster->mPath.mEndpointId, cluster->mPath.mClusterId,
                                                         ThreadBorderRouterManagement::Commands::SetActiveDatasetRequest::Id),
                                     Protocols::InteractionModel::Status::Timeout);
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        LogErrorOnFailure(cluster->mDelegate.CommitActiveDataset());
    }
}

void ThreadBorderRouterManagementCluster::ReportAttributeChanged(AttributeId attributeId)
{
    app::DefaultServerCluster::NotifyAttributeChanged(attributeId);
}

} // namespace chip::app::Clusters
