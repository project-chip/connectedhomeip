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

#include <lib/core/CHIPCore.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Access;

using Entry         = AccessControl::Entry;
using EntryIterator = AccessControl::EntryIterator;
using Target        = Entry::Target;

AccessControl accessControl(Examples::GetAccessControlDelegate());

constexpr ClusterId kOnOffCluster         = 0x0006;
constexpr ClusterId kLevelControlCluster  = 0x0008;
constexpr ClusterId kAccessControlCluster = 0x001F;
constexpr ClusterId kColorControlCluster  = 0x0300;

constexpr DeviceTypeId kColorLightDeviceType = 0x0102;

constexpr NodeId kPaseVerifier0 = 0xFFFFFFFB0000'0000;
constexpr NodeId kPaseVerifier1 = 0xFFFFFFFB0000'0001;
constexpr NodeId kPaseVerifier3 = 0xFFFFFFFB0000'0003;
constexpr NodeId kPaseVerifier5 = 0xFFFFFFFB0000'0005;

constexpr NodeId kGroup2 = 0xFFFFFFFFFFFF'0002;
constexpr NodeId kGroup4 = 0xFFFFFFFFFFFF'0004;
constexpr NodeId kGroup6 = 0xFFFFFFFFFFFF'0006;
constexpr NodeId kGroup8 = 0xFFFFFFFFFFFF'0008;

constexpr AuthMode authModes[] = { AuthMode::kPase, AuthMode::kCase, AuthMode::kGroup };

constexpr FabricIndex fabricIndexes[] = { 1, 2, 3 };

constexpr Privilege privileges[] = { Privilege::kView, Privilege::kProxyView, Privilege::kOperate, Privilege::kManage,
                                     Privilege::kAdminister };

constexpr NodeId subjects[][3] = { {
                                       kPaseVerifier0,
                                       kPaseVerifier3,
                                       kPaseVerifier5,
                                   },
                                   {
                                       0x0123456789ABCDEF,  // CASE node
                                       0xFFFFFFFD'00000001, // CAT1
                                       0xFFFFFFFC'00000002, // CAT2
                                   },
                                   {
                                       kGroup4,
                                       kGroup6,
                                       kGroup8,
                                   } };

