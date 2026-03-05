#include "GroupcastCluster.h"
#include <access/AccessControl.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/Attributes.h>
#include <clusters/Groupcast/Metadata.h>
#include <credentials/GroupDataProvider.h>

using chip::Protocols::InteractionModel::Status;

using namespace chip::Credentials;
using GroupDataProvider = GroupDataProvider;
using GroupInfo         = GroupDataProvider::GroupInfo;
using GroupEndpoint     = GroupDataProvider::GroupEndpoint;
using GroupInfoIterator = GroupDataProvider::GroupInfoIterator;
using EndpointIterator  = GroupDataProvider::EndpointIterator;

namespace chip {
namespace app {
namespace Clusters {
namespace {

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Groupcast::Commands::JoinGroup::kMetadataEntry,        Groupcast::Commands::LeaveGroup::kMetadataEntry,
    Groupcast::Commands::UpdateGroupKey::kMetadataEntry,   Groupcast::Commands::ConfigureAuxiliaryACL::kMetadataEntry,
    Groupcast::Commands::GroupcastTesting::kMetadataEntry,
};

constexpr CommandId kGeneratedCommands[] = {
    Groupcast::Commands::LeaveGroupResponse::Id,
};

} // namespace

GroupcastCluster::GroupcastCluster(GroupcastContext && context) : GroupcastCluster(std::move(context), {}) {}

GroupcastCluster::GroupcastCluster(GroupcastContext && context, BitFlags<Groupcast::Feature> features) :
    DefaultServerCluster({ kRootEndpointId, Groupcast::Id }), mContext(std::move(context)), mFeatures(features),
    mMembershipChangedTimer(*this), mGroupcastTestingTimer(*this)
{
    mContext.groupDataProvider.SetListener(this);
    UpdateUsedMcastAddrCount();
}

GroupcastCluster::~GroupcastCluster() {}

CHIP_ERROR GroupcastCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    mContext.groupDataProvider.SetListener(this);
    SetDataModelProvider(context.provider);

    return CHIP_NO_ERROR;
}

void GroupcastCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mGroupcastTestingTimer.Cancel();
    mMembershipChangedTimer.Cancel();
    mContext.groupDataProvider.RemoveListener(this);
    ResetDataModelProvider();
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus GroupcastCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Groupcast::Attributes::FeatureMap::Id:
        return encoder.Encode(Features());
    case Groupcast::Attributes::ClusterRevision::Id:
        return encoder.Encode(Groupcast::kRevision);
    case Groupcast::Attributes::Membership::Id:
        return ReadMembership(request.subjectDescriptor, request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMembershipCount::Id:
        return ReadMaxMembershipCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::MaxMcastAddrCount::Id:
        return ReadMaxMcastAddrCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::UsedMcastAddrCount::Id:
        return ReadUsedMcastAddrCount(request.path.mEndpointId, encoder);
    case Groupcast::Attributes::FabricUnderTest::Id:
        return encoder.Encode(mFabricUnderTest);
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

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::UnsupportedCommand;

    switch (request.path.mCommandId)
    {
    case Groupcast::Commands::JoinGroup::Id: {
        Groupcast::Commands::JoinGroup::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = JoinGroup(fabric_index, data);
    }
    break;
    case Groupcast::Commands::LeaveGroup::Id: {
        Groupcast::Commands::LeaveGroup::DecodableType data;
        Groupcast::Commands::LeaveGroupResponse::Type response;
        EndpointList endpoints;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = LeaveGroup(fabric_index, data, endpoints);
        if (Protocols::InteractionModel::Status::Success == status)
        {
            NotifyAttributeChanged(Groupcast::Attributes::Membership::Id);
            response.groupID   = data.groupID;
            response.endpoints = DataModel::List<const chip::EndpointId>(endpoints.entries, endpoints.count);
            handler->AddResponse(request.path, response);
            return std::nullopt; // Response added, must return nullopt.
        }
    }
    break;
    case Groupcast::Commands::UpdateGroupKey::Id: {
        Groupcast::Commands::UpdateGroupKey::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = UpdateGroupKey(fabric_index, data);
    }
    break;
    case Groupcast::Commands::ConfigureAuxiliaryACL::Id: {
        Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        status = ConfigureAuxiliaryACL(fabric_index, data);
    }
    break;
    case Groupcast::Commands::GroupcastTesting::Id: {
        Groupcast::Commands::GroupcastTesting::DecodableType data;
        ReturnErrorOnFailure(data.Decode(arguments, fabric_index));
        return GroupcastTesting(fabric_index, data);
    }
    break;
    default:
        break;
    }

    return status;
}

CHIP_ERROR GroupcastCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR GroupcastCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<CommandId> & builder)
{
    return builder.ReferenceExisting(kGeneratedCommands);
}


