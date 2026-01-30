/**
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/group-key-mgmt-server/GroupKeyManagementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/GroupKeyManagement/ClusterId.h>
#include <clusters/GroupKeyManagement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GroupKeyManagement;
using namespace chip::app::Clusters::GroupKeyManagement::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

namespace {
struct GroupTableCodec
{
    static constexpr TLV::Tag TagFabric()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kFabricIndex);
    }
    static constexpr TLV::Tag TagGroup()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kGroupId);
    }
    static constexpr TLV::Tag TagEndpoints()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kEndpoints);
    }
    static constexpr TLV::Tag TagGroupName()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kGroupName);
    }

    GroupDataProvider * mProvider = nullptr;
    chip::FabricIndex mFabric;
    GroupDataProvider::GroupInfo mInfo;

    GroupTableCodec(GroupDataProvider * provider, chip::FabricIndex fabric_index, GroupDataProvider::GroupInfo & info) :
        mProvider(provider), mFabric(fabric_index), mInfo(info)
    {}

    static constexpr bool kIsFabricScoped = true;

    auto GetFabricIndex() const { return mFabric; }

    CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        // FabricIndex
        ReturnErrorOnFailure(DataModel::Encode(writer, TagFabric(), mFabric));
        // GroupId
        ReturnErrorOnFailure(DataModel::Encode(writer, TagGroup(), mInfo.group_id));
        // Endpoints
        TLV::TLVType inner;
        ReturnErrorOnFailure(writer.StartContainer(TagEndpoints(), TLV::kTLVType_Array, inner));
        GroupDataProvider::GroupEndpoint mapping;
        auto iter = mProvider->IterateEndpoints(mFabric, std::make_optional(mInfo.group_id));
        if (nullptr != iter)
        {
            while (iter->Next(mapping))
            {
                ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), static_cast<uint16_t>(mapping.endpoint_id)));
            }
            iter->Release();
        }
        ReturnErrorOnFailure(writer.EndContainer(inner));
        // GroupName
        uint32_t name_size = static_cast<uint32_t>(strnlen(mInfo.name, GroupDataProvider::GroupInfo::kGroupNameMax));
        ReturnErrorOnFailure(writer.PutString(TagGroupName(), mInfo.name, name_size));

        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

/*
 * This struct is used to build the response when the KeySetReadAllIndicies command
 * is invoked. It follows the format expected by AddResponse() by using a struct that
 * can be encoded with DataModel::Encode like the one in
 * GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Type. This struct however
 * specifies a different Encode() function that loops through all the elements pointed
 * to by the iterator and encodes each of them.
 */
struct KeySetReadAllIndicesResponse
{
    static constexpr CommandId GetCommandId() { return GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Id; }
    static constexpr ClusterId GetClusterId() { return GroupKeyManagement::Id; }

    GroupDataProvider::KeySetIterator * mIterator = nullptr;

    KeySetReadAllIndicesResponse(GroupDataProvider::KeySetIterator * iter) : mIterator(iter) {}

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        TLV::TLVType array;
        ReturnErrorOnFailure(writer.StartContainer(
            TLV::ContextTag(GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Fields::kGroupKeySetIDs),
            TLV::kTLVType_Array, array));