constexpr Target targets[] = {
    { .flags = Target::kCluster, .cluster = kOnOffCluster },
    { .flags = Target::kEndpoint, .endpoint = 3 },
    { .flags = Target::kDeviceType, .deviceType = kColorLightDeviceType },
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
    ReturnErrorCodeIf(authMode != entryData.authMode, CHIP_ERROR_INCORRECT_STATE);
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
    ReturnErrorCodeIf(fabricIndex != entryData.fabricIndex, CHIP_ERROR_INCORRECT_STATE);
    Privilege privilege = Privilege::kView;
    ReturnErrorOnFailure(entry.GetPrivilege(privilege));
    ReturnErrorCodeIf(privilege != entryData.privilege, CHIP_ERROR_INCORRECT_STATE);
    size_t subjectCount = 0;
    ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
    ReturnErrorCodeIf(subjectCount != entryData.GetSubjectCount(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject = kUndefinedNodeId;
        ReturnErrorOnFailure(entry.GetSubject(i, subject));
        ReturnErrorCodeIf(subject != entryData.subjects[i], CHIP_ERROR_INCORRECT_STATE);
    }
    size_t targetCount = 0;
    ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
    ReturnErrorCodeIf(targetCount != entryData.GetTargetCount(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t i = 0; i < targetCount; ++i)
    {
        Target target;
        ReturnErrorOnFailure(entry.GetTarget(i, target));
        ReturnErrorCodeIf(target != entryData.targets[i], CHIP_ERROR_INCORRECT_STATE);
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
    ReturnErrorCodeIf(ac.ReadEntry(count, entry) == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE);
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

constexpr EntryData entryData1[] = {
    {
        .fabricIndex = 1,
        .privilege   = Privilege::kAdminister,
        .authMode    = AuthMode::kCase,
        .subjects    = { 0x1111111111111111 },
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
        .subjects    = { 0x2222222222222222 },
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
        .authMode    = AuthMode::kPase,
        .subjects    = { kPaseVerifier1 },
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
};

struct CheckData
{
    SubjectDescriptor subjectDescriptor;
    RequestPath requestPath;
    Privilege privilege;
    bool allow;
};

constexpr CheckData checkData1[] =
{
    // Checks for entry 0
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = kAccessControlCluster, .endpoint = 0 },
        .privilege = Privilege::kAdminister,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 3, .endpoint = 4 },
        .privilege = Privilege::kOperate,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 5, .endpoint = 6 },
        .privilege = Privilege::kView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 7, .endpoint = 8 },
        .privilege = Privilege::kProxyView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    // Checks for entry 1
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 11, .endpoint = 13 },
        .privilege = Privilege::kView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 11, .endpoint = 13 },
        .privilege = Privilege::kOperate,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 11, .endpoint = 13 },
        .privilege = Privilege::kView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 11, .endpoint = 13 },
        .privilege = Privilege::kView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 11, .endpoint = 13 },
        .privilege = Privilege::kView,
        .allow = false
    },
    // Checks for entry 2
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = kAccessControlCluster, .endpoint = 0 },
        .privilege = Privilege::kAdminister,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 3, .endpoint = 4 },
        .privilege = Privilege::kOperate,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 5, .endpoint = 6 },
        .privilege = Privilege::kView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 7, .endpoint = 8 },
        .privilege = Privilege::kProxyView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { 0x2222222222222222 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x1111111111111111 }, },
        .requestPath = { .cluster = 1, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    // Checks for entry 3
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 11 },
        .privilege = Privilege::kOperate,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1122334455667788 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 13 },
        .privilege = Privilege::kOperate,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 11 },
        .privilege = Privilege::kOperate,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 11 },
        .privilege = Privilege::kOperate,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = 123, .endpoint = 11 },
        .privilege = Privilege::kOperate,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { 0x1234567812345678 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 11 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    // Checks for entry 4
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier0 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier3 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 2 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 1 },
        .privilege = Privilege::kManage,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kPaseVerifier1 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 2 },
        .privilege = Privilege::kAdminister,
        .allow = false
    },
    // Checks for entry 5
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kOnOffCluster, .endpoint = 3 },
        .privilege = Privilege::kProxyView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kColorControlCluster, .endpoint = 2 },
        .privilege = Privilege::kProxyView,
        .allow = true
    },
    {
        .subjectDescriptor = { .fabricIndex = 1, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup4 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kColorControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kColorControlCluster, .endpoint = 1 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 3 },
        .privilege = Privilege::kProxyView,
        .allow = false
    },
    {
        .subjectDescriptor = { .fabricIndex = 2, .authMode = AuthMode::kGroup, .subjects = { kGroup2 }, },
        .requestPath = { .cluster = kLevelControlCluster, .endpoint = 1 },
        .privilege = Privilege::kOperate,
        .allow = false
    },
};

void MetaTest(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, entryData1, ArraySize(entryData1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CompareAccessControl(accessControl, entryData1, ArraySize(entryData1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, accessControl.DeleteEntry(3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CompareAccessControl(accessControl, entryData1, ArraySize(entryData1)) != CHIP_NO_ERROR);
}

void TestCheck(nlTestSuite * inSuite, void * inContext)
{
    LoadAccessControl(accessControl, entryData1, ArraySize(entryData1));
    for (const auto & checkData : checkData1)
    {
        CHIP_ERROR expectedResult = checkData.allow ? CHIP_NO_ERROR : CHIP_ERROR_ACCESS_DENIED;
        NL_TEST_ASSERT(inSuite,
                       accessControl.Check(checkData.subjectDescriptor, checkData.requestPath, checkData.privilege) ==
                           expectedResult);
    }
}

void TestCreateReadEntry(nlTestSuite * inSuite, void * inContext)
{
    for (size_t i = 0; i < ArraySize(entryData1); ++i)
    {
        NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, entryData1 + i, 1) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, CompareAccessControl(accessControl, entryData1, i + 1) == CHIP_NO_ERROR);
    }
}

void TestDeleteEntry(nlTestSuite * inSuite, void * inContext)
{
    EntryData data[ArraySize(entryData1)];
    for (size_t pos = 0; pos < ArraySize(data); ++pos)
    {
        for (size_t count = ArraySize(data) - pos; count > 0; --count)
        {
            memcpy(data, entryData1, sizeof(data));
            NL_TEST_ASSERT(inSuite, ClearAccessControl(accessControl) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, data, ArraySize(data)) == CHIP_NO_ERROR);

            memmove(&data[pos], &data[pos + count], (ArraySize(data) - count - pos) * sizeof(data[0]));

            for (size_t i = 0; i < count; ++i)
            {
                NL_TEST_ASSERT(inSuite, accessControl.DeleteEntry(pos) == CHIP_NO_ERROR);
            }

            NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, data, ArraySize(data) - count) == CHIP_NO_ERROR);
        }
    }
}

