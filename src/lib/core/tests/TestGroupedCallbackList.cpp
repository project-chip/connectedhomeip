/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/GroupedCallbackList.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/logging/CHIPLogging.h>

#include <unordered_map>

using namespace chip::Callback;

// Expose Cancelable anchor for testing
template <typename... T>
struct TestGroupedCallbackList : public GroupedCallbackList<T...>
{
    Cancelable * Anchor() { return this; }
};

static void CallbackFn(void *) {}
static void CallbackWithIntFn(void *, int) {}

typedef void (*CallWithIntFn)(void *, int);

static void * StringContext(char const * string)
{
    return const_cast<void *>(static_cast<void const *>(string));
}

static void ValidateList(Cancelable const * anchor)
{
#if 0 // for manual debugging
    {
        ChipLogDetail(NotSpecified, "ANCHOR: %p", anchor);
        Cancelable * ca = anchor->mNext;
        while (ca != anchor)
        {
            auto * cb = Callback<>::FromCancelable(ca);
            ChipLogDetail(NotSpecified, "%s%p (prev=%p, cancel=%p) %s", (ca->mPrev->mNext == ca) ? "> " : "  ", ca, ca->mPrev,
                          ca->mCancel, static_cast<char const *>(cb->mContext));
            ca = ca->mNext;
        }
    }
#endif
    {
        EXPECT_TRUE(anchor->mPrev->mNext == anchor);
        EXPECT_TRUE(anchor->mNext->mPrev == anchor);

        std::unordered_map<Cancelable const *, size_t> index;
        index[anchor] = 0;

        size_t lastPrevIndex = 0;
        Cancelable * ca      = anchor->mNext;
        for (size_t i = 1; ca != anchor; i++, ca = ca->mNext)
        {
            EXPECT_TRUE(index.find(ca) == index.end()); // cycle?
            index[ca] = i;

            Cancelable * prev = ca->mPrev;
            auto search       = index.find(prev);
            EXPECT_TRUE(search != index.end());       // prev should point backwards
            EXPECT_GE(search->second, lastPrevIndex); // should be monotonic
            lastPrevIndex = search->second;
        }
    }
}

TEST(GroupedCallbackListTest, Trivial)
{
    TestGroupedCallbackList<CallFn> list;
    Callback<> * out = nullptr;
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_FALSE(list.Peek(out));

    Callback cbOne(CallbackFn, StringContext("cbOne"));
    list.Enqueue(&cbOne);
    EXPECT_FALSE(list.IsEmpty());
    EXPECT_TRUE(list.Peek(out));
    EXPECT_TRUE(out == &cbOne);

    cbOne.Cancel();
    EXPECT_TRUE(list.IsEmpty());
}

TEST(GroupedCallbackListTest, EnqueueAllAndPeek)
{
    TestGroupedCallbackList<CallFn, CallWithIntFn> list;
    Callback cbOne(CallbackFn, StringContext("cbOne"));
    Callback cbTwo(CallbackWithIntFn, StringContext("cbTwo"));
    list.Enqueue(&cbOne, &cbTwo);
    ValidateList(list.Anchor());
    Callback<CallFn> * outOne        = nullptr;
    Callback<CallWithIntFn> * outTwo = nullptr;
    EXPECT_TRUE(list.Peek(outOne, outTwo));
    EXPECT_TRUE(outOne == &cbOne);
    EXPECT_TRUE(outTwo == &cbTwo);
}

TEST(GroupedCallbackListTest, EnqueueSparseAndPeek)
{
    TestGroupedCallbackList<CallFn, CallFn> list;
    Callback cbTwo(CallbackFn, StringContext("cbTwo"));
    list.Enqueue(nullptr, &cbTwo);
    ValidateList(list.Anchor());
    Callback<> * outOne = &cbTwo; // poison
    Callback<> * outTwo = nullptr;
    EXPECT_TRUE(list.Peek(outOne, outTwo));
    EXPECT_TRUE(outOne == nullptr);
    EXPECT_TRUE(outTwo == &cbTwo);
}

