#include "GroupcastLogic.h"
#include <access/AccessControl.h>
#include <app/util/endpoint-config-api.h>
#include <credentials/GroupDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

using GroupDataProvider = Credentials::GroupDataProvider;
using GroupInfo         = Credentials::GroupDataProvider::GroupInfo;
using GroupEndpoint     = Credentials::GroupDataProvider::GroupEndpoint;
using GroupInfoIterator = Credentials::GroupDataProvider::GroupInfoIterator;
using EndpointIterator  = Credentials::GroupDataProvider::EndpointIterator;

GroupcastLogic::GroupcastLogic(GroupcastContext & context) : mContext(context), mFeatures()
{
    mContext.groupDataProvider.SetListener(this);
    mUsedMcastAddrCount = GetUsedMcastAddrCount();
}

GroupcastLogic::GroupcastLogic(GroupcastContext & context, BitFlags<Groupcast::Feature> features) :
    mContext(context), mFeatures(features)
{
    mContext.groupDataProvider.SetListener(this);
    mUsedMcastAddrCount = GetUsedMcastAddrCount();
}

GroupcastLogic::~GroupcastLogic()
{
    mContext.groupDataProvider.RemoveListener(this);
}

CHIP_ERROR GroupcastLogic::ReadMembership(const chip::Access::SubjectDescriptor * subject, EndpointId endpoint,
                                          AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(nullptr != subject, CHIP_ERROR_INVALID_ARGUMENT);
    FabricIndex fabric_index = subject->fabricIndex;

    GroupDataProvider * groups = &Provider();

    CHIP_ERROR err = aEncoder.EncodeList([fabric_index, groups](const auto & encoder) -> CHIP_ERROR {
        EndpointList endpoints;
        CHIP_ERROR status              = CHIP_NO_ERROR;
        GroupInfoIterator * group_iter = groups->IterateGroupInfo(fabric_index);
        VerifyOrReturnError(nullptr != group_iter, CHIP_ERROR_NO_MEMORY);

        GroupInfo info;
        while (group_iter->Next(info) && (CHIP_NO_ERROR == status))
        {
            // Group Key
            KeysetId keyset_id = kInvalidKeysetId;
            // Since keys are managed by the GroupKeyManagement cluster, groups may not have an associated keyset
            status = groups->GetGroupKey(fabric_index, info.group_id, keyset_id).NoErrorIf(CHIP_ERROR_NOT_FOUND);
            if (CHIP_NO_ERROR != status)
            {
                break;
            }

            // Endpoints
            EndpointIterator * end_iter = groups->IterateEndpoints(fabric_index, info.group_id);
            if (nullptr == end_iter)
            {
                status = CHIP_ERROR_NO_MEMORY;
                break;
            }

            // Return endpoints in kMaxMembershipEndpoints chunks or less
            size_t group_total = end_iter->Count();
            size_t group_count = 0;
            size_t split_count = 0;
            GroupEndpoint mapping;
            while (end_iter->Next(mapping) && (CHIP_NO_ERROR == status))
            {
                group_count++;
                endpoints.entries[split_count++] = mapping.endpoint_id;
                if ((group_count == group_total) || (split_count == kMaxMembershipEndpoints))
                {
                    Groupcast::Structs::MembershipStruct::Type group;
                    group.fabricIndex     = fabric_index;
                    group.groupID         = info.group_id;
                    group.keySetID        = keyset_id;
                    group.hasAuxiliaryACL = MakeOptional(info.HasAuxiliaryACL());
                    group.mcastAddrPolicy = info.UsePerGroupAddress() ? Groupcast::MulticastAddrPolicyEnum::kPerGroup
                                                                      : Groupcast::MulticastAddrPolicyEnum::kIanaAddr;
                    group.endpoints       = MakeOptional(DataModel::List<const chip::EndpointId>(endpoints.entries, split_count));
                    status                = encoder.Encode(group);
                    split_count           = 0;
                }
            }
            end_iter->Release();
        }
        group_iter->Release();

        return status;
    });

    return err;
}