void TestFabricFilteredCreateEntry(nlTestSuite * inSuite, void * inContext)
{
    for (auto & fabricIndex : fabricIndexes)
    {
        for (size_t count = 0; count < ArraySize(entryData1); ++count)
        {
            NL_TEST_ASSERT(inSuite, ClearAccessControl(accessControl) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, entryData1, count) == CHIP_NO_ERROR);

            constexpr size_t expectedIndexes[][ArraySize(entryData1)] = {
                { 0, 1, 2, 2, 3, 3 },
                { 0, 0, 0, 1, 1, 2 },
                { 0, 0, 0, 0, 0, 0 },
            };
            const size_t expectedIndex = expectedIndexes[&fabricIndex - fabricIndexes][count];

            Entry entry;
            NL_TEST_ASSERT(inSuite, accessControl.PrepareEntry(entry) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(fabricIndex) == CHIP_NO_ERROR);

            size_t outIndex            = 999;
            FabricIndex outFabricIndex = 123;
            NL_TEST_ASSERT(inSuite, accessControl.CreateEntry(&outIndex, entry, &outFabricIndex) == CHIP_NO_ERROR);

            NL_TEST_ASSERT(inSuite, outIndex == expectedIndex);
            NL_TEST_ASSERT(inSuite, outFabricIndex == fabricIndex);
        }
    }
}

void TestFabricFilteredReadEntry(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, entryData1, ArraySize(entryData1)) == CHIP_NO_ERROR);

    for (auto & fabricIndex : fabricIndexes)
    {
        constexpr size_t indexes[] = { 0, 1, 2, 3 };
        for (auto & index : indexes)
        {
            constexpr size_t illegalIndex                          = ArraySize(entryData1);
            constexpr size_t expectedIndexes[][ArraySize(indexes)] = {
                { 0, 1, 3, illegalIndex },
                { 2, 4, 5, illegalIndex },
                { illegalIndex, illegalIndex, illegalIndex, illegalIndex },
            };
            const size_t expectedIndex = expectedIndexes[&fabricIndex - fabricIndexes][&index - indexes];

            Entry entry;
            CHIP_ERROR err = accessControl.ReadEntry(index, entry, &fabricIndex);

            if (expectedIndex != illegalIndex)
            {
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, CompareEntry(entry, entryData1[expectedIndex]) == CHIP_NO_ERROR);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
            }
        }
    }
}

void TestIterator(nlTestSuite * inSuite, void * inContext)
{
    LoadAccessControl(accessControl, entryData1, ArraySize(entryData1));

    FabricIndex fabricIndex;
    EntryIterator iterator;
    Entry entry;
    size_t count;

    NL_TEST_ASSERT(inSuite, accessControl.Entries(iterator) == CHIP_NO_ERROR);
    count = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        NL_TEST_ASSERT(inSuite, CompareEntry(entry, entryData1[count]) == CHIP_NO_ERROR);
        count++;
    }
    NL_TEST_ASSERT(inSuite, count == ArraySize(entryData1));

    fabricIndex = kUndefinedFabricIndex;
    NL_TEST_ASSERT(inSuite, accessControl.Entries(iterator, &fabricIndex) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, iterator.Next(entry) != CHIP_NO_ERROR);

    fabricIndex = 1;
    NL_TEST_ASSERT(inSuite, accessControl.Entries(iterator, &fabricIndex) == CHIP_NO_ERROR);
    size_t fabric1[] = { 0, 1, 3 };
    count            = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        NL_TEST_ASSERT(inSuite, CompareEntry(entry, entryData1[fabric1[count]]) == CHIP_NO_ERROR);
        count++;
    }
    NL_TEST_ASSERT(inSuite, count == ArraySize(fabric1));

    fabricIndex = 2;
    NL_TEST_ASSERT(inSuite, accessControl.Entries(iterator, &fabricIndex) == CHIP_NO_ERROR);
    size_t fabric2[] = { 2, 4, 5 };
    count            = 0;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        NL_TEST_ASSERT(inSuite, CompareEntry(entry, entryData1[fabric2[count]]) == CHIP_NO_ERROR);
        count++;
    }
    NL_TEST_ASSERT(inSuite, count == ArraySize(fabric1));
}

