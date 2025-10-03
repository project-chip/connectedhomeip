#include "GroupcastServer.h"
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>

#include <access/examples/GroupcastAccessControlDelegate.h>
#include <app/server/Server.h>
#include <credentials/GroupcastDataProvider.h>

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

GroupcastCluster::GroupcastCluster() : DefaultServerCluster({ kRootEndpointId, Groupcast::Id }) {}

DataModel::ActionReturnStatus GroupcastCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        break;
    case Groupcast::Attributes::Membership::Id:
        return mLogic.ReadMembership(request.path.mEndpointId, encoder);

    case Groupcast::Attributes::MaxMembershipCount::Id:
        return mLogic.ReadMaxMembershipCount(request.path.mEndpointId, encoder);
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus GroupcastCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                               AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        break;
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

CHIP_ERROR GroupcastCluster::Attributes(const ConcreteClusterPath & path,
                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) {}

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
        mLogic.LeaveGroup(fabric_index, data, response);
        handler->AddResponse(request.path, response);
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

// CHIP_ERROR GroupcastCluster::AcceptedCommands(const ConcreteClusterPath & path,
// ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) {} CHIP_ERROR GroupcastCluster::GeneratedCommands(const
// ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) {} bool GroupcastCluster::PathsContains(const
// ConcreteClusterPath & path);

} // namespace Clusters
} // namespace app
} // namespace chip
