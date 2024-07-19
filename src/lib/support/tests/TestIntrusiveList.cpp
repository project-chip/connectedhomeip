/*
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

#include <ctime>
#include <list>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/IntrusiveList.h>

namespace {

using namespace chip;

class TestIntrusiveList : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        unsigned seed = static_cast<unsigned>(std::time(nullptr));
        printf("Running " __FILE__ " using seed %d \n", seed);
        std::srand(seed);
    }
};

class ListNode : public IntrusiveListNodeBase<>
{
};

TEST_F(TestIntrusiveList, TestIntrusiveListRandom)
{
    IntrusiveList<ListNode> l1;
    ListNode node[100];
    std::list<ListNode *> l2;

    auto op = [&](auto fun) {
        if (l2.empty())
            return;

        auto l1p = l1.begin();
        auto l2p = l2.begin();
        for (size_t pos = static_cast<size_t>(std::rand()) % l2.size(); pos > 0; --pos)
        {
            ++l1p;
            ++l2p;
        }

        fun(l1p, l2p);
    };

    for (auto & n : node)
    {
        switch (std::rand() % 5)
        {
        case 0: // PushFront
            l1.PushFront(&n);
            l2.push_front(&n);
            break;
        case 1: // PushBack
            l1.PushBack(&n);
            l2.push_back(&n);
            break;
        case 2: // InsertBefore
            op([&](auto & l1p, auto & l2p) {
                l1.InsertBefore(l1p, &n);
                l2.insert(l2p, &n);
            });
            break;
        case 3: // InsertAfter
            op([&](auto & l1p, auto & l2p) {
                l1.InsertAfter(l1p, &n);
                l2.insert(++l2p, &n);
            });
            break;
        case 4: // Remove
            op([&](auto & l1p, auto & l2p) {
                l1.Remove(&*l1p);
                l2.erase(l2p);
            });
            break;
        default:
            break;
        }

        EXPECT_TRUE(std::equal(l1.begin(), l1.end(), l2.begin(), l2.end(),
                               [](const ListNode & p1, const ListNode * p2) { return &p1 == p2; }));
    }

    while (!l1.Empty())
    {
        l1.Remove(&*l1.begin());
    }
}

TEST_F(TestIntrusiveList, TestContains)
{
    ListNode a, b, c;
    IntrusiveList<ListNode> list;

    EXPECT_FALSE(list.Contains(&a));
    EXPECT_FALSE(list.Contains(&b));
    EXPECT_FALSE(list.Contains(&c));

    list.PushBack(&a);
    list.PushFront(&c);

    EXPECT_TRUE(list.Contains(&a));
    EXPECT_FALSE(list.Contains(&b));
    EXPECT_TRUE(list.Contains(&c));

    list.PushBack(&b);

    EXPECT_TRUE(list.Contains(&a));
    EXPECT_TRUE(list.Contains(&b));
    EXPECT_TRUE(list.Contains(&c));

    list.Remove(&a);
    list.Remove(&c);

    EXPECT_FALSE(list.Contains(&a));
    EXPECT_TRUE(list.Contains(&b));
    EXPECT_FALSE(list.Contains(&c));

    // all nodes have to be removed from the list on destruction. Lists do NOT do
    // this automatically
    list.Remove(&b);
}

TEST_F(TestIntrusiveList, TestClear)
{
    ListNode a, b, c;
    IntrusiveList<ListNode> list;

    EXPECT_FALSE(list.Contains(&a));
    EXPECT_FALSE(list.Contains(&b));
    EXPECT_FALSE(list.Contains(&c));

    list.PushBack(&a);
    list.PushFront(&c);

    EXPECT_TRUE(list.Contains(&a));
    EXPECT_FALSE(list.Contains(&b));
    EXPECT_TRUE(list.Contains(&c));

    list.PushBack(&b);

    EXPECT_TRUE(list.Contains(&a));
    EXPECT_TRUE(list.Contains(&b));
    EXPECT_TRUE(list.Contains(&c));

    list.Clear();

    EXPECT_FALSE(list.Contains(&a));
    EXPECT_FALSE(list.Contains(&b));
    EXPECT_FALSE(list.Contains(&c));
}

TEST_F(TestIntrusiveList, TestReplaceNode)
{
    ListNode a, b;
    IntrusiveList<ListNode> list;
    list.PushBack(&a);

    list.Replace(&a, &b);
    EXPECT_FALSE(a.IsInList());
    EXPECT_TRUE(b.IsInList());
    EXPECT_FALSE(list.Empty());
    EXPECT_FALSE(list.Contains(&a));
    EXPECT_TRUE(list.Contains(&b));
    list.Remove(&b);
}

TEST_F(TestIntrusiveList, TestMoveList)
{
    ListNode a, b;

    {
        // Test case 1: Move construct an empty list
        IntrusiveList<ListNode> listA;
        IntrusiveList<ListNode> listB(std::move(listA));
        EXPECT_TRUE(listA.Empty()); // NOLINT(bugprone-use-after-move)
        EXPECT_TRUE(listB.Empty());
    }

    {
        // Test case 2: Move construct an non-empty list
        IntrusiveList<ListNode> listA;
        listA.PushBack(&a);

        IntrusiveList<ListNode> listB(std::move(listA));
        EXPECT_TRUE(listA.Empty()); // NOLINT(bugprone-use-after-move)
        EXPECT_TRUE(listB.Contains(&a));
        listB.Remove(&a);
    }

    {
        // Test case 3: Move assign an empty list
        IntrusiveList<ListNode> listA;
        IntrusiveList<ListNode> listB;
        listB = std::move(listA);
        EXPECT_TRUE(listA.Empty()); // NOLINT(bugprone-use-after-move)
        EXPECT_TRUE(listB.Empty());
    }

    {
        // Test case 4: Move assign to a non-empty list
        IntrusiveList<ListNode> listA;
        listA.PushBack(&a);

        IntrusiveList<ListNode> listB;
        listB = std::move(listA);
        EXPECT_TRUE(listA.Empty()); // NOLINT(bugprone-use-after-move)
        EXPECT_TRUE(listB.Contains(&a));
        listB.Remove(&a);
    }
}

class ListNodeAutoUnlink : public IntrusiveListNodeBase<IntrusiveMode::AutoUnlink>
{
};

TEST_F(TestIntrusiveList, TestAutoUnlink)
{
    IntrusiveList<ListNodeAutoUnlink, IntrusiveMode::AutoUnlink> list;

    // Test case 1: Test node->Unlink()
    {
        ListNodeAutoUnlink a;
        EXPECT_FALSE(list.Contains(&a));
        list.PushBack(&a);
        EXPECT_TRUE(list.Contains(&a));
        a.Unlink();
        EXPECT_FALSE(list.Contains(&a));
        EXPECT_TRUE(list.Empty());
    }

    // Test case 2: The node is automatically removed when goes out of scope
    {
        ListNodeAutoUnlink a;
        EXPECT_FALSE(list.Contains(&a));
        list.PushBack(&a);
        EXPECT_TRUE(list.Contains(&a));
    }
    EXPECT_TRUE(list.Empty());
}

} // namespace
