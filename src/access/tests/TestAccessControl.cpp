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

#include <lib/core/CHIPCore.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Access;

constexpr EndpointId kEndpoint0 = 0;
constexpr EndpointId kEndpoint1 = 1;
constexpr EndpointId kEndpoint2 = 2;

constexpr ClusterId kOnOffCluster         = 0x00000006;
constexpr ClusterId kLevelControlCluster  = 0x00000008;
constexpr ClusterId kColorControlCluster  = 0x00000300;
constexpr ClusterId kAccessControlCluster = 0x0000001F;

constexpr size_t kSubjectsPerEntry = 4;
constexpr size_t kTargetsPerEntry  = 3;

// Used to detect empty subjects, targets, etc.
constexpr int kEmptyFlags = 0;

// For test purposes, store all subjects as node, use tags to discriminate
// passcode/group, and don't allow 0.
constexpr NodeId kTestSubjectMask     = 0xFFFFFFFFFFFF0000;
constexpr NodeId kTestSubjectPasscode = 0xDDDDDDDDDDDD0000;
constexpr NodeId kTestSubjectGroup    = 0xEEEEEEEEEEEE0000;
constexpr NodeId kTestSubjectEmpty    = 0x0000000000000000;

constexpr SubjectId Passcode(PasscodeId passcode)
{
    // For test purposes, stuff passcode into node with tag
    return { .node = kTestSubjectPasscode | passcode };
}

constexpr SubjectId Node(NodeId node)
{
    return { .node = node };
}

constexpr SubjectId CAT1(NodeId node)
{
    return { .node = node };
}

constexpr SubjectId CAT2(NodeId node)
{
    return { .node = node };
}

constexpr SubjectId Group(GroupId group)
{
    // For test purposes, stuff group into node with tag
    return { .node = kTestSubjectGroup | group };
}

struct TestTarget
{
    enum Flag
    {
        kDeviceType = 1 << 0,
        kEndpoint   = 1 << 1,
        kCluster    = 1 << 2,
    };

    int flags = kEmptyFlags;
    DeviceTypeId deviceType;
    EndpointId endpoint;
    ClusterId cluster;
};

TestTarget Target(EndpointId endpoint, ClusterId cluster)
{
    return { .flags = TestTarget::kEndpoint | TestTarget::kCluster, .endpoint = endpoint, .cluster = cluster };
}

TestTarget Target(EndpointId endpoint)
{
    return { .flags = TestTarget::kEndpoint, .endpoint = endpoint };
}

TestTarget Target(ClusterId cluster)
{
    return { .flags = TestTarget::kCluster, .cluster = cluster };
}

struct TestEntryDelegate
{
    FabricIndex fabricIndex = 0;
    AuthMode authMode       = AuthMode::kNone;
    Privilege privilege     = Privilege::kView;
    SubjectId subjects[kSubjectsPerEntry + 1];
    TestTarget targets[kTargetsPerEntry + 1];
    const char * tag = "";
    bool touched     = false;
};

TestEntryDelegate entries[] = {
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kAdminister,
        .subjects    = { Node(0x1122334455667788) },
        .tag         = "1-admin",
    },
    {
        .fabricIndex = 2,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kAdminister,
        .subjects    = { Node(0x8877665544332211) },
        .tag         = "2-admin",
    },
    {
        .fabricIndex = 2,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kView,
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kPase,
        .privilege   = Privilege::kView,
        .subjects    = { Passcode(0) },
        .targets     = { Target(kEndpoint2, kOnOffCluster) },
        .tag         = "1-pase-view-onoff-2",
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kView,
        .targets     = { Target(kEndpoint1, kLevelControlCluster) },
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kOperate,
        .targets     = { Target(kEndpoint1, kColorControlCluster) },
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kView,
        .targets     = { Target(kEndpoint2, kOnOffCluster) },
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kView,
        .subjects    = { CAT1(86), CAT2(99) },
        .targets     = { Target(kEndpoint0), Target(kAccessControlCluster) },
    },
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kGroup,
        .privilege   = Privilege::kView,
        .subjects    = { Group(7) },
    },
    {
        .fabricIndex = 3,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kAdminister,
    },
};
constexpr int kNumEntries = sizeof(entries) / sizeof(entries[0]);

void ResetTouchedEntries()
{
    for (int i = 0; i < kNumEntries; ++i)
    {
        entries[i].touched = false;
    }
}

