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

// Some cluster IDs
const ClusterId ONOFF = 0x00000006;
const ClusterId LEVELCONTROL = 0x00000008;
const ClusterId COLORCONTROL = 0x00000300;
const ClusterId ACCESSCONTROL = 0x0000001F;

// Used to detect empty subjects, targets, etc.
const int SENTINEL = 0;

struct TestSubject
{
    SubjectId id = SENTINEL;
};

#if 0
TestSubject Passcode(PasscodeId id)
{
    return { .id = id };
}
#endif

TestSubject Node(NodeId id)
{
    return { .id = id };
}

#if 0
TestSubject CAT1()
{
    return {};
}

TestSubject CAT2()
{
    return {};
}

TestSubject Group(GroupId id)
{
    return { .id = id };
}
#endif

struct TestTarget
{
    enum Flag
    {
        DeviceType = 1 << 0,
        Endpoint = 1 << 1,
        Cluster = 1 << 2,
    };

    int flags = SENTINEL;
    DeviceTypeId deviceType;
    EndpointId endpoint;
    ClusterId cluster;
};

TestTarget Target(EndpointId endpoint, ClusterId cluster)
{
    return {
        .flags = TestTarget::Endpoint | TestTarget::Cluster,
        .endpoint = endpoint,
        .cluster = cluster
    };
}

#if 0
TestTarget Target(EndpointId endpoint)
{
    return { .flags = TestTarget::Endpoint, .endpoint = endpoint };
}

TestTarget Target(ClusterId cluster)
{
    return { .flags = TestTarget::Cluster, .cluster = cluster };
}
#endif

struct TestEntryDelegate
{
    FabricIndex fabricIndex = 0;
    AuthMode authMode = AuthMode::None; // aka SENTINEL
    Privilege privilege = Privilege::View;
    TestSubject subjects[Config::kSubjectsPerEntry + 1];
    TestTarget targets[Config::kTargetsPerEntry + 1];
};

TestEntryDelegate entries[] =
{
    {
        .fabricIndex = 1,
        .authMode = AuthMode::Case,
        .privilege = Privilege::Administer,
        .subjects = { Node(0x1122334455667788) },
    },
    {
        .fabricIndex = 2,
        .authMode = AuthMode::Case,
        .privilege = Privilege::Administer,
        .subjects = { Node(0x8877665544332211) },
    },
    {
        .fabricIndex = 2,
        .authMode = AuthMode::Case,
        .privilege = Privilege::View,
    },
    {
        .fabricIndex = 1,
        .authMode = AuthMode::Pase,
        .privilege = Privilege::View,
        .targets = { Target(2, ONOFF )},
    },
    {
        .fabricIndex = 1,
        .authMode = AuthMode::Case,
        .privilege = Privilege::View,
        .targets = { Target(1, LEVELCONTROL )},
    },
    {
        .fabricIndex = 1,
        .authMode = AuthMode::Case,
        .privilege = Privilege::Operate,
        .targets = { Target(1, COLORCONTROL )},
    },
    {
        .fabricIndex = 1,
        .authMode = AuthMode::Case,
        .privilege = Privilege::View,
        .targets = { Target(2, ONOFF )},
    },
    {} // sentinel entry
};

class TestEntry : public Entry
{
public:
    virtual ~TestEntry() = default;

    bool MatchesAuthMode(AuthMode authMode) const override
    {
        return delegate->authMode == authMode;
    }

    bool MatchesFabric(FabricIndex fabricIndex) const override
    {
        return delegate->fabricIndex == fabricIndex;
    }

    bool MatchesPrivilege(Privilege privilege) const override
    {
        switch (privilege)
        {
            case Privilege::ProxyView:
                return (delegate->privilege == Privilege::ProxyView)
                        || (delegate->privilege == Privilege::Administer);
            case Privilege::View: if (delegate->privilege == Privilege::View) return true;
            // fall through
            case Privilege::Operate: if (delegate->privilege == Privilege::Operate) return true;
            // fall through
            case Privilege::Manage: if (delegate->privilege == Privilege::Manage) return true;
            // fall through
            case Privilege::Administer: return delegate->privilege == Privilege::Administer;
        }
        return false;
    }

    bool MatchesSubject(SubjectId subject) const override
    {
        TestSubject* p = delegate->subjects;
        if (p->id == SENTINEL)
            return true;
        for (; p->id != SENTINEL; ++p)
        {
            if (p->id == subject)
                return true;
        }
        return false;
    }

