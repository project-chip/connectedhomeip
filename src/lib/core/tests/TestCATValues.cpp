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

#include <pw_unit_test/framework.h>

#include <lib/core/CASEAuthTag.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

TEST(TestCATValues, TestEqualityOperator)
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
                EXPECT_EQ(inner, outer);
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
                EXPECT_EQ(inner, outer);
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
                EXPECT_EQ(inner, outer);
            }
        }
    }
}

TEST(TestCATValues, TestInequalityOperator)
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
            EXPECT_NE(inner, outer);
        }
    }
}

TEST(TestCATValues, TestValidity)
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
            EXPECT_TRUE(candidate.AreValid());
        }
    }

    {
        auto versionZero1             = CATValues{ { 0x1111'0000, 0x2222'0002, 0x3333'0003 } };
        auto versionZero2             = CATValues{ { 0x2222'0000 } };
        auto collidingId              = CATValues{ { 0x1111'0001, 0x3333'0003, 0x1111'0002 } };
        CATValues invalidCandidates[] = { versionZero1, versionZero2, collidingId };
        for (auto & candidate : invalidCandidates)
        {
            EXPECT_FALSE(candidate.AreValid());
        }
    }
}

TEST(TestCATValues, TestMembership)
{
    auto a = CATValues{ { 0x1111'0001 } };
    auto b = CATValues{ { 0x1111'0001, 0x2222'0002 } };
    auto c = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };

    EXPECT_TRUE(a.Contains(0x1111'0001));
    EXPECT_EQ(a.GetNumTagsPresent(), 1u);
    EXPECT_FALSE(a.Contains(0x1111'0002));
    EXPECT_FALSE(a.Contains(0x2222'0002));
    EXPECT_TRUE(a.ContainsIdentifier(0x1111));
    EXPECT_FALSE(a.ContainsIdentifier(0x2222));
    EXPECT_TRUE(a.AreValid());

    EXPECT_TRUE(b.Contains(0x1111'0001));
    EXPECT_TRUE(b.Contains(0x2222'0002));
    EXPECT_EQ(b.GetNumTagsPresent(), 2u);
    EXPECT_TRUE(b.ContainsIdentifier(0x1111));
    EXPECT_TRUE(b.ContainsIdentifier(0x2222));
    EXPECT_TRUE(b.AreValid());

    EXPECT_TRUE(c.Contains(0x1111'0001));
    EXPECT_TRUE(c.Contains(0x2222'0002));
    EXPECT_TRUE(c.Contains(0x3333'0003));
    EXPECT_EQ(c.GetNumTagsPresent(), 3u);
    EXPECT_TRUE(c.ContainsIdentifier(0x1111));
    EXPECT_TRUE(c.ContainsIdentifier(0x2222));
    EXPECT_TRUE(c.ContainsIdentifier(0x3333));
    EXPECT_TRUE(c.AreValid());
}

TEST(TestCATValues, TestSubjectMatching)
{
    // Check operational node IDs don't match
    auto a = CATValues{ { 0x2222'0002 } };
    EXPECT_FALSE(a.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0x0001'0002'0003'0004ull)));
    EXPECT_FALSE(a.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0x0001'0002'2222'0002ull)));

    auto b = CATValues{ { 0x1111'0001 } };
    EXPECT_TRUE(b.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'1111'0001ull)));
    EXPECT_FALSE(b.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'1111'0002ull)));

    auto c = CATValues{ { 0x1111'0001, 0x2222'0002 } };
    EXPECT_TRUE(c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0001ull)));
    EXPECT_TRUE(c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0002ull)));
    EXPECT_FALSE(c.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'2222'0003ull)));

    auto d = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'0003 } };
    EXPECT_TRUE(d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0001ull)));
    EXPECT_TRUE(d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0002ull)));
    EXPECT_TRUE(d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0003ull)));
    EXPECT_FALSE(d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0004ull)));
    EXPECT_FALSE(d.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'ffffull)));

    auto e = CATValues{ { 0x1111'0001, 0x2222'0002, 0x3333'ffff } };
    EXPECT_TRUE(e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0001ull)));
    EXPECT_TRUE(e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0002ull)));
    EXPECT_TRUE(e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0003ull)));
    EXPECT_TRUE(e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'0004ull)));
    EXPECT_TRUE(e.CheckSubjectAgainstCATs(static_cast<chip::NodeId>(0xFFFF'FFFD'3333'ffffull)));
}