bool EntriesTouchedToTag(const char * tag, FabricIndex fabricIndex)
{
    bool expectedTouched = true;

    for (int i = 0; i < kNumEntries; ++i)
    {
        auto & entry = entries[i];
        if (entry.fabricIndex == fabricIndex)
        {
            if (entry.touched != expectedTouched)
            {
                return false;
            }
            if (strcmp(entry.tag, tag) == 0)
            {
                expectedTouched = false;
            }
        }
        else if (entry.touched)
        {
            return false;
        }
    }

    return true;
}

class TestEntry : public Entry
{
public:
    virtual ~TestEntry() = default;

    bool MatchesAuthMode(AuthMode authMode) const override { return delegate->authMode == authMode; }

    bool MatchesFabric(FabricIndex fabricIndex) const override
    {
        return (delegate->fabricIndex == 0) || (delegate->fabricIndex == fabricIndex);
    }

    bool MatchesPrivilege(Privilege privilege) const override
    {
        switch (privilege)
        {
        case Privilege::kProxyView:
            return (delegate->privilege == Privilege::kProxyView) || (delegate->privilege == Privilege::kAdminister);
        case Privilege::kView:
            if (delegate->privilege == Privilege::kView)
                return true;
            FALLTHROUGH; // fall through
        case Privilege::kOperate:
            if (delegate->privilege == Privilege::kOperate)
                return true;
            FALLTHROUGH; // fall through
        case Privilege::kManage:
            if (delegate->privilege == Privilege::kManage)
                return true;
            FALLTHROUGH; // fall through
        case Privilege::kAdminister:
            return delegate->privilege == Privilege::kAdminister;
        }
        return false;
    }

