#include "GroupcastCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/Attributes.h>
#include <clusters/Groupcast/Metadata.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace {

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Groupcast::Commands::JoinGroup::kMetadataEntry,        Groupcast::Commands::LeaveGroup::kMetadataEntry,
    Groupcast::Commands::UpdateGroupKey::kMetadataEntry,   Groupcast::Commands::ConfigureAuxiliaryACL::kMetadataEntry,
    Groupcast::Commands::GroupcastTesting::kMetadataEntry,
};
} // namespace

CHIP_ERROR GroupcastCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    mLogic.SetDataModelProvider(context.provider);
    mLogic.SetListener(this);

    return CHIP_NO_ERROR;
}

void GroupcastCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mGroupcastTestingTimer.Cancel();
    mMembershipChangedTimer.Cancel();
    mLogic.ResetDataModelProvider();
    mLogic.RemoveListener();
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus GroupcastCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        return encoder.Encode(mLogic.Features());
    case Groupcast::Attributes::ClusterRevision::Id:
        return encoder.Encode(Groupcast::kRevision);
    case Groupcast::Attributes::Membership::Id:
        return mLogic.ReadMembership(request.subjectDescriptor, request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMembershipCount::Id:
        return mLogic.ReadMaxMembershipCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMcastAddrCount::Id:
        return mLogic.ReadMaxMcastAddrCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::UsedMcastAddrCount::Id:
        return mLogic.ReadUsedMcastAddrCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::FabricUnderTest::Id:
        return encoder.Encode(mFabricUnderTest);
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

CHIP_ERROR GroupcastCluster::Attributes(const ConcreteClusterPath & path,
                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Groupcast::Attributes::kMandatoryMetadata), {});
}

std::optional<DataModel::ActionReturnStatus> GroupcastCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                             chip::TLV::TLVReader & arguments,
                                                                             CommandHandler * handler)
{
    VerifyOrReturnValue(nullptr != handler, Protocols::InteractionModel::Status::InvalidAction);
    FabricIndex fabric_index = handler->GetAccessingFabricIndex();

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::UnsupportedCommand;

    switch (request.path.mCommandId)
    {
    case Groupcast::Commands::JoinGroup::Id: {
        Groupcast::Commands::JoinGroup::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = mLogic.JoinGroup(fabric_index, data);
    }
    break;
    case Groupcast::Commands::LeaveGroup::Id: {
        Groupcast::Commands::LeaveGroup::DecodableType data;
        Groupcast::Commands::LeaveGroupResponse::Type response;
        GroupcastLogic::EndpointList endpoints;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = mLogic.LeaveGroup(fabric_index, data, endpoints);
        if (Protocols::InteractionModel::Status::Success == status)
        {
            NotifyAttributeChanged(Groupcast::Attributes::Membership::Id);
            response.groupID   = data.groupID;
            response.endpoints = DataModel::List<const chip::EndpointId>(endpoints.entries, endpoints.count);
            handler->AddResponse(request.path, response);
            return std::nullopt; // Response added, must return nullopt.
        }
    }
    break;
    case Groupcast::Commands::UpdateGroupKey::Id: {
        Groupcast::Commands::UpdateGroupKey::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = mLogic.UpdateGroupKey(fabric_index, data);
    }
    break;
    case Groupcast::Commands::ConfigureAuxiliaryACL::Id: {
        Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = mLogic.ConfigureAuxiliaryACL(fabric_index, data);
    }
    break;
    case Groupcast::Commands::GroupcastTesting::Id: {
        Groupcast::Commands::GroupcastTesting::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return GroupcastTesting(fabric_index, data);
    }
    break;
    default:
        break;
    }

    return status;
}

CHIP_ERROR GroupcastCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

Status GroupcastCluster::GroupcastTesting(FabricIndex fabricIndex, Groupcast::Commands::GroupcastTesting::DecodableType data)
{
    VerifyOrReturnError(mFabricUnderTest == kUndefinedFabricIndex || mFabricUnderTest == fabricIndex, Status::ConstraintError);

    if (data.testOperation == Groupcast::GroupcastTestingEnum::kDisableTesting)
    {
        // cancel any existing GroupcastTesting timer
        mGroupcastTestingTimer.Cancel();
        mTestingState = data.testOperation;
        SetFabricUnderTest(kUndefinedFabricIndex);
        return Status::Success;
    }

    constexpr uint16_t kDefaultDurationSeconds = 60;
    uint16_t durationSeconds                   = kDefaultDurationSeconds;
    if (data.durationSeconds.HasValue())
    {
        constexpr uint16_t kMinDurationSeconds = 10, kMaxDurationSeconds = 1200;
        VerifyOrReturnError(data.durationSeconds.Value() >= kMinDurationSeconds &&
                                data.durationSeconds.Value() <= kMaxDurationSeconds,
                            Status::ConstraintError);
        durationSeconds = data.durationSeconds.Value();
    }

    mGroupcastTestingTimer.Start(durationSeconds);

    mTestingState = data.testOperation;
    SetFabricUnderTest(fabricIndex);
    return Status::Success;
}

void GroupcastCluster::SetFabricUnderTest(FabricIndex fabricUnderTest)
{
    SetAttributeValue(mFabricUnderTest, fabricUnderTest, Groupcast::Attributes::FabricUnderTest::Id);
}

void GroupcastCluster::OnMembershipChanged()
{
    mMembershipChangedTimer.Start();
}

void GroupcastCluster::OnUsedMcastAddrCountChange()
{
    NotifyAttributeChanged(Groupcast::Attributes::UsedMcastAddrCount::Id);
}

// MembershipChangedTimer implementation
void GroupcastCluster::MembershipChangedTimer::Start()
{
    VerifyOrReturn(!mCluster.mTimerDelegate.IsTimerActive(this));
    constexpr System::Clock::Milliseconds32 kChangeTemporisation = System::Clock::Milliseconds32(250);
    ReturnAndLogOnFailure(mCluster.mTimerDelegate.StartTimer(this, kChangeTemporisation), AppServer,
                          "Failed to start MembershipChangedTimer");
}

void GroupcastCluster::MembershipChangedTimer::Cancel()
{
    mCluster.mTimerDelegate.CancelTimer(this);
}

void GroupcastCluster::MembershipChangedTimer::TimerFired()
{
    mCluster.NotifyAttributeChanged(Groupcast::Attributes::Membership::Id);
}

// GroupcastTestingTimer implementation
void GroupcastCluster::GroupcastTestingTimer::Start(uint32_t seconds)
{
    Cancel();
    ReturnAndLogOnFailure(mCluster.mTimerDelegate.StartTimer(this, System::Clock::Seconds32(seconds)), AppServer,
                          "Failed to start GroupcastTestingTimer");
}

void GroupcastCluster::GroupcastTestingTimer::Cancel()
{
    if (mCluster.mTimerDelegate.IsTimerActive(this))
    {
        mCluster.mTimerDelegate.CancelTimer(this);
    }
}

void GroupcastCluster::GroupcastTestingTimer::TimerFired()
{
    mCluster.SetFabricUnderTest(kUndefinedFabricIndex);
    mCluster.mTestingState = Groupcast::GroupcastTestingEnum::kDisableTesting;
}
} // namespace Clusters
} // namespace app
} // namespace chip