void TestPrepareEntry(nlTestSuite * inSuite, void * inContext)
{
    Entry entry;
    for (auto authMode : authModes)
    {
        for (auto fabricIndex : fabricIndexes)
        {
            for (auto privilege : privileges)
            {
                NL_TEST_ASSERT(inSuite, accessControl.PrepareEntry(entry) == CHIP_NO_ERROR);

                size_t subjectCount;
                size_t targetCount;

                NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(subjectCount) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.GetTargetCount(targetCount) == CHIP_NO_ERROR);

                NL_TEST_ASSERT(inSuite, subjectCount == 0);
                NL_TEST_ASSERT(inSuite, targetCount == 0);

                NL_TEST_ASSERT(inSuite, entry.SetAuthMode(authMode) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(fabricIndex) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.SetPrivilege(privilege) == CHIP_NO_ERROR);

                int subjectIndex;
                switch (authMode)
                {
                default:
                case AuthMode::kPase:
                    subjectIndex = 0;
                    break;
                case AuthMode::kCase:
                    subjectIndex = 1;
                    break;
                case AuthMode::kGroup:
                    subjectIndex = 2;
                    break;
                }

                for (auto subject : subjects[subjectIndex])
                {
                    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, subject) == CHIP_NO_ERROR);
                }

                for (auto & target : targets)
                {
                    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, target) == CHIP_NO_ERROR);
                }

                AuthMode a;
                FabricIndex f;
                Privilege p;

                NL_TEST_ASSERT(inSuite, entry.GetAuthMode(a) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.GetFabricIndex(f) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.GetPrivilege(p) == CHIP_NO_ERROR);

                NL_TEST_ASSERT(inSuite, a == authMode);
                NL_TEST_ASSERT(inSuite, f == fabricIndex);
                NL_TEST_ASSERT(inSuite, p == privilege);

                NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(subjectCount) == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, entry.GetTargetCount(targetCount) == CHIP_NO_ERROR);

                NL_TEST_ASSERT(inSuite, subjectCount == 3);
                NL_TEST_ASSERT(inSuite, targetCount == 3);

                for (size_t i = 0; i < ArraySize(subjects[subjectIndex]); ++i)
                {
                    NodeId n;
                    NL_TEST_ASSERT(inSuite, entry.GetSubject(i, n) == CHIP_NO_ERROR);
                    NL_TEST_ASSERT(inSuite, n == subjects[subjectIndex][i]);
                }

                for (size_t i = 0; i < ArraySize(targets); ++i)
                {
                    Target t;
                    NL_TEST_ASSERT(inSuite, entry.GetTarget(i, t) == CHIP_NO_ERROR);
                    NL_TEST_ASSERT(inSuite, t == targets[i]);
                }
            }
        }
    }
}

