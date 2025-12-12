#include "GroupcastLogic.h"
#include <app/server/Server.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupcastDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

using chip::Groupcast::DataProvider;
using chip::Groupcast::GroupData;
using namespace chip::Credentials;

CHIP_ERROR GroupcastLogic::ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataProvider * provider = DataProvider::Instance();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = aEncoder.EncodeList([&provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR status = CHIP_NO_ERROR;
        for (auto & fabric : Server::GetInstance().GetFabricTable())
        {
            // Fabric
            auto fabric_index = fabric.GetFabricIndex();
            auto result       = provider->IterateGroups(fabric_index, [&](auto & iter) -> CHIP_ERROR {
                // Group
                GroupData entry;
                while (iter.Next(entry) && (CHIP_NO_ERROR == status))
                {
                    Groupcast::Structs::MembershipStruct::Type group;
                    group.fabricIndex = fabric_index;
                    group.groupID     = entry.groupID;
                    group.endpoints   = DataModel::List<const chip::EndpointId>(entry.endpoints, entry.endpointCount);
                    status            = encoder.Encode(group);
                }
                return status;
            });
            ReturnErrorOnFailure(result);
        }
        return status;
    });
    return err;
}

CHIP_ERROR GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataProvider * provider = DataProvider::Instance();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INCORRECT_STATE);
    return aEncoder.Encode(provider->GetMaxMembershipCount());
}

CHIP_ERROR GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    const FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabric_index);

    DataProvider * groupcast = DataProvider::Instance();
    VerifyOrReturnError(nullptr != groupcast, CHIP_ERROR_INCORRECT_STATE);
    GroupData group;

    // Compressed fabric Id
    VerifyOrReturnValue(nullptr != fabric, CHIP_ERROR_INTERNAL);

    // Group
    group.groupID = data.groupID;

    // Endpoints
    group.endpointCount = 0;
    {
        auto iter = data.endpoints.begin();
        while (iter.Next() && (group.endpointCount < chip::Groupcast::kMaxEndpointsPerGroup))
        {
            group.endpoints[group.endpointCount++] = iter.GetValue();
        }
    }

    // Key handing
    {
        GroupDataProvider * groups = GetGroupDataProvider();
        VerifyOrReturnValue(nullptr != groups, CHIP_ERROR_INTERNAL);

        GroupDataProvider::KeySet ks;
        if (CHIP_NO_ERROR == groups->GetKeySet(fabric_index, data.keyID, ks))
        {
            // Existing key
            VerifyOrReturnValue(!data.key.HasValue(), CHIP_ERROR_INTERNAL);
        }
        else
        {
            // New key
            VerifyOrReturnValue(data.key.HasValue(), CHIP_ERROR_INTERNAL);
            uint32_t grace_period = data.gracePeriod.HasValue() ? data.gracePeriod.Value() : 0;
            (void) grace_period; // TODO
            ks.keyset_id = data.keyID;

            // Translate HEX key to binary
            const chip::ByteSpan & key          = data.key.Value();
            GroupDataProvider::EpochKey & epoch = ks.epoch_keys[0];
            VerifyOrReturnValue(key.size() == 2 * GroupDataProvider::EpochKey::kLengthBytes, CHIP_ERROR_INTERNAL);
            size_t key_size =
                chip::Encoding::HexToBytes((char *) key.data(), key.size(), epoch.key, GroupDataProvider::EpochKey::kLengthBytes);
            VerifyOrReturnValue(key_size == GroupDataProvider::EpochKey::kLengthBytes, CHIP_ERROR_INTERNAL);
            ks.num_keys_used = 1;
            {
                // Get compressed fabric
                uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
                MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
                ReturnErrorOnFailure(fabric->GetCompressedFabricIdBytes(compressed_fabric_id));
                // Set keys
                ReturnErrorOnFailure(groups->SetKeySet(fabric_index, compressed_fabric_id, ks));
            }
        }
    }

    // Add group
    ReturnErrorOnFailure(groupcast->SetGroup(fabric->GetFabricIndex(), group));

    return Server::GetInstance().GetTransportManager().MulticastGroupJoinLeave(Transport::PeerAddress::Groupcast(), true);
}

CHIP_ERROR GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                      Groupcast::Commands::LeaveGroupResponse::Type & response)
{
    DataProvider * provider = DataProvider::Instance();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INCORRECT_STATE);
    GroupData old_data(data.groupID);

    response.groupID = old_data.groupID;
    ReturnErrorOnFailure(provider->GetGroup(fabric_index, old_data));
    if (data.endpoints.HasValue())
    {
        // Remove endpoints
        GroupData new_data(data.groupID);

        for (size_t i = 0; i < old_data.endpointCount; ++i)
        {
            bool found = false;
            auto iter  = data.endpoints.Value().begin();
            while (!found && iter.Next())
            {
                found = (old_data.endpoints[i] == iter.GetValue());
            }
            if (!found)
            {
                // Keep: not in the remove group
                new_data.endpoints[new_data.endpointCount++] = old_data.endpoints[i];
            }
        }
        if (new_data.endpointCount > 0)
        {
            return provider->SetEndpoints(fabric_index, new_data);
        }
    }

    // Remove whole group
    DataModel::List<const chip::EndpointId> endpoints(old_data.endpoints, old_data.endpointCount);
    response.endpoints.SetValue(endpoints);
    return provider->RemoveGroup(fabric_index, data.groupID);
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

} // namespace Clusters
} // namespace app
} // namespace chip
