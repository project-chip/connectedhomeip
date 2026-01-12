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
    Groupcast::Commands::JoinGroup::kMetadataEntry,
    Groupcast::Commands::LeaveGroup::kMetadataEntry,
    Groupcast::Commands::UpdateGroupKey::kMetadataEntry,
    Groupcast::Commands::ExpireGracePeriod::kMetadataEntry,
    Groupcast::Commands::ConfigureAuxiliaryACL::kMetadataEntry,
};
} // namespace

GroupcastCluster::GroupcastCluster(BitFlags<Groupcast::Feature> features) :
    DefaultServerCluster({ kRootEndpointId, Groupcast::Id }), mLogic(features)
{}

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
        return mLogic.ReadMembership(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMembershipCount::Id:
        return mLogic.ReadMaxMembershipCount(request.path.mEndpointId, encoder);
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
    switch (request.path.mCommandId)
    {
    case Groupcast::Commands::JoinGroup::Id: {
        Groupcast::Commands::JoinGroup::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.JoinGroup(fabric_index, data);
    }
    case Groupcast::Commands::LeaveGroup::Id: {
        Groupcast::Commands::LeaveGroup::DecodableType data;
        Groupcast::Commands::LeaveGroupResponse::Type response;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        TEMPORARY_RETURN_IGNORED mLogic.LeaveGroup(fabric_index, data, response);
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }
    case Groupcast::Commands::UpdateGroupKey::Id: {
        Groupcast::Commands::UpdateGroupKey::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.UpdateGroupKey(fabric_index, data);
    }
    case Groupcast::Commands::ExpireGracePeriod::Id: {
        Groupcast::Commands::ExpireGracePeriod::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.ExpireGracePeriod(fabric_index, data);
    }
    case Groupcast::Commands::ConfigureAuxiliaryACL::Id: {
        Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return mLogic.ConfigureAuxiliaryACL(fabric_index, data);
    }
    }
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}

CHIP_ERROR GroupcastCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

} // namespace Clusters
} // namespace app
} // namespace chip