Status GroupcastCluster::GroupcastTesting(FabricIndex fabricIndex, Groupcast::Commands::GroupcastTesting::DecodableType data)
{
    VerifyOrReturnError(mFabricUnderTest == kUndefinedFabricIndex || mFabricUnderTest == fabricIndex, Status::ConstraintError);

    if (data.testOperation == Groupcast::GroupcastTestingEnum::kDisableTesting)
    {
        // cancel any existing GroupcastTesting timer
        mGroupcastTestingTimer.Cancel();
        mTestingState = data.testOperation;
        SetFabricUnderTest(kUndefinedFabricIndex);
        return Status::Success;
    }

    constexpr uint16_t kDefaultDurationSeconds = 60;
    uint16_t durationSeconds                   = kDefaultDurationSeconds;
    if (data.durationSeconds.HasValue())
    {
        constexpr uint16_t kMinDurationSeconds = 10, kMaxDurationSeconds = 1200;
        VerifyOrReturnError(data.durationSeconds.Value() >= kMinDurationSeconds &&
                                data.durationSeconds.Value() <= kMaxDurationSeconds,
                            Status::ConstraintError);
        durationSeconds = data.durationSeconds.Value();
    }

    mGroupcastTestingTimer.Start(durationSeconds);

    mTestingState = data.testOperation;
    SetFabricUnderTest(fabricIndex);
    return Status::Success;
}

void GroupcastCluster::SetFabricUnderTest(FabricIndex fabricUnderTest)
{
    SetAttributeValue(mFabricUnderTest, fabricUnderTest, Groupcast::Attributes::FabricUnderTest::Id);
}

// MembershipChangedTimer implementation
void GroupcastCluster::MembershipChangedTimer::Start()
{
    VerifyOrReturn(!mCluster.GetTimerDelegate().IsTimerActive(this));
    constexpr System::Clock::Milliseconds32 kChangeTemporisation = System::Clock::Milliseconds32(250);
    ReturnAndLogOnFailure(mCluster.GetTimerDelegate().StartTimer(this, kChangeTemporisation), AppServer,
                          "Failed to start MembershipChangedTimer");
}

void GroupcastCluster::MembershipChangedTimer::Cancel()
{
    mCluster.GetTimerDelegate().CancelTimer(this);
}

void GroupcastCluster::MembershipChangedTimer::TimerFired()
{
    mCluster.NotifyAttributeChanged(Groupcast::Attributes::Membership::Id);
}

// GroupcastTestingTimer implementation
void GroupcastCluster::GroupcastTestingTimer::Start(uint32_t seconds)
{
    Cancel();
    ReturnAndLogOnFailure(mCluster.GetTimerDelegate().StartTimer(this, System::Clock::Seconds32(seconds)), AppServer,
                          "Failed to start GroupcastTestingTimer");
}

void GroupcastCluster::GroupcastTestingTimer::Cancel()
{
    if (mCluster.GetTimerDelegate().IsTimerActive(this))
    {
        mCluster.GetTimerDelegate().CancelTimer(this);
    }
}

void GroupcastCluster::GroupcastTestingTimer::TimerFired()
{
    mCluster.SetFabricUnderTest(kUndefinedFabricIndex);
    mCluster.mTestingState = Groupcast::GroupcastTestingEnum::kDisableTesting;
}

// Methods moved from GroupcastLogic