    bool MatchesTarget(EndpointId endpoint, ClusterId cluster) const override
    {
        TestTarget* p = delegate->targets;
        if (p->flags == SENTINEL)
            return true;
        for (; p->flags != SENTINEL; ++p)
        {
            if (((p->flags & TestTarget::Endpoint) == 0 || p->endpoint == endpoint)
                    && ((p->flags & TestTarget::Cluster) == 0 || p->cluster == cluster))
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
        fabricFiltered = true;
        this->fabricIndex = fabricIndex_;
        entry.delegate = nullptr;
        FindNext();
    }

    void FindNext()
    {
        next = (entry.delegate == nullptr) ? entries : next + 1;

        while (next->authMode != AuthMode::None && fabricFiltered && next->fabricIndex != fabricIndex)
        {
            ++next;
        }

        if (next->authMode == AuthMode::None)
        {
            next = nullptr;
        }
    }

    Entry& Next() override
    {
        if (HasNext())
        {
            entry.delegate = next;
            FindNext();
        }
        return entry;
    }

    bool HasNext() override
    {
        return next != nullptr;
    }

    void Release() override
    {
    }

    bool fabricFiltered;
    FabricIndex fabricIndex;

    TestEntry entry;
    TestEntryDelegate * next;
};

class TestDataProvider : public DataProvider
{
public:
    TestDataProvider() = default;
    virtual ~TestDataProvider() = default;

    CHIP_ERROR Init() override
    {
        return CHIP_NO_ERROR;
    }

    void Finish() override
    {
    }

    EntryIterator* Entries() const override
    {
        iterator.Initialize();
        return &iterator;
    }

    EntryIterator* Entries(FabricIndex fabricIndex) const override
    {
        iterator.Initialize(fabricIndex);
        return &iterator;
    }

    mutable TestEntryIterator iterator;
};

TestDataProvider testDataProvider;
AccessControl testAccessControl(testDataProvider);

#define USE_CONTEXT() \
        AccessControl & context = *reinterpret_cast<AccessControl*>(inContext);

void MetaTestIterator(nlTestSuite * inSuite, void * inContext)
{
    EntryIterator* iterator = testDataProvider.Entries();
    NL_TEST_ASSERT(inSuite, iterator != nullptr);

    TestEntryDelegate* p = entries;
    while (iterator->HasNext())
    {
        auto & entry = iterator->Next();
        NL_TEST_ASSERT_LOOP(inSuite, int(p - entries), static_cast<TestEntry&>(entry).delegate == p);
        ++p;
    }

    NL_TEST_ASSERT(inSuite, p == entries + sizeof(entries)/sizeof(entries[0]) - 1);
}

void TestCheck(nlTestSuite * inSuite, void * inContext)
{
    USE_CONTEXT();

    CHIP_ERROR err;

    // TODO: make this table driven, add a bunch more test cases

    err = context.Check(
            {.subject = 0x1122334455667788, .authMode = AuthMode::Case, .fabricIndex = 1},
            {.endpoint = 1, .cluster = ONOFF},
            Privilege::Administer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = context.Check(
            {.subject = 0x8877665544332211, .authMode = AuthMode::Case, .fabricIndex = 1},
            {.endpoint = 1, .cluster = ONOFF},
            Privilege::Administer);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_ACCESS_DENIED);
}

void TestInstance(nlTestSuite * inSuite, void * inContext)
{
    USE_CONTEXT();

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
    USE_CONTEXT();
    CHIP_ERROR err = context.Init();
    return err == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

int Teardown(void * inContext)
{
    USE_CONTEXT();
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
    const nlTest tests[] =
    {
        NL_TEST_DEF("MetaTestIterator", MetaTestIterator),
        NL_TEST_DEF("TestInstance", TestInstance),
        NL_TEST_DEF("TestCheck", TestCheck),
        NL_TEST_SENTINEL()
    };

    nlTestSuite suite =
    {
        .name = "AccessControl",
        .tests = tests,
        .setup = Setup,
        .tear_down = Teardown,
        .initialize = Initialize,
        .terminate = Terminate,
    };

    nlTestRunner(&suite, &testAccessControl);
    return nlTestRunnerStats(&suite);
}

CHIP_REGISTER_TEST_SUITE(TestAccessControl);
