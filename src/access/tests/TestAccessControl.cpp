/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "access/AccessControl.h"
#include "access/examples/ExampleAccessControlDelegate.h"

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>

namespace chip {
namespace Access {

using namespace chip;
using namespace chip::Access;

using Entry         = AccessControl::Entry;
using EntryIterator = AccessControl::EntryIterator;
using Target        = Entry::Target;

AccessControl accessControl;

constexpr ClusterId kOnOffCluster         = 0x0000'0006;
constexpr ClusterId kLevelControlCluster  = 0x0000'0008;
constexpr ClusterId kAccessControlCluster = 0x0000'001F;
constexpr ClusterId kColorControlCluster  = 0x0000'0300;

constexpr NodeId kPaseVerifier0 = NodeIdFromPAKEKeyId(0x0000);
constexpr NodeId kPaseVerifier1 = NodeIdFromPAKEKeyId(0x0001);
constexpr NodeId kPaseVerifier3 = NodeIdFromPAKEKeyId(0x0003);

constexpr NodeId kOperationalNodeId0 = 0x0123456789ABCDEF;
constexpr NodeId kOperationalNodeId1 = 0x1234567812345678;
constexpr NodeId kOperationalNodeId2 = 0x1122334455667788;
constexpr NodeId kOperationalNodeId3 = 0x1111111111111111;
constexpr NodeId kOperationalNodeId4 = 0x2222222222222222;
constexpr NodeId kOperationalNodeId5 = 0x3333333333333333;

constexpr CASEAuthTag kCASEAuthTag0 = 0x0001'0001;
constexpr CASEAuthTag kCASEAuthTag1 = 0x0002'0001;
constexpr CASEAuthTag kCASEAuthTag2 = 0xABCD'0002;
constexpr CASEAuthTag kCASEAuthTag3 = 0xABCD'0008;
constexpr CASEAuthTag kCASEAuthTag4 = 0xABCD'ABCD;

constexpr NodeId kCASEAuthTagAsNodeId0 = NodeIdFromCASEAuthTag(kCASEAuthTag0);
constexpr NodeId kCASEAuthTagAsNodeId1 = NodeIdFromCASEAuthTag(kCASEAuthTag1);
constexpr NodeId kCASEAuthTagAsNodeId2 = NodeIdFromCASEAuthTag(kCASEAuthTag2);
constexpr NodeId kCASEAuthTagAsNodeId3 = NodeIdFromCASEAuthTag(kCASEAuthTag3);
constexpr NodeId kCASEAuthTagAsNodeId4 = NodeIdFromCASEAuthTag(kCASEAuthTag4);

constexpr NodeId kGroup2 = NodeIdFromGroupId(0x0002);
constexpr NodeId kGroup4 = NodeIdFromGroupId(0x0004);
constexpr NodeId kGroup6 = NodeIdFromGroupId(0x0006);
constexpr NodeId kGroup8 = NodeIdFromGroupId(0x0008);

constexpr AuthMode authModes[] = { AuthMode::kCase, AuthMode::kGroup };

constexpr FabricIndex fabricIndexes[] = { 1, 2, 3 };

constexpr Privilege privileges[] = { Privilege::kView, Privilege::kProxyView, Privilege::kOperate, Privilege::kManage,
                                     Privilege::kAdminister };

constexpr NodeId subjects[][3] = { {
                                       kOperationalNodeId0,
                                       kCASEAuthTagAsNodeId1,
                                       kCASEAuthTagAsNodeId2,
                                   },
                                   {
                                       kGroup4,
                                       kGroup6,
                                       kGroup8,
                                   } };

constexpr Target targets[] = {
    { .flags = Target::kCluster, .cluster = kOnOffCluster },
    { .flags = Target::kEndpoint, .endpoint = 3 },
    { .flags = Target::kCluster | Target::kEndpoint, .cluster = kLevelControlCluster, .endpoint = 5 },
};

constexpr FabricIndex invalidFabricIndexes[] = { kUndefinedFabricIndex };

// clang-format off
constexpr NodeId validCaseSubjects[] = {
    0x0000'0000'0000'0001, // min operational
    0x0000'0000'0000'0002,
    0x0123'4567'89AB'CDEF,
    0xFFFF'FFEF'FFFF'FFFE,
    0xFFFF'FFEF'FFFF'FFFF, // max operational

    NodeIdFromCASEAuthTag(0x0000'0001),
    NodeIdFromCASEAuthTag(0x0000'0002),
    NodeIdFromCASEAuthTag(0x0000'FFFE),
    NodeIdFromCASEAuthTag(0x0000'FFFF),

    NodeIdFromCASEAuthTag(0x0001'0001),
    NodeIdFromCASEAuthTag(0x0001'0002),
    NodeIdFromCASEAuthTag(0x0001'FFFE),
    NodeIdFromCASEAuthTag(0x0001'FFFF),

    NodeIdFromCASEAuthTag(0xFFFE'0001),
    NodeIdFromCASEAuthTag(0xFFFE'0002),
    NodeIdFromCASEAuthTag(0xFFFE'FFFE),
    NodeIdFromCASEAuthTag(0xFFFE'FFFF),

    NodeIdFromCASEAuthTag(0xFFFF'0001),
    NodeIdFromCASEAuthTag(0xFFFF'0002),
    NodeIdFromCASEAuthTag(0xFFFF'FFFE),
    NodeIdFromCASEAuthTag(0xFFFF'FFFF),
};
// clang-format on

// clang-format off
constexpr NodeId validGroupSubjects[] = {
    NodeIdFromGroupId(0x0001), // start of fabric-scoped
    NodeIdFromGroupId(0x0002),
    NodeIdFromGroupId(0x7FFE),
    NodeIdFromGroupId(0x7FFF), // end of fabric-scoped
    NodeIdFromGroupId(0x8000), // start of universal
    NodeIdFromGroupId(0x8001),
    NodeIdFromGroupId(0xFFFB),
    NodeIdFromGroupId(0xFFFC), // end of universal
    NodeIdFromGroupId(0xFFFD), // all proxies
    NodeIdFromGroupId(0xFFFE), // all non sleepy
    NodeIdFromGroupId(0xFFFF), // all nodes
};
// clang-format on

// clang-format off
constexpr NodeId validPaseSubjects[] = {
    NodeIdFromPAKEKeyId(0x0000), // start
    NodeIdFromPAKEKeyId(0x0001),
    NodeIdFromPAKEKeyId(0xFFFE),
    NodeIdFromPAKEKeyId(0xFFFF), // end
};
// clang-format on

// clang-format off
constexpr NodeId invalidSubjects[] = {
    0x0000'0000'0000'0000, // unspecified

    0xFFFF'FFF0'0000'0000, // start reserved
    0xFFFF'FFF0'0000'0001,
    0xFFFF'FFF0'FFFF'FFFE,
    0xFFFF'FFF0'FFFF'FFFF, // end reserved

    0xFFFF'FFF1'0000'0000, // start reserved
    0xFFFF'FFF1'0000'0001,
    0xFFFF'FFF1'FFFF'FFFE,
    0xFFFF'FFF1'FFFF'FFFF, // end reserved

    0xFFFF'FFF2'0000'0000, // start reserved
    0xFFFF'FFF2'0000'0001,
    0xFFFF'FFF2'FFFF'FFFE,
    0xFFFF'FFF2'FFFF'FFFF, // end reserved

    0xFFFF'FFF3'0000'0000, // start reserved
    0xFFFF'FFF3'0000'0001,
    0xFFFF'FFF3'FFFF'FFFE,
    0xFFFF'FFF3'FFFF'FFFF, // end reserved

    0xFFFF'FFF4'0000'0000, // start reserved
    0xFFFF'FFF4'0000'0001,
    0xFFFF'FFF4'FFFF'FFFE,
    0xFFFF'FFF4'FFFF'FFFF, // end reserved

    0xFFFF'FFF5'0000'0000, // start reserved
    0xFFFF'FFF5'0000'0001,
    0xFFFF'FFF5'FFFF'FFFE,
    0xFFFF'FFF5'FFFF'FFFF, // end reserved

    0xFFFF'FFF6'0000'0000, // start reserved
    0xFFFF'FFF6'0000'0001,
    0xFFFF'FFF6'FFFF'FFFE,
    0xFFFF'FFF6'FFFF'FFFF, // end reserved

    0xFFFF'FFF7'0000'0000, // start reserved
    0xFFFF'FFF7'0000'0001,
    0xFFFF'FFF7'FFFF'FFFE,
    0xFFFF'FFF7'FFFF'FFFF, // end reserved

    0xFFFF'FFF8'0000'0000, // start reserved
    0xFFFF'FFF8'0000'0001,
    0xFFFF'FFF8'FFFF'FFFE,
    0xFFFF'FFF8'FFFF'FFFF, // end reserved

    0xFFFF'FFF9'0000'0000, // start reserved
    0xFFFF'FFF9'0000'0001,
    0xFFFF'FFF9'FFFF'FFFE,
    0xFFFF'FFF9'FFFF'FFFF, // end reserved

    0xFFFF'FFFA'0000'0000, // start reserved
    0xFFFF'FFFA'0000'0001,
    0xFFFF'FFFA'FFFF'FFFE,
    0xFFFF'FFFA'FFFF'FFFF, // end reserved

    0xFFFF'FFFB'0001'0000, // PASE with unused bits used
    0xFFFF'FFFB'0001'0001, // PASE with unused bits used
    0xFFFF'FFFB'0001'FFFE, // PASE with unused bits used
    0xFFFF'FFFB'0001'FFFF, // PASE with unused bits used

    0xFFFF'FFFB'FFFE'0000, // PASE with unused bits used
    0xFFFF'FFFB'FFFE'0001, // PASE with unused bits used
    0xFFFF'FFFB'FFFE'FFFE, // PASE with unused bits used
    0xFFFF'FFFB'FFFE'FFFF, // PASE with unused bits used

    0xFFFF'FFFB'FFFF'0000, // PASE with unused bits used
    0xFFFF'FFFB'FFFF'0001, // PASE with unused bits used
    0xFFFF'FFFB'FFFF'FFFE, // PASE with unused bits used
    0xFFFF'FFFB'FFFF'FFFF, // PASE with unused bits used

    0xFFFF'FFFC'0000'0000, // start reserved
    0xFFFF'FFFC'0000'0001,
    0xFFFF'FFFC'FFFF'FFFE,
    0xFFFF'FFFC'FFFF'FFFF, // end reserved

    0xFFFF'FFFD'0000'0000, // CAT with version 0
    0xFFFF'FFFD'0001'0000, // CAT with version 0
    0xFFFF'FFFD'FFFE'0000, // CAT with version 0
    0xFFFF'FFFD'FFFF'0000, // CAT with version 0

    0xFFFF'FFFE'0000'0000, // start temporary local
    0xFFFF'FFFE'0000'0001,
    0xFFFF'FFFE'FFFF'FFFE,
    0xFFFF'FFFE'FFFF'FFFF, // end temporary local (used for placeholder)

    0xFFFF'FFFF'0000'0000, // start reserved
    0xFFFF'FFFF'0000'0001,
    0xFFFF'FFFF'FFFE'FFFE,
    0xFFFF'FFFF'FFFE'FFFF, // end reserved

    0xFFFF'FFFF'FFFF'0000, // group 0
};
// clang-format on

// clang-format off
constexpr ClusterId validClusters[] = {
    0x0000'0000, // start std
    0x0000'0001,
    0x0000'7FFE,
    0x0000'7FFF, // end std

    0x0001'FC00, // start MS
    0x0001'FC01,
    0x0001'FFFD,
    0x0001'FFFE, // end MS

    0xFFF1'FC00, // start MS
    0xFFF1'FC01,
    0xFFF1'FFFD,
    0xFFF1'FFFE, // end MS

    0xFFF4'FC00, // start MS
    0xFFF4'FC01,
    0xFFF4'FFFD,
    0xFFF4'FFFE, // end MS
};
// clang-format on

// clang-format off
constexpr ClusterId invalidClusters[] = {
    0x0000'8000, // start unused
    0x0000'8001,
    0x0000'FBFE,
    0x0000'FBFF, // end unused
    0x0000'FC00, // start MS
    0x0000'FC01,
    0x0000'FFFD,
    0x0000'FFFE, // end MS
    0x0000'FFFF, // wildcard

    0x0001'0000, // start std
    0x0001'0001,
    0x0001'7FFE,
    0x0001'7FFF, // end std
    0x0001'8000, // start unused
    0x0001'8001,
    0x0001'FBFE,
    0x0001'FBFF, // end unused
    0x0001'FFFF, // wildcard

    0xFFF4'0000, // start std
    0xFFF4'0001,
    0xFFF4'7FFE,
    0xFFF4'7FFF, // end std
    0xFFF4'8000, // start unused
    0xFFF4'8001,
    0xFFF4'FBFE,
    0xFFF4'FBFF, // end unused
    0xFFF4'FFFF, // wildcard

    0xFFFD'0000, // start std
    0xFFFD'0001,
    0xFFFD'7FFE,
    0xFFFD'7FFF, // end std
    0xFFFD'8000, // start unused
    0xFFFD'8001,
    0xFFFD'FBFE,
    0xFFFD'FBFF, // end unused
    0xFFFD'FC00, // start MS
    0xFFFD'FC01,
    0xFFFD'FFFD,
    0xFFFD'FFFE, // end MS
    0xFFFD'FFFF, // wildcard

    0xFFFE'0000, // start std
    0xFFFE'0001,
    0xFFFE'7FFE,
    0xFFFE'7FFF, // end std
    0xFFFE'8000, // start unused
    0xFFFE'8001,
    0xFFFE'FBFE,
    0xFFFE'FBFF, // end unused
    0xFFFE'FC00, // start MS
    0xFFFE'FC01,
    0xFFFE'FFFD,
    0xFFFE'FFFE, // end MS
    0xFFFE'FFFF, // wildcard

    0xFFFF'0000, // start std
    0xFFFF'0001,
    0xFFFF'7FFE,
    0xFFFF'7FFF, // end std
    0xFFFF'8000, // start unused
    0xFFFF'8001,
    0xFFFF'FBFE,
    0xFFFF'FBFF, // end unused
    0xFFFF'FC00, // start MS
    0xFFFF'FC01,
    0xFFFF'FFFD,
    0xFFFF'FFFE, // end MS
    0xFFFF'FFFF, // wildcard
};
// clang-format on

// clang-format off
constexpr EndpointId validEndpoints[] = {
    0x0000, // start
    0x0001,
    0xFFFD,
    0xFFFE, // end
};
// clang-format on

// clang-format off
constexpr EndpointId invalidEndpoints[] = {
    kInvalidEndpointId
};
// clang-format on

// clang-format off
constexpr DeviceTypeId validDeviceTypes[] = {
    0x0000'0000, // start
    0x0000'0001,
    0x0000'BFFE,
    0x0000'BFFF, // end

    0x0001'0000, // start
    0x0001'0001,
    0x0001'BFFE,
    0x0001'BFFF, // end

    0xFFFD'0000, // start
    0xFFFD'0001,
    0xFFFD'BFFE,
    0xFFFD'BFFF, // end

    0xFFFE'0000, // start
    0xFFFE'0001,
    0xFFFE'BFFE,
    0xFFFE'BFFF, // end
};
// clang-format on

// clang-format off
constexpr DeviceTypeId invalidDeviceTypes[] = {
    0x0000'C000, // start unused
    0x0000'C001,
    0x0000'FFFD,
    0x0000'FFFE, // end unused
    0x0000'FFFF, // wildcard

    0x0001'C000, // start unused
    0x0001'C001,
    0x0001'FFFD,
    0x0001'FFFE, // end unused
    0x0001'FFFF, // wildcard

    0xFFFE'C000, // start unused
    0xFFFE'C001,
    0xFFFE'FFFD,
    0xFFFE'FFFE, // end unused
    0xFFFE'FFFF, // wildcard

    0xFFFF'0000, // start used
    0xFFFF'0001,
    0xFFFF'BFFE,
    0xFFFF'BFFF, // end used
    0xFFFF'C000, // start unused
    0xFFFF'C001,
    0xFFFF'FFFD,
    0xFFFF'FFFE, // end unused
    0xFFFF'FFFF, // wildcard
};
// clang-format on

class DeviceTypeResolver : public AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return false; }
} testDeviceTypeResolver;

// For testing, supports one subject and target, allows any value (valid or invalid)
class TestEntryDelegate : public Entry::Delegate
{
public:
    void Release() override {}

    CHIP_ERROR GetAuthMode(AuthMode & authMode) const override
    {
        authMode = mAuthMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const override
    {
        fabricIndex = mFabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPrivilege(Privilege & privilege) const override
    {
        privilege = mPrivilege;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetAuthMode(AuthMode authMode) override
    {
        mAuthMode = authMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) override
    {
        mFabricIndex = fabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPrivilege(Privilege privilege) override
    {
        mPrivilege = privilege;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubjectCount(size_t & count) const override
    {
        count = mSubjectCount;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSubject(size_t index, NodeId & subject) const override
    {
        VerifyOrDie(index < mSubjectCount);
        subject = mSubject;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSubject(size_t index, NodeId subject) override
    {
        VerifyOrDie(index < mSubjectCount);
        mSubject = subject;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddSubject(size_t * index, NodeId subject) override
    {
        VerifyOrDie(mSubjectCount == 0);
        mSubjectCount = 1;
        mSubject      = subject;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveSubject(size_t index) override
    {
        VerifyOrDie(mSubjectCount == 1);
        mSubjectCount = 0;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetTargetCount(size_t & count) const override
    {
        count = mTargetCount;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetTarget(size_t index, Target & target) const override
    {
        VerifyOrDie(index < mTargetCount);
        target = mTarget;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetTarget(size_t index, const Target & target) override
    {
        VerifyOrDie(index < mTargetCount);
        mTarget = target;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddTarget(size_t * index, const Target & target) override
    {
        VerifyOrDie(mTargetCount == 0);
        mTargetCount = 1;
        mTarget      = target;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveTarget(size_t index) override
    {
        VerifyOrDie(mTargetCount == 1);
        mTargetCount = 0;
        return CHIP_NO_ERROR;
    }

    FabricIndex mFabricIndex = 1;
    Privilege mPrivilege     = Privilege::kView;
    AuthMode mAuthMode       = AuthMode::kCase;
    NodeId mSubject          = kOperationalNodeId0;
    Target mTarget           = { .flags = Target::kCluster, .cluster = kOnOffCluster };
    size_t mSubjectCount     = 1;
    size_t mTargetCount      = 1;
};

bool operator==(const Target & a, const Target & b)
{
    if (a.flags != b.flags)
        return false;
    if ((a.flags & Target::kCluster) && a.cluster != b.cluster)
        return false;
    if ((a.flags & Target::kEndpoint) && a.endpoint != b.endpoint)
        return false;
    if ((a.flags & Target::kDeviceType) && a.deviceType != b.deviceType)
        return false;
    return true;
}

bool operator!=(const Target & a, const Target & b)
{
    return !(a == b);
}

struct EntryData
{
    static constexpr int kMaxSubjects = 3;
    static constexpr int kMaxTargets  = 3;

    FabricIndex fabricIndex       = kUndefinedFabricIndex;
    Privilege privilege           = Privilege::kView;
    AuthMode authMode             = AuthMode::kNone;
    NodeId subjects[kMaxSubjects] = { 0 };
    Target targets[kMaxTargets]   = { { 0 } };

    void Clear() { *this = EntryData(); }

    bool IsEmpty() const { return authMode == AuthMode::kNone; }

    size_t GetSubjectCount() const
    {
        size_t count = 0;
        for (auto & subject : subjects)
        {
            if (subject == kUndefinedNodeId)
            {
                break;
            }
            count++;
        }
        return count;
    }

    void AddSubject(size_t * index, NodeId subject)
    {
        size_t count = GetSubjectCount();
        if (count < kMaxSubjects)
        {
            subjects[count] = subject;
            if (index)
            {
                *index = count;
            }
        }
    }

    void RemoveSubject(size_t index)
    {
        size_t count = GetSubjectCount();
        if (index < count)
        {
            while (++index < kMaxSubjects)
            {
                subjects[index - 1] = subjects[index];
            }
            subjects[kMaxSubjects - 1] = { 0 };
        }
    }

    size_t GetTargetCount() const
    {
        size_t count = 0;
        for (auto & target : targets)
        {
            if (target.flags == 0)
            {
                break;
            }
            count++;
        }
        return count;
    }

    void AddTarget(size_t * index, const Target & target)
    {
        size_t count = GetTargetCount();
        if (count < kMaxTargets)
        {
            targets[count] = target;
            if (index)
            {
                *index = count;
            }
        }
    }

    void RemoveTarget(size_t index)
    {
        size_t count = GetTargetCount();
        if (index < count)
        {
            while (++index < kMaxTargets)
            {
                targets[index - 1] = targets[index];
            }
            targets[kMaxTargets - 1] = { 0 };
        }
    }
};

CHIP_ERROR CompareEntry(const Entry & entry, const EntryData & entryData)
{
    AuthMode authMode = AuthMode::kNone;
    ReturnErrorOnFailure(entry.GetAuthMode(authMode));
    VerifyOrReturnError(authMode == entryData.authMode, CHIP_ERROR_INCORRECT_STATE);
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
    VerifyOrReturnError(fabricIndex == entryData.fabricIndex, CHIP_ERROR_INCORRECT_STATE);
    Privilege privilege = Privilege::kView;
    ReturnErrorOnFailure(entry.GetPrivilege(privilege));
    VerifyOrReturnError(privilege == entryData.privilege, CHIP_ERROR_INCORRECT_STATE);
    size_t subjectCount = 0;
    ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
    VerifyOrReturnError(subjectCount == entryData.GetSubjectCount(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject = kUndefinedNodeId;
        ReturnErrorOnFailure(entry.GetSubject(i, subject));
        VerifyOrReturnError(subject == entryData.subjects[i], CHIP_ERROR_INCORRECT_STATE);
    }
    size_t targetCount = 0;
    ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
    VerifyOrReturnError(targetCount == entryData.GetTargetCount(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t i = 0; i < targetCount; ++i)
    {
        Target target;
        ReturnErrorOnFailure(entry.GetTarget(i, target));
        VerifyOrReturnError(target == entryData.targets[i], CHIP_ERROR_INCORRECT_STATE);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoadEntry(Entry & entry, const EntryData & entryData)
{
    ReturnErrorOnFailure(entry.SetAuthMode(entryData.authMode));
    ReturnErrorOnFailure(entry.SetFabricIndex(entryData.fabricIndex));
    ReturnErrorOnFailure(entry.SetPrivilege(entryData.privilege));
    for (size_t i = 0; i < entryData.GetSubjectCount(); ++i)
    {
        ReturnErrorOnFailure(entry.AddSubject(nullptr, entryData.subjects[i]));
    }
    for (size_t i = 0; i < entryData.GetTargetCount(); ++i)
    {
        ReturnErrorOnFailure(entry.AddTarget(nullptr, entryData.targets[i]));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClearAccessControl(AccessControl & ac)
{
    CHIP_ERROR err;
    do
    {
        err = accessControl.DeleteEntry(0);
    } while (err == CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CompareAccessControl(AccessControl & ac, const EntryData * entryData, size_t count)
{
    Entry entry;
    for (size_t i = 0; i < count; ++i, ++entryData)
    {
        ReturnErrorOnFailure(ac.ReadEntry(i, entry));
        ReturnErrorOnFailure(CompareEntry(entry, *entryData));
    }
    VerifyOrReturnError(ac.ReadEntry(count, entry) != CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoadAccessControl(AccessControl & ac, const EntryData * entryData, size_t count)
{
    Entry entry;
    for (size_t i = 0; i < count; ++i, ++entryData)
    {
        ReturnErrorOnFailure(ac.PrepareEntry(entry));
        ReturnErrorOnFailure(LoadEntry(entry, *entryData));
        ReturnErrorOnFailure(ac.CreateEntry(nullptr, entry));
    }
    return CHIP_NO_ERROR;
}

constexpr size_t kNumFabric1EntriesInEntryData1 = 4;
constexpr size_t kNumFabric2EntriesInEntryData1 = 5;

constexpr EntryData entryData1[] = {
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subjects    = { kOperationalNodeId3 },
    },
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kView,
        .authMode    = AuthMode::kCase,
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subjects    = { kOperationalNodeId4 },
    },
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kOperate,
        .authMode    = AuthMode::kCase,
        .targets     = { { .flags = Target::kCluster, .cluster = kOnOffCluster } },
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kManage,
        .authMode    = AuthMode::kCase,
        .subjects    = { kOperationalNodeId5 },
        .targets     = { { .flags = Target::kCluster | Target::kEndpoint, .cluster = kOnOffCluster, .endpoint = 2 } },
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kProxyView,
        .authMode    = AuthMode::kGroup,
        .subjects    = { kGroup2 },
        .targets     = { { .flags = Target::kCluster | Target::kEndpoint, .cluster = kLevelControlCluster, .endpoint = 1 },
                         { .flags = Target::kCluster, .cluster = kOnOffCluster },
                         { .flags = Target::kEndpoint, .endpoint = 2 } },
    },
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subjects    = { kCASEAuthTagAsNodeId0 },
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kManage,
        .authMode    = AuthMode::kCase,
        .subjects    = { kCASEAuthTagAsNodeId3, kCASEAuthTagAsNodeId1 },
        .targets     = { { .flags = Target::kCluster, .cluster = kOnOffCluster } },
    },
    {
        .fabricIndex = 2,
        .privilege   = Privilege::kOperate,
        .authMode    = AuthMode::kCase,
        .subjects    = { kCASEAuthTagAsNodeId4, kCASEAuthTagAsNodeId1 },
        .targets     = { { .flags = Target::kCluster, .cluster = kLevelControlCluster } },
    },
};

constexpr size_t entryData1Count = MATTER_ARRAY_SIZE(entryData1);
static_assert(entryData1Count == (kNumFabric1EntriesInEntryData1 + kNumFabric2EntriesInEntryData1),
              "Must maintain both fabric counts for some tests");

struct CheckData
{
    SubjectDescriptor subjectDescriptor;
    RequestPath requestPath;
    Privilege privilege;
    bool allow;
};

constexpr CheckData checkData1[] = {
    // Checks for implicit PASE
    { .subjectDescriptor = { .fabricIndex = 0, .authMode = AuthMode::kPase, .subject = kPaseVerifier0 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subject = kPaseVerifier0 },
      .requestPath       = { .cluster = 3, .endpoint = 4 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subject = kPaseVerifier0 },
      .requestPath       = { .cluster = 5, .endpoint = 6 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subject = kPaseVerifier1 },
      .requestPath       = { .cluster = 5, .endpoint = 6 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 3, .authMode = AuthMode::kPase, .subject = kPaseVerifier3 },
      .requestPath       = { .cluster = 7, .endpoint = 8 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    // Checks for entry 0
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = kAccessControlCluster, .endpoint = 0 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 3, .endpoint = 4 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 5, .endpoint = 6 },
      .privilege         = Privilege::kView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 7, .endpoint = 8 },
      .privilege         = Privilege::kProxyView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    // Checks for entry 1
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 11, .endpoint = 13 },
      .privilege         = Privilege::kView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 11, .endpoint = 13 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 11, .endpoint = 13 },
      .privilege         = Privilege::kView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 11, .endpoint = 13 },
      .privilege         = Privilege::kView,
      .allow             = false },
    // Checks for entry 2
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = kAccessControlCluster, .endpoint = 0 },
      .privilege         = Privilege::kAdminister,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 3, .endpoint = 4 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 5, .endpoint = 6 },
      .privilege         = Privilege::kView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 7, .endpoint = 8 },
      .privilege         = Privilege::kProxyView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kOperationalNodeId4 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = 1, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    // Checks for entry 3
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 11 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId2 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 13 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 11 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = 123, .endpoint = 11 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId1 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 11 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    // Checks for entry 4
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId3 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 2 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 1 },
      .privilege         = Privilege::kManage,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kOperationalNodeId5 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 2 },
      .privilege         = Privilege::kAdminister,
      .allow             = false },
    // Checks for entry 5
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 3 },
      .privilege         = Privilege::kProxyView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kColorControlCluster, .endpoint = 2 },
      .privilege         = Privilege::kProxyView,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subject = kGroup2 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup4 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kColorControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kColorControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 3 },
      .privilege         = Privilege::kProxyView,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subject = kGroup2 },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    // Checks for entry 6
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kUndefinedCAT, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 1,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kUndefinedCAT, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 1,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag1, kUndefinedCAT, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    // Checks for entry 7
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kUndefinedCAT, kUndefinedCAT } },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kCASEAuthTag2, kUndefinedCAT } },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kCASEAuthTag3, kUndefinedCAT } },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kCASEAuthTag4, kUndefinedCAT } },
      .requestPath       = { .cluster = kOnOffCluster, .endpoint = 1 },
      .privilege         = Privilege::kManage,
      .allow             = true },
    // Checks for entry 8
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kCASEAuthTag3, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 1 },
      .privilege         = Privilege::kOperate,
      .allow             = false },
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag0, kCASEAuthTag4, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 2 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
    { .subjectDescriptor = { .fabricIndex = 2,
                             .authMode    = AuthMode::kCase,
                             .cats        = { kCASEAuthTag1, kUndefinedCAT, kUndefinedCAT } },
      .requestPath       = { .cluster = kLevelControlCluster, .endpoint = 2 },
      .privilege         = Privilege::kOperate,
      .allow             = true },
};

class TestAccessControl : public ::testing::Test
{
public: // protected
    void SetUp() override { ASSERT_EQ(ClearAccessControl(accessControl), CHIP_NO_ERROR); }
    static void SetUpTestSuite()
    {
        AccessControl::Delegate * delegate = Examples::GetAccessControlDelegate();
        SetAccessControl(accessControl);
        VerifyOrDie(GetAccessControl().Init(delegate, testDeviceTypeResolver) == CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        GetAccessControl().Finish();
        ResetAccessControlToDefault();
    }
};

TEST_F(TestAccessControl, MetaTest)
{
    EXPECT_EQ(LoadAccessControl(accessControl, entryData1, entryData1Count), CHIP_NO_ERROR);
    EXPECT_EQ(CompareAccessControl(accessControl, entryData1, entryData1Count), CHIP_NO_ERROR);
    EXPECT_EQ(accessControl.DeleteEntry(3), CHIP_NO_ERROR);
    EXPECT_NE(CompareAccessControl(accessControl, entryData1, entryData1Count), CHIP_NO_ERROR);
}

TEST_F(TestAccessControl, TestAclValidateAuthModeSubject)
{
    TestEntryDelegate delegate; // outlive entry
    Entry entry;

    // Use prepared entry for valid cases
    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kView), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId0), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { .flags = Target::kCluster, .cluster = kOnOffCluster }), CHIP_NO_ERROR);

    // Each case tries to update the first entry, then add a second entry, then unconditionally delete it
    EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);

    // CASE and group may have empty subjects list
    {
        EXPECT_EQ(entry.RemoveSubject(0), CHIP_NO_ERROR);

        EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);

        EXPECT_EQ(entry.SetAuthMode(AuthMode::kGroup), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);

        EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId0), CHIP_NO_ERROR);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    for (auto subject : validCaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kGroup), CHIP_NO_ERROR);
    for (auto subject : validGroupSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    // Use test entry for invalid cases (to ensure it can hold invalid data)
    entry.SetDelegate(delegate);

    // Operational PASE not supported
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kPase), CHIP_NO_ERROR);
    for (auto subject : validPaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    for (auto subject : validGroupSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : validPaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : invalidSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kGroup), CHIP_NO_ERROR);
    for (auto subject : validCaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : validPaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : invalidSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kPase), CHIP_NO_ERROR);
    for (auto subject : validCaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : validGroupSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : invalidSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    EXPECT_EQ(entry.SetAuthMode(AuthMode::kNone), CHIP_NO_ERROR);
    for (auto subject : validCaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : validGroupSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : validPaseSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
    for (auto subject : invalidSubjects)
    {
        EXPECT_EQ(entry.SetSubject(0, subject), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    // Next cases have no subject
    EXPECT_EQ(entry.RemoveSubject(0), CHIP_NO_ERROR);

    // PASE must have subject
    {
        EXPECT_EQ(entry.SetAuthMode(AuthMode::kPase), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    // None is not a real auth mode but also shouldn't work with no subject
    {
        EXPECT_EQ(entry.SetAuthMode(AuthMode::kNone), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
}

TEST_F(TestAccessControl, TestAclValidateFabricIndex)
{
    TestEntryDelegate delegate; // outlive entry
    Entry entry;

    // Use prepared entry for valid cases
    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kView), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId0), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { .flags = Target::kCluster, .cluster = kOnOffCluster }), CHIP_NO_ERROR);

    // Each case tries to update the first entry, then add a second entry, then unconditionally delete it
    EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);

    for (auto fabricIndex : fabricIndexes)
    {
        EXPECT_EQ(entry.SetFabricIndex(fabricIndex), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    // Use test entry for invalid cases (to ensure it can hold invalid data)
    entry.SetDelegate(delegate);

    for (auto fabricIndex : invalidFabricIndexes)
    {
        EXPECT_EQ(entry.SetFabricIndex(fabricIndex), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
}

TEST_F(TestAccessControl, TestAclValidatePrivilege)
{
    TestEntryDelegate delegate; // outlive entry
    Entry entry;

    // Use prepared entry for valid cases
    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kView), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId0), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { .flags = Target::kCluster, .cluster = kOnOffCluster }), CHIP_NO_ERROR);

    // Each case tries to update the first entry, then add a second entry, then unconditionally delete it
    EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);

    for (auto privilege : privileges)
    {
        EXPECT_EQ(entry.SetPrivilege(privilege), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    // Use test entry for invalid cases (to ensure it can hold invalid data)
    entry.SetDelegate(delegate);

    // Cannot grant administer privilege to group auth mode
    {
        EXPECT_EQ(entry.SetPrivilege(Privilege::kAdminister), CHIP_NO_ERROR);
        EXPECT_EQ(entry.SetAuthMode(AuthMode::kGroup), CHIP_NO_ERROR);
        EXPECT_EQ(entry.SetSubject(0, kGroup4), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }
}

TEST_F(TestAccessControl, TestAclValidateTarget)
{
    TestEntryDelegate delegate; // outlive entry
    Entry entry;

    // Use prepared entry for valid cases
    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kView), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId0), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { .flags = Target::kCluster, .cluster = kOnOffCluster }), CHIP_NO_ERROR);

    // Each case tries to update the first entry, then add a second entry, then unconditionally delete it
    EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);

    for (auto cluster : validClusters)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kCluster, .cluster = cluster }), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto endpoint : validEndpoints)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kEndpoint, .endpoint = endpoint }), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto deviceType : validDeviceTypes)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kDeviceType, .deviceType = deviceType }), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto cluster : validClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            EXPECT_EQ(
                entry.SetTarget(0, { .flags = Target::kCluster | Target::kEndpoint, .cluster = cluster, .endpoint = endpoint }),
                CHIP_NO_ERROR);
            EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto deviceType : validDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kCluster | Target::kDeviceType, .cluster = cluster, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_EQ(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_EQ(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    // Use test entry for invalid cases (to ensure it can hold invalid data)
    entry.SetDelegate(delegate);

    // Cannot target endpoint and device type
    for (auto endpoint : validEndpoints)
    {
        for (auto deviceType : validDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kEndpoint | Target::kDeviceType, .endpoint = endpoint, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    // Cannot target all
    for (auto cluster : validClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            for (auto deviceType : validDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    // Cannot target none
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = 0 }), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto cluster : invalidClusters)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kCluster, .cluster = cluster }), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto endpoint : invalidEndpoints)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kEndpoint, .endpoint = endpoint }), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto deviceType : invalidDeviceTypes)
    {
        EXPECT_EQ(entry.SetTarget(0, { .flags = Target::kDeviceType, .deviceType = deviceType }), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
        EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
        accessControl.DeleteEntry(1);
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            EXPECT_EQ(
                entry.SetTarget(0, { .flags = Target::kCluster | Target::kEndpoint, .cluster = cluster, .endpoint = endpoint }),
                CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            EXPECT_EQ(
                entry.SetTarget(0, { .flags = Target::kCluster | Target::kEndpoint, .cluster = cluster, .endpoint = endpoint }),
                CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            EXPECT_EQ(
                entry.SetTarget(0, { .flags = Target::kCluster | Target::kEndpoint, .cluster = cluster, .endpoint = endpoint }),
                CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto deviceType : invalidDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kCluster | Target::kDeviceType, .cluster = cluster, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto deviceType : validDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kCluster | Target::kDeviceType, .cluster = cluster, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto deviceType : invalidDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kCluster | Target::kDeviceType, .cluster = cluster, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto endpoint : invalidEndpoints)
    {
        for (auto deviceType : invalidDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kEndpoint | Target::kDeviceType, .endpoint = endpoint, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto endpoint : invalidEndpoints)
    {
        for (auto deviceType : validDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kEndpoint | Target::kDeviceType, .endpoint = endpoint, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto endpoint : validEndpoints)
    {
        for (auto deviceType : invalidDeviceTypes)
        {
            EXPECT_EQ(entry.SetTarget(
                          0, { .flags = Target::kEndpoint | Target::kDeviceType, .endpoint = endpoint, .deviceType = deviceType }),
                      CHIP_NO_ERROR);
            EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
            EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
            accessControl.DeleteEntry(1);
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            for (auto deviceType : invalidDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            for (auto deviceType : validDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            for (auto deviceType : invalidDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            for (auto deviceType : invalidDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : invalidClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            for (auto deviceType : validDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto endpoint : invalidEndpoints)
        {
            for (auto deviceType : validDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }

    for (auto cluster : validClusters)
    {
        for (auto endpoint : validEndpoints)
        {
            for (auto deviceType : invalidDeviceTypes)
            {
                EXPECT_EQ(entry.SetTarget(0,
                                          { .flags      = Target::kCluster | Target::kEndpoint | Target::kDeviceType,
                                            .cluster    = cluster,
                                            .endpoint   = endpoint,
                                            .deviceType = deviceType }),
                          CHIP_NO_ERROR);
                EXPECT_NE(accessControl.UpdateEntry(0, entry), CHIP_NO_ERROR);
                EXPECT_NE(accessControl.CreateEntry(nullptr, entry), CHIP_NO_ERROR);
                accessControl.DeleteEntry(1);
            }
        }
    }
}

TEST_F(TestAccessControl, TestCheck)
{
    LoadAccessControl(accessControl, entryData1, entryData1Count);
    for (const auto & checkData : checkData1)
    {
        CHIP_ERROR expectedResult = checkData.allow ? CHIP_NO_ERROR : CHIP_ERROR_ACCESS_DENIED;
        auto requestPath          = checkData.requestPath;
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
        requestPath.requestType = Access::RequestType::kAttributeReadRequest;
#endif
        EXPECT_EQ(accessControl.Check(checkData.subjectDescriptor, requestPath, checkData.privilege), expectedResult);
    }
}

TEST_F(TestAccessControl, TestCreateReadEntry)
{
    for (size_t i = 0; i < entryData1Count; ++i)
    {
        EXPECT_EQ(LoadAccessControl(accessControl, entryData1 + i, 1), CHIP_NO_ERROR);
        EXPECT_EQ(CompareAccessControl(accessControl, entryData1, i + 1), CHIP_NO_ERROR);
    }
}

TEST_F(TestAccessControl, TestDeleteEntry)
{
    EntryData data[entryData1Count];
    for (size_t pos = 0; pos < MATTER_ARRAY_SIZE(data); ++pos)
    {
        for (size_t count = MATTER_ARRAY_SIZE(data) - pos; count > 0; --count)
        {
            memcpy(data, entryData1, sizeof(data));
            EXPECT_EQ(ClearAccessControl(accessControl), CHIP_NO_ERROR);
            EXPECT_EQ(LoadAccessControl(accessControl, data, MATTER_ARRAY_SIZE(data)), CHIP_NO_ERROR);

            memmove(&data[pos], &data[pos + count], (MATTER_ARRAY_SIZE(data) - count - pos) * sizeof(data[0]));

            for (size_t i = 0; i < count; ++i)
            {
                EXPECT_EQ(accessControl.DeleteEntry(pos), CHIP_NO_ERROR);
            }

            EXPECT_EQ(CompareAccessControl(accessControl, data, MATTER_ARRAY_SIZE(data) - count), CHIP_NO_ERROR);
        }
    }

    // Test fabric removal
    {
        memcpy(data, entryData1, sizeof(data));
        EXPECT_EQ(ClearAccessControl(accessControl), CHIP_NO_ERROR);
        EXPECT_EQ(LoadAccessControl(accessControl, data, MATTER_ARRAY_SIZE(data)), CHIP_NO_ERROR);

        // After deleting Fabric index 1, we should have the number of entries of Fabric index 2
        EXPECT_EQ(accessControl.DeleteAllEntriesForFabric(1), CHIP_NO_ERROR);
        size_t numEntriesForFabricIndex2 = 0;
        size_t numTotalEntries           = 0;
        EXPECT_EQ(accessControl.GetEntryCount(2, numEntriesForFabricIndex2), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.GetEntryCount(numTotalEntries), CHIP_NO_ERROR);
        EXPECT_EQ(numEntriesForFabricIndex2, kNumFabric2EntriesInEntryData1);
        EXPECT_EQ(numTotalEntries, kNumFabric2EntriesInEntryData1);

        // Delete fabric 2 as well, we should be at zero
        numTotalEntries = 1000;
        EXPECT_EQ(accessControl.DeleteAllEntriesForFabric(2), CHIP_NO_ERROR);
        EXPECT_EQ(accessControl.GetEntryCount(numTotalEntries), CHIP_NO_ERROR);
        EXPECT_EQ(numTotalEntries, 0u);
    }
}

TEST_F(TestAccessControl, TestFabricFilteredCreateEntry)
{
    for (auto & fabricIndex : fabricIndexes)
    {
        for (size_t count = 0; count < entryData1Count; ++count)
        {
            EXPECT_EQ(ClearAccessControl(accessControl), CHIP_NO_ERROR);
            EXPECT_EQ(LoadAccessControl(accessControl, entryData1, count), CHIP_NO_ERROR);

            constexpr size_t expectedIndexes[][entryData1Count] = {
                { 0, 1, 2, 2, 3, 3, 3, 4, 4 },
                { 0, 0, 0, 1, 1, 2, 3, 3, 4 },
                { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            };
            const size_t expectedIndex = expectedIndexes[&fabricIndex - fabricIndexes][count];

            Entry entry;
            EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
            EXPECT_EQ(entry.SetFabricIndex(fabricIndex), CHIP_NO_ERROR);
            EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);

            size_t outIndex            = 999;
            FabricIndex outFabricIndex = 123;
            EXPECT_EQ(accessControl.CreateEntry(&outIndex, entry, &outFabricIndex), CHIP_NO_ERROR);

            EXPECT_EQ(outIndex, expectedIndex);
            EXPECT_EQ(outFabricIndex, fabricIndex);
        }
    }
}

TEST_F(TestAccessControl, TestFabricFilteredReadEntry)
{
    EXPECT_EQ(LoadAccessControl(accessControl, entryData1, entryData1Count), CHIP_NO_ERROR);

    for (auto & fabricIndex : fabricIndexes)
    {
        constexpr size_t indexes[] = { 0, 1, 2, 3, 4, 5 };
        for (auto & index : indexes)
        {
            constexpr size_t illegalIndex                                  = entryData1Count;
            constexpr size_t expectedIndexes[][MATTER_ARRAY_SIZE(indexes)] = {
                { 0, 1, 3, 6, illegalIndex, illegalIndex },
                { 2, 4, 5, 7, 8, illegalIndex },
                { illegalIndex, illegalIndex, illegalIndex, illegalIndex, illegalIndex, illegalIndex },
            };
            const size_t expectedIndex = expectedIndexes[&fabricIndex - fabricIndexes][&index - indexes];

            Entry entry;
            CHIP_ERROR err = accessControl.ReadEntry(index, entry, &fabricIndex);

            if (expectedIndex != illegalIndex)
            {
                EXPECT_EQ(err, CHIP_NO_ERROR);
                EXPECT_EQ(CompareEntry(entry, entryData1[expectedIndex]), CHIP_NO_ERROR);
            }
            else
            {
                EXPECT_NE(err, CHIP_NO_ERROR);
            }
        }
    }
}

TEST_F(TestAccessControl, TestIterator)
{
    LoadAccessControl(accessControl, entryData1, entryData1Count);

    FabricIndex fabricIndex;
    EntryIterator iterator;
    Entry entry;
    size_t count;

    EXPECT_EQ(accessControl.Entries(iterator), CHIP_NO_ERROR);
    count = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        EXPECT_EQ(CompareEntry(entry, entryData1[count]), CHIP_NO_ERROR);
        count++;
    }
    EXPECT_EQ(count, entryData1Count);

    fabricIndex = kUndefinedFabricIndex;
    EXPECT_EQ(accessControl.Entries(iterator, &fabricIndex), CHIP_NO_ERROR);
    EXPECT_NE(iterator.Next(entry), CHIP_NO_ERROR);

    fabricIndex = 1;
    EXPECT_EQ(accessControl.Entries(iterator, &fabricIndex), CHIP_NO_ERROR);
    size_t fabric1[] = { 0, 1, 3, 6 };
    count            = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        EXPECT_EQ(CompareEntry(entry, entryData1[fabric1[count]]), CHIP_NO_ERROR);
        count++;
    }
    EXPECT_EQ(count, MATTER_ARRAY_SIZE(fabric1));

    fabricIndex = 2;
    EXPECT_EQ(accessControl.Entries(iterator, &fabricIndex), CHIP_NO_ERROR);
    size_t fabric2[] = { 2, 4, 5, 7, 8 };
    count            = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        EXPECT_EQ(CompareEntry(entry, entryData1[fabric2[count]]), CHIP_NO_ERROR);
        count++;
    }
    EXPECT_EQ(count, MATTER_ARRAY_SIZE(fabric2));
}

TEST_F(TestAccessControl, TestPrepareEntry)
{
    Entry entry;
    for (auto authMode : authModes)
    {
        for (auto fabricIndex : fabricIndexes)
        {
            for (auto privilege : privileges)
            {
                EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);

                size_t subjectCount = 0;
                size_t targetCount  = 0;

                EXPECT_EQ(entry.GetSubjectCount(subjectCount), CHIP_NO_ERROR);
                EXPECT_EQ(entry.GetTargetCount(targetCount), CHIP_NO_ERROR);

                EXPECT_EQ(subjectCount, 0u);
                EXPECT_EQ(targetCount, 0u);

                EXPECT_EQ(entry.SetAuthMode(authMode), CHIP_NO_ERROR);
                EXPECT_EQ(entry.SetFabricIndex(fabricIndex), CHIP_NO_ERROR);
                EXPECT_EQ(entry.SetPrivilege(privilege), CHIP_NO_ERROR);

                int subjectIndex;
                switch (authMode)
                {
                default:
                case AuthMode::kCase:
                    subjectIndex = 0;
                    break;
                case AuthMode::kGroup:
                    subjectIndex = 1;
                    break;
                }

                for (auto subject : subjects[subjectIndex])
                {
                    EXPECT_EQ(entry.AddSubject(nullptr, subject), CHIP_NO_ERROR);
                }

                for (auto & target : targets)
                {
                    EXPECT_EQ(entry.AddTarget(nullptr, target), CHIP_NO_ERROR);
                }

                AuthMode a;
                FabricIndex f;
                Privilege p;

                EXPECT_EQ(entry.GetAuthMode(a), CHIP_NO_ERROR);
                EXPECT_EQ(entry.GetFabricIndex(f), CHIP_NO_ERROR);
                EXPECT_EQ(entry.GetPrivilege(p), CHIP_NO_ERROR);

                EXPECT_EQ(a, authMode);
                EXPECT_EQ(f, fabricIndex);
                EXPECT_EQ(p, privilege);

                EXPECT_EQ(entry.GetSubjectCount(subjectCount), CHIP_NO_ERROR);
                EXPECT_EQ(entry.GetTargetCount(targetCount), CHIP_NO_ERROR);

                EXPECT_EQ(subjectCount, MATTER_ARRAY_SIZE(subjects[0]));
                EXPECT_EQ(targetCount, MATTER_ARRAY_SIZE(targets));

                for (size_t i = 0; i < MATTER_ARRAY_SIZE(subjects[subjectIndex]); ++i)
                {
                    NodeId n;
                    EXPECT_EQ(entry.GetSubject(i, n), CHIP_NO_ERROR);
                    EXPECT_EQ(n, subjects[subjectIndex][i]);
                }

                for (size_t i = 0; i < MATTER_ARRAY_SIZE(targets); ++i)
                {
                    Target t;
                    EXPECT_EQ(entry.GetTarget(i, t), CHIP_NO_ERROR);
                    EXPECT_EQ(t, targets[i]);
                }
            }
        }
    }
}

TEST_F(TestAccessControl, TestSubjectsTargets)
{
    Entry entry;
    size_t index;

    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kAdminister), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kCluster, 1, 0, 0 }), CHIP_NO_ERROR);
    index = 999;
    EXPECT_EQ(accessControl.CreateEntry(&index, entry), CHIP_NO_ERROR);
    EXPECT_EQ(index, 0u);

    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(2), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kManage), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kOperationalNodeId1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kEndpoint, 0, 2, 0 }), CHIP_NO_ERROR);
    index = 999;
    EXPECT_EQ(accessControl.CreateEntry(&index, entry), CHIP_NO_ERROR);
    EXPECT_EQ(index, 1u);

    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetFabricIndex(3), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kOperate), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kGroup), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, kGroup2), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kCluster, 2, 0, 0 }), CHIP_NO_ERROR);
    index = 999;
    EXPECT_EQ(accessControl.CreateEntry(&index, entry), CHIP_NO_ERROR);
    EXPECT_EQ(index, 2u);

    FabricIndex fabricIndex = 0;
    Privilege privilege     = Privilege::kView;
    AuthMode authMode       = AuthMode::kNone;
    size_t count            = 0;
    NodeId subject          = kUndefinedNodeId;
    Target target;

    EXPECT_EQ(accessControl.ReadEntry(0, entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetFabricIndex(fabricIndex), CHIP_NO_ERROR);
    EXPECT_EQ(fabricIndex, 1);
    EXPECT_EQ(entry.GetPrivilege(privilege), CHIP_NO_ERROR);
    EXPECT_EQ(privilege, Privilege::kAdminister);
    EXPECT_EQ(entry.GetAuthMode(authMode), CHIP_NO_ERROR);
    EXPECT_EQ(authMode, AuthMode::kCase);
    EXPECT_EQ(entry.GetSubjectCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
    EXPECT_EQ(entry.GetTargetCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(entry.GetTarget(0, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, Target::kCluster);
    EXPECT_EQ(target.cluster, 1u);

    EXPECT_EQ(accessControl.ReadEntry(1, entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetFabricIndex(fabricIndex), CHIP_NO_ERROR);
    EXPECT_EQ(fabricIndex, 2);
    EXPECT_EQ(entry.GetPrivilege(privilege), CHIP_NO_ERROR);
    EXPECT_EQ(privilege, Privilege::kManage);
    EXPECT_EQ(entry.GetAuthMode(authMode), CHIP_NO_ERROR);
    EXPECT_EQ(authMode, AuthMode::kCase);
    EXPECT_EQ(entry.GetSubjectCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(entry.GetSubject(0, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, kOperationalNodeId1);
    EXPECT_EQ(entry.GetTargetCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(entry.GetTarget(0, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, Target::kEndpoint);
    EXPECT_EQ(target.endpoint, 2);

    EXPECT_EQ(accessControl.ReadEntry(2, entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetFabricIndex(fabricIndex), CHIP_NO_ERROR);
    EXPECT_EQ(fabricIndex, 3);
    EXPECT_EQ(entry.GetPrivilege(privilege), CHIP_NO_ERROR);
    EXPECT_EQ(privilege, Privilege::kOperate);
    EXPECT_EQ(entry.GetAuthMode(authMode), CHIP_NO_ERROR);
    EXPECT_EQ(authMode, AuthMode::kGroup);
    EXPECT_EQ(entry.GetSubjectCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(entry.GetSubject(0, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, kGroup2);
    EXPECT_EQ(entry.GetTargetCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(entry.GetTarget(0, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, Target::kCluster);
    EXPECT_EQ(target.cluster, 2u);

    EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);

    EXPECT_EQ(entry.SetFabricIndex(11), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetPrivilege(Privilege::kProxyView), CHIP_NO_ERROR);
    EXPECT_EQ(entry.SetAuthMode(AuthMode::kCase), CHIP_NO_ERROR);

    EXPECT_EQ(entry.AddSubject(nullptr, 0x11111111AAAAAAAA), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, 0x22222222BBBBBBBB), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddSubject(nullptr, 0x33333333CCCCCCCC), CHIP_NO_ERROR);

    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kCluster | Target::kEndpoint, 11, 22, 0 }), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kCluster | Target::kEndpoint, 33, 44, 0 }), CHIP_NO_ERROR);
    EXPECT_EQ(entry.AddTarget(nullptr, { Target::kCluster | Target::kEndpoint, 0xAAAAFC01, 0x6666, 0 }), CHIP_NO_ERROR);

    index = 999;
    EXPECT_EQ(accessControl.CreateEntry(&index, entry), CHIP_NO_ERROR);
    EXPECT_EQ(index, 3u);

    EXPECT_EQ(accessControl.ReadEntry(3, entry), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetFabricIndex(fabricIndex), CHIP_NO_ERROR);
    EXPECT_EQ(fabricIndex, 11);
    EXPECT_EQ(entry.GetPrivilege(privilege), CHIP_NO_ERROR);
    EXPECT_EQ(privilege, Privilege::kProxyView);
    EXPECT_EQ(entry.GetAuthMode(authMode), CHIP_NO_ERROR);
    EXPECT_EQ(authMode, AuthMode::kCase);
    EXPECT_EQ(entry.GetSubjectCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 3u);
    EXPECT_EQ(entry.GetSubject(0, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, 0x11111111AAAAAAAAu);
    EXPECT_EQ(entry.GetSubject(1, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, 0x22222222BBBBBBBBu);
    EXPECT_EQ(entry.GetSubject(2, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, 0x33333333CCCCCCCCu);
    EXPECT_EQ(entry.GetTargetCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 3u);
    EXPECT_EQ(entry.GetTarget(0, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, (Target::kCluster | Target::kEndpoint));
    EXPECT_EQ(target.cluster, 11u);
    EXPECT_EQ(target.endpoint, 22);
    EXPECT_EQ(entry.GetTarget(1, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, (Target::kCluster | Target::kEndpoint));
    EXPECT_EQ(target.cluster, 33u);
    EXPECT_EQ(target.endpoint, 44);
    EXPECT_EQ(entry.GetTarget(2, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, (Target::kCluster | Target::kEndpoint));
    EXPECT_EQ(target.cluster, 0xAAAAFC01);
    EXPECT_EQ(target.endpoint, 0x6666);

    EXPECT_EQ(entry.RemoveSubject(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetSubjectCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
    EXPECT_EQ(entry.GetSubject(0, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, 0x11111111AAAAAAAAu);
    EXPECT_EQ(entry.GetSubject(1, subject), CHIP_NO_ERROR);
    EXPECT_EQ(subject, 0x33333333CCCCCCCCu);
    EXPECT_NE(entry.GetSubject(2, subject), CHIP_NO_ERROR);

    EXPECT_EQ(entry.RemoveTarget(1), CHIP_NO_ERROR);
    EXPECT_EQ(entry.GetTargetCount(count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
    EXPECT_EQ(entry.GetTarget(0, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, (Target::kCluster | Target::kEndpoint));
    EXPECT_EQ(target.cluster, 11u);
    EXPECT_EQ(target.endpoint, 22u);
    EXPECT_EQ(entry.GetTarget(1, target), CHIP_NO_ERROR);
    EXPECT_EQ(target.flags, (Target::kCluster | Target::kEndpoint));
    EXPECT_EQ(target.cluster, 0xAAAAFC01);
    EXPECT_EQ(target.endpoint, 0x6666);
    EXPECT_NE(entry.GetTarget(2, target), CHIP_NO_ERROR);
}

TEST_F(TestAccessControl, TestUpdateEntry)
{
    EntryData data[entryData1Count];
    memcpy(data, entryData1, sizeof(data));
    EXPECT_EQ(LoadAccessControl(accessControl, data, MATTER_ARRAY_SIZE(data)), CHIP_NO_ERROR);

    for (size_t i = 0; i < MATTER_ARRAY_SIZE(data); ++i)
    {
        EntryData updateData;
        updateData.authMode    = authModes[i % MATTER_ARRAY_SIZE(authModes)];
        updateData.fabricIndex = fabricIndexes[i % MATTER_ARRAY_SIZE(fabricIndexes)];
        updateData.privilege   = privileges[i % (MATTER_ARRAY_SIZE(privileges) - 1)];

        updateData.AddSubject(nullptr, subjects[i % MATTER_ARRAY_SIZE(authModes)][i % MATTER_ARRAY_SIZE(subjects[0])]);
        updateData.AddTarget(nullptr, targets[i % MATTER_ARRAY_SIZE(targets)]);

        data[i] = updateData;

        {
            Entry entry;
            EXPECT_EQ(accessControl.PrepareEntry(entry), CHIP_NO_ERROR);
            EXPECT_EQ(LoadEntry(entry, updateData), CHIP_NO_ERROR);
            EXPECT_EQ(accessControl.UpdateEntry(i, entry), CHIP_NO_ERROR);
        }

        EXPECT_EQ(CompareAccessControl(accessControl, data, MATTER_ARRAY_SIZE(data)), CHIP_NO_ERROR);
    }
}

} // namespace Access
} // namespace chip
