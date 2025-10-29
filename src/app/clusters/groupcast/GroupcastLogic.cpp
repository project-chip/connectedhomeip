#include "GroupcastLogic.h"

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR GroupcastLogic::ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                      Groupcast::Commands::LeaveGroupResponse::Type & response)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::ExpireGracePeriod(FabricIndex fabric_index,
                                             const Groupcast::Commands::ExpireGracePeriod::DecodableType & data)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GroupcastLogic::ConfigureAuxiliaryACL(FabricIndex fabric_index,
                                                 const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data)
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
