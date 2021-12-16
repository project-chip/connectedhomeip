/*
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
#pragma once

#include <access/AccessControl.h>
#include <lib/core/CHIPTLV.h>
#include <type_traits>

namespace chip {
namespace Access {

struct AccessControlEntryCodecConstants
{
    uint32_t encodedAuthPase;
    uint32_t encodedAuthCase;
    uint32_t encodedAuthGroup;
    uint32_t encodedPrivilegeView;
    uint32_t encodedPrivilegeProxyView;
    uint32_t encodedPrivilegeOperate;
    uint32_t encodedPrivilegeManage;
    uint32_t encodedPrivilegeAdminister;

    TLV::Tag fabricIndexTag;
    TLV::Tag privilegeTag;
    TLV::Tag authModeTag;
    TLV::Tag subjectsTag;
    TLV::Tag targetsTag;

    TLV::Tag targetClusterTag;
    TLV::Tag targetEndpointTag;
    TLV::Tag targetDeviceTypeTag;
};
static_assert(std::is_trivially_copyable<AccessControlEntryCodecConstants>::value, "Constants type must be trivially copyable");

template <typename T>
class AccessControlEntryCodec
{
public:
    static_assert(std::is_base_of<AccessControlEntryCodecConstants, T>::value,
                  "T must inherit from AccessControlEntryCodecConstants");

    AccessControlEntryCodec() : mConstants(T()) {}

    CHIP_ERROR Convert(AuthMode from, uint32_t & to) const
    {
        switch (from)
        {
        case AuthMode::kPase:
            to = mConstants.encodedAuthPase;
            break;
        case AuthMode::kCase:
            to = mConstants.encodedAuthCase;
            break;
        case AuthMode::kGroup:
            to = mConstants.encodedAuthGroup;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Convert(uint32_t from, AuthMode & to) const
    {
        if (from == mConstants.encodedAuthPase)
        {
            to = AuthMode::kPase;
        }
        else if (from == mConstants.encodedAuthCase)
        {
            to = AuthMode::kCase;
        }
        else if (from == mConstants.encodedAuthGroup)
        {
            to = AuthMode::kGroup;
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Convert(Privilege from, uint32_t & to) const
    {
        switch (from)
        {
        case Privilege::kView:
            to = mConstants.encodedPrivilegeView;
            break;
        case Privilege::kProxyView:
            to = mConstants.encodedPrivilegeProxyView;
            break;
        case Privilege::kOperate:
            to = mConstants.encodedPrivilegeOperate;
            break;
        case Privilege::kManage:
            to = mConstants.encodedPrivilegeManage;
            break;
        case Privilege::kAdminister:
            to = mConstants.encodedPrivilegeAdminister;
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Convert(uint32_t from, Privilege & to) const
    {
        if (from == mConstants.encodedPrivilegeView)
        {
            to = Privilege::kView;
        }
        else if (from == mConstants.encodedPrivilegeProxyView)
        {
            to = Privilege::kProxyView;
        }
        else if (from == mConstants.encodedPrivilegeOperate)
        {
            to = Privilege::kOperate;
        }
        else if (from == mConstants.encodedPrivilegeManage)
        {
            to = Privilege::kManage;
        }
        else if (from == mConstants.encodedPrivilegeAdminister)
        {
            to = Privilege::kAdminister;
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
    {
        TLV::TLVType accessControlEntryContainer;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, accessControlEntryContainer));
        {
            FabricIndex fabricIndex;
            ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
            ReturnErrorOnFailure(aWriter.Put(mConstants.fabricIndexTag, fabricIndex));
        }
        {
            Privilege privilege;
            ReturnErrorOnFailure(entry.GetPrivilege(privilege));
            uint32_t privilegeTemp;
            ReturnErrorOnFailure(Convert(privilege, privilegeTemp));
            ReturnErrorOnFailure(aWriter.Put(mConstants.privilegeTag, privilegeTemp));
        }
        {
            AuthMode authMode;
            ReturnErrorOnFailure(entry.GetAuthMode(authMode));
            uint32_t authModeTemp;
            ReturnErrorOnFailure(Convert(authMode, authModeTemp));
            ReturnErrorOnFailure(aWriter.Put(mConstants.authModeTag, authModeTemp));
        }
        {
            size_t count = 0;
            ReturnErrorOnFailure(entry.GetSubjectCount(count));
            if (count > 0)
            {
                TLV::TLVType subjectsContainer;
                ReturnErrorOnFailure(aWriter.StartContainer(mConstants.subjectsTag, TLV::kTLVType_Array, subjectsContainer));
                for (size_t i = 0; i < count; ++i)
                {
                    NodeId subject;
                    ReturnErrorOnFailure(entry.GetSubject(i, subject));
                    ReturnErrorOnFailure(aWriter.Put(TLV::AnonymousTag, subject));
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
                ReturnErrorOnFailure(aWriter.StartContainer(mConstants.targetsTag, TLV::kTLVType_Array, targetsContainer));
                for (size_t i = 0; i < count; ++i)
                {
                    TLV::TLVType targetContainer;
                    ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, targetContainer));
                    AccessControl::Entry::Target target;
                    ReturnErrorOnFailure(entry.GetTarget(i, target));
                    if (target.flags & AccessControl::Entry::Target::kCluster)
                    {
                        ReturnErrorOnFailure(aWriter.Put(mConstants.targetClusterTag, target.cluster));
                    }
                    if (target.flags & AccessControl::Entry::Target::kEndpoint)
                    {
                        ReturnErrorOnFailure(aWriter.Put(mConstants.targetEndpointTag, target.endpoint));
                    }
                    if (target.flags & AccessControl::Entry::Target::kDeviceType)
                    {
                        ReturnErrorOnFailure(aWriter.Put(mConstants.targetDeviceTypeTag, target.deviceType));
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
        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(TLV::IsContextTag(aReader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
            auto tag = aReader.GetTag();
            if (tag == mConstants.fabricIndexTag)
            {
                chip::FabricIndex fabricIndex;
                ReturnErrorOnFailure(aReader.Get(fabricIndex));
                ReturnErrorOnFailure(entry.SetFabricIndex(fabricIndex));
            }
            else if (tag == mConstants.privilegeTag)
            {
                uint32_t privilegeTemp;
                ReturnErrorOnFailure(aReader.Get(privilegeTemp));
                Privilege privilege;
                ReturnErrorOnFailure(Convert(privilegeTemp, privilege));
                ReturnErrorOnFailure(entry.SetPrivilege(privilege));
            }
            else if (tag == mConstants.authModeTag)
            {
                uint32_t authModeTemp;
                ReturnErrorOnFailure(aReader.Get(authModeTemp));
                AuthMode authMode;
                ReturnErrorOnFailure(Convert(authModeTemp, authMode));
                ReturnErrorOnFailure(entry.SetAuthMode(authMode));
            }
            else if (tag == mConstants.subjectsTag)
            {
                TLV::TLVType subjectsContainer;
                VerifyOrReturnError(TLV::kTLVType_Array == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                ReturnErrorOnFailure(aReader.EnterContainer(subjectsContainer));
                while ((err = aReader.Next()) == CHIP_NO_ERROR)
                {
                    NodeId subject = kUndefinedNodeId;
                    ReturnErrorOnFailure(aReader.Get(subject));
                    ReturnErrorOnFailure(entry.AddSubject(nullptr, subject));
                }
                VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
                ReturnErrorOnFailure(aReader.ExitContainer(subjectsContainer));
            }
            else if (tag == mConstants.targetsTag)
            {
                TLV::TLVType targetsContainer;
                VerifyOrReturnError(TLV::kTLVType_Array == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                ReturnErrorOnFailure(aReader.EnterContainer(targetsContainer));
                while ((err = aReader.Next()) == CHIP_NO_ERROR)
                {
                    AccessControl::Entry::Target target;
                    TLV::TLVType targetContainer;
                    VerifyOrReturnError(TLV::kTLVType_Structure == aReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
                    ReturnErrorOnFailure(aReader.EnterContainer(targetContainer));
                    while ((err = aReader.Next()) == CHIP_NO_ERROR)
                    {
                        VerifyOrReturnError(TLV::IsContextTag(aReader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
                        auto targetTag = TLV::TagNumFromTag(aReader.GetTag());
                        if (targetTag == mConstants.targetClusterTag)
                        {
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(aReader.Get(target.cluster));
                                target.flags |= target.kCluster;
                            }
                        }
                        else if (targetTag == mConstants.targetEndpointTag)
                        {
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(aReader.Get(target.endpoint));
                                target.flags |= target.kEndpoint;
                            }
                        }
                        else if (targetTag == mConstants.targetDeviceTypeTag)
                        {
                            if (aReader.GetType() != TLV::kTLVType_Null)
                            {
                                ReturnErrorOnFailure(aReader.Get(target.deviceType));
                                target.flags |= target.kDeviceType;
                            }
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
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(aReader.ExitContainer(accessControlEntryContainer));
        return CHIP_NO_ERROR;
    }

    AccessControl::Entry entry;

private:
    const AccessControlEntryCodecConstants mConstants;
};

} // namespace Access
} // namespace chip
