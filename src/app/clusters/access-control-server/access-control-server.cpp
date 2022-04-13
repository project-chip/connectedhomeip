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
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

namespace AccessControlCluster = chip::app::Clusters::AccessControl;

// TODO(#13590): generated code doesn't automatically handle max length so do it manually
constexpr int kExtensionDataMaxLength = 128;

namespace {

struct Subject
{
    NodeId nodeId;
    AccessControlCluster::AuthMode authMode;
};

struct AccessControlEntryCodec
{
    static CHIP_ERROR Convert(AuthMode from, AccessControlCluster::AuthMode & to)
    {
        switch (from)
        {
        case AuthMode::kPase:
            to = AccessControlCluster::AuthMode::kPase;
            break;
        case AuthMode::kCase:
            to = AccessControlCluster::AuthMode::kCase;
            break;
        case AuthMode::kGroup:
            to = AccessControlCluster::AuthMode::kGroup;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(AccessControlCluster::AuthMode from, AuthMode & to)
    {
        switch (from)
        {
        case AccessControlCluster::AuthMode::kPase:
            to = AuthMode::kPase;
            break;
        case AccessControlCluster::AuthMode::kCase:
            to = AuthMode::kCase;
            break;
        case AccessControlCluster::AuthMode::kGroup:
            to = AuthMode::kGroup;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(Privilege from, AccessControlCluster::Privilege & to)
    {
        switch (from)
        {
        case Privilege::kView:
            to = AccessControlCluster::Privilege::kView;
            break;
        case Privilege::kProxyView:
            to = AccessControlCluster::Privilege::kProxyView;
            break;
        case Privilege::kOperate:
            to = AccessControlCluster::Privilege::kOperate;
            break;
        case Privilege::kManage:
            to = AccessControlCluster::Privilege::kManage;
            break;
        case Privilege::kAdminister:
            to = AccessControlCluster::Privilege::kAdminister;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(AccessControlCluster::Privilege from, Privilege & to)
    {
        switch (from)
        {
        case AccessControlCluster::Privilege::kView:
            to = Privilege::kView;
            break;
        case AccessControlCluster::Privilege::kProxyView:
            to = Privilege::kProxyView;
            break;
        case AccessControlCluster::Privilege::kOperate:
            to = Privilege::kOperate;
            break;
        case AccessControlCluster::Privilege::kManage:
            to = Privilege::kManage;
            break;
        case AccessControlCluster::Privilege::kAdminister:
            to = Privilege::kAdminister;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(NodeId from, Subject & to)
    {
        if (IsOperationalNodeId(from) || IsCASEAuthTag(from))
        {
            to = { .nodeId = from, .authMode = AccessControlCluster::AuthMode::kCase };
        }
        else if (IsGroupId(from))
        {
            to = { .nodeId = GroupIdFromNodeId(from), .authMode = AccessControlCluster::AuthMode::kGroup };
        }
        else if (IsPAKEKeyId(from))
        {
            to = { .nodeId = PAKEKeyIdFromNodeId(from), .authMode = AccessControlCluster::AuthMode::kPase };
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(Subject from, NodeId & to)
    {
        switch (from.authMode)
        {
        case AccessControlCluster::AuthMode::kPase:
            ReturnErrorCodeIf(from.nodeId & ~kMaskPAKEKeyId, CHIP_ERROR_INVALID_ARGUMENT);
            to = NodeIdFromPAKEKeyId(static_cast<PasscodeId>(from.nodeId));
            break;
        case AccessControlCluster::AuthMode::kCase:
            to = from.nodeId;
            break;
        case AccessControlCluster::AuthMode::kGroup:
            ReturnErrorCodeIf(from.nodeId & ~kMaskGroupId, CHIP_ERROR_INVALID_ARGUMENT);
            to = NodeIdFromGroupId(static_cast<GroupId>(from.nodeId));
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(const AccessControl::Entry::Target & from, AccessControlCluster::Structs::Target::Type & to)
    {
        if (from.flags & AccessControl::Entry::Target::kCluster)
        {
            to.cluster.SetNonNull(from.cluster);
        }
        else
        {
            to.cluster.SetNull();
        }
        if (from.flags & AccessControl::Entry::Target::kEndpoint)
        {
            to.endpoint.SetNonNull(from.endpoint);
        }
        else
        {
            to.endpoint.SetNull();
        }
        if (from.flags & AccessControl::Entry::Target::kDeviceType)
        {
            to.deviceType.SetNonNull(from.deviceType);
        }
        else
        {
            to.deviceType.SetNull();
        }
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Convert(const AccessControlCluster::Structs::Target::Type & from, AccessControl::Entry::Target & to)
    {
        to.flags = 0;
        if (!from.cluster.IsNull())
        {
            to.flags |= AccessControl::Entry::Target::kCluster;
            to.cluster = from.cluster.Value();
        }
        if (!from.endpoint.IsNull())
        {
            to.flags |= AccessControl::Entry::Target::kEndpoint;
            to.endpoint = from.endpoint.Value();
        }
        if (!from.deviceType.IsNull())
        {
            to.flags |= AccessControl::Entry::Target::kDeviceType;
            to.deviceType = from.deviceType.Value();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EncodeForRead(TLV::TLVWriter & aWriter, TLV::Tag aTag, FabricIndex accessingFabricIndex) const
    {
        AccessControlCluster::Structs::AccessControlEntry::Type staging;

        ReturnErrorOnFailure(entry.GetFabricIndex(staging.fabricIndex));

        {
            Privilege privilege;
            ReturnErrorOnFailure(entry.GetPrivilege(privilege));
            ReturnErrorOnFailure(Convert(privilege, staging.privilege));
        }

        {
            AuthMode authMode;
            ReturnErrorOnFailure(entry.GetAuthMode(authMode));
            ReturnErrorOnFailure(Convert(authMode, staging.authMode));
        }

        NodeId subjectBuffer[CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY];
        size_t subjectCount;
        ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
        if (subjectCount > 0)
        {
            for (size_t i = 0; i < subjectCount; ++i)
            {
                NodeId subject;
                ReturnErrorOnFailure(entry.GetSubject(i, subject));
                Subject tmp;
                ReturnErrorOnFailure(AccessControlEntryCodec::Convert(subject, tmp));
                subjectBuffer[i] = tmp.nodeId;
            }
            staging.subjects.SetNonNull(subjectBuffer, subjectCount);
        }

        AccessControlCluster::Structs::Target::Type targetBuffer[CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY];
        size_t targetCount;
        ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
        if (targetCount > 0)
        {
            for (size_t i = 0; i < targetCount; ++i)
            {
                AccessControl::Entry::Target target;
                ReturnErrorOnFailure(entry.GetTarget(i, target));
                ReturnErrorOnFailure(Convert(target, targetBuffer[i]));
            }
            staging.targets.SetNonNull(targetBuffer, targetCount);
        }

        return staging.EncodeForRead(aWriter, aTag, accessingFabricIndex);
    }

    CHIP_ERROR Decode(TLV::TLVReader & aReader)
    {
        AccessControlCluster::Structs::AccessControlEntry::DecodableType staging;

        ReturnErrorOnFailure(staging.Decode(aReader));

        ReturnErrorOnFailure(GetAccessControl().PrepareEntry(entry));

        ReturnErrorOnFailure(entry.SetFabricIndex(staging.fabricIndex));

        {
            Privilege privilege;
            ReturnErrorOnFailure(Convert(staging.privilege, privilege));
            ReturnErrorOnFailure(entry.SetPrivilege(privilege));
        }

        {
            AuthMode authMode;
            ReturnErrorOnFailure(Convert(staging.authMode, authMode));
            ReturnErrorOnFailure(entry.SetAuthMode(authMode));
        }

        if (!staging.subjects.IsNull())
        {
            auto iterator = staging.subjects.Value().begin();
            while (iterator.Next())
            {
                Subject tmp = { .nodeId = iterator.GetValue(), .authMode = staging.authMode };
                NodeId subject;
                ReturnErrorOnFailure(Convert(tmp, subject));
                ReturnErrorOnFailure(entry.AddSubject(nullptr, subject));
            }
            ReturnErrorOnFailure(iterator.GetStatus());
        }

        if (!staging.targets.IsNull())
        {
            auto iterator = staging.targets.Value().begin();
            while (iterator.Next())
            {
                AccessControl::Entry::Target target;
                ReturnErrorOnFailure(Convert(iterator.GetValue(), target));
                ReturnErrorOnFailure(entry.AddTarget(nullptr, target));
            }
            ReturnErrorOnFailure(iterator.GetStatus());
        }

        return CHIP_NO_ERROR;
    }

    static constexpr bool kIsFabricScoped = true;

    auto GetFabricIndex() const
    {
        FabricIndex fabricIndex = kUndefinedFabricIndex;
        // Ignore the error value
        entry.GetFabricIndex(fabricIndex);
        return fabricIndex;
    }

    void SetFabricIndex(FabricIndex fabricIndex) { entry.SetFabricIndex(fabricIndex); }

    AccessControl::Entry entry;
};

class AccessControlAttribute : public chip::app::AttributeAccessInterface
{
public:
    AccessControlAttribute() : AttributeAccessInterface(Optional<EndpointId>(0), AccessControlCluster::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    static constexpr uint16_t ClusterRevision = 1;

private:
    CHIP_ERROR ReadAcl(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadExtension(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteAcl(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR WriteExtension(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
};

constexpr uint16_t AccessControlAttribute::ClusterRevision;

CHIP_ERROR LogAclChangedEvent(const AccessControl::Entry & entry, const Access::SubjectDescriptor & subjectDescriptor,
                              AccessControlCluster::ChangeTypeEnum changeType)
{
    CHIP_ERROR err;

    // Record AccessControlEntry event
    DataModel::Nullable<chip::NodeId> adminNodeID;
    DataModel::Nullable<uint16_t> adminPasscodeID;
    DataModel::Nullable<AccessControlCluster::Structs::AccessControlEntry::Type> latestValue;

    // Populate AccessControlEntryStruct from AccessControl entry.
    AccessControlCluster::Structs::AccessControlEntry::Type staging;

    ReturnErrorOnFailure(entry.GetFabricIndex(staging.fabricIndex));

    {
        Privilege privilege;
        ReturnErrorOnFailure(entry.GetPrivilege(privilege));
        ReturnErrorOnFailure(AccessControlEntryCodec::Convert(privilege, staging.privilege));
    }

    {
        AuthMode authMode;
        ReturnErrorOnFailure(entry.GetAuthMode(authMode));
        ReturnErrorOnFailure(AccessControlEntryCodec::Convert(authMode, staging.authMode));
    }

    NodeId subjectBuffer[CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY];
    size_t subjectCount;
    ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
    if (subjectCount > 0)
    {
        for (size_t i = 0; i < subjectCount; ++i)
        {
            NodeId subject;
            ReturnErrorOnFailure(entry.GetSubject(i, subject));
            Subject tmp;
            ReturnErrorOnFailure(AccessControlEntryCodec::Convert(subject, tmp));
            subjectBuffer[i] = tmp.nodeId;
        }
        staging.subjects.SetNonNull(subjectBuffer, subjectCount);
    }

    AccessControlCluster::Structs::Target::Type targetBuffer[CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY];
    size_t targetCount;
    ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
    if (targetCount > 0)
    {
        for (size_t i = 0; i < targetCount; ++i)
        {
            AccessControl::Entry::Target target;
            ReturnErrorOnFailure(entry.GetTarget(i, target));
            ReturnErrorOnFailure(AccessControlEntryCodec::Convert(target, targetBuffer[i]));
        }
        staging.targets.SetNonNull(targetBuffer, targetCount);
    }

    latestValue.SetNonNull(staging);

    if (subjectDescriptor.authMode == Access::AuthMode::kCase)
    {
        adminNodeID.SetNonNull(subjectDescriptor.subject);
    }
    else if (subjectDescriptor.authMode == Access::AuthMode::kPase)
    {
        adminPasscodeID.SetNonNull(PAKEKeyIdFromNodeId(subjectDescriptor.subject));
    }

    AccessControlCluster::Events::AccessControlEntryChanged::Type event{ adminNodeID, adminPasscodeID, changeType, latestValue,
                                                                         subjectDescriptor.fabricIndex };

    EventNumber eventNumber;
    err = LogEvent(event, 0, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "AccessControlCluster: log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR LogExtensionChangedEvent(const AccessControlCluster::Structs::ExtensionEntry::Type & item,
                                    const Access::SubjectDescriptor & subjectDescriptor,
                                    AccessControlCluster::ChangeTypeEnum changeType)
{
    AccessControlCluster::Events::AccessControlExtensionChanged::Type event{ .changeType       = changeType,
                                                                             .adminFabricIndex = subjectDescriptor.fabricIndex };

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
    case AccessControlCluster::Attributes::ClusterRevision::Id:
        return aEncoder.Encode(ClusterRevision);
    case AccessControlCluster::Attributes::Extension::Id:
        return ReadExtension(aEncoder);
    // TODO: For the following 3 attributes, need to add API surface to AccessControl to runtime get value from actual impl used.
    case AccessControlCluster::Attributes::SubjectsPerAccessControlEntry::Id: {
        uint16_t value = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY;
        return aEncoder.Encode(value);
    }
    case AccessControlCluster::Attributes::TargetsPerAccessControlEntry::Id: {
        uint16_t value = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY;
        return aEncoder.Encode(value);
    }
    case AccessControlCluster::Attributes::AccessControlEntriesPerFabric::Id: {
        uint16_t value = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC;
        return aEncoder.Encode(value);
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::ReadAcl(AttributeValueEncoder & aEncoder)
{
    AccessControlEntryCodec codec;
    AccessControl::EntryIterator iterator;

    ReturnErrorOnFailure(GetAccessControl().Entries(iterator));

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR err;
        while ((err = iterator.Next(codec.entry)) == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(encoder.Encode(codec));
        }
        ReturnErrorCodeIf(err != CHIP_NO_ERROR && err != CHIP_ERROR_SENTINEL, err);
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

    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<AccessControlEntryCodec> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t oldCount = 0;
        size_t allCount;
        size_t newCount;
        size_t maxCount;

        {
            AccessControl::EntryIterator it;
            AccessControl::Entry entry;
            ReturnErrorOnFailure(GetAccessControl().Entries(it, &accessingFabricIndex));
            while (it.Next(entry) == CHIP_NO_ERROR)
            {
                oldCount++;
            }
        }

        ReturnErrorOnFailure(GetAccessControl().GetEntryCount(allCount));
        ReturnErrorOnFailure(list.ComputeSize(&newCount));
        ReturnErrorOnFailure(GetAccessControl().GetMaxEntryCount(maxCount));
        VerifyOrReturnError(allCount >= oldCount, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(static_cast<size_t>(allCount - oldCount + newCount) <= maxCount,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));

        auto iterator = list.begin();
        size_t i      = 0;
        while (iterator.Next())
        {
            if (i < oldCount)
            {
                ReturnErrorOnFailure(GetAccessControl().UpdateEntry(i, iterator.GetValue().entry, &accessingFabricIndex));
                ReturnErrorOnFailure(LogAclChangedEvent(iterator.GetValue().entry, aDecoder.GetSubjectDescriptor(),
                                                        AccessControlCluster::ChangeTypeEnum::kChanged));
            }
            else
            {
                ReturnErrorOnFailure(GetAccessControl().CreateEntry(nullptr, iterator.GetValue().entry, &accessingFabricIndex));
                ReturnErrorOnFailure(LogAclChangedEvent(iterator.GetValue().entry, aDecoder.GetSubjectDescriptor(),
                                                        AccessControlCluster::ChangeTypeEnum::kAdded));
            }
            ++i;
        }
        ReturnErrorOnFailure(iterator.GetStatus());

        while (i < oldCount)
        {
            AccessControl::Entry entry;

            --oldCount;
            ReturnErrorOnFailure(GetAccessControl().ReadEntry(oldCount, entry, &accessingFabricIndex));
            ReturnErrorOnFailure(
                LogAclChangedEvent(entry, aDecoder.GetSubjectDescriptor(), AccessControlCluster::ChangeTypeEnum::kRemoved));
            ReturnErrorOnFailure(GetAccessControl().DeleteEntry(oldCount, &accessingFabricIndex));
        }
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        AccessControlEntryCodec item;
        ReturnErrorOnFailure(aDecoder.Decode(item));

        ReturnErrorOnFailure(GetAccessControl().CreateEntry(nullptr, item.entry, &accessingFabricIndex));
        ReturnErrorOnFailure(
            LogAclChangedEvent(item.entry, aDecoder.GetSubjectDescriptor(), AccessControlCluster::ChangeTypeEnum::kAdded));
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

AccessControlAttribute gAttribute;

class : public FabricTableDelegate
{
public:
    void OnFabricDeletedFromStorage(CompressedFabricId compressedId, FabricIndex fabricIndex) override
    {
        auto & storage = Server::GetInstance().GetPersistentStorage();
        DefaultStorageKeyAllocator key;
        storage.SyncDeleteKeyValue(key.AccessControlExtensionEntry(fabricIndex));
    }
    void OnFabricRetrievedFromStorage(FabricInfo * fabricInfo) override {}
    void OnFabricPersistedToStorage(FabricInfo * fabricInfo) override {}

} fabricTableDelegate;

} // namespace

void MatterAccessControlPluginServerInitCallback()
{
    ChipLogProgress(DataManagement, "AccessControlCluster: initializing");

    registerAttributeAccessOverride(&gAttribute);

    Server::GetInstance().GetFabricTable().AddFabricDelegate(&fabricTableDelegate);
}
