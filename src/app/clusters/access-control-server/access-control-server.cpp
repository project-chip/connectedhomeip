/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <access/AccessControl.h>

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/server/AclStorage.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

namespace AccessControlCluster = chip::app::Clusters::AccessControl;

using AclEvent       = Clusters::AccessControl::Events::AccessControlEntryChanged::Type;
using ChangeTypeEnum = Clusters::AccessControl::ChangeTypeEnum;
using Entry          = AccessControl::Entry;
using EntryListener  = AccessControl::EntryListener;
using ExtensionEvent = Clusters::AccessControl::Events::AccessControlExtensionChanged::Type;

// TODO(#13590): generated code doesn't automatically handle max length so do it manually
constexpr int kExtensionDataMaxLength = 128;

constexpr uint16_t kClusterRevision = 1;

namespace {

class AccessControlAttribute : public AttributeAccessInterface, public EntryListener
{
public:
    AccessControlAttribute() : AttributeAccessInterface(Optional<EndpointId>(0), AccessControlCluster::Id) {}

    /// IM-level implementation of read
    ///
    /// Returns appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /// IM-level implementation of write
    ///
    /// Returns appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

public:
    void OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index, const Entry * entry,
                        ChangeType changeType) override;

private:
    /// Business logic implementation of write, returns generic CHIP_ERROR.
    CHIP_ERROR ReadImpl(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

    /// Business logic implementation of write, returns generic CHIP_ERROR.
    CHIP_ERROR WriteImpl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);