CHIP_ERROR GroupcastLogic::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    GroupDataProvider & groups = Provider();
    return aEncoder.Encode(groups.getMaxMembershipCount());
}

CHIP_ERROR GroupcastLogic::ReadMaxMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    GroupDataProvider & groups = Provider();
    return aEncoder.Encode(groups.getMaxMcastAddrCount());
}

CHIP_ERROR GroupcastLogic::ReadUsedMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mUsedMcastAddrCount);
}

Status GroupcastLogic::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
{
    GroupDataProvider & groups = Provider();
    CHIP_ERROR err             = CHIP_NO_ERROR;

    // Check groupID
    VerifyOrReturnError(data.groupID != kUndefinedGroupId, Status::ConstraintError);

    // Check useAuxiliaryACL
    if (data.useAuxiliaryACL.HasValue())
    {
        // AuxiliaryACL can only be present if LN feature is supported
        VerifyOrReturnError(mFeatures.Has(Groupcast::Feature::kListener), Status::ConstraintError);
    }

    // Check endpoints
    size_t endpoint_count = 0;
    err                   = data.endpoints.ComputeSize(&endpoint_count);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    if (mFeatures.Has(Groupcast::Feature::kListener) && !mFeatures.Has(Groupcast::Feature::kSender))
    {
        // Listener only, endpoints cannot be empty
        VerifyOrReturnError(endpoint_count > 0 && endpoint_count <= kMaxCommandEndpoints, Status::ConstraintError);
    }
    else if (!mFeatures.Has(Groupcast::Feature::kListener) && mFeatures.Has(Groupcast::Feature::kSender))
    {
        // Sender only, endpoints must be empty
        VerifyOrReturnError(0 == endpoint_count, Status::ConstraintError);
    }
    // Verify endpoint values
    {
        // The endpoint list SHALL not contain the root endpoint and must be a valid endpoint on the device.
        VerifyOrDie(mDataModelProvider != nullptr);
        ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;
        VerifyOrReturnError(CHIP_NO_ERROR == mDataModelProvider->Endpoints(endpointsBuilder), Status::Failure);
        auto endpointsSpan = endpointsBuilder.TakeBuffer();
        auto iter          = data.endpoints.begin();
        while (iter.Next())
        {
            bool foundEndpoint = false;
            EndpointId ep      = iter.GetValue();
            VerifyOrReturnError((ep > kRootEndpointId), Status::UnsupportedEndpoint);
            for (const auto & epEntry : endpointsSpan)
            {
                if (epEntry.id == ep)
                {
                    foundEndpoint = true;
                    break;
                }
            }

            VerifyOrReturnError(foundEndpoint, Status::UnsupportedEndpoint);
        }
    }

    // Check fabric membership entries limit
    GroupDataProvider::GroupInfo info;
    err = groups.GetGroupInfo(fabric_index, data.groupID, info);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, Status::Failure);
    // If the group is new, the fabric entries will increase
    uint16_t new_count              = (CHIP_ERROR_NOT_FOUND == err) ? info.count + 1 : info.count;
    uint16_t max_fabric_memberships = static_cast<uint16_t>(groups.getMaxMembershipCount() / 2);
    VerifyOrReturnError(new_count <= max_fabric_memberships, Status::ResourceExhausted);

    // Key handling
    Status stat = SetKeySet(fabric_index, data.groupID, data.keySetID, data.key);
    VerifyOrReturnError(Status::Success == stat, stat);

    // Add/update entry in the group table
    info.group_id = data.groupID;
    info.flags    = 0;
    if (data.useAuxiliaryACL.HasValue() && data.useAuxiliaryACL.Value())
    {
        info.flags |= chip::to_underlying(GroupInfo::Flags::kHasAuxiliaryACL);
    }

    if (data.mcastAddrPolicy.HasValue() && (Groupcast::MulticastAddrPolicyEnum::kPerGroup == data.mcastAddrPolicy.Value()))
    {
        info.flags |= chip::to_underlying(GroupInfo::Flags::kMcastAddrPolicy);
    }

    err = groups.SetGroupInfo(fabric_index, info);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    if (data.replaceEndpoints.HasValue() && data.replaceEndpoints.Value())
    {
        // Replace endpoints
        err = groups.RemoveEndpoints(fabric_index, data.groupID);
        VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    }

    // Add Endpoints
    {
        size_t group_count = 0;
        auto iter          = data.endpoints.begin();
        while (iter.Next() && (group_count++ < kMaxCommandEndpoints))
        {
            err = groups.AddEndpoint(fabric_index, data.groupID, iter.GetValue());
            VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
        }
    }

    return Status::Success;
}