CHIP_ERROR GroupcastCluster::ReadMembership(const chip::Access::SubjectDescriptor * subject, EndpointId endpoint,
                                            AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(nullptr != subject, CHIP_ERROR_INVALID_ARGUMENT);
    FabricIndex fabric_index = subject->fabricIndex;

    GroupDataProvider * groups = &Provider();

    CHIP_ERROR err = aEncoder.EncodeList([fabric_index, groups, this](const auto & encoder) -> CHIP_ERROR {
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

            Groupcast::Structs::MembershipStruct::Type group;
            group.fabricIndex     = fabric_index;
            group.groupID         = info.group_id;
            group.keySetID        = keyset_id;
            group.mcastAddrPolicy = info.UsePerGroupAddress() ? Groupcast::MulticastAddrPolicyEnum::kPerGroup
                                                              : Groupcast::MulticastAddrPolicyEnum::kIanaAddr;
            if (mFeatures.Has(Groupcast::Feature::kListener))
            {
                group.hasAuxiliaryACL = MakeOptional(info.HasAuxiliaryACL());
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
                    group.endpoints = MakeOptional(DataModel::List<const chip::EndpointId>(endpoints.entries, split_count));
                    status          = encoder.Encode(group);
                    split_count     = 0;
                }
            }
            end_iter->Release();
            if (group_count == 0)
            {
                if (mFeatures.Has(Groupcast::Feature::kListener))
                {
                    // If listener is supported, the membership SHALL contain the endpoints list.
                    group.endpoints = MakeOptional(DataModel::List<const chip::EndpointId>());
                }

                status = encoder.Encode(group);
            }
        }
        group_iter->Release();

        return status;
    });

    return err;
}

CHIP_ERROR GroupcastCluster::ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    GroupDataProvider & groups = Provider();
    return aEncoder.Encode(groups.getMaxMembershipCount());
}

CHIP_ERROR GroupcastCluster::ReadMaxMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    GroupDataProvider & groups = Provider();
    return aEncoder.Encode(groups.getMaxMcastAddrCount());
}

CHIP_ERROR GroupcastCluster::ReadUsedMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mUsedMcastAddrCount);
}

Status GroupcastCluster::JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data)
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

    // ReplaceEndpoints can only be present if kListener feature is supported
    VerifyOrReturnError(!data.replaceEndpoints.HasValue() || mFeatures.Has(Groupcast::Feature::kListener), Status::ConstraintError);

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
    err               = groups.GetGroupInfo(fabric_index, data.groupID, info);
    bool is_new_group = (CHIP_ERROR_NOT_FOUND == err);
    VerifyOrReturnError(is_new_group || (CHIP_NO_ERROR == err), Status::Failure);
    // If the group is new, the fabric entries will increase
    uint16_t new_count              = (is_new_group) ? info.count + 1 : info.count;
    uint16_t max_fabric_memberships = static_cast<uint16_t>(groups.getMaxMembershipCount() / 2);
    VerifyOrReturnError(new_count <= max_fabric_memberships, Status::ResourceExhausted);

    // Gather group info
    info.group_id = data.groupID;
    info.flags    = 0;
    if (data.useAuxiliaryACL.HasValue() && data.useAuxiliaryACL.Value())
    {
        info.flags |= chip::to_underlying(GroupInfo::Flags::kHasAuxiliaryACL);
    }
    if (data.mcastAddrPolicy.HasValue() && (Groupcast::MulticastAddrPolicyEnum::kPerGroup == data.mcastAddrPolicy.Value()))
    {
        // PerGroup address
        VerifyOrReturnError(!is_new_group || (mUsedMcastAddrCount < groups.getMaxMcastAddrCount()), Status::ResourceExhausted);
        info.flags |= chip::to_underlying(GroupInfo::Flags::kMcastAddrPolicy);
    }
    else
    {
        // Iana address
        VerifyOrReturnError(!is_new_group || (mIanaAddressUsed || (mUsedMcastAddrCount < groups.getMaxMcastAddrCount())),
                            Status::ResourceExhausted);
    }

    // Key handling
    Status stat = SetKeySet(fabric_index, data.groupID, data.keySetID, data.key);
    VerifyOrReturnError(Status::Success == stat, stat);

    // Add/update entry in the group table
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

Status GroupcastCluster::LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
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
            // For leave group, the leaveGroupResponse SHALL NOT contain the endpoints that were removed.
            err = RemoveGroup(fabric_index, info.group_id, data, nullptr /* endpoints */);
        }
        iter->Release();
    }
    else
    {
        // Modify specific group
        err = RemoveGroup(fabric_index, data.groupID, data, &endpoints);
    }

    return err;
}

Status GroupcastCluster::UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data)
{
    // Validate that the group exists early before trying to set the keyset
    GroupDataProvider::GroupInfo info;
    CHIP_ERROR err = Provider().GetGroupInfo(fabric_index, data.groupID, info);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    return SetKeySet(fabric_index, data.groupID, data.keySetID, data.key);
}

