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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>

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

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

public:
    void OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index, const Entry * entry,
                        ChangeType changeType) override;

private:
    CHIP_ERROR ReadAcl(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadExtension(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteAcl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR WriteExtension(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
} sAttribute;

CHIP_ERROR LogExtensionChangedEvent(const AccessControlCluster::Structs::ExtensionEntry::Type & item,
                                    const Access::SubjectDescriptor & subjectDescriptor,
                                    AccessControlCluster::ChangeTypeEnum changeType)
{
    ExtensionEvent event{ .changeType = changeType, .adminFabricIndex = subjectDescriptor.fabricIndex };

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

CHIP_ERROR AccessControlAttribute::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
            CHIP_ERROR err;
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
    DefaultStorageKeyAllocator key;

    auto & fabrics = Server::GetInstance().GetFabricTable();

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & fabric : fabrics)
        {
            uint8_t buffer[kExtensionDataMaxLength] = { 0 };
            uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
            CHIP_ERROR errStorage = storage.SyncGetKeyValue(key.AccessControlExtensionEntry(fabric.GetFabricIndex()), buffer, size);
            ReturnErrorCodeIf(errStorage == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
            if (errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                continue;
            }
            ReturnErrorOnFailure(errStorage);
            AccessControlCluster::Structs::ExtensionEntry::Type item = {
                .data        = ByteSpan(buffer, size),
                .fabricIndex = fabric.GetFabricIndex(),
            };
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AccessControlAttribute::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
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
    DefaultStorageKeyAllocator key;

    FabricIndex accessingFabricIndex = aDecoder.AccessingFabricIndex();

    uint8_t buffer[kExtensionDataMaxLength] = { 0 };
    uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
    CHIP_ERROR errStorage = storage.SyncGetKeyValue(key.AccessControlExtensionEntry(accessingFabricIndex), buffer, size);
    ReturnErrorCodeIf(errStorage == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(errStorage != CHIP_NO_ERROR && errStorage != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, errStorage);

    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<AccessControlCluster::Structs::ExtensionEntry::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t count = 0;
        ReturnErrorOnFailure(list.ComputeSize(&count));

        if (count == 0)
        {
            ReturnErrorCodeIf(errStorage == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
            ReturnErrorOnFailure(storage.SyncDeleteKeyValue(key.AccessControlExtensionEntry(accessingFabricIndex)));
            AccessControlCluster::Structs::ExtensionEntry::Type item = {
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
            ReturnErrorOnFailure(storage.SyncSetKeyValue(key.AccessControlExtensionEntry(accessingFabricIndex), item.data.data(),
                                                         static_cast<uint16_t>(item.data.size())));
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
        AccessControlCluster::Structs::ExtensionEntry::DecodableType item;
        ReturnErrorOnFailure(aDecoder.Decode(item));
        // TODO(#13590): generated code doesn't automatically handle max length so do it manually
        ReturnErrorCodeIf(item.data.size() > kExtensionDataMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));
        ReturnErrorOnFailure(storage.SyncSetKeyValue(key.AccessControlExtensionEntry(accessingFabricIndex), item.data.data(),
                                                     static_cast<uint16_t>(item.data.size())));
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
    AclEvent event{ .changeType = ChangeTypeEnum::kChanged, .adminFabricIndex = subjectDescriptor->fabricIndex };

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

} // namespace

void MatterAccessControlPluginServerInitCallback()
{
    ChipLogProgress(DataManagement, "AccessControlCluster: initializing");

    registerAttributeAccessOverride(&sAttribute);
    GetAccessControl().AddEntryListener(sAttribute);
}
