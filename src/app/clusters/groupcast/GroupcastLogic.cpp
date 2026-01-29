#include "GroupcastLogic.h"
#include <app/server/Server.h>
#include <credentials/GroupDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

using GroupDataProvider = Credentials::GroupDataProvider;
using GroupInfo         = Credentials::GroupDataProvider::GroupInfo;
using GroupEndpoint     = Credentials::GroupDataProvider::GroupEndpoint;
using GroupInfoIterator = Credentials::GroupDataProvider::GroupInfoIterator;
using EndpointIterator  = Credentials::GroupDataProvider::EndpointIterator;

CHIP_ERROR GroupcastLogic::ReadMembership(const chip::Access::SubjectDescriptor * subject, EndpointId endpoint,
                                          AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(nullptr != subject, CHIP_ERROR_INVALID_ARGUMENT);
    FabricIndex fabric_index = subject->fabricIndex;

    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = aEncoder.EncodeList([fabric_index, groups](const auto & encoder) -> CHIP_ERROR {
        EndpointList endpoints;
        CHIP_ERROR status              = CHIP_NO_ERROR;
        GroupInfoIterator * group_iter = groups->IterateGroupInfo(fabric_index);
        VerifyOrReturnError(nullptr != group_iter, CHIP_ERROR_NO_MEMORY);

        GroupInfo info;
        while (group_iter->Next(info) && (CHIP_NO_ERROR == status))
        {
            // Group Key
            KeysetId keyset_id = 0;
            ReturnErrorOnFailure(groups->GetGroupKey(fabric_index, info.group_id, keyset_id));

            // Endpoints
            EndpointIterator * end_iter = groups->IterateEndpoints(fabric_index, info.group_id);
            if (nullptr == end_iter)
            {
                status = CHIP_ERROR_NO_MEMORY;
                break;
            }

            endpoints.count = 0;
            GroupEndpoint mapping;
            while (end_iter->Next(mapping) && (CHIP_NO_ERROR == status))
            {
                endpoints.entries[endpoints.count++] = mapping.endpoint_id;
            }

            Groupcast::Structs::MembershipStruct::Type group;
            group.fabricIndex     = fabric_index;
            group.groupID         = info.group_id;
            group.keySetID        = keyset_id;
            group.hasAuxiliaryACL = info.use_aux_acl;
            group.endpoints       = DataModel::List<const chip::EndpointId>(endpoints.entries, endpoints.count);
            status                = encoder.Encode(group);
            end_iter->Release();
        }
        group_iter->Release();

        return status;
    });

    return err;
}

CHIP_ERROR GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(kMaxMembershipCount);
}

