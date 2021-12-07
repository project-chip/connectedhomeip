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

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
    {
        TLV::TLVType accessControlEntryContainer;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, accessControlEntryContainer));
        using Fields = AccessControlCluster::Structs::AccessControlEntry::Fields;
        {
            FabricIndex fabricIndex;
            ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
            ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(Fields::kFabricIndex)), fabricIndex));
        }
        {
            Privilege privilege;
            ReturnErrorOnFailure(entry.GetPrivilege(privilege));
            AccessControlCluster::Privilege privilegeTemp;
            ReturnErrorOnFailure(Convert(privilege, privilegeTemp));
            ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(Fields::kPrivilege)), privilegeTemp));
        }
        {
            AuthMode authMode;
            ReturnErrorOnFailure(entry.GetAuthMode(authMode));
            AccessControlCluster::AuthMode authModeTemp;
            ReturnErrorOnFailure(Convert(authMode, authModeTemp));
            ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(Fields::kAuthMode)), authModeTemp));
        }
        {
            size_t count = 0;
            ReturnErrorOnFailure(entry.GetSubjectCount(count));
            if (count > 0)
            {
                TLV::TLVType subjectsContainer;
                ReturnErrorOnFailure(aWriter.StartContainer(TLV::ContextTag(to_underlying(Fields::kSubjects)), TLV::kTLVType_Array,
                                                            subjectsContainer));
                for (size_t i = 0; i < count; ++i)
                {
                    NodeId subject;
                    ReturnErrorOnFailure(entry.GetSubject(i, subject));
                    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::AnonymousTag, subject));
                }
                ReturnErrorOnFailure(aWriter.EndContainer(subjectsContainer));
            }
        }
        {
            size_t count = 0;
            ReturnErrorOnFailure(entry.GetTargetCount(count));
            if (count > 0)
            {
                TLV::TLVType targetsContainer;
                ReturnErrorOnFailure(aWriter.StartContainer(TLV::ContextTag(to_underlying(Fields::kTargets)), TLV::kTLVType_Array,
                                                            targetsContainer));
                using TargetFields = AccessControlCluster::Structs::Target::Fields;
                for (size_t i = 0; i < count; ++i)
                {
                    TLV::TLVType targetContainer;
                    ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, targetContainer));
                    AccessControl::Entry::Target target;
                    ReturnErrorOnFailure(entry.GetTarget(i, target));
                    if (target.flags & AccessControl::Entry::Target::kCluster)
                    {
                        ReturnErrorOnFailure(
                            DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(TargetFields::kCluster)), target.cluster));
                    }
                    if (target.flags & AccessControl::Entry::Target::kEndpoint)
                    {
                        ReturnErrorOnFailure(
                            DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(TargetFields::kEndpoint)), target.endpoint));
                    }
                    if (target.flags & AccessControl::Entry::Target::kDeviceType)
                    {
                        ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(to_underlying(TargetFields::kDeviceType)),
                                                               target.deviceType));
                    }
                    ReturnErrorOnFailure(aWriter.EndContainer(targetContainer));
                }
                ReturnErrorOnFailure(aWriter.EndContainer(targetsContainer));
            }
        }
        ReturnErrorOnFailure(aWriter.EndContainer(accessControlEntryContainer));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Decode(TLV::TLVReader & aReader)
    {
        ReturnErrorOnFailure(GetAccessControl().PrepareEntry(entry));
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLV::TLVType accessControlEntryContainer;
        VerifyOrReturnError(TLV::kTLVType_Structure == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
        ReturnErrorOnFailure(aReader.EnterContainer(accessControlEntryContainer));
        using Fields = AccessControlCluster::Structs::AccessControlEntry::Fields;
        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(TLV::IsContextTag(aReader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
            switch (TLV::TagNumFromTag(aReader.GetTag()))
            {
            case to_underlying(Fields::kFabricIndex): {
                chip::FabricIndex fabricIndex;
                ReturnErrorOnFailure(DataModel::Decode(aReader, fabricIndex));
                ReturnErrorOnFailure(entry.SetFabricIndex(fabricIndex));
                break;
            }
            case to_underlying(Fields::kPrivilege): {
                AccessControlCluster::Privilege privilegeTemp;
                ReturnErrorOnFailure(DataModel::Decode(aReader, privilegeTemp));
                Privilege privilege;
                ReturnErrorOnFailure(Convert(privilegeTemp, privilege));
                ReturnErrorOnFailure(entry.SetPrivilege(privilege));
                break;
            }
            case to_underlying(Fields::kAuthMode): {
                AccessControlCluster::AuthMode authModeTemp;
                ReturnErrorOnFailure(DataModel::Decode(aReader, authModeTemp));
                AuthMode authMode;
                ReturnErrorOnFailure(Convert(authModeTemp, authMode));
                ReturnErrorOnFailure(entry.SetAuthMode(authMode));
                break;
            }
            case to_underlying(Fields::kSubjects): {
                TLV::TLVType subjectsContainer;
                VerifyOrReturnError(TLV::kTLVType_Array == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                ReturnErrorOnFailure(aReader.EnterContainer(subjectsContainer));
                while ((err = aReader.Next()) == CHIP_NO_ERROR)
                {
                    NodeId subject = kUndefinedNodeId;
                    ReturnErrorOnFailure(DataModel::Decode(aReader, subject));
                    ReturnErrorOnFailure(entry.AddSubject(nullptr, subject));
                }
                VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
                ReturnErrorOnFailure(aReader.ExitContainer(subjectsContainer));
                break;
            }
            case to_underlying(Fields::kTargets): {
                TLV::TLVType targetsContainer;
                VerifyOrReturnError(TLV::kTLVType_Array == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                ReturnErrorOnFailure(aReader.EnterContainer(targetsContainer));
                while ((err = aReader.Next()) == CHIP_NO_ERROR)
                {
                    AccessControl::Entry::Target target;
                    TLV::TLVType targetContainer;
                    VerifyOrReturnError(TLV::kTLVType_Structure == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                    ReturnErrorOnFailure(aReader.EnterContainer(targetContainer));
                    using TargetFields = AccessControlCluster::Structs::Target::Fields;
                    while ((err = aReader.Next()) == CHIP_NO_ERROR)
                    {
                        VerifyOrReturnError(TLV::IsContextTag(aReader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
                        switch (TLV::TagNumFromTag(aReader.GetTag()))
                        {
                        case to_underlying(TargetFields::kCluster):
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(DataModel::Decode(aReader, target.cluster));
                                target.flags |= target.kCluster;
                            }
                            break;
                        case to_underlying(TargetFields::kEndpoint):
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(DataModel::Decode(aReader, target.endpoint));
                                target.flags |= target.kEndpoint;
                            }
                            break;
                        case to_underlying(TargetFields::kDeviceType):
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(DataModel::Decode(aReader, target.deviceType));
                                target.flags |= target.kDeviceType;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
                    ReturnErrorOnFailure(aReader.ExitContainer(targetContainer));
                    ReturnErrorOnFailure(entry.AddTarget(nullptr, target));
                }
                VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
                ReturnErrorOnFailure(aReader.ExitContainer(targetsContainer));
                break;
            }
            default:
                break;
            }
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(aReader.ExitContainer(accessControlEntryContainer));
        return CHIP_NO_ERROR;
    }

    AccessControl::Entry entry;
};

class AccessControlAttribute : public chip::app::AttributeAccessInterface
{
public:
    AccessControlAttribute() : AttributeAccessInterface(Optional<EndpointId>(0), Clusters::AccessControl::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadAcl(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadExtension(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteAcl(AttributeValueDecoder & aDecoder);
    CHIP_ERROR WriteExtension(AttributeValueDecoder & aDecoder);
};

CHIP_ERROR AccessControlAttribute::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Clusters::AccessControl::Attributes::Acl::Id:
        return ReadAcl(aEncoder);
    case Clusters::AccessControl::Attributes::Extension::Id:
        return ReadExtension(aEncoder);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::ReadAcl(AttributeValueEncoder & aEncoder)
{
    AccessControlEntryCodec codec;
    AccessControl::EntryIterator iterator;

    GetAccessControl().Entries(iterator);

    CHIP_ERROR err = aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        while (iterator.Next(codec.entry) == CHIP_NO_ERROR)
        {
            encoder.Encode(codec);
        }
        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR AccessControlAttribute::ReadExtension(AttributeValueEncoder & aEncoder)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControlAttribute::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Clusters::AccessControl::Attributes::Acl::Id:
        return WriteAcl(aDecoder);
    case Clusters::AccessControl::Attributes::Extension::Id:
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
    return CHIP_NO_ERROR;
}

AccessControlAttribute gAttribute;

AccessControl gAccessControl(Examples::GetAccessControlDelegate());

void MatterAccessControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);

    // TODO: move access control setup to lower level
    //       (it's OK and convenient here during development)
    gAccessControl.Init();
    SetAccessControl(gAccessControl);
}