void TestSubjectsTargets(nlTestSuite * inSuite, void * inContext)
{
    Entry entry;
    NL_TEST_ASSERT(inSuite, accessControl.PrepareEntry(entry) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetPrivilege(Privilege::kAdminister) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetAuthMode(AuthMode::kCase) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, { Target::kCluster, 1, 0, 0 }) == CHIP_NO_ERROR);

    size_t index = 999;
    NL_TEST_ASSERT(inSuite, accessControl.CreateEntry(&index, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, int(index) == 0);

    NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetPrivilege(Privilege::kManage) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetAuthMode(AuthMode::kPase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, 0x0000000011111111) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, { Target::kEndpoint, 0, 2, 0 }) == CHIP_NO_ERROR);

    index = 999;
    NL_TEST_ASSERT(inSuite, accessControl.CreateEntry(&index, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, int(index) == 1);

    NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetPrivilege(Privilege::kOperate) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetAuthMode(AuthMode::kGroup) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, 0x0000000022222222) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, { Target::kDeviceType, 0, 0, 3 }) == CHIP_NO_ERROR);

    index = 999;
    NL_TEST_ASSERT(inSuite, accessControl.CreateEntry(&index, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, int(index) == 2);

    FabricIndex fabricIndex;
    Privilege privilege;
    AuthMode authMode;
    size_t count;
    NodeId subject;
    Target target;

    NL_TEST_ASSERT(inSuite, accessControl.ReadEntry(0, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetFabricIndex(fabricIndex) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabricIndex == 1);
    NL_TEST_ASSERT(inSuite, entry.GetPrivilege(privilege) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, privilege == Privilege::kAdminister);
    NL_TEST_ASSERT(inSuite, entry.GetAuthMode(authMode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, authMode == AuthMode::kCase);
    NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 0);
    NL_TEST_ASSERT(inSuite, entry.GetTargetCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 1);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(0, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kCluster && target.cluster == 1);

    NL_TEST_ASSERT(inSuite, accessControl.ReadEntry(1, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetFabricIndex(fabricIndex) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabricIndex == 2);
    NL_TEST_ASSERT(inSuite, entry.GetPrivilege(privilege) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, privilege == Privilege::kManage);
    NL_TEST_ASSERT(inSuite, entry.GetAuthMode(authMode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, authMode == AuthMode::kPase);
    NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 1);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(0, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x0000000011111111);
    NL_TEST_ASSERT(inSuite, entry.GetTargetCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 2);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(0, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kCluster && target.cluster == 1);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(1, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kEndpoint && target.endpoint == 2);

    NL_TEST_ASSERT(inSuite, accessControl.ReadEntry(2, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetFabricIndex(fabricIndex) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabricIndex == 3);
    NL_TEST_ASSERT(inSuite, entry.GetPrivilege(privilege) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, privilege == Privilege::kOperate);
    NL_TEST_ASSERT(inSuite, entry.GetAuthMode(authMode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, authMode == AuthMode::kGroup);
    NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 2);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(0, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x0000000011111111);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(1, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x0000000022222222);
    NL_TEST_ASSERT(inSuite, entry.GetTargetCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 3);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(0, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kCluster && target.cluster == 1);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(1, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kEndpoint && target.endpoint == 2);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(2, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, target.flags == Target::kDeviceType && target.deviceType == 3);

    NL_TEST_ASSERT(inSuite, accessControl.PrepareEntry(entry) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.SetFabricIndex(11) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetPrivilege(Privilege::kProxyView) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.SetAuthMode(AuthMode::kCase) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, 0x11111111AAAAAAAA) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, 0x22222222BBBBBBBB) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddSubject(nullptr, 0x33333333CCCCCCCC) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, { Target::kCluster | Target::kEndpoint, 11, 22, 0 }) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.AddTarget(nullptr, { Target::kCluster | Target::kDeviceType, 33, 0, 44 }) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(
        inSuite, entry.AddTarget(nullptr, { Target::kCluster | Target::kDeviceType, 0xAAAA5555, 0, 0xBBBB6666 }) == CHIP_NO_ERROR);

    index = 999;
    NL_TEST_ASSERT(inSuite, accessControl.CreateEntry(&index, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, int(index) == 3);

    NL_TEST_ASSERT(inSuite, accessControl.ReadEntry(3, entry) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetFabricIndex(fabricIndex) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabricIndex == 11);
    NL_TEST_ASSERT(inSuite, entry.GetPrivilege(privilege) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, privilege == Privilege::kProxyView);
    NL_TEST_ASSERT(inSuite, entry.GetAuthMode(authMode) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, authMode == AuthMode::kCase);
    NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 3);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(0, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x11111111AAAAAAAA);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(1, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x22222222BBBBBBBB);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(2, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x33333333CCCCCCCC);
    NL_TEST_ASSERT(inSuite, entry.GetTargetCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 3);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(0, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   target.flags == (Target::kCluster | Target::kEndpoint) && target.cluster == 11 && target.endpoint == 22);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(1, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   target.flags == (Target::kCluster | Target::kDeviceType) && target.cluster == 33 && target.deviceType == 44);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(2, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   target.flags == (Target::kCluster | Target::kDeviceType) && target.cluster == 0xAAAA5555 &&
                       target.deviceType == 0xBBBB6666);

    NL_TEST_ASSERT(inSuite, entry.RemoveSubject(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetSubjectCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 2);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(0, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x11111111AAAAAAAA);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(1, subject) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, subject == 0x33333333CCCCCCCC);
    NL_TEST_ASSERT(inSuite, entry.GetSubject(2, subject) != CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, entry.RemoveTarget(1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, entry.GetTargetCount(count) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == 2);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(0, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   target.flags == (Target::kCluster | Target::kEndpoint) && target.cluster == 11 && target.endpoint == 22);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(1, target) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   target.flags == (Target::kCluster | Target::kDeviceType) && target.cluster == 0xAAAA5555 &&
                       target.deviceType == 0xBBBB6666);
    NL_TEST_ASSERT(inSuite, entry.GetTarget(2, target) != CHIP_NO_ERROR);
}

void TestUpdateEntry(nlTestSuite * inSuite, void * inContext)
{
    EntryData data[6];
    memcpy(data, entryData1, sizeof(data));
    NL_TEST_ASSERT(inSuite, LoadAccessControl(accessControl, data, 6) == CHIP_NO_ERROR);

    EntryData updateData;
    for (size_t i = 0; i < 6; ++i)
    {
        updateData.authMode    = authModes[i % 3];
        updateData.fabricIndex = fabricIndexes[i % 3];
        updateData.privilege   = privileges[i % 3];

        if (i < 3)
        {
            updateData.AddSubject(nullptr, subjects[i][i]);
        }
        else
        {
            updateData.AddTarget(nullptr, targets[i - 3]);
        }

        data[i] = updateData;

        {
            Entry entry;
            NL_TEST_ASSERT(inSuite, accessControl.PrepareEntry(entry) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, LoadEntry(entry, updateData) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, accessControl.UpdateEntry(i, entry) == CHIP_NO_ERROR);
        }

        NL_TEST_ASSERT(inSuite, CompareAccessControl(accessControl, data, 6) == CHIP_NO_ERROR);
    }
}

int Setup(void * inContext)
{
    SetAccessControl(accessControl);
    GetAccessControl().Init();
    return SUCCESS;
}

int Teardown(void * inContext)
{
    GetAccessControl().Finish();
    return SUCCESS;
}

int Initialize(void * inContext)
{
    return ClearAccessControl(accessControl) == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int Terminate(void * inContext)
{
    return SUCCESS;
}

} // namespace

int TestAccessControl()
{
    // clang-format off
    constexpr nlTest tests[] = {
        NL_TEST_DEF("MetaTest", MetaTest),
        NL_TEST_DEF("TestPrepareEntry", TestPrepareEntry),
        NL_TEST_DEF("TestCreateReadEntry", TestCreateReadEntry),
        NL_TEST_DEF("TestUpdateEntry", TestUpdateEntry),
        NL_TEST_DEF("TestDeleteEntry", TestDeleteEntry),
        NL_TEST_DEF("TestSubjectsTargets", TestSubjectsTargets),
        NL_TEST_DEF("TestIterator", TestIterator),
        NL_TEST_DEF("TestFabricFilteredReadEntry", TestFabricFilteredReadEntry),
        NL_TEST_DEF("TestFabricFilteredCreateEntry", TestFabricFilteredCreateEntry),
        NL_TEST_DEF("TestCheck", TestCheck),
        NL_TEST_SENTINEL()
    };
    // clang-format on

    nlTestSuite suite = {
        .name       = "AccessControl",
        .tests      = tests,
        .setup      = Setup,
        .tear_down  = Teardown,
        .initialize = Initialize,
        .terminate  = Terminate,
    };

    nlTestRunner(&suite, nullptr);
    return nlTestRunnerStats(&suite);
}

CHIP_REGISTER_TEST_SUITE(TestAccessControl);