Status GroupcastLogic::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                  EndpointList & endpoints)
{
    GroupDataProvider & groups = Provider();
    Status err                 = Status::Success;

    endpoints.count = 0;
    if (kUndefinedGroupId == data.groupID)
    {
        // Apply changes to all groups
        GroupInfoIterator * iter = groups.IterateGroupInfo(fabric_index);
        VerifyOrReturnError(nullptr != iter, Status::ResourceExhausted);
        VerifyOrReturnError(iter->Count() > 0, Status::NotFound);

        GroupInfo info;
        while (iter->Next(info) && (Status::Success == err))
        {
            err = RemoveGroup(fabric_index, info.group_id, data, endpoints);
        }
        iter->Release();
    }
    else
    {
        // Modify specific group
        err = RemoveGroup(fabric_index, data.groupID, data, endpoints);
    }

    return err;
}

Status GroupcastLogic::UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data)
{
    return SetKeySet(fabric_index, data.groupID, data.keySetID, data.key);
}

Status GroupcastLogic::ConfigureAuxiliaryACL(FabricIndex fabric_index,
                                             const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data)
{
    GroupDataProvider & groups = Provider();
    CHIP_ERROR err             = CHIP_NO_ERROR;

    // AuxiliaryACL can only be present if LN feature is supported
    VerifyOrReturnError(mFeatures.Has(Groupcast::Feature::kListener), Status::ConstraintError);

    // Get group info
    GroupDataProvider::GroupInfo info;
    err = groups.GetGroupInfo(fabric_index, data.groupID, info);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::NotFound);

    // Update group info
    if (data.useAuxiliaryACL)
    {
        info.flags |= chip::to_underlying(GroupInfo::Flags::kHasAuxiliaryACL);
    }
    else
    {
        info.flags &= ~chip::to_underlying(GroupInfo::Flags::kHasAuxiliaryACL);
    }
    err = groups.SetGroupInfo(fabric_index, info);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    return Status::Success;
}

Status GroupcastLogic::SetKeySet(FabricIndex fabric_index, GroupId group_id, KeysetId keyset_id,
                                 const chip::Optional<chip::ByteSpan> & key)
{
    GroupDataProvider & groups = Provider();
    GroupDataProvider::KeySet ks;

    CHIP_ERROR err = groups.GetKeySet(fabric_index, keyset_id, ks);
    if (key.HasValue())
    {
        // Key provided, the keyset must not exist
        VerifyOrReturnError(CHIP_NO_ERROR != err, Status::AlreadyExists);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err, Status::Failure);

        // Create new key
        const FabricInfo * fabric = Fabrics().FindFabricWithIndex(fabric_index);
        VerifyOrReturnValue(nullptr != fabric, Status::NotFound);

        ks.keyset_id     = keyset_id;
        ks.policy        = GroupDataProvider::SecurityPolicy::kTrustFirst;
        ks.num_keys_used = 1;

        GroupDataProvider::EpochKey & epoch = ks.epoch_keys[0];
        VerifyOrReturnValue(key.Value().size() == GroupDataProvider::EpochKey::kLengthBytes, Status::ConstraintError);
        memcpy(epoch.key, key.Value().data(), GroupDataProvider::EpochKey::kLengthBytes);
        {
            // Get compressed fabric
            uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
            MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
            err = fabric->GetCompressedFabricIdBytes(compressed_fabric_id);
            VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
            // Set keys
            err = groups.SetKeySet(fabric_index, compressed_fabric_id, ks);
            VerifyOrReturnError(CHIP_ERROR_INVALID_LIST_LENGTH != err, Status::ResourceExhausted);
            VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
        }
    }
    else
    {
        // No key provided, the keyset must exist
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);
        VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    }

    // Assign keyset to group
    err = groups.SetGroupKey(fabric_index, group_id, keyset_id);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    return Status::Success;
}