TEST(GroupedCallbackListTest, EnqueueAndClear)
{
    TestGroupedCallbackList<CallFn, CallWithIntFn> list;
    Callback cbOne(CallbackFn, StringContext("cbOne"));
    Callback cbTwo(CallbackWithIntFn, StringContext("cbTwo"));
    list.Enqueue(&cbOne, &cbTwo);
    Callback cbThree(CallbackFn, StringContext("cbThree"));
    list.Enqueue(&cbThree, nullptr);
    ValidateList(list.Anchor());
    EXPECT_FALSE(list.IsEmpty());
    EXPECT_TRUE(cbOne.IsRegistered());
    EXPECT_TRUE(cbTwo.IsRegistered());
    EXPECT_TRUE(cbThree.IsRegistered());

    list.Clear();
    ValidateList(list.Anchor());
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_FALSE(cbOne.IsRegistered());
    EXPECT_FALSE(cbTwo.IsRegistered());
    EXPECT_FALSE(cbThree.IsRegistered());
}

TEST(GroupedCallbackListTest, Complex)
{
    TestGroupedCallbackList<CallFn, CallFn> list;
    ValidateList(list.Anchor());
    EXPECT_TRUE(list.IsEmpty());

    Callback cbZero(CallbackFn, StringContext("cbZero"));
    list.Enqueue(&cbZero, nullptr);
    ValidateList(list.Anchor());
    EXPECT_FALSE(list.IsEmpty());
    EXPECT_TRUE(cbZero.IsRegistered());

    Callback cbOne(CallbackFn, StringContext("cbOne"));
    Callback cbTwo(CallbackFn, StringContext("cbTwo"));
    list.Enqueue(&cbOne, &cbTwo);
    ValidateList(list.Anchor());
    EXPECT_TRUE(cbOne.IsRegistered());
    EXPECT_TRUE(cbTwo.IsRegistered());

    cbZero.Cancel();
    ValidateList(list.Anchor());
    EXPECT_FALSE(cbZero.IsRegistered());

    Callback cbThree(CallbackFn, StringContext("cbThree"));
    list.Enqueue(&cbThree, nullptr);
    ValidateList(list.Anchor());

    Callback cbFour(CallbackFn, StringContext("cbFour"));
    list.Enqueue(nullptr, &cbFour);
    ValidateList(list.Anchor());

    cbOne.Cancel(); // also cancels cbTwo
    ValidateList(list.Anchor());
    EXPECT_FALSE(cbOne.IsRegistered());
    EXPECT_FALSE(cbTwo.IsRegistered());

    Callback<> * outA = &cbZero;
    Callback<> * outB = &cbZero;
    EXPECT_TRUE(list.Take(outA, outB));
    ValidateList(list.Anchor());
    EXPECT_TRUE(outA == &cbThree);
    EXPECT_TRUE(outB == nullptr);

    EXPECT_TRUE(list.Take(outA, outB));
    ValidateList(list.Anchor());
    EXPECT_TRUE(outA == nullptr);
    EXPECT_TRUE(outB == &cbFour);

    EXPECT_TRUE(list.IsEmpty());
}

TEST(GroupedCallbackListTest, EnqueueTakeAll)
{
    TestGroupedCallbackList<CallFn, CallFn> listA;
    Callback cbOne(CallbackFn, StringContext("cbOne"));
    Callback cbTwo(CallbackFn, StringContext("cbTwo"));
    listA.Enqueue(&cbOne, &cbTwo);
    ValidateList(listA.Anchor());
    EXPECT_FALSE(listA.IsEmpty());
    EXPECT_TRUE(cbOne.IsRegistered());
    EXPECT_TRUE(cbTwo.IsRegistered());

    TestGroupedCallbackList<CallFn, CallFn> listB;
    Callback cbThree(CallbackFn, StringContext("cbThree"));
    listB.Enqueue(&cbThree, nullptr);
    ValidateList(listB.Anchor());
    EXPECT_FALSE(listB.IsEmpty());
    EXPECT_TRUE(cbThree.IsRegistered());

    listB.EnqueueTakeAll(listA);
    ValidateList(listA.Anchor());
    ValidateList(listB.Anchor());
    EXPECT_TRUE(cbThree.IsRegistered());
    EXPECT_TRUE(cbOne.IsRegistered());
    EXPECT_TRUE(cbTwo.IsRegistered());
    EXPECT_FALSE(listB.IsEmpty());
    EXPECT_TRUE(listA.IsEmpty());
}
