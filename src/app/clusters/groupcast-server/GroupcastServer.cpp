#include "GroupcastServer.h"


using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace {
static GroupcastCluster sInstance;
} // namespace

GroupcastCluster & GroupcastCluster::Instance()
{
    return sInstance;
}

GroupcastCluster::GroupcastCluster() : DefaultServerCluster({ kRootEndpointId, Groupcast::Id })
{
}

DataModel::ActionReturnStatus GroupcastCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        break;
    case Groupcast::Attributes::ClusterRevision::Id:
        return aEncoder.Encode(Groupcast::kRevision);
    case Groupcast::Attributes::Membership::Id:
        return mLogic.ReadMembership(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMembershipCount::Id:
        return mLogic.ReadMaxMembershipCount(request.path.mEndpointId, encoder);
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

CHIP_ERROR GroupcastCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using chip::app::Clusters::Groupcast::Attributes;
    // Ensure space for all possible attributes (2 mandatory + global)
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(2 + DefaultServerCluster::GlobalAttributes().size()));
    // Mandatory attributes
    ReturnErrorOnFailure(builder.AppendElements({
        Attributes::Membership::kMetadataEntry,
        Attributes::MaxMembershipCount::kMetadataEntry,
    }));
    // Global attributes
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

std::optional<DataModel::ActionReturnStatus> GroupcastCluster::InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & arguments, CommandHandler * handler)
{
    VerifyOrReturnValue(nullptr != handler, Protocols::InteractionModel::Status::InvalidAction);
    FabricIndex fabric_index = handler->GetAccessingFabricIndex();
    switch (request.path.mCommandId)
    {
    case Groupcast::Commands::JoinGroup::Id:
    {
        Groupcast::Commands::JoinGroup::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.JoinGroup(fabric_index, data);
    }
    case Groupcast::Commands::LeaveGroup::Id:
    {
        Groupcast::Commands::LeaveGroup::DecodableType data;
        Groupcast::Commands::LeaveGroupResponse::Type response;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        mLogic.LeaveGroup(fabric_index, data, response);
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }
    case Groupcast::Commands::UpdateGroupKey::Id:
    {
        Groupcast::Commands::UpdateGroupKey::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.UpdateGroupKey(fabric_index, data);
    }
    case Groupcast::Commands::ExpireGracePeriod::Id:
    {
        Groupcast::Commands::ExpireGracePeriod::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.ExpireGracePeriod(fabric_index, data);
    }
    case Groupcast::Commands::ConfigureAuxiliaryACL::Id:
    {
        Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.ConfigureAuxiliaryACL(fabric_index, data);
    }
    }
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}

} // namespace Clusters
} // namespace app
} // namespace chip
