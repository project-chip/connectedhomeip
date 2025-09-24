#include "GroupcastLogic.h"

#include <app/server/Server.h>
#include <algorithm>

namespace chip {
namespace app {
namespace Clusters {

using Protocols::InteractionModel::Status;
using DataModel::ActionReturnStatus;
static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;


DataModel::ActionReturnStatus GroupcastLogic::ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                  Groupcast::Commands::LeaveGroupResponse::Type &response)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::ExpireGracePeriod(FabricIndex fabric_index, const Groupcast::Commands::ExpireGracePeriod::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

DataModel::ActionReturnStatus GroupcastLogic::ConfigureAuxiliaryACL(FabricIndex fabric_index, const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::RegisterAccessControl(FabricIndex fabric_index, GroupId group_id)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Clusters
} // namespace app
} // namespace chip