CHIP_ERROR GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnValue(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    // Check groupID
    VerifyOrReturnError(data.groupID != kUndefinedGroupId, CHIP_ERROR_INVALID_ARGUMENT);

    // Check useAuxiliaryACL
    if (data.useAuxiliaryACL.HasValue())
    {
        // AuxiliaryACL can only be present if LN feature is supported
        VerifyOrReturnError(mFeatures.Has(Groupcast::Feature::kListener), CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Check endpoints
    size_t endpoint_count = 0;
    ReturnErrorOnFailure(data.endpoints.ComputeSize(&endpoint_count));
    if (mFeatures.Has(Groupcast::Feature::kListener) && !mFeatures.Has(Groupcast::Feature::kSender))
    {
        // Listener only, endpoints cannot be empty
        VerifyOrReturnError(endpoint_count > 0 && endpoint_count <= kMaxCommandEndpoints, CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (!mFeatures.Has(Groupcast::Feature::kListener) && mFeatures.Has(Groupcast::Feature::kSender))
    {
        // Sender only, endpoints must be empty
        VerifyOrReturnError(0 == endpoint_count, CHIP_ERROR_INVALID_ARGUMENT);
    }
    // Verify endpoint values
    {
        auto iter = data.endpoints.begin();
        while (iter.Next())
        {
            EndpointId ep = iter.GetValue();
            VerifyOrReturnError((ep > 0) && (kInvalidEndpointId != ep), CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    // Check fabric membership entries limit
    GroupDataProvider::GroupInfo info;
    CHIP_ERROR err = groups->GetGroupInfo(fabric_index, data.groupID, info);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);
    // If the group is new, the fabric entries will increase
    uint16_t new_count = (CHIP_ERROR_NOT_FOUND == err) ? info.count + 1 : info.count;
    VerifyOrReturnError(new_count <= kMaxFabricMemberships, CHIP_ERROR_NO_MEMORY);

    // Key handing
    if (data.key.HasValue())
    {
        // Create a new keyset
        ReturnErrorOnFailure(SetKeySet(fabric_index, data.keySetID, data.key.Value()));
    }
    else
    {
        // The keyset must exist
        GroupDataProvider::KeySet ks;
        ReturnErrorOnFailure(groups->GetKeySet(fabric_index, data.keySetID, ks));
    }
    // Assign keyset to group
    ReturnErrorOnFailure(groups->SetGroupKey(fabric_index, data.groupID, data.keySetID));

    // Add/update entry in the group table
    info.group_id      = data.groupID;
    info.use_aux_acl   = data.useAuxiliaryACL.HasValue() && data.useAuxiliaryACL.Value();
    info.use_iana_addr = true;
    ReturnErrorOnFailure(groups->SetGroupInfo(fabric_index, info));

    if (data.replaceEndpoints.HasValue() && data.replaceEndpoints.Value())
    {
        // Replace endpoints
        ReturnErrorOnFailure(groups->RemoveEndpoints(fabric_index, data.groupID));
    }

    // Add Endpoints
    {
        size_t group_count = 0;
        auto iter          = data.endpoints.begin();
        while (iter.Next() && (group_count++ < kMaxCommandEndpoints))
        {
            ReturnErrorOnFailure(groups->AddEndpoint(fabric_index, data.groupID, iter.GetValue()));
        }
    }

    // Join multicast
    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetTransportManager().MulticastGroupJoinLeave(
        Transport::PeerAddress::Groupcast(), true);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                      EndpointList & endpoints)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    endpoints.count = 0;
    if (kUndefinedGroupId == data.groupID)
    {
        // Apply changes to all groups
        GroupInfoIterator * iter = groups->IterateGroupInfo(fabric_index);
        VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

        GroupInfo info;
        while (iter->Next(info))
        {
            ReturnErrorOnFailure(RemoveGroup(fabric_index, info.group_id, data, endpoints));
        }
        iter->Release();
    }
    else
    {
        // Modify specific group
        ReturnErrorOnFailure(RemoveGroup(fabric_index, data.groupID, data, endpoints));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnValue(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    // Key handing
    if (data.key.HasValue())
    {
        // Create a new keyset
        ReturnErrorOnFailure(SetKeySet(fabric_index, data.keySetID, data.key.Value()));
    }
    // Assign keyset to group
    ReturnErrorOnFailure(groups->SetGroupKey(fabric_index, data.groupID, data.keySetID));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::ConfigureAuxiliaryACL(FabricIndex fabric_index,
                                                 const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    // Get group info
    GroupDataProvider::GroupInfo info;
    ReturnErrorOnFailure(groups->GetGroupInfo(fabric_index, data.groupID, info));

    // Update group info
    info.use_aux_acl = data.useAuxiliaryACL;
    ReturnErrorOnFailure(groups->SetGroupInfo(fabric_index, info));

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::SetKeySet(FabricIndex fabric_index, KeysetId keyset_id, const chip::ByteSpan & key)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    GroupDataProvider::KeySet ks;

    CHIP_ERROR err = groups->GetKeySet(fabric_index, keyset_id, ks);
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New key
        const FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabric_index);
        VerifyOrReturnValue(nullptr != fabric, CHIP_ERROR_INTERNAL);

        ks.keyset_id     = keyset_id;
        ks.policy        = GroupDataProvider::SecurityPolicy::kTrustFirst;
        ks.num_keys_used = 1;

        // Translate HEX key to binary
        GroupDataProvider::EpochKey & epoch = ks.epoch_keys[0];
        VerifyOrReturnValue(key.size() == 2 * GroupDataProvider::EpochKey::kLengthBytes, CHIP_ERROR_INTERNAL);
        size_t key_size =
            chip::Encoding::HexToBytes((char *) key.data(), key.size(), epoch.key, GroupDataProvider::EpochKey::kLengthBytes);
        VerifyOrReturnValue(key_size == GroupDataProvider::EpochKey::kLengthBytes, CHIP_ERROR_INTERNAL);
        {
            // Get compressed fabric
            uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
            MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
            ReturnErrorOnFailure(fabric->GetCompressedFabricIdBytes(compressed_fabric_id));
            // Set keys
            err = groups->SetKeySet(fabric_index, compressed_fabric_id, ks);
        }
    }
    else if (CHIP_NO_ERROR == err)
    {
        // Cannot set an existing key
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }
    return err;
}

CHIP_ERROR GroupcastLogic::RemoveGroup(FabricIndex fabric_index, GroupId group_id,
                                       const Groupcast::Commands::LeaveGroup::DecodableType & data, EndpointList & endpoints)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    if (data.endpoints.HasValue())
    {
        // Remove endpoints
        auto iter = data.endpoints.Value().begin();
        while (iter.Next() && (endpoints.count < kMaxCommandEndpoints))
        {
            auto endpoint_id = iter.GetValue();
            if (groups->HasEndpoint(fabric_index, group_id, endpoint_id))
            {
                ReturnErrorOnFailure(RemoveEndpoint(fabric_index, group_id, endpoint_id, endpoints));
            }
        }
    }
    else
    {
        // Remove whole group (with all endpoints)
        EndpointIterator * iter = groups->IterateEndpoints(fabric_index, data.groupID);
        VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);
        GroupEndpoint mapping;
        while (iter->Next(mapping) && (endpoints.count < kMaxMembershipEndpoints))
        {
            ReturnErrorOnFailure(RemoveEndpoint(fabric_index, group_id, mapping.endpoint_id, endpoints));
        }
        iter->Release();
        ReturnErrorOnFailure(groups->RemoveGroupInfo(fabric_index, data.groupID));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::RemoveEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id,
                                          EndpointList & endpoints)
{
    GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(groups->RemoveEndpoint(fabric_index, group_id, endpoint_id));

    bool found = false;
    for (size_t i = 0; !found && i < endpoints.count; ++i)
    {
        found = (endpoints.entries[i] == endpoint_id);
    }
    if (!found)
    {
        endpoints.entries[endpoints.count++] = endpoint_id;
    }
    return CHIP_NO_ERROR;
}

} // namespace Clusters
} // namespace app
} // namespace chip
