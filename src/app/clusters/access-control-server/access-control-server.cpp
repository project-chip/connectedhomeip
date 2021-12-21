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
#include <access/AccessControlEntryCodec.h>
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

struct Constants : public AccessControlEntryCodecConstants
{
    Constants()
    {
        encodedAuthPase            = static_cast<uint32_t>(AccessControlCluster::AuthMode::kPase);
        encodedAuthCase            = static_cast<uint32_t>(AccessControlCluster::AuthMode::kCase);
        encodedAuthGroup           = static_cast<uint32_t>(AccessControlCluster::AuthMode::kGroup);
        encodedPrivilegeView       = static_cast<uint32_t>(AccessControlCluster::Privilege::kView);
        encodedPrivilegeProxyView  = static_cast<uint32_t>(AccessControlCluster::Privilege::kProxyView);
        encodedPrivilegeOperate    = static_cast<uint32_t>(AccessControlCluster::Privilege::kOperate);
        encodedPrivilegeManage     = static_cast<uint32_t>(AccessControlCluster::Privilege::kManage);
        encodedPrivilegeAdminister = static_cast<uint32_t>(AccessControlCluster::Privilege::kAdminister);
        fabricIndexTag    = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::AccessControlEntry::Fields::kFabricIndex));
        privilegeTag      = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::AccessControlEntry::Fields::kPrivilege));
        authModeTag       = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::AccessControlEntry::Fields::kAuthMode));
        subjectsTag       = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::AccessControlEntry::Fields::kSubjects));
        targetsTag        = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::AccessControlEntry::Fields::kTargets));
        targetClusterTag  = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::Target::Fields::kCluster));
        targetEndpointTag = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::Target::Fields::kEndpoint));
        targetDeviceTypeTag = TLV::ContextTag(to_underlying(AccessControlCluster::Structs::Target::Fields::kDeviceType));
    }
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
    AccessControlEntryCodec<Constants> codec;
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
    DataModel::DecodableList<AccessControlEntryCodec<Constants>> list;
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
