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
#include "access/Config.h"
#include "access/DataProvider.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip::access;

constexpr EndpointId kEndpoint0 = 0;
constexpr EndpointId kEndpoint1 = 1;
constexpr EndpointId kEndpoint2 = 2;

constexpr ClusterId kOnOffCluster         = 0x00000006;
constexpr ClusterId kLevelControlCluster  = 0x00000008;
constexpr ClusterId kColorControlCluster  = 0x00000300;
constexpr ClusterId kAccessControlCluster = 0x0000001F;

// Used to detect empty subjects, targets, etc.
constexpr int kEmptyFlags = 0;

struct TestSubject
{
    enum Flag
    {
        kPasscode = 1 << 0,
        kNode     = 1 << 1,
        kCAT1     = 1 << 2,
        kCAT2     = 1 << 3,
        kGroup    = 1 << 4,
    };

    int flags = kEmptyFlags;
    SubjectId id;
};

TestSubject Passcode(PasscodeId id)
{
    return { .flags = TestSubject::kPasscode, .id = id };
}

TestSubject Node(NodeId id)
{
    return { .flags = TestSubject::kNode, .id = id };
}

TestSubject CAT1(CatId id)
{
    return { .flags = TestSubject::kCAT1, .id = id };
}

TestSubject CAT2(CatId id)
{
    return { .flags = TestSubject::kCAT2, .id = id };
}

TestSubject Group(GroupId id)
{
    return { .flags = TestSubject::kGroup, .id = id };
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
    AuthMode authMode       = AuthMode::kNone; // kNone used as sentinel
    Privilege privilege     = Privilege::kView;
    TestSubject subjects[Config::kSubjectsPerEntry + 1];
    TestTarget targets[Config::kTargetsPerEntry + 1];
};

TestEntryDelegate entries[] = {
    {
        .fabricIndex = 1,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kAdminister,
        .subjects    = { Node(0x1122334455667788) },
    },
    {
        .fabricIndex = 2,
        .authMode    = AuthMode::kCase,
        .privilege   = Privilege::kAdminister,
        .subjects    = { Node(0x8877665544332211) },
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
    {} // sentinel entry
};

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
        TestSubject * p = delegate->subjects;
        if (p->flags == kEmptyFlags)
            return true;
        for (; p->flags != kEmptyFlags; ++p)
        {
            if (p->id == subject)
                return true;
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
        FindNext();
    }

    void Initialize(FabricIndex fabricIndex_)
    {
        fabricFiltered    = true;
        this->fabricIndex = fabricIndex_;
        entry.delegate    = nullptr;
        FindNext();
    }

    void FindNext()
    {
        next = (entry.delegate == nullptr) ? entries : next + 1;

        while (next->authMode != AuthMode::kNone && fabricFiltered && next->fabricIndex != fabricIndex)
        {
            ++next;
        }

        if (next->authMode == AuthMode::kNone)
        {
            next = nullptr;
        }
    }

    Entry & Next() override
    {
        if (HasNext())
        {
            entry.delegate = next;
            FindNext();
        }
        return entry;
    }

    bool HasNext() override { return next != nullptr; }

    void Release() override {}

    bool fabricFiltered;
    FabricIndex fabricIndex;

    TestEntry entry;
    TestEntryDelegate * next;
};

class TestDataProvider : public DataProvider
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
    while (iterator->HasNext())
    {
        auto & entry = iterator->Next();
        NL_TEST_ASSERT_LOOP(inSuite, int(p - entries), static_cast<TestEntry &>(entry).delegate == p);
        ++p;
    }

    NL_TEST_ASSERT(inSuite, p == entries + sizeof(entries) / sizeof(entries[0]) - 1);

    iterator->Release();
}

void TestCheck(nlTestSuite * inSuite, void * inContext)
{
    AccessControl & context = *reinterpret_cast<AccessControl *>(inContext);

    CHIP_ERROR err;

    // TODO: make this table driven, add a bunch more test cases

    err = context.Check({ .subject = 0x1122334455667788, .authMode = AuthMode::kCase, .fabricIndex = 1 },
                        { .endpoint = kEndpoint1, .cluster = kOnOffCluster }, Privilege::kAdminister);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = context.Check({ .subject = 0x8877665544332211, .authMode = AuthMode::kCase, .fabricIndex = 1 },
                        { .endpoint = kEndpoint1, .cluster = kOnOffCluster }, Privilege::kAdminister);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_ACCESS_DENIED);
}

void TestInstance(nlTestSuite * inSuite, void * inContext)
{
    AccessControl & context = *reinterpret_cast<AccessControl *>(inContext);

    AccessControl * instance = AccessControl::GetInstance();
    NL_TEST_ASSERT(inSuite, instance != nullptr);

    AccessControl::SetInstance(&context);
    NL_TEST_ASSERT(inSuite, AccessControl::GetInstance() == &context);

    AccessControl::SetInstance(nullptr);
    NL_TEST_ASSERT(inSuite, AccessControl::GetInstance() == nullptr);

    AccessControl::SetInstance(instance);
    NL_TEST_ASSERT(inSuite, AccessControl::GetInstance() == instance);
}

int Setup(void * inContext)
{
    AccessControl & context = *reinterpret_cast<AccessControl *>(inContext);
    CHIP_ERROR err          = context.Init();
    return err == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int Teardown(void * inContext)
{
    AccessControl & context = *reinterpret_cast<AccessControl *>(inContext);
    context.Finish();
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
    const nlTest tests[] = { NL_TEST_DEF("MetaTestIterator", MetaTestIterator), NL_TEST_DEF("TestInstance", TestInstance),
                             NL_TEST_DEF("TestCheck", TestCheck), NL_TEST_SENTINEL() };

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