        GroupDataProvider::KeySet keyset;
        while (mIterator && mIterator->Next(keyset))
        {
            ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), keyset.keyset_id));
        }

        ReturnErrorOnFailure(writer.EndContainer(array));
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};
CHIP_ERROR ReadGroupKeyMap(FabricTable & fabricTable, GroupDataProvider & provider, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&fabricTable, &provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR encodeStatus = CHIP_NO_ERROR;

        for (auto & fabric : fabricTable)
        {
            auto fabric_index = fabric.GetFabricIndex();
            auto iter         = provider.IterateGroupKeys(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupKey mapping;
            while (iter->Next(mapping))
            {
                GroupKeyManagement::Structs::GroupKeyMapStruct::Type key = {
                    .groupId       = mapping.group_id,
                    .groupKeySetID = mapping.keyset_id,
                    .fabricIndex   = fabric_index,
                };
                encodeStatus = encoder.Encode(key);
                if (encodeStatus != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            iter->Release();
            if (encodeStatus != CHIP_NO_ERROR)
            {
                break;
            }
        }
        return encodeStatus;
    });
}

CHIP_ERROR WriteGroupKeyMap(GroupDataProvider & provider, const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    auto fabric_index = aDecoder.AccessingFabricIndex();

    if (!aPath.IsListItemOperation())
    {
        Attributes::GroupKeyMap::TypeInfo::DecodableType list;
        size_t new_count;

        ReturnErrorOnFailure(aDecoder.Decode(list));
        ReturnErrorOnFailure(list.ComputeSize(&new_count));

        // Remove existing keys, ignore errors
        TEMPORARY_RETURN_IGNORED provider.RemoveGroupKeys(fabric_index);

        // Add the new keys
        auto iter = list.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            const auto & value = iter.GetValue();
            VerifyOrReturnError(fabric_index == value.fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
            // Cannot map to IPK, see `GroupKeyMapStruct` in Group Key Management cluster spec
            VerifyOrReturnError(value.groupKeySetID != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));

            ReturnErrorOnFailure(
                provider.SetGroupKeyAt(value.fabricIndex, i++, GroupDataProvider::GroupKey(value.groupId, value.groupKeySetID)));
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        Structs::GroupKeyMapStruct::DecodableType value;
        size_t current_count = 0;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        VerifyOrReturnError(fabric_index == value.fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
        // Cannot map to IPK, see `GroupKeyMapStruct` in Group Key Management cluster spec
        VerifyOrReturnError(value.groupKeySetID != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        {
            auto iter     = provider.IterateGroupKeys(fabric_index);
            current_count = iter->Count();
            iter->Release();
        }

        ReturnErrorOnFailure(provider.SetGroupKeyAt(value.fabricIndex, current_count,
                                                    GroupDataProvider::GroupKey(value.groupId, value.groupKeySetID)));
    }
    else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadGroupTable(FabricTable & fabricTable, GroupDataProvider & provider, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&fabricTable, &provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR encodeStatus = CHIP_NO_ERROR;

        for (auto & fabric : fabricTable)
        {
            auto fabric_index = fabric.GetFabricIndex();
            auto iter         = provider.IterateGroupInfo(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupInfo info;
            while (iter->Next(info))
            {
                encodeStatus = encoder.Encode(GroupTableCodec(&provider, fabric_index, info));
                if (encodeStatus != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            iter->Release();
            if (encodeStatus != CHIP_NO_ERROR)
            {
                break;
            }
        }

        return encodeStatus;
    });
}

CHIP_ERROR ReadMaxGroupsPerFabric(GroupDataProvider & provider, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(provider.GetMaxGroupsPerFabric());
}

CHIP_ERROR ReadMaxGroupKeysPerFabric(GroupDataProvider & provider, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(provider.GetMaxGroupKeysPerFabric());
}

const FabricInfo * GetFabricInfoOrNull(CommandHandler * handler, FabricTable & fabricTable)
{
    return fabricTable.FindFabricWithIndex(handler->GetAccessingFabricIndex());
}

Status ValidateKeySetWriteArguments(const Commands::KeySetWrite::DecodableType & commandData)
{
    // SPEC: If the EpochKey0 field is null or its associated EpochStartTime0 field is null, then this command SHALL fail with an
    // INVALID_COMMAND status code responded to the client.
    if (commandData.groupKeySet.epochKey0.IsNull() || commandData.groupKeySet.epochStartTime0.IsNull())
    {
        return Status::InvalidCommand;
    }

    // SPEC: If the EpochStartTime0 is set to 0, then this command SHALL fail with an INVALID_COMMAND status code responded to the
    // client.
    if (0 == commandData.groupKeySet.epochStartTime0.Value())
    {
        return Status::InvalidCommand;
    }

    // By now we at least have epochKey0.
    static_assert(GroupDataProvider::EpochKey::kLengthBytes == 16,
                  "Expect EpochKey internal data structure to have a length of 16 bytes.");

    // SPEC: If the EpochKey0 field's length is not exactly 16 bytes, then this command SHALL fail with a CONSTRAINT_ERROR status
    // code responded to the client.
    if (commandData.groupKeySet.epochKey0.Value().size() != GroupDataProvider::EpochKey::kLengthBytes)
    {
        return Status::ConstraintError;
    }

    // Already known to be false by now
    bool epoch_key0_is_null    = false;
    uint64_t epoch_start_time0 = commandData.groupKeySet.epochStartTime0.Value();

    bool epoch_key1_is_null        = commandData.groupKeySet.epochKey1.IsNull();
    bool epoch_start_time1_is_null = commandData.groupKeySet.epochStartTime1.IsNull();

    uint64_t epoch_start_time1 = 0; // Will be overridden when known to be present.

    // SPEC: If exactly one of the EpochKey1 or EpochStartTime1 is null, rather than both being null, or neither being null, then
    // this command SHALL fail with an INVALID_COMMAND status code responded to the client.
    if (epoch_key1_is_null != epoch_start_time1_is_null)
    {
        return Status::InvalidCommand;
    }

    if (!epoch_key1_is_null)
    {
        // SPEC: If the EpochKey1 field is not null, then the EpochKey0 field SHALL NOT be null. Otherwise this command SHALL fail
        // with an INVALID_COMMAND status code responded to the client.
        if (epoch_key0_is_null)
        {
            return Status::InvalidCommand;
        }

        // SPEC: If the EpochKey1 field is not null, and the field's length is not exactly 16 bytes, then this command SHALL fail
        // with a CONSTRAINT_ERROR status code responded to the client.
        if (commandData.groupKeySet.epochKey1.Value().size() != GroupDataProvider::EpochKey::kLengthBytes)
        {
            return Status::ConstraintError;
        }

        // By now, if EpochKey1 was present, we know EpochStartTime1 was also present.
        epoch_start_time1 = commandData.groupKeySet.epochStartTime1.Value();

        // SPEC: If the EpochKey1 field is not null, its associated EpochStartTime1 field SHALL NOT be null and SHALL contain a
        // later epoch start time than the epoch start time found in the EpochStartTime0 field. Otherwise this command SHALL fail
        // with an INVALID_COMMAND status code responded to the client.
        bool epoch1_later_than_epoch0 = epoch_start_time1 > epoch_start_time0;
        if (!epoch1_later_than_epoch0)
        {
            return Status::InvalidCommand;
        }
    }

    bool epoch_key2_is_null        = commandData.groupKeySet.epochKey2.IsNull();
    bool epoch_start_time2_is_null = commandData.groupKeySet.epochStartTime2.IsNull();

    // SPEC: If exactly one of the EpochKey2 or EpochStartTime2 is null, rather than both being null, or neither being null, then
    // this command SHALL fail with an INVALID_COMMAND status code responded to the client.
    if (epoch_key2_is_null != epoch_start_time2_is_null)
    {
        return Status::InvalidCommand;
    }

    if (!epoch_key2_is_null)
    {
        // SPEC: If the EpochKey2 field is not null, then the EpochKey1 and EpochKey0 fields SHALL NOT be null. Otherwise this
        // command SHALL fail with an INVALID_COMMAND status code responded to the client.
        if (epoch_key0_is_null || epoch_key1_is_null)
        {
            return Status::InvalidCommand;
        }

        // SPEC: If the EpochKey2 field is not null, and the field's length is not exactly 16 bytes, then this command SHALL fail
        // with a CONSTRAINT_ERROR status code responded to the client.
        if (commandData.groupKeySet.epochKey2.Value().size() != GroupDataProvider::EpochKey::kLengthBytes)
        {
            return Status::ConstraintError;
        }

        // By now, if EpochKey2 was present, we know EpochStartTime2 was also present.
        uint64_t epoch_start_time2 = commandData.groupKeySet.epochStartTime2.Value();

        // SPEC: If the EpochKey2 field is not null, its associated EpochStartTime2 field SHALL NOT be null and SHALL contain a
        // later epoch start time than the epoch start time found in the EpochStartTime1 field. Otherwise this command SHALL fail
        // with an INVALID_COMMAND status code responded to the client.
        bool epoch2_later_than_epoch1 = epoch_start_time2 > epoch_start_time1;
        if (!epoch2_later_than_epoch1)
        {
            return Status::InvalidCommand;
        }
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> HandleKeySetWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                               const Commands::KeySetWrite::DecodableType & commandData,
                                                               Credentials::GroupDataProvider * provider, const FabricInfo * fabric)
{
    // Pre-validate all complex data dependency assumptions about the epoch keys
    Status status = ValidateKeySetWriteArguments(commandData);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status, "Failure to validate KeySet data dependencies.");
        return std::nullopt;
    }

    if (commandData.groupKeySet.groupKeySecurityPolicy == GroupKeySecurityPolicyEnum::kUnknownEnumValue)
    {
        // If a client indicates an enumeration value to the server, that is not
        // supported by the server, because it is ... a new value unrecognized
        // by a legacy server, then the server SHALL generate a general
        // constraint error
        commandObj->AddStatus(commandPath, Status::ConstraintError, "Received unknown GroupKeySecurityPolicyEnum value");
        return std::nullopt;
    }

    if (!GroupKeyManagementCluster::IsMCSPSupported() &&
        commandData.groupKeySet.groupKeySecurityPolicy == GroupKeySecurityPolicyEnum::kCacheAndSync)
    {
        // When CacheAndSync is not supported in the FeatureMap of this cluster,
        // any action attempting to set CacheAndSync in the
        // GroupKeySecurityPolicy field SHALL fail with an INVALID_COMMAND
        // error.
        commandObj->AddStatus(commandPath, Status::InvalidCommand,
                              "Received a CacheAndSync GroupKeySecurityPolicyEnum when MCSP not supported");
        return std::nullopt;
    }

    // All flight checks completed: by now we know that non-null keys are all valid and correct size.
    bool epoch_key1_present = !commandData.groupKeySet.epochKey1.IsNull();
    bool epoch_key2_present = !commandData.groupKeySet.epochKey2.IsNull();

    GroupDataProvider::KeySet keyset(commandData.groupKeySet.groupKeySetID, commandData.groupKeySet.groupKeySecurityPolicy, 0);

    // Epoch Key 0 always present
    keyset.epoch_keys[0].start_time = commandData.groupKeySet.epochStartTime0.Value();
    memcpy(keyset.epoch_keys[0].key, commandData.groupKeySet.epochKey0.Value().data(), GroupDataProvider::EpochKey::kLengthBytes);
    keyset.num_keys_used++;

    // Epoch Key 1
    if (epoch_key1_present)
    {
        keyset.epoch_keys[1].start_time = commandData.groupKeySet.epochStartTime1.Value();
        memcpy(keyset.epoch_keys[1].key, commandData.groupKeySet.epochKey1.Value().data(),
               GroupDataProvider::EpochKey::kLengthBytes);
        keyset.num_keys_used++;
    }

    // Epoch Key 2
    if (epoch_key2_present)
    {
        keyset.epoch_keys[2].start_time = commandData.groupKeySet.epochStartTime2.Value();
        memcpy(keyset.epoch_keys[2].key, commandData.groupKeySet.epochKey2.Value().data(),
               GroupDataProvider::EpochKey::kLengthBytes);
        keyset.num_keys_used++;
    }

    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
    CHIP_ERROR err = fabric->GetCompressedFabricIdBytes(compressed_fabric_id);
    if (CHIP_NO_ERROR != err)
    {
        return Status::Failure;
    }

    // Set KeySet
    err = provider->SetKeySet(fabric->GetFabricIndex(), compressed_fabric_id, keyset);
    if (CHIP_ERROR_INVALID_LIST_LENGTH == err)
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted, "Not enough space left to add a new KeySet");
        return std::nullopt;
    }

    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetWrite OK");
    }
    else
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetWrite: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Send response
    return StatusIB(err).mStatus;
}

std::optional<DataModel::ActionReturnStatus> HandleKeySetRead(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                              const Commands::KeySetRead::DecodableType & commandData,
                                                              Credentials::GroupDataProvider * provider, const FabricInfo * fabric)
{
    FabricIndex fabricIndex = fabric->GetFabricIndex();
    GroupDataProvider::KeySet keyset;
    if (CHIP_NO_ERROR != provider->GetKeySet(fabricIndex, commandData.groupKeySetID, keyset))
    {
        // KeySet ID not found
        commandObj->AddStatus(commandPath, Status::NotFound, "Keyset ID not found in KeySetRead");
        return std::nullopt;
    }

    // In KeySetReadResponse, EpochKey0, EpochKey1 and EpochKey2 key contents shall be null
    GroupKeyManagement::Commands::KeySetReadResponse::Type response;
    response.groupKeySet.groupKeySetID          = keyset.keyset_id;
    response.groupKeySet.groupKeySecurityPolicy = keyset.policy;

    // Keyset 0
    if (keyset.num_keys_used > 0)
    {
        response.groupKeySet.epochStartTime0.SetNonNull(keyset.epoch_keys[0].start_time);
    }
    else
    {
        response.groupKeySet.epochStartTime0.SetNull();
    }
    response.groupKeySet.epochKey0.SetNull();

    // Keyset 1
    if (keyset.num_keys_used > 1)
    {
        response.groupKeySet.epochStartTime1.SetNonNull(keyset.epoch_keys[1].start_time);
    }
    else
    {
        response.groupKeySet.epochStartTime1.SetNull();
    }
    response.groupKeySet.epochKey1.SetNull();

    // Keyset 2
    if (keyset.num_keys_used > 2)
    {
        response.groupKeySet.epochStartTime2.SetNonNull(keyset.epoch_keys[2].start_time);
    }
    else
    {
        response.groupKeySet.epochStartTime2.SetNull();
    }
    response.groupKeySet.epochKey2.SetNull();

    commandObj->AddResponse(commandPath, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> HandleKeySetRemove(CommandHandler * commandObj,
                                                                const ConcreteCommandPath & commandPath,
                                                                const Commands::KeySetRemove::DecodableType & commandData,
                                                                Credentials::GroupDataProvider * provider,
                                                                const FabricInfo * fabric)

{
    if (commandData.groupKeySetID == GroupDataProvider::kIdentityProtectionKeySetId)
    {
        // SPEC: This command SHALL fail with an INVALID_COMMAND status code back to the initiator if the GroupKeySetID being
        // removed is 0, which is the Key Set associated with the Identity Protection Key (IPK).
        commandObj->AddStatus(commandPath, Status::InvalidCommand, "Attempted to KeySetRemove the identity protection key!");
        return std::nullopt;
    }

    // Remove keyset
    FabricIndex fabricIndex = fabric->GetFabricIndex();
    CHIP_ERROR err          = provider->RemoveKeySet(fabricIndex, commandData.groupKeySetID);

    if (CHIP_NO_ERROR == err)
    {
        return err;
    }

    Status status = (CHIP_ERROR_NOT_FOUND == err || CHIP_ERROR_KEY_NOT_FOUND == err) ? Status::NotFound : Status::Failure;

    // Send status response.
    commandObj->AddStatus(commandPath, status, "KeySetRemove failed");
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
HandleKeySetReadAllIndices(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                           const Commands::KeySetReadAllIndices::DecodableType & commandData,
                           Credentials::GroupDataProvider * provider, const FabricInfo * fabric)
{
    FabricIndex fabricIndex = fabric->GetFabricIndex();
    auto keysIt             = provider->IterateKeySets(fabricIndex);
    if (nullptr == keysIt)
    {
        commandObj->AddStatus(commandPath, Status::Failure, "Failed iteration of key set indices!");
        return std::nullopt;
    }

    commandObj->AddResponse(commandPath, KeySetReadAllIndicesResponse(keysIt));
    keysIt->Release();
    return std::nullopt;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {

std::optional<DataModel::ActionReturnStatus> GroupKeyManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      chip::TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    const FabricInfo * fabric = GetFabricInfoOrNull(handler, mContext.fabricTable);

    if (fabric == nullptr)
    {
        ChipLogError(Zcl, "GroupKeyManagement: Failed to find fabric for index %u", handler->GetAccessingFabricIndex());
        return CHIP_ERROR_INTERNAL;
    }

    const FabricIndex fabric_index = fabric->GetFabricIndex();

    GroupDataProvider * provider = &mContext.groupDataProvider;

    switch (request.path.mCommandId)
    {
    case GroupKeyManagement::Commands::KeySetWrite::Id: {
        GroupKeyManagement::Commands::KeySetWrite::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabric_index));
        return HandleKeySetWrite(handler, request.path, request_data, provider, fabric);
    }
    case GroupKeyManagement::Commands::KeySetRead::Id: {
        GroupKeyManagement::Commands::KeySetRead::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabric_index));
        return HandleKeySetRead(handler, request.path, request_data, provider, fabric);
    }
    case GroupKeyManagement::Commands::KeySetRemove::Id: {
        GroupKeyManagement::Commands::KeySetRemove::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabric_index));
        return HandleKeySetRemove(handler, request.path, request_data, provider, fabric);
    }
    case GroupKeyManagement::Commands::KeySetReadAllIndices::Id: {
        GroupKeyManagement::Commands::KeySetReadAllIndices::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabric_index));
        return HandleKeySetReadAllIndices(handler, request.path, request_data, provider, fabric);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus GroupKeyManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case GroupKeyManagement::Attributes::ClusterRevision::Id:
        return encoder.Encode(GroupKeyManagement::kRevision);
    case Attributes::FeatureMap::Id: {
        BitFlags<GroupKeyManagement::Feature> features;
        if (IsMCSPSupported())
        {
            features.Set(Clusters::GroupKeyManagement::Feature::kCacheAndSync);
        }
        return encoder.Encode(features);
    }
    case GroupKeyManagement::Attributes::GroupKeyMap::Id:
        return ReadGroupKeyMap(mContext.fabricTable, mContext.groupDataProvider, encoder);
    case GroupKeyManagement::Attributes::GroupTable::Id:
        return ReadGroupTable(mContext.fabricTable, mContext.groupDataProvider, encoder);
    case GroupKeyManagement::Attributes::MaxGroupsPerFabric::Id:
        return ReadMaxGroupsPerFabric(mContext.groupDataProvider, encoder);
    case GroupKeyManagement::Attributes::MaxGroupKeysPerFabric::Id:
        return ReadMaxGroupKeysPerFabric(mContext.groupDataProvider, encoder);
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus GroupKeyManagementCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                        AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case GroupKeyMap::Id: {
        return NotifyAttributeChangedIfSuccess(request.path.mAttributeId,
                                               WriteGroupKeyMap(mContext.groupDataProvider, request.path, decoder));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR GroupKeyManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(GroupKeyManagement::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>());
}

CHIP_ERROR GroupKeyManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Commands::KeySetWrite::kMetadataEntry,
        Commands::KeySetRead::kMetadataEntry,
        Commands::KeySetRemove::kMetadataEntry,
        Commands::KeySetReadAllIndices::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR GroupKeyManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = {
        Commands::KeySetReadAllIndicesResponse::Id,
        Commands::KeySetReadResponse::Id,
    };
    return builder.ReferenceExisting(kGeneratedCommands);
}

} // namespace Clusters
} // namespace app
} // namespace chip