    CHIP_ERROR ReadAcl(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadExtension(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteAcl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR WriteExtension(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
} sAttribute;

CHIP_ERROR LogExtensionChangedEvent(const AccessControlCluster::Structs::AccessControlExtensionStruct::Type & item,
                                    const Access::SubjectDescriptor & subjectDescriptor,
                                    AccessControlCluster::ChangeTypeEnum changeType)
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

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, 0, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "AccessControlCluster: log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR CheckExtensionEntryDataFormat(const ByteSpan & data)
{
    CHIP_ERROR err;

    TLV::TLVReader reader;
    reader.Init(data);

    auto containerType = chip::TLV::kTLVType_List;
    err                = reader.Next(containerType, chip::TLV::AnonymousTag());
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.EnterContainer(containerType);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(chip::TLV::IsProfileTag(reader.GetTag()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.ExitContainer(containerType);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    err = reader.Next();
    VerifyOrReturnError(err == CHIP_END_OF_TLV, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::ReadImpl(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case AccessControlCluster::Attributes::Acl::Id:
        return ReadAcl(aEncoder);
    case AccessControlCluster::Attributes::Extension::Id:
        return ReadExtension(aEncoder);
    case AccessControlCluster::Attributes::SubjectsPerAccessControlEntry::Id: {
        size_t value = 0;
        ReturnErrorOnFailure(GetAccessControl().GetMaxSubjectsPerEntry(value));
        return aEncoder.Encode(static_cast<uint16_t>(value));
    }
    case AccessControlCluster::Attributes::TargetsPerAccessControlEntry::Id: {
        size_t value = 0;
        ReturnErrorOnFailure(GetAccessControl().GetMaxTargetsPerEntry(value));
        return aEncoder.Encode(static_cast<uint16_t>(value));
    }
    case AccessControlCluster::Attributes::AccessControlEntriesPerFabric::Id: {
        size_t value = 0;
        ReturnErrorOnFailure(GetAccessControl().GetMaxEntriesPerFabric(value));
        return aEncoder.Encode(static_cast<uint16_t>(value));
    }
    case AccessControlCluster::Attributes::ClusterRevision::Id:
        return aEncoder.Encode(kClusterRevision);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::ReadAcl(AttributeValueEncoder & aEncoder)
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
            ReturnErrorCodeIf(err != CHIP_NO_ERROR && err != CHIP_ERROR_SENTINEL, err);
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AccessControlAttribute::ReadExtension(AttributeValueEncoder & aEncoder)
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
            ReturnErrorCodeIf(errStorage == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
            if (errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                continue;
            }
            ReturnErrorOnFailure(errStorage);
            AccessControlCluster::Structs::AccessControlExtensionStruct::Type item = {
                .data        = ByteSpan(buffer, size),
                .fabricIndex = fabric.GetFabricIndex(),
            };
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AccessControlAttribute::WriteImpl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case AccessControlCluster::Attributes::Acl::Id:
        return WriteAcl(aPath, aDecoder);
    case AccessControlCluster::Attributes::Extension::Id:
        return WriteExtension(aPath, aDecoder);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::WriteAcl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
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
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        VerifyOrReturnError((oldCount + 1) <= maxCount, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

        AclStorage::DecodableEntry decodableEntry;
        ReturnErrorOnFailure(aDecoder.Decode(decodableEntry));

        ReturnErrorOnFailure(GetAccessControl().CreateEntry(&aDecoder.GetSubjectDescriptor(), accessingFabricIndex, nullptr,
                                                            decodableEntry.GetEntry()));
    }
    else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::WriteExtension(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    auto & storage = Server::GetInstance().GetPersistentStorage();

    FabricIndex accessingFabricIndex = aDecoder.AccessingFabricIndex();

    uint8_t buffer[kExtensionDataMaxLength] = { 0 };
    uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
    CHIP_ERROR errStorage                   = storage.SyncGetKeyValue(
        DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(), buffer, size);
    ReturnErrorCodeIf(errStorage == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(errStorage != CHIP_NO_ERROR && errStorage != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, errStorage);

    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<AccessControlCluster::Structs::AccessControlExtensionStruct::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t count = 0;
        ReturnErrorOnFailure(list.ComputeSize(&count));

        if (count == 0)
        {
            ReturnErrorCodeIf(errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
            ReturnErrorOnFailure(storage.SyncDeleteKeyValue(
                DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName()));
            AccessControlCluster::Structs::AccessControlExtensionStruct::Type item = {
                .data        = ByteSpan(buffer, size),
                .fabricIndex = accessingFabricIndex,
            };
            ReturnErrorOnFailure(
                LogExtensionChangedEvent(item, aDecoder.GetSubjectDescriptor(), AccessControlCluster::ChangeTypeEnum::kRemoved));
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
            ReturnErrorCodeIf(item.data.size() > kExtensionDataMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

            ReturnErrorOnFailure(CheckExtensionEntryDataFormat(item.data));

            ReturnErrorOnFailure(
                storage.SyncSetKeyValue(DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(),
                                        item.data.data(), static_cast<uint16_t>(item.data.size())));
            ReturnErrorOnFailure(LogExtensionChangedEvent(item, aDecoder.GetSubjectDescriptor(),
                                                          errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND
                                                              ? AccessControlCluster::ChangeTypeEnum::kAdded
                                                              : AccessControlCluster::ChangeTypeEnum::kChanged));
        }
        else
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ReturnErrorCodeIf(errStorage != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        AccessControlCluster::Structs::AccessControlExtensionStruct::DecodableType item;
        ReturnErrorOnFailure(aDecoder.Decode(item));
        // TODO(#13590): generated code doesn't automatically handle max length so do it manually
        ReturnErrorCodeIf(item.data.size() > kExtensionDataMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        ReturnErrorOnFailure(CheckExtensionEntryDataFormat(item.data));

        ReturnErrorOnFailure(
            storage.SyncSetKeyValue(DefaultStorageKeyAllocator::AccessControlExtensionEntry(accessingFabricIndex).KeyName(),
                                    item.data.data(), static_cast<uint16_t>(item.data.size())));
        ReturnErrorOnFailure(
            LogExtensionChangedEvent(item, aDecoder.GetSubjectDescriptor(), AccessControlCluster::ChangeTypeEnum::kAdded));
    }
    else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return CHIP_NO_ERROR;
}

void AccessControlAttribute::OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                                            const Entry * entry, ChangeType changeType)
{
    // NOTE: If the entry was changed internally by the system (e.g. creating
    // entries at startup from persistent storage, or deleting entries when a
    // fabric is removed), then there won't be a subject descriptor, and also
    // it won't be appropriate to create an event.
    if (subjectDescriptor == nullptr)
    {
        return;
    }

    CHIP_ERROR err;
    AclEvent event{ .changeType = ChangeTypeEnum::kChanged, .fabricIndex = subjectDescriptor->fabricIndex };

    if (changeType == ChangeType::kAdded)
    {
        event.changeType = ChangeTypeEnum::kAdded;
    }
    else if (changeType == ChangeType::kRemoved)
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

    EventNumber eventNumber;

    if (entry != nullptr)
    {
        // NOTE: don't destroy encodable entry before staging entry is used!
        AclStorage::EncodableEntry encodableEntry(*entry);
        SuccessOrExit(err = encodableEntry.Stage());
        event.latestValue.SetNonNull(encodableEntry.GetStagingEntry());
        SuccessOrExit(err = LogEvent(event, 0, eventNumber));
    }
    else
    {
        SuccessOrExit(err = LogEvent(event, 0, eventNumber));
    }

    return;

exit:
    ChipLogError(DataManagement, "AccessControlCluster: event failed %" CHIP_ERROR_FORMAT, err.Format());
}

CHIP_ERROR ChipErrorToImErrorMap(CHIP_ERROR err)
{
    // Map some common errors into an underlying IM error
    // Separate logging is done to not lose the original error location in case such
    // this are available.
    CHIP_ERROR mappedError = err;

    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        mappedError = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    else if (err == CHIP_ERROR_NOT_FOUND)
    {
        // Not found is generally also illegal argument: caused a lookup into an invalid location,
        // like invalid subjects or targets.
        mappedError = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    else if (err == CHIP_ERROR_NO_MEMORY)
    {
        mappedError = CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (mappedError != err)
    {
        ChipLogError(DataManagement, "Re-mapped %" CHIP_ERROR_FORMAT " into %" CHIP_ERROR_FORMAT " for IM return codes",
                     err.Format(), mappedError.Format());
    }

    return mappedError;
}

CHIP_ERROR AccessControlAttribute::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // Note: We are not generating any errors under ReadImpl ourselves; it's
    // just the IM encoding machinery that does it.  And we should propagate
    // those errors through as-is, without mapping them to other errors, because
    // they are used to communicate various state within said enoding machinery.
    return ReadImpl(aPath, aEncoder);
}

CHIP_ERROR AccessControlAttribute::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    return ChipErrorToImErrorMap(WriteImpl(aPath, aDecoder));
}

} // namespace

void MatterAccessControlPluginServerInitCallback()
{
    ChipLogProgress(DataManagement, "AccessControlCluster: initializing");

    registerAttributeAccessOverride(&sAttribute);
    GetAccessControl().AddEntryListener(sAttribute);
}
