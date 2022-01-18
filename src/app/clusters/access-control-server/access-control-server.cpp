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
#include <access/examples/ExampleAccessControlDelegate.h>

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

namespace AccessControlCluster = chip::app::Clusters::AccessControl;

namespace {

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

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
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
                ReturnErrorOnFailure(entry.GetSubject(i, subjectBuffer[i]));
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

        return staging.Encode(aWriter, aTag);
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
                ReturnErrorOnFailure(entry.AddSubject(nullptr, iterator.GetValue()));
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
    CHIP_ERROR WriteAcl(AttributeValueDecoder & aDecoder);
    CHIP_ERROR WriteExtension(AttributeValueDecoder & aDecoder);
};

constexpr uint16_t AccessControlAttribute::ClusterRevision;

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
    return aEncoder.EncodeEmptyList();
}

CHIP_ERROR AccessControlAttribute::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case AccessControlCluster::Attributes::Acl::Id:
        return WriteAcl(aDecoder);
    case AccessControlCluster::Attributes::Extension::Id:
        return WriteExtension(aDecoder);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::WriteAcl(AttributeValueDecoder & aDecoder)
{
    DataModel::DecodableList<AccessControlEntryCodec> list;
    ReturnErrorOnFailure(aDecoder.Decode(list));

    size_t oldCount;
    size_t newCount;
    size_t maxCount;
    ReturnErrorOnFailure(GetAccessControl().GetEntryCount(oldCount));
    ReturnErrorOnFailure(list.ComputeSize(&newCount));
    ReturnErrorOnFailure(GetAccessControl().GetMaxEntryCount(maxCount));
    ReturnErrorCodeIf(newCount > maxCount, CHIP_ERROR_INVALID_LIST_LENGTH);

    auto iterator = list.begin();
    size_t i      = 0;
    while (iterator.Next())
    {
        if (i < oldCount)
        {
            ReturnErrorOnFailure(GetAccessControl().UpdateEntry(i, iterator.GetValue().entry));
        }
        else
        {
            ReturnErrorOnFailure(GetAccessControl().CreateEntry(nullptr, iterator.GetValue().entry));
        }
        ++i;
    }
    ReturnErrorOnFailure(iterator.GetStatus());

    while (i < oldCount)
    {
        --oldCount;
        ReturnErrorOnFailure(GetAccessControl().DeleteEntry(oldCount));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::WriteExtension(AttributeValueDecoder & aDecoder)
{
    DataModel::DecodableList<AccessControlCluster::Structs::ExtensionEntry::DecodableType> list;
    ReturnErrorOnFailure(aDecoder.Decode(list));
    return CHIP_NO_ERROR;
}

AccessControlAttribute gAttribute;

AccessControl gAccessControl(Examples::GetAccessControlDelegate());

} // namespace

void MatterAccessControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);

    // TODO: move access control setup to lower level
    //       (it's OK and convenient here during development)
    gAccessControl.Init();
    SetAccessControl(gAccessControl);
}