Status GroupcastLogic::RemoveGroup(FabricIndex fabric_index, GroupId group_id,
                                   const Groupcast::Commands::LeaveGroup::DecodableType & data, EndpointList & endpoints)
{
    GroupDataProvider & groups = Provider();
    Status stat                = Status::Success;

    if (data.endpoints.HasValue())
    {
        // Remove endpoints
        auto iter = data.endpoints.Value().begin();
        while (iter.Next() && (endpoints.count < kMaxCommandEndpoints))
        {
            auto endpoint_id = iter.GetValue();
            if (groups.HasEndpoint(fabric_index, group_id, endpoint_id))
            {
                stat = RemoveGroupEndpoint(fabric_index, group_id, endpoint_id, endpoints);
                VerifyOrReturnError(Status::Success == stat, stat);
            }
        }
    }
    else
    {
        // Remove whole group (with all endpoints)
        CHIP_ERROR err = groups.RemoveGroupInfo(fabric_index, group_id);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);
        VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    }

    return Status::Success;
}

Status GroupcastLogic::RemoveGroupEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id,
                                           EndpointList & endpoints)
{
    GroupDataProvider & groups = Provider();

    CHIP_ERROR err = groups.RemoveEndpoint(fabric_index, group_id, endpoint_id);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    bool found = false;
    for (size_t i = 0; !found && i < endpoints.count; ++i)
    {
        found = (endpoints.entries[i] == endpoint_id);
    }
    if (!found)
    {
        endpoints.entries[endpoints.count++] = endpoint_id;
    }

    return Status::Success;
}

void GroupcastLogic::OnGroupAdded(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & new_group)
{
    (void) fabric_index;
    (void) new_group;
    NotifyMembershipChanged();
    uint16_t address_count = GetUsedMcastAddrCount();
    if (address_count != mUsedMcastAddrCount)
    {
        mUsedMcastAddrCount = address_count;
        NotifyUsedMcastAddrCountChange();
    }
}

void GroupcastLogic::OnGroupRemoved(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & old_group)
{
    (void) fabric_index;
    (void) old_group;
    NotifyMembershipChanged();
    uint16_t address_count = GetUsedMcastAddrCount();
    if (address_count != mUsedMcastAddrCount)
    {
        mUsedMcastAddrCount = address_count;
        NotifyUsedMcastAddrCountChange();
    }
}

uint16_t GroupcastLogic::GetUsedMcastAddrCount()
{
    uint16_t per_group_count = 0;
    uint16_t iana_address    = 0;
    // Iterate all fabrics
    for (const FabricInfo & fabric : Fabrics())
    {
        // Count distinct group addresses
        GroupInfoIterator * iter = Provider().IterateGroupInfo(fabric.GetFabricIndex());
        VerifyOrReturnValue(nullptr != iter, 0);
        GroupInfo group;
        while (iter->Next(group))
        {
            if (group.UsePerGroupAddress())
            {
                per_group_count++;
            }
            else
            {
                iana_address = 1;
            }
        }
        iter->Release();
    }
    return per_group_count + iana_address;
}

void GroupcastLogic::NotifyMembershipChanged()
{
    if (mListener != nullptr)
    {
        mListener->OnMembershipChanged();
    }
}

void GroupcastLogic::NotifyUsedMcastAddrCountChange()
{
    if (mListener != nullptr)
    {
        mListener->OnUsedMcastAddrCountChange();
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
