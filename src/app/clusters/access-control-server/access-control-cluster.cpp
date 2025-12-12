/**
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <access/AccessConfig.h>
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
#include <access/AccessRestrictionProvider.h>
#include <app/clusters/access-control-server/ArlEncoder.h>
#endif

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/access-control-server/access-control-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/reporting/reporting.h>
#include <app/server/AclStorage.h>
#include <app/server/Server.h>

#include <clusters/AccessControl/ClusterId.h>
#include <clusters/AccessControl/Metadata.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::AccessControl;
using namespace chip::app::Clusters::AccessControl::Commands;
using namespace chip::app::Clusters::AccessControl::Attributes;
using namespace chip::DeviceLayer;
using namespace chip::Access;

using AclEvent = Events::AccessControlEntryChanged::Type;

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
using ExtensionEvent = Events::AccessControlExtensionChanged::Type;

// TODO(#13590): generated code doesn't automatically handle max length so do it manually
constexpr int kExtensionDataMaxLength = 128;
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
using ArlReviewEvent = Events::FabricRestrictionReviewUpdate::Type;
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

namespace {
CHIP_ERROR ReadAcl(AttributeValueEncoder & aEncoder)
{
    AccessControl::EntryIterator iterator;
    AccessControl::Entry entry;
    AclStorage::EncodableEntry encodableEntry(entry);
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & info : Server::GetInstance().GetFabricTable())
        {
            auto fabric = info.GetFabricIndex();
            ReturnErrorOnFailure(GetAccessControl().Entries(fabric, iterator));
            CHIP_ERROR err = CHIP_NO_ERROR;
            while ((err = iterator.Next(entry)) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encoder.Encode(encodableEntry));
            }
            VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_ERROR_SENTINEL, err);
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR IsValidAclEntryList(const DataModel::DecodableList<AclStorage::DecodableEntry> & list)
{
    auto validationIterator = list.begin();
    while (validationIterator.Next())
    {
        VerifyOrReturnError(validationIterator.GetValue().GetEntry().IsValid(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    return validationIterator.GetStatus();
}

CHIP_ERROR WriteAcl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    FabricIndex accessingFabricIndex = aDecoder.AccessingFabricIndex();

    size_t oldCount;
    ReturnErrorOnFailure(GetAccessControl().GetEntryCount(accessingFabricIndex, oldCount));
    size_t maxCount;
    ReturnErrorOnFailure(GetAccessControl().GetMaxEntriesPerFabric(maxCount));

    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<AclStorage::DecodableEntry> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t newCount;
        ReturnErrorOnFailure(list.ComputeSize(&newCount));

        VerifyOrReturnError(newCount <= maxCount, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

        // Validating all ACL entries in the ReplaceAll list before Updating or Deleting any entries. If any of the entries has an
        // invalid field, the whole "ReplaceAll" list will be rejected.
        ReturnErrorOnFailure(IsValidAclEntryList(list));

        auto iterator = list.begin();
        size_t i      = 0;
        while (iterator.Next())
        {
            if (i < oldCount)
            {
                ReturnErrorOnFailure(GetAccessControl().UpdateEntry(&aDecoder.GetSubjectDescriptor(), accessingFabricIndex, i,
                                                                    iterator.GetValue().GetEntry()));
            }
            else
            {
                ReturnErrorOnFailure(GetAccessControl().CreateEntry(&aDecoder.GetSubjectDescriptor(), accessingFabricIndex, nullptr,
                                                                    iterator.GetValue().GetEntry()));
            }
            ++i;
        }
        ReturnErrorOnFailure(iterator.GetStatus());

        while (i < oldCount)
        {
            --oldCount;
            ReturnErrorOnFailure(GetAccessControl().DeleteEntry(&aDecoder.GetSubjectDescriptor(), accessingFabricIndex, oldCount));
        }
        return CHIP_NO_ERROR;
    }

    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        VerifyOrReturnError((oldCount + 1) <= maxCount, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

        AclStorage::DecodableEntry decodableEntry;
        ReturnErrorOnFailure(aDecoder.Decode(decodableEntry));

        return GetAccessControl().CreateEntry(&aDecoder.GetSubjectDescriptor(), accessingFabricIndex, nullptr,
                                              decodableEntry.GetEntry());
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
CHIP_ERROR ReadExtension(AttributeValueEncoder & aEncoder)
{
    auto & storage = Server::GetInstance().GetPersistentStorage();
    auto & fabrics = Server::GetInstance().GetFabricTable();

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & fabric : fabrics)
        {
            uint8_t buffer[kExtensionDataMaxLength] = { 0 };
            uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
            CHIP_ERROR errStorage                   = storage.SyncGetKeyValue(
                DefaultStorageKeyAllocator::AccessControlExtensionEntry(fabric.GetFabricIndex()).KeyName(), buffer, size);
            VerifyOrReturnError(errStorage != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
            if (errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                continue;
            }
            ReturnErrorOnFailure(errStorage);
            Structs::AccessControlExtensionStruct::Type item = {
                .data        = ByteSpan(buffer, size),
                .fabricIndex = fabric.GetFabricIndex(),
            };
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR
GenerateExtensionChangedEvent(const Structs::AccessControlExtensionStruct::Type & item,
                              const Access::SubjectDescriptor & subjectDescriptor, ChangeTypeEnum changeType,
                              ServerClusterContext * context)
{
    ExtensionEvent event{ .changeType = changeType, .fabricIndex = subjectDescriptor.fabricIndex };

    if (subjectDescriptor.authMode == Access::AuthMode::kCase)
    {
        event.adminNodeID.SetNonNull(subjectDescriptor.subject);
    }
    else if (subjectDescriptor.authMode == Access::AuthMode::kPase)
    {
        event.adminPasscodeID.SetNonNull(PAKEKeyIdFromNodeId(subjectDescriptor.subject));
    }

    event.latestValue.SetNonNull(item);

    return context->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId).has_value() ? CHIP_NO_ERROR
                                                                                                         : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR CheckExtensionEntryDataFormat(const ByteSpan & data)
{
    CHIP_ERROR err;

    TLV::TLVReader reader;
    reader.Init(data);

    auto containerType = TLV::kTLVType_List;
    err                = reader.Next(containerType, TLV::AnonymousTag());
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.EnterContainer(containerType);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(TLV::IsProfileTag(reader.GetTag()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.ExitContainer(containerType);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.Next();
    VerifyOrReturnError(err == CHIP_END_OF_TLV, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteExtension(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder, ServerClusterContext * context)
{
    auto & storage = Server::GetInstance().GetPersistentStorage();

    FabricIndex accessingFabricIndex = aDecoder.AccessingFabricIndex();

    uint8_t buffer[kExtensionDataMaxLength] = { 0 };
    uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
    CHIP_ERROR errStorage                   = storage.SyncGetKeyValue(
        DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(), buffer, size);

    // Every operation MUST generate an event. eventChangeType and eventItem will be set by the
    // processing logic and an event will be generated at the final step.
    ChangeTypeEnum eventChangeType;
    Structs::AccessControlExtensionStruct::Type eventItem;

    VerifyOrReturnError(errStorage != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(errStorage == CHIP_NO_ERROR || errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, errStorage);

    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<Structs::AccessControlExtensionStruct::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t count = 0;
        ReturnErrorOnFailure(list.ComputeSize(&count));

        if (count == 0)
        {
            VerifyOrReturnError(errStorage != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
            ReturnErrorOnFailure(storage.SyncDeleteKeyValue(
                DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName()));
            eventItem = {
                .data        = ByteSpan(buffer, size),
                .fabricIndex = accessingFabricIndex,
            };
            eventChangeType = ChangeTypeEnum::kRemoved;
        }
        else if (count == 1)
        {
            auto iterator = list.begin();
            if (!iterator.Next())
            {
                ReturnErrorOnFailure(iterator.GetStatus());
                // If counted an item, iterator doesn't return it, iterator has no error, that's bad.
                return CHIP_ERROR_INCORRECT_STATE;
            }
            auto & item = iterator.GetValue();
            // TODO(#13590): generated code doesn't automatically handle max length so do it manually
            VerifyOrReturnError(item.data.size() <= kExtensionDataMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

            ReturnErrorOnFailure(CheckExtensionEntryDataFormat(item.data));

            ReturnErrorOnFailure(
                storage.SyncSetKeyValue(DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(),
                                        item.data.data(), static_cast<uint16_t>(item.data.size())));
            eventItem = item;
            eventChangeType =
                errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ? ChangeTypeEnum::kAdded : ChangeTypeEnum::kChanged;
        }
        else
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        VerifyOrReturnError(errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        Structs::AccessControlExtensionStruct::DecodableType item;
        ReturnErrorOnFailure(aDecoder.Decode(item));
        // TODO(#13590): generated code doesn't automatically handle max length so do it manually
        VerifyOrReturnError(item.data.size() <= kExtensionDataMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        ReturnErrorOnFailure(CheckExtensionEntryDataFormat(item.data));

        ReturnErrorOnFailure(
            storage.SyncSetKeyValue(DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(),
                                    item.data.data(), static_cast<uint16_t>(item.data.size())));
        eventItem       = item;
        eventChangeType = ChangeTypeEnum::kAdded;
    }
    else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return GenerateExtensionChangedEvent(eventItem, aDecoder.GetSubjectDescriptor(), eventChangeType, context);
}
#endif

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
CHIP_ERROR ReadCommissioningArl(AttributeValueEncoder & aEncoder)
{
    auto accessRestrictionProvider = GetAccessControl().GetAccessRestrictionProvider();

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        if (accessRestrictionProvider != nullptr)
        {
            auto entries = accessRestrictionProvider->GetCommissioningEntries();

            for (auto & entry : entries)
            {
                ArlEncoder::CommissioningEncodableEntry encodableEntry(entry);
                ReturnErrorOnFailure(encoder.Encode(encodableEntry));
            }
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ReadArl(AttributeValueEncoder & aEncoder)
{
    auto accessRestrictionProvider = GetAccessControl().GetAccessRestrictionProvider();

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        if (accessRestrictionProvider != nullptr)
        {
            for (const auto & info : Server::GetInstance().GetFabricTable())
            {
                auto fabric = info.GetFabricIndex();
                // get entries for fabric
                std::vector<AccessRestrictionProvider::Entry> entries;
                ReturnErrorOnFailure(accessRestrictionProvider->GetEntries(fabric, entries));
                for (const auto & entry : entries)
                {
                    ArlEncoder::EncodableEntry encodableEntry(entry);
                    ReturnErrorOnFailure(encoder.Encode(encodableEntry));
                }
            }
        }
        return CHIP_NO_ERROR;
    });
}
#endif

CHIP_ERROR ChipErrorToImErrorMap(CHIP_ERROR err)
{
    // Map some common errors into an underlying IM error
    // Separate logging is done to not lose the original error location in case such
    // this are available.
    Protocols::InteractionModel::Status statusOfErr;

    if (err == CHIP_ERROR_INVALID_ARGUMENT || err == CHIP_ERROR_NOT_FOUND)
    {
        // Not found is generally also illegal argument: caused a lookup into an invalid location,
        // like invalid subjects or targets.
        statusOfErr = Protocols::InteractionModel::Status::ConstraintError;
    }
    else if (err == CHIP_ERROR_NO_MEMORY)
    {
        statusOfErr = Protocols::InteractionModel::Status::ResourceExhausted;
    }
    else
    {
        return err;
    }

    ChipLogError(DataManagement, "Mapped %" CHIP_ERROR_FORMAT " into " ChipLogFormatIMStatus, err.Format(),
                 ChipLogValueIMStatus(statusOfErr));

    return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(statusOfErr);
}

CHIP_ERROR WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder,
                     ServerClusterContext * context)
{
    switch (request.path.mAttributeId)
    {
    case Acl::Id:
        return WriteAcl(request.path, decoder);
#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    case Extension::Id:
        return WriteExtension(request.path, decoder, context);
#endif
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR AccessControlCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(DataManagement, "AccessControlCluster: initializing");
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    GetAccessControl().AddEntryListener(*this);

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    auto accessRestrictionProvider = GetAccessControl().GetAccessRestrictionProvider();
    if (accessRestrictionProvider != nullptr)
    {
        accessRestrictionProvider->AddListener(*this);
    }
#endif

    return CHIP_NO_ERROR;
}

void AccessControlCluster::Shutdown()
{
    ChipLogProgress(DataManagement, "AccessControlCluster: shutdown");

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    auto accessRestrictionProvider = GetAccessControl().GetAccessRestrictionProvider();
    if (accessRestrictionProvider != nullptr)
    {
        accessRestrictionProvider->RemoveListener(*this);
    }
#endif

    GetAccessControl().RemoveEntryListener(*this);
    DefaultServerCluster::Shutdown();
}

DataModel::ActionReturnStatus AccessControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & encoder)
{
    // in many cases attributes are numeric. Do a fallback to handle the general case
    // This is to save some flash:
    //   - value is considered a uint32_value and encoded as such
    //   - valueFetchError is the underlying CHIP_ERROR that we get when fetching `value`
    size_t value                       = 0;
    CHIP_ERROR valueFetchError         = CHIP_NO_ERROR;
    const Access::AccessControl & ctrl = GetAccessControl();

    switch (request.path.mAttributeId)
    {
    case AccessControl::Attributes::Acl::Id:
        return ReadAcl(encoder);
#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    case AccessControl::Attributes::Extension::Id:
        return ReadExtension(encoder);
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    case AccessControl::Attributes::CommissioningARL::Id:
        return ReadCommissioningArl(encoder);
    case AccessControl::Attributes::Arl::Id:
        return ReadArl(encoder);
#endif
    case AccessControl::Attributes::SubjectsPerAccessControlEntry::Id:
        valueFetchError = ctrl.GetMaxSubjectsPerEntry(value);
        break;
    case AccessControl::Attributes::TargetsPerAccessControlEntry::Id:
        valueFetchError = ctrl.GetMaxTargetsPerEntry(value);
        break;
    case AccessControl::Attributes::AccessControlEntriesPerFabric::Id:
        valueFetchError = ctrl.GetMaxEntriesPerFabric(value);
        break;
    case AccessControl::Attributes::FeatureMap::Id: {
        value = 0;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
        value |= to_underlying(Clusters::AccessControl::Feature::kManagedDevice);
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
        value |= to_underlying(Clusters::AccessControl::Feature::kExtension);
#endif // CHIP_CONFIG_ENABLE_ACL_EXTENSIONS

        break;
    }
    case AccessControl::Attributes::ClusterRevision::Id:
        value = kRevision;
        break;
    default:
        valueFetchError = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        break;
    }

    ReturnErrorOnFailure(valueFetchError);
    return encoder.Encode(static_cast<uint32_t>(value));
}

DataModel::ActionReturnStatus AccessControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                   AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, ChipErrorToImErrorMap(WriteImpl(request, decoder, mContext)));
}

CHIP_ERROR AccessControlCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS || CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    AttributeListBuilder::OptionalAttributeEntry kOptionalAttributes[] = {
#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
        { .enabled = true, .metadata = Attributes::Extension::kMetadataEntry },
#endif
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
        { .enabled = true, .metadata = Attributes::CommissioningARL::kMetadataEntry },
        { .enabled = true, .metadata = Attributes::Arl::kMetadataEntry },
#endif
    };

    return listBuilder.Append(Span(AccessControl::Attributes::kMandatoryMetadata), Span(kOptionalAttributes));
#else
    return listBuilder.Append(Span(AccessControl::Attributes::kMandatoryMetadata), {});
#endif
}

CHIP_ERROR AccessControlCluster::EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo)
{
    eventInfo.readPrivilege = Access::Privilege::kAdminister;
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

CHIP_ERROR AccessControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Commands::ReviewFabricRestrictions::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR AccessControlCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = {
        Commands::ReviewFabricRestrictionsResponse::Id,
    };
    return builder.ReferenceExisting(kGeneratedCommands);
}

std::optional<DataModel::ActionReturnStatus> AccessControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & inputArguments,
                                                                                 CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case ReviewFabricRestrictions::Id:
        return HandleCommand<ReviewFabricRestrictions::DecodableType>(
            inputArguments, handler->GetAccessingFabricIndex(),
            [&](const auto & data) { return HandleReviewFabricRestrictions(handler, request.path, data); });
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus> AccessControlCluster::HandleReviewFabricRestrictions(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Clusters::AccessControl::Commands::ReviewFabricRestrictions::DecodableType & commandData)
{
    if (commandPath.mEndpointId != kRootEndpointId)
    {
        ChipLogError(DataManagement, "AccessControlCluster: invalid endpoint in ReviewFabricRestrictions request");
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    uint64_t token;
    std::vector<AccessRestrictionProvider::Entry> entries;
    auto entryIter = commandData.arl.begin();
    while (entryIter.Next())
    {
        AccessRestrictionProvider::Entry entry;
        entry.fabricIndex    = commandObj->GetAccessingFabricIndex();
        entry.endpointNumber = entryIter.GetValue().endpoint;
        entry.clusterId      = entryIter.GetValue().cluster;

        auto restrictionIter = entryIter.GetValue().restrictions.begin();
        while (restrictionIter.Next())
        {
            AccessRestrictionProvider::Restriction restriction;
            if (ArlEncoder::Convert(restrictionIter.GetValue().type, restriction.restrictionType) != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "AccessControlCluster: invalid restriction type conversion");
                return Protocols::InteractionModel::Status::InvalidCommand;
            }

            if (!restrictionIter.GetValue().id.IsNull())
            {
                restriction.id.SetValue(restrictionIter.GetValue().id.Value());
            }
            entry.restrictions.push_back(restriction);
        }

        if (restrictionIter.GetStatus() != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "AccessControlCluster: invalid ARL data");
            return Protocols::InteractionModel::Status::InvalidCommand;
        }

        entries.push_back(entry);
    }

    if (entryIter.GetStatus() != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "AccessControlCluster: invalid ARL data");
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    CHIP_ERROR err = GetAccessControl().GetAccessRestrictionProvider()->RequestFabricRestrictionReview(
        commandObj->GetAccessingFabricIndex(), entries, token);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "AccessControlCluster: restriction review failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Protocols::InteractionModel::ClusterStatusCode(err);
    }

    Clusters::AccessControl::Commands::ReviewFabricRestrictionsResponse::Type response;
    response.token = token;
    commandObj->AddResponse(commandPath, response);
    return std::nullopt;
}

void AccessControlCluster::MarkCommissioningRestrictionListChanged()
{
    NotifyAttributeChanged(AccessControl::Attributes::CommissioningARL::Id);
}

void AccessControlCluster::MarkRestrictionListChanged(FabricIndex fabricIndex)
{
    NotifyAttributeChanged(AccessControl::Attributes::Arl::Id);
}

void AccessControlCluster::OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, Optional<CharSpan> instruction,
                                                           Optional<CharSpan> arlRequestFlowUrl)
{
    ArlReviewEvent event{ .token = token, .fabricIndex = fabricIndex };

    event.instruction       = instruction;
    event.ARLRequestFlowUrl = arlRequestFlowUrl;

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

#endif //  CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

void AccessControlCluster::OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                                          const Access::AccessControl::Entry * entry,
                                          Access::AccessControl::EntryListener::ChangeType changeType)
{
    // NOTE: If the entry was changed internally by the system (e.g. creating
    // entries at startup from persistent storage, or deleting entries when a
    // fabric is removed), then there won't be a subject descriptor, and also
    // it won't be appropriate to create an event.
    VerifyOrReturn((mContext != nullptr) && (subjectDescriptor != nullptr));
    AclEvent event{ .changeType = ChangeTypeEnum::kChanged, .fabricIndex = subjectDescriptor->fabricIndex };

    if (changeType == Access::AccessControl::EntryListener::ChangeType::kAdded)
    {
        event.changeType = ChangeTypeEnum::kAdded;
    }
    else if (changeType == Access::AccessControl::EntryListener::ChangeType::kRemoved)
    {
        event.changeType = ChangeTypeEnum::kRemoved;
    }

    if (subjectDescriptor->authMode == Access::AuthMode::kCase)
    {
        event.adminNodeID.SetNonNull(subjectDescriptor->subject);
    }
    else if (subjectDescriptor->authMode == Access::AuthMode::kPase)
    {
        event.adminPasscodeID.SetNonNull(PAKEKeyIdFromNodeId(subjectDescriptor->subject));
    }

    if (entry != nullptr)
    {
        AclStorage::EncodableEntry encodableEntry(*entry);
        CHIP_ERROR err = encodableEntry.Stage();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "AccessControlCluster: event failed %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }

        event.latestValue.SetNonNull(encodableEntry.GetStagingEntry());
        // NOTE: EncodableEntry can only be constructed from ref so we need to use it within the right scope
        // after we determined the entry is not null. This is why we repeat the generate event call.
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
        return;
    }

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

} // namespace Clusters
} // namespace app
} // namespace chip