Status GroupcastCluster::ConfigureAuxiliaryACL(FabricIndex fabric_index,
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

Status GroupcastCluster::SetKeySet(FabricIndex fabric_index, GroupId group_id, KeysetId keyset_id,
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

Status GroupcastCluster::RemoveGroup(FabricIndex fabric_index, GroupId group_id,
                                     const Groupcast::Commands::LeaveGroup::DecodableType & data, EndpointList * endpoints)
{
    GroupDataProvider & groups = Provider();
    Status stat                = Status::Success;

    if (data.endpoints.HasValue())
    {
        // Remove endpoints
        auto iter = data.endpoints.Value().begin();
        while (iter.Next())
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
        if (endpoints != nullptr)
        {
            // Get the endpoints list for the LeaveGroupResponse
            EndpointIterator * epIter = groups.IterateEndpoints(fabric_index, group_id);
            VerifyOrReturnError(nullptr != epIter, Status::ResourceExhausted);

            if (epIter->Count() <= kMaxCommandEndpoints)
            {
                GroupEndpoint ep;
                while (epIter->Next(ep) && (endpoints->count < kMaxCommandEndpoints))
                {
                    endpoints->entries[endpoints->count++] = ep.endpoint_id;
                }
            }
            epIter->Release();
        }
        // Remove whole group (with all endpoints)
        CHIP_ERROR err = groups.RemoveGroupInfo(fabric_index, group_id);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);
        VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    }

    return Status::Success;
}

Status GroupcastCluster::RemoveGroupEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id,
                                             EndpointList * endpoints)
{
    GroupDataProvider & groups = Provider();

    CHIP_ERROR err = groups.RemoveEndpoint(fabric_index, group_id, endpoint_id,
                                           mFeatures.Has(Groupcast::Feature::kSender)
                                               ? GroupDataProvider::GroupCleanupPolicy::kKeepGroupIfEmpty
                                               : GroupDataProvider::GroupCleanupPolicy::kDeleteGroupIfEmpty);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    VerifyOrReturnError(endpoints != nullptr, Status::Success);

    bool found = false;
    for (size_t i = 0; !found && i < endpoints->count; ++i)
    {
        found = (endpoints->entries[i] == endpoint_id);
    }
    if (!found)
    {
        endpoints->entries[endpoints->count++] = endpoint_id;
    }

    return Status::Success;
}

void GroupcastCluster::OnGroupAdded(FabricIndex fabric_index, const GroupInfo & new_group)
{
    (void) fabric_index;
    (void) new_group;
    NotifyMembershipChanged();
    NotifyUsedMcastAddrCountOnChange();
}

void GroupcastCluster::OnGroupRemoved(FabricIndex fabric_index, const GroupInfo & old_group)
{
    (void) fabric_index;
    (void) old_group;
    NotifyMembershipChanged();
    NotifyUsedMcastAddrCountOnChange();
}

void GroupcastCluster::OnGroupModified(FabricIndex fabric_index, const GroupId & modified_group_id)
{
    (void) fabric_index;
    (void) modified_group_id;
    NotifyMembershipChanged();
    NotifyUsedMcastAddrCountOnChange();
}

void GroupcastCluster::UpdateUsedMcastAddrCount()
{
    uint16_t per_group_count = 0;
    uint16_t iana_address    = 0;
    // Iterate all fabrics
    for (const FabricInfo & fabric : Fabrics())
    {
        // Count distinct group addresses
        GroupInfoIterator * iter = Provider().IterateGroupInfo(fabric.GetFabricIndex());
        VerifyOrReturn(nullptr != iter);
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
    mIanaAddressUsed    = (iana_address > 0);
    mUsedMcastAddrCount = per_group_count + iana_address;
}

void GroupcastCluster::NotifyMembershipChanged()
{
    mMembershipChangedTimer.Start();
}

void GroupcastCluster::NotifyUsedMcastAddrCountOnChange()
{
    uint16_t old_count = mUsedMcastAddrCount;
    UpdateUsedMcastAddrCount();
    if (old_count != mUsedMcastAddrCount)
    {
        NotifyAttributeChanged(Groupcast::Attributes::UsedMcastAddrCount::Id);
    }
}
} // namespace Clusters
} // namespace app
} // namespace chip