    bool MatchesSubject(SubjectId subject) const override
    {
        SubjectId * p = delegate->subjects;
        if (p->node == kTestSubjectEmpty)
            return true;
        for (; p->node != kTestSubjectEmpty; ++p)
        {
            // Don't call ::MatchesSubject because of special storage/tags
            if ((p->node & kTestSubjectMask) == kTestSubjectPasscode)
            {
                if ((p->node & ~kTestSubjectMask) == subject.passcode)
                {
                    return true;
                }
            }
            else if ((p->node & kTestSubjectMask) == kTestSubjectGroup)
            {
                if ((p->node & ~kTestSubjectMask) == subject.group)
                {
                    return true;
                }
            }
            else
            {
                // TODO: handle CASE Authenticated Tags (CAT1/CAT2)
                if (p->node == subject.node)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool MatchesTarget(EndpointId endpoint, ClusterId cluster) const override
    {
        TestTarget * p = delegate->targets;
        if (p->flags == kEmptyFlags)
            return true;
        for (; p->flags != kEmptyFlags; ++p)
        {
            if (((p->flags & TestTarget::kEndpoint) == 0 || p->endpoint == endpoint) &&
                ((p->flags & TestTarget::kCluster) == 0 || p->cluster == cluster))
                return true;
        }
        return false;
    }

    TestEntryDelegate * delegate;
};

class TestEntryIterator : public EntryIterator
{
public:
    virtual ~TestEntryIterator() = default;

    void Initialize()
    {
        fabricFiltered = false;
        entry.delegate = nullptr;
    }

    void Initialize(FabricIndex fabricIndex_)
    {
        fabricFiltered    = true;
        this->fabricIndex = fabricIndex_;
        entry.delegate    = nullptr;
    }

    Entry * Next() override
    {
        do
        {
            if (entry.delegate == nullptr)
            {
                entry.delegate = entries;
            }
            else if ((entry.delegate - entries) < kNumEntries)
            {
                entry.delegate++;
            }
        } while ((entry.delegate - entries) < kNumEntries && fabricFiltered && entry.delegate->fabricIndex != fabricIndex);

        if ((entry.delegate - entries) < kNumEntries)
        {
            entry.delegate->touched = true;
            return &entry;
        }

        return nullptr;
    }

    void Release() override {}

    bool fabricFiltered;
    FabricIndex fabricIndex;

    TestEntry entry;
    TestEntryDelegate * next;
};

class TestDataProvider : public AccessControlDataProvider
{
public:
    TestDataProvider()          = default;
    virtual ~TestDataProvider() = default;

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    void Finish() override {}

    EntryIterator * Entries() const override
    {
        iterator.Initialize();
        return &iterator;
    }

    EntryIterator * Entries(FabricIndex fabricIndex) const override
    {
        iterator.Initialize(fabricIndex);
        return &iterator;
    }

    mutable TestEntryIterator iterator;
};

TestDataProvider testDataProvider;
AccessControl testAccessControl(testDataProvider);

void MetaTestIterator(nlTestSuite * inSuite, void * inContext)
{
    EntryIterator * iterator = testDataProvider.Entries();
    NL_TEST_ASSERT(inSuite, iterator != nullptr);

    TestEntryDelegate * p = entries;
    while (auto entry = iterator->Next())
    {
        NL_TEST_ASSERT_LOOP(inSuite, int(p - entries), static_cast<TestEntry *>(entry)->delegate == p);
        ++p;
    }

    NL_TEST_ASSERT(inSuite, p == entries + kNumEntries);

    iterator->Release();
}

// Given the entries, test many cases to ensure AccessControl::Check both
// returns the correct answer and has used the expected entries to do so
void TestCheck(nlTestSuite * inSuite, void * inContext)
{
    AccessControl & context = *reinterpret_cast<AccessControl *>(inContext);

    constexpr struct
    {
        SubjectDescriptor subjectDescriptor;
        RequestPath requestPath;
        Privilege privilege;
        CHIP_ERROR expectedResult;
        const char * expectedTag;
    } checks[] = {
        // clang-format off
        {
            { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { Node(0x1122334455667788) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_NO_ERROR,
            "1-admin",
        },
        {
            { .fabricIndex = 1, .authMode = AuthMode::kCase, .subjects = { Node(0x1111222233334444) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_ERROR_ACCESS_DENIED,
            "(end)",
        },
        {
            { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { Passcode(kDefaultCommissioningPasscodeId) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_ERROR_ACCESS_DENIED,
            "(end)",
        },
        {
            { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { Node(0x8877665544332211) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_NO_ERROR,
            "2-admin",
        },
        {
            { .fabricIndex = 2, .authMode = AuthMode::kCase, .subjects = { Node(0x1122334455667788) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_ERROR_ACCESS_DENIED,
            "(end)",
        },
        {
            { .fabricIndex = 2, .authMode = AuthMode::kPase, .subjects = { Passcode(kDefaultCommissioningPasscodeId) } },
            { .endpoint = kEndpoint1, .cluster = kOnOffCluster },
            Privilege::kAdminister,
            CHIP_ERROR_ACCESS_DENIED,
            "(end)",
        },
        {
            { .fabricIndex = 1, .authMode = AuthMode::kPase, .subjects = { Passcode(kDefaultCommissioningPasscodeId) } },
            { .endpoint = kEndpoint2, .cluster = kOnOffCluster },
            Privilege::kView,
            CHIP_NO_ERROR,
            "1-pase-view-onoff-2",
        },
        // clang-format on
    };

    for (int i = 0; i < int(sizeof(checks) / sizeof(checks[0])); ++i)
    {
        auto & check = checks[i];
        ResetTouchedEntries();
        CHIP_ERROR err = context.Check(check.subjectDescriptor, check.requestPath, check.privilege);
        NL_TEST_ASSERT_LOOP(inSuite, i, err == check.expectedResult);
        NL_TEST_ASSERT_LOOP(inSuite, i, EntriesTouchedToTag(check.expectedTag, check.subjectDescriptor.fabricIndex));
    }
}

void TestGlobalInstance(nlTestSuite * inSuite, void * inContext)
{
    // Initial instance should not be nullptr
    AccessControl * instance = GetAccessControl();
    NL_TEST_ASSERT(inSuite, instance != nullptr);

    // Attempting to set nullptr should have no effect
    SetAccessControl(nullptr);
    NL_TEST_ASSERT(inSuite, GetAccessControl() == instance);

    // Setting another instance should have immediate effect
    SetAccessControl(&testAccessControl);
    NL_TEST_ASSERT(inSuite, GetAccessControl() == &testAccessControl);

    // Restoring initial instance should also work
    SetAccessControl(instance);
    NL_TEST_ASSERT(inSuite, GetAccessControl() == instance);
}

int Setup(void * inContext)
{
    CHIP_ERROR err = testDataProvider.Init();
    if (err != CHIP_NO_ERROR)
        return FAILURE;
    err = testAccessControl.Init();
    if (err != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int Teardown(void * inContext)
{
    testDataProvider.Finish();
    testAccessControl.Finish();
    return SUCCESS;
}

int Initialize(void * inContext)
{
    return SUCCESS;
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
        NL_TEST_DEF("MetaTestIterator", MetaTestIterator),
        NL_TEST_DEF("TestGlobalInstance", TestGlobalInstance),
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

    nlTestRunner(&suite, &testAccessControl);
    return nlTestRunnerStats(&suite);
}

CHIP_REGISTER_TEST_SUITE(TestAccessControl);
