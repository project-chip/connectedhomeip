/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <lib/core/CASEAuthTag.h>

using namespace chip;

void TestEqualityOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a                 = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };
        auto b                 = CATValues{ { 0x1111'0001, 0x3333'0003, 0x2222'0002 } };
        auto c                 = CATValues{ { 0x2222'0002, 0x1111'0001, 0x3333'0003 } };
        auto d                 = CATValues{ { 0x2222'0002, 0x3333'0003, 0x1111'0001 } };
        auto e                 = CATValues{ { 0x3333'0003, 0x1111'0001, 0x2222'0002 } };
        auto f                 = CATValues{ { 0x3333'0003, 0x2222'0002, 0x1111'0001 } };
        CATValues candidates[] = { a, b, c, d, e, f };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
    {
        auto a                 = CATValues{ {} };
        auto b                 = CATValues{ {} };
        CATValues candidates[] = { a, b };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }

    {
        auto a                 = CATValues{ { 0x1111'0001, kUndefinedCAT, 0x2222'0002 } };
        auto b                 = CATValues{ { 0x2222'0002, kUndefinedCAT, 0x1111'0001 } };
        auto c                 = CATValues{ { 0x1111'0001, 0x2222'0002 } };
        auto d                 = CATValues{ { 0x2222'0002, 0x1111'0001 } };
        CATValues candidates[] = { a, b, c, d };

        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
}

void TestInequalityOperator(nlTestSuite * inSuite, void * inContext)
{
    auto a                 = CATValues{ { 0x1111'0001 } };
    auto b                 = CATValues{ { 0x1111'0001, 0x2222'0002 } };
    auto c                 = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };
    auto d                 = CATValues{ { 0x2222'0002 } };
    auto e                 = CATValues{ { 0x2222'0002, 0x3333'0003 } };
    auto f                 = CATValues{ { 0x2222'0002, 0x3333'0003, 0x4444'0004 } };
    auto g                 = CATValues{ { 0x3333'0003 } };
    auto h                 = CATValues{ { 0x3333'0003, 0x4444'0004 } };
    auto i                 = CATValues{ { 0x3333'0003, 0x4444'0004, 0x5555'0005 } };
    auto j                 = CATValues{ { 0x4444'0004 } };
    auto k                 = CATValues{ { 0x4444'0004, 0x5555'0005 } };
    auto l                 = CATValues{ { 0x4444'0004, 0x5555'0005, 0x6666'0006 } };
    auto m                 = CATValues{ { 0x5555'0005 } };
    auto n                 = CATValues{ { 0x5555'0005, 0x6666'0006 } };
    auto o                 = CATValues{ { 0x5555'0005, 0x6666'0006, 0x7777'0007 } };
    auto p                 = CATValues{ { 0x6666'0006 } };
    auto q                 = CATValues{ { 0x6666'0006, 0x7777'0007 } };
    auto r                 = CATValues{ { 0x6666'0006, 0x7777'0007, 0x8888'0008 } };
    auto s                 = CATValues{ { 0x7777'0007 } };
    auto t                 = CATValues{ { 0x7777'0007, 0x8888'0008 } };
    auto u                 = CATValues{ { 0x7777'0007, 0x8888'0008, 0x9999'0009 } };
    CATValues candidates[] = { a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u };
    for (auto & outer : candidates)
    {
        for (auto & inner : candidates)
        {
            if (&inner == &outer)
            {
                continue;
            }
            NL_TEST_ASSERT(inSuite, inner != outer);
        }
    }
}

void TestValidity(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a                      = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };
        auto b                      = CATValues{ { 0x1111'0001, 0x3333'0003, 0x2222'0002 } };
        auto c                      = CATValues{ { 0x2222'0002 } };
        auto d                      = CATValues{ { 0x2222'0002, 0x3333'0003 } };
        auto e                      = CATValues{ { 0x2222'0002, kUndefinedCAT, 0x3333'0003 } };
        CATValues validCandidates[] = { a, b, c, d, e };
        for (auto & candidate : validCandidates)
        {
            NL_TEST_ASSERT(inSuite, candidate.AreValid());
        }
    }

    {
        auto versionZero1             = CATValues{ { 0x1111'0000, 0x2222'0002, 0x3333'0003 } };
        auto versionZero2             = CATValues{ { 0x2222'0000 } };
        auto collidingId              = CATValues{ { 0x1111'0001, 0x3333'0003, 0x1111'0002 } };
        CATValues invalidCandidates[] = { versionZero1, versionZero2, collidingId };
        for (auto & candidate : invalidCandidates)
        {
            NL_TEST_ASSERT(inSuite, !candidate.AreValid());
        }
    }
}

void TestMembership(nlTestSuite * inSuite, void * inContext)
{
    auto a = CATValues{ { 0x1111'0001 } };
    auto b = CATValues{ { 0x1111'0001, 0x2222'0002 } };
    auto c = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };

    NL_TEST_ASSERT(inSuite, a.Contains(0x1111'0001));
    NL_TEST_ASSERT(inSuite, a.GetNumTagsPresent() == 1);
    NL_TEST_ASSERT(inSuite, !a.Contains(0x1111'0002));
    NL_TEST_ASSERT(inSuite, !a.Contains(0x2222'0002));
    NL_TEST_ASSERT(inSuite, a.ContainsIdentifier(0x1111));
    NL_TEST_ASSERT(inSuite, !a.ContainsIdentifier(0x2222));
    NL_TEST_ASSERT(inSuite, a.AreValid());

    NL_TEST_ASSERT(inSuite, b.Contains(0x1111'0001));
    NL_TEST_ASSERT(inSuite, b.Contains(0x2222'0002));
    NL_TEST_ASSERT(inSuite, b.GetNumTagsPresent() == 2);
    NL_TEST_ASSERT(inSuite, b.ContainsIdentifier(0x1111));
    NL_TEST_ASSERT(inSuite, b.ContainsIdentifier(0x2222));
    NL_TEST_ASSERT(inSuite, b.AreValid());

    NL_TEST_ASSERT(inSuite, c.Contains(0x1111'0001));
    NL_TEST_ASSERT(inSuite, c.Contains(0x2222'0002));
    NL_TEST_ASSERT(inSuite, c.Contains(0x3333'0003));
    NL_TEST_ASSERT(inSuite, c.GetNumTagsPresent() == 3);
    NL_TEST_ASSERT(inSuite, c.ContainsIdentifier(0x1111));
    NL_TEST_ASSERT(inSuite, c.ContainsIdentifier(0x2222));
    NL_TEST_ASSERT(inSuite, c.ContainsIdentifier(0x3333));
    NL_TEST_ASSERT(inSuite, c.AreValid());
}

void TestSubjectMatching(nlTestSuite * inSuite, void * inContext)
{
    // Check operational node IDs don't match
    auto a = CATValues{ { 0x2222'0002 } };
    NL_TEST_ASSERT(inSuite, !a.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0x0001'0002'0003'0004ull)));
    NL_TEST_ASSERT(inSuite, !a.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0x0001'0002'2222'0002ull)));

    auto b = CATValues{ { 0x1111'0001 } };
    NL_TEST_ASSERT(inSuite, b.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'1111'0001ull)));
    NL_TEST_ASSERT(inSuite, !b.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'1111'0002ull)));

    auto c = CATValues{ { 0x1111'0001, 0x2222'0002 } };
    NL_TEST_ASSERT(inSuite, c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0001ull)));
    NL_TEST_ASSERT(inSuite, c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0002ull)));
    NL_TEST_ASSERT(inSuite, !c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0003ull)));

    auto d = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };
    NL_TEST_ASSERT(inSuite, d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0001ull)));
    NL_TEST_ASSERT(inSuite, d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0002ull)));
    NL_TEST_ASSERT(inSuite, d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0003ull)));
    NL_TEST_ASSERT(inSuite, !d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0004ull)));
    NL_TEST_ASSERT(inSuite, !d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'ffffull)));

    auto e = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'ffff } };
    NL_TEST_ASSERT(inSuite, e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0001ull)));
    NL_TEST_ASSERT(inSuite, e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0002ull)));
    NL_TEST_ASSERT(inSuite, e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0003ull)));
    NL_TEST_ASSERT(inSuite, e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0004ull)));
    NL_TEST_ASSERT(inSuite, e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'ffffull)));
}
// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Equality operator", TestEqualityOperator),
    NL_TEST_DEF("Inequality operator", TestInequalityOperator),
    NL_TEST_DEF("Validity checks", TestValidity),
    NL_TEST_DEF("Set operations", TestMembership),
    NL_TEST_DEF("Subject matching for ACL", TestSubjectMatching),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestCATValues_Setup(void * inContext)
{
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestCATValues_Teardown(void * inContext)
{
    return SUCCESS;
}

int TestCATValues()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "CATValues",
        &sTests[0],
        TestCATValues_Setup,
        TestCATValues_Teardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCATValues)
