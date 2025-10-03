#include "GroupcastLogic.h"

#include <access/examples/GroupcastAccessControlDelegate.h>
#include <algorithm>
#include <app/server/Server.h>
#include <credentials/GroupcastDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

using DataModel::CHIP_ERROR;
using Protocols::InteractionModel::Status;
// using chip::Groupcast::kLengthBytes;
using chip::Groupcast::DataProvider;
static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

CHIP_ERROR GroupcastLogic::ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataProvider & provider = DataProvider::Instance();

    CHIP_ERROR err = aEncoder.EncodeList([&provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR status = CHIP_NO_ERROR;
        for (auto & fabric : Server::GetInstance().GetFabricTable())
        {
            auto fabric_index = fabric.GetFabricIndex();
            auto iter         = provider.IterateGroups(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);
            chip::Groupcast::Group entry;
            while (iter->Next(entry))
            {
                Groupcast::Structs::MembershipStruct::Type target;
                target.fabricIndex = fabric_index;
                target.groupId     = entry.group_id;
                target.endpoints   = DataModel::List<const chip::EndpointId>(entry.endpoints, entry.endpoint_count);
                status             = encoder.Encode(target);
                if (status != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            iter->Release();
            if (status != CHIP_NO_ERROR)
            {
                break;
            }
        }
        return status;
    });
    return err;
}

CHIP_ERROR GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataProvider & provider = DataProvider::Instance();
    return aEncoder.Encode(provider.GetMaxMembershipCount());
}

CHIP_ERROR GroupcastLogic::RegisterAccessControl(FabricIndex fabric_index, GroupId group_id)
{
    Access::AccessControl::Delegate * access = Access::Groupcast::GetAccessControlDelegate();
    VerifyOrReturnError(nullptr != access, CHIP_ERROR_INTERNAL);
    Access::AccessControl::Entry e;
    ReturnErrorOnFailure(access->PrepareEntry(e));
    size_t index = 0; // TODO: GetEntryCount(size_t & value)
    if (CHIP_NO_ERROR == access->ReadEntry(index, e, &fabric_index))
    {
        // Existing entry
        size_t subject_count = 0;
        NodeId subject       = 0;
        // Check if the given group is already a subject
        ReturnErrorOnFailure(e.GetSubjectCount(subject_count));
        bool found = false;
        for (size_t i = 0; !found && (i < subject_count); ++i)
        {
            ReturnErrorOnFailure(e.GetSubject(i, subject));
            found = (subject == group_id);
        }
        // ChipLogDetail(DeviceLayer, "~~~ GROUPCAST: EXISTING: %u", (unsigned)found);
        if (!found)
        {
            // New group subject
            size_t subject_index = 0;
            ReturnErrorOnFailure(e.AddSubject(&subject_index, group_id));
            ReturnErrorOnFailure(access->UpdateEntry(index, e, &fabric_index));
        }
    }
    else
    {
        // New entry
        e.SetFabricIndex(fabric_index);
        e.SetPrivilege(Access::Privilege::kOperate);
        e.SetAuthMode(Access::AuthMode::kGroup);
        e.AddSubject(&index, group_id);
        ReturnErrorOnFailure(access->CreateEntry(&index, e, &fabric_index));
        // ChipLogDetail(DeviceLayer, "~~~ GROUPCAST: NEW: #%04x, i:%u", (unsigned)err.AsInteger(), (unsigned)index);
        VerifyOrReturnError(0 == index, CHIP_ERROR_SENTINEL);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    Server & server           = Server::GetInstance();
    const FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabric_index);
    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
    DataProvider & provider = DataProvider::Instance();
    chip::Groupcast::Group target;
    uint8_t key[kLengthBytes] = { 0 };
    size_t key_size           = 0;
    uint32_t grace_period     = data.gracePeriod.HasValue() ? data.gracePeriod.Value() : 0;
    CHIP_ERROR err            = CHIP_NO_ERROR;

    // uint32_t features = 0;
    // chip::app::Clusters::Groupcast::Attributes::FeatureMap::Get(commandPath.mEndpointId, &features);

    // Collect arguments

    // Compressed fabric Id
    VerifyOrExit(nullptr != fabric, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(CHIP_NO_ERROR == fabric->GetCompressedFabricIdBytes(compressed_fabric_id), err = CHIP_ERROR_INTERNAL);

    // Group
    target.group_id = data.groupId;

    // Endpoints
    target.endpoint_count = 0;
    {
        auto iter = data.endpoints.begin();
        while (iter.Next() && (target.endpoint_count < chip::Groupcast::kEndpointsMax))
        {
            target.endpoints[target.endpoint_count++] = iter.GetValue();
        }
    }

    // Parse Key
    VerifyOrExit(data.key.size() == 2 * sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);
    key_size = chip::Encoding::HexToBytes((char *) data.key.data(), data.key.size(), key, sizeof(key));
    VerifyOrExit(key_size == sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);

    // Store provider configuration
    err = provider.JoinGroup(fabric->GetFabricIndex(), compressed_fabric_id, target, ByteSpan(key), grace_period);
    VerifyOrExit(CHIP_NO_ERROR == err, );

    // ACL
    err = RegisterAccessControl(fabric->GetFabricIndex(), target.group_id);
    VerifyOrExit(CHIP_NO_ERROR == err, );

    err = server.GetTransportManager().MulticastGroupJoinLeave(Transport::PeerAddress::Groupcast(), true);
    VerifyOrExit(CHIP_NO_ERROR == err, );
exit:
    return err;
}

CHIP_ERROR GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                      Groupcast::Commands::LeaveGroupResponse::Type & response)
{
    DataProvider & provider = DataProvider::Instance();
    chip::Groupcast::Group old_group;
    CHIP_ERROR stat = Status::NotFound;

    // 0 = data.groupId
    DataProvider::GroupIterator * iter = provider.IterateGroups(fabric_index);
    VerifyOrReturnValue(nullptr != iter, Status::ResourceExhausted);
    VerifyOrReturnValue(0 == iter->Count(), Status::NotFound, iter->Release());
    while (iter->Next(old_group))
    {
        if (old_group.group_id == data.groupId)
        {
            chip::Groupcast::Group new_group(data.groupId);
            for (size_t i = 0; (i < old_group.endpoint_count) && (i < chip::Groupcast::kEndpointsMax); ++i)
            {
                // if(old_group.endpoints[i])
                auto iter  = data.endpoints.Value().begin();
                bool found = false;
                while (iter.Next() && !found)
                {
                    found = old_group.endpoints[i] == iter.GetValue();
                }
                // const chip::EndpointId *foo = std::find(std::begin(array), std::end(array), old_group.endpoints[i]);
            }
        }
    }
    iter->Release();
    return stat;

    // if(CHIP_NO_ERROR == err)
    // // Response
    // {
    //     Groupcast::Commands::LeaveGroupResponse::Type response;
    //     handler->AddResponse(request.path, response);
    // }
    return stat;
}

} // namespace Clusters
} // namespace app
} // namespace chip
