#include "GroupcastServer.h"
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>

#include <app/server/Server.h>
#include <credentials/GroupcastDataProvider.h>
#include <access/examples/GroupcastAccessControlDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace {
static GroupcastCluster sInstance;


CHIP_ERROR ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();

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

CHIP_ERROR ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();
    return aEncoder.Encode(provider.GetMaxMembershipCount());
}

CHIP_ERROR RegisterAccessControl(FabricIndex fabric_index, GroupId group_id)
{
    Access::AccessControl::Delegate *access = Access::Groupcast::GetAccessControlDelegate();
    VerifyOrReturnError(nullptr != access, CHIP_ERROR_INTERNAL);
    Access::AccessControl::Entry e;
    ReturnErrorOnFailure(access->PrepareEntry(e));
    size_t index = 0; // TODO: GetEntryCount(size_t & value)
    if(CHIP_NO_ERROR == access->ReadEntry(index, e, &fabric_index)) {
        // Existing entry
        size_t subject_count = 0;
        NodeId subject = 0;
        // Check if the given group is already a subject
        ReturnErrorOnFailure(e.GetSubjectCount(subject_count));
        bool found = false;
        for(size_t i=0; !found && (i < subject_count); ++i) {
            ReturnErrorOnFailure(e.GetSubject(i, subject));
            found = (subject == group_id);
        }
        // ChipLogDetail(DeviceLayer, "~~~ GROUPCAST: EXISTING: %u", (unsigned)found);
        if(!found) {
            // New group subject
            size_t subject_index = 0;
            ReturnErrorOnFailure(e.AddSubject(&subject_index, group_id));
            ReturnErrorOnFailure(access->UpdateEntry(index, e, &fabric_index));
        }
    }
    else {
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

CHIP_ERROR JoinGroup(FabricIndex fabricIndex, const chip::app::Clusters::Groupcast::Commands::JoinGroup::DecodableType & commandData)
{
    static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;
    Server & server                          = Server::GetInstance();
    const FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
    chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();
    chip::Groupcast::Group target;
    uint8_t key[kLengthBytes] = { 0 };
    size_t key_size   = 0;
    CHIP_ERROR err  = CHIP_NO_ERROR;

    // uint32_t features = 0;
    // chip::app::Clusters::Groupcast::Attributes::FeatureMap::Get(commandPath.mEndpointId, &features);

    // Collect arguments

    // Compressed fabric Id
    VerifyOrExit(nullptr != fabric, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(CHIP_NO_ERROR == fabric->GetCompressedFabricIdBytes(compressed_fabric_id), err = CHIP_ERROR_INTERNAL);

    // Group
    target.group_id = commandData.groupId;

    // Endpoints
    target.endpoint_count = 0;
    {
        auto iter             = commandData.endpoints.begin();
        while (iter.Next() && (target.endpoint_count < chip::Groupcast::kEndpointsMax))
        {
            target.endpoints[target.endpoint_count++] = iter.GetValue();
        }
    }

    // Parse Key
    VerifyOrExit(commandData.key.size() == 2 * sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);
    key_size = chip::Encoding::HexToBytes((char *) commandData.key.data(), commandData.key.size(), key, sizeof(key));
    VerifyOrExit(key_size == sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);

    // Store provider configuration
    err = provider.JoinGroup(fabric->GetFabricIndex(), compressed_fabric_id, target, ByteSpan(key), commandData.gracePeriod);
    VerifyOrExit(CHIP_NO_ERROR == err,);

    // ACL
    err = RegisterAccessControl(fabric->GetFabricIndex(), target.group_id);
    VerifyOrExit(CHIP_NO_ERROR == err,);

    server.GetTransportManager().MulticastGroupJoinLeave(Transport::PeerAddress::Groupcast(), true);
exit:
    return err;
}

} // namespace

GroupcastCluster & GroupcastCluster::Instance()
{
    return sInstance;
}

GroupcastCluster::GroupcastCluster() : DefaultServerCluster({ kRootEndpointId, Groupcast::Id })
{
}

CHIP_ERROR GroupcastCluster::Startup(ServerClusterContext & context)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void GroupcastCluster::Shutdown() {}

// [[nodiscard]] Span<const ConcreteClusterPath> GroupcastCluster::GetPaths() const {}
// [[nodiscard]] DataVersion GroupcastCluster::GetDataVersion(const ConcreteClusterPath & path) const {}
// [[nodiscard]] BitFlags<DataModel::ClusterQualityFlags> GroupcastCluster::GetClusterFlags(const ConcreteClusterPath &) const {}
// void GroupcastCluster::ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType) {}

DataModel::ActionReturnStatus GroupcastCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        break;
    case Groupcast::Attributes::Membership::Id:
        return ReadMembership(request.path.mEndpointId, encoder);

    case Groupcast::Attributes::MaxMembershipCount::Id:
        return ReadMaxMembershipCount(request.path.mEndpointId, encoder);
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

CHIP_ERROR GroupcastCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) {}

std::optional<DataModel::ActionReturnStatus> GroupcastCluster::InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & arguments, CommandHandler * handler)
{
    FabricIndex fabric_index = handler->GetAccessingFabricIndex();
    switch (request.path.mCommandId)
    {
    case Groupcast::Commands::JoinGroup::Id:
    {
        Groupcast::Commands::JoinGroup::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return JoinGroup(fabric_index, data);
    }
    }
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}

// CHIP_ERROR GroupcastCluster::AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) {}
// CHIP_ERROR GroupcastCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) {}
// bool GroupcastCluster::PathsContains(const ConcreteClusterPath & path);

} // namespace Clusters
} // namespace app
} // namespace chip
