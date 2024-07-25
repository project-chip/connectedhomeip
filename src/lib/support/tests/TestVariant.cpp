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

#include <functional>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Variant.h>

namespace {

struct Simple
{
    bool operator==(const Simple &) const { return true; }
};

struct Pod
{
    Pod(int v1, int v2) : m1(v1), m2(v2) {}
    bool operator==(const Pod & other) const { return m1 == other.m1 && m2 == other.m2; }

    int m1;
    int m2;
};

struct Movable
{
    Movable(int v1, int v2) : m1(v1), m2(v2) {}

    Movable(Movable &)             = delete;
    Movable & operator=(Movable &) = delete;

    Movable(Movable &&)             = default;
    Movable & operator=(Movable &&) = default;

    int m1;
    int m2;
};

struct Count
{
    Count() { ++created; }
    ~Count() { ++destroyed; }

    Count(const Count &) { ++created; }
    Count & operator=(Count &) = default;

    Count(Count &&) { ++created; }
    Count & operator=(Count &&) = default;

    static int created;
    static int destroyed;
};

int Count::created   = 0;
int Count::destroyed = 0;

using namespace chip;

TEST(TestVariant, TestVariantSimple)
{
    Variant<Simple, Pod> v;
    EXPECT_FALSE(v.Valid());
    v.Set<Pod>(5, 10);
    EXPECT_TRUE(v.Valid());
    EXPECT_TRUE(v.Is<Pod>());
    EXPECT_EQ(v.Get<Pod>().m1, 5);
    EXPECT_EQ(v.Get<Pod>().m2, 10);
}

TEST(TestVariant, TestVariantMovable)
{
    Variant<Simple, Movable> v;
    v.Set<Simple>();
    v.Set<Movable>(Movable{ 5, 10 });
    EXPECT_EQ(v.Get<Movable>().m1, 5);
    EXPECT_EQ(v.Get<Movable>().m2, 10);
    auto & m = v.Get<Movable>();
    EXPECT_EQ(m.m1, 5);
    EXPECT_EQ(m.m2, 10);
    v.Set<Simple>();
}

TEST(TestVariant, TestVariantCtorDtor)
{
    {
        Variant<Simple, Count> v;
        EXPECT_EQ(Count::created, 0);
        v.Set<Simple>();
        EXPECT_EQ(Count::created, 0);
        v.Get<Simple>();
        EXPECT_EQ(Count::created, 0);
    }
    {
        Variant<Simple, Count> v;
        EXPECT_EQ(Count::created, 0);
        v.Set<Simple>();
        EXPECT_EQ(Count::created, 0);
        v.Set<Count>();
        EXPECT_EQ(Count::created, 1);
        EXPECT_EQ(Count::destroyed, 0);
        v.Get<Count>();
        EXPECT_EQ(Count::created, 1);
        EXPECT_EQ(Count::destroyed, 0);
        v.Set<Simple>();
        EXPECT_EQ(Count::created, 1);
        EXPECT_EQ(Count::destroyed, 1);
        v.Set<Count>();
        EXPECT_EQ(Count::created, 2);
        EXPECT_EQ(Count::destroyed, 1);
    }
    EXPECT_EQ(Count::destroyed, 2);

    {
        Variant<Simple, Count> v1;
        v1.Set<Count>();
        Variant<Simple, Count> v2(v1);
    }
    EXPECT_EQ(Count::created, 4);
    EXPECT_EQ(Count::destroyed, 4);

    {
        Variant<Simple, Count> v1;
        v1.Set<Count>();
        Variant<Simple, Count> v2(std::move(v1));
    }
    EXPECT_EQ(Count::created, 6);
    EXPECT_EQ(Count::destroyed, 6);

    {
        Variant<Simple, Count> v1, v2;
        v1.Set<Count>();
        v2 = v1;
    }
    EXPECT_EQ(Count::created, 8);
    EXPECT_EQ(Count::destroyed, 8);

    {
        Variant<Simple, Count> v1, v2;
        v1.Set<Count>();
        v2 = std::move(v1);
    }
    EXPECT_EQ(Count::created, 10);
    EXPECT_EQ(Count::destroyed, 10);
}

TEST(TestVariant, TestVariantCopy)
{
    Variant<Simple, Pod> v1;
    v1.Set<Pod>(5, 10);
    Variant<Simple, Pod> v2 = v1;
    EXPECT_TRUE(v1.Valid());
    EXPECT_EQ(v1.Get<Pod>().m1, 5);
    EXPECT_EQ(v1.Get<Pod>().m2, 10);
    EXPECT_TRUE(v2.Valid());
    EXPECT_EQ(v2.Get<Pod>().m1, 5);
    EXPECT_EQ(v2.Get<Pod>().m2, 10);
}

TEST(TestVariant, TestVariantMove)
{
    Variant<Simple, Movable> v1;
    v1.Set<Movable>(5, 10);
    Variant<Simple, Movable> v2 = std::move(v1);
    EXPECT_FALSE(v1.Valid()); // NOLINT(bugprone-use-after-move)
    EXPECT_TRUE(v2.Valid());
    EXPECT_EQ(v2.Get<Movable>().m1, 5);
    EXPECT_EQ(v2.Get<Movable>().m2, 10);
}

TEST(TestVariant, TestVariantCopyAssign)
{
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    v1.Set<Pod>(5, 10);
    v2 = v1;
    EXPECT_TRUE(v1.Valid());
    EXPECT_EQ(v1.Get<Pod>().m1, 5);
    EXPECT_EQ(v1.Get<Pod>().m2, 10);
    EXPECT_TRUE(v2.Valid());
    EXPECT_EQ(v2.Get<Pod>().m1, 5);
    EXPECT_EQ(v2.Get<Pod>().m2, 10);
}

TEST(TestVariant, TestVariantMoveAssign)
{
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    v1.Set<Pod>(5, 10);
    v2 = std::move(v1);
    EXPECT_FALSE(v1.Valid()); // NOLINT(bugprone-use-after-move)
    EXPECT_TRUE(v2.Valid());
    EXPECT_EQ(v2.Get<Pod>().m1, 5);
    EXPECT_EQ(v2.Get<Pod>().m2, 10);
}

TEST(TestVariant, TestVariantInPlace)
{
    int i = 0;

    Variant<std::reference_wrapper<int>> v1 = Variant<std::reference_wrapper<int>>(InPlaceTemplate<std::reference_wrapper<int>>, i);
    EXPECT_TRUE(v1.Valid());
    EXPECT_TRUE(v1.Is<std::reference_wrapper<int>>());
    EXPECT_EQ(&v1.Get<std::reference_wrapper<int>>().get(), &i);

    Variant<std::reference_wrapper<int>> v2 = Variant<std::reference_wrapper<int>>::Create<std::reference_wrapper<int>>(i);
    EXPECT_TRUE(v2.Valid());
    EXPECT_TRUE(v2.Is<std::reference_wrapper<int>>());
    EXPECT_EQ(&v2.Get<std::reference_wrapper<int>>().get(), &i);
}

TEST(TestVariant, TestVariantCompare)
{
    Variant<Simple, Pod> v0;
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    Variant<Simple, Pod> v3;
    Variant<Simple, Pod> v4;

    v1.Set<Simple>();
    v2.Set<Pod>(5, 10);
    v3.Set<Pod>(5, 10);
    v4.Set<Pod>(5, 11);

    EXPECT_TRUE(v0 == v0);
    EXPECT_FALSE(v0 == v1);
    EXPECT_FALSE(v0 == v2);
    EXPECT_FALSE(v0 == v3);
    EXPECT_FALSE(v0 == v4);

    EXPECT_FALSE(v1 == v0);
    EXPECT_TRUE(v1 == v1);
    EXPECT_FALSE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 == v4);

    EXPECT_FALSE(v2 == v0);
    EXPECT_FALSE(v2 == v1);
    EXPECT_TRUE(v2 == v2);
    EXPECT_TRUE(v2 == v3);
    EXPECT_FALSE(v2 == v4);

    EXPECT_FALSE(v3 == v0);
    EXPECT_FALSE(v3 == v1);
    EXPECT_TRUE(v3 == v2);
    EXPECT_TRUE(v3 == v3);
    EXPECT_FALSE(v3 == v4);

    EXPECT_FALSE(v4 == v0);
    EXPECT_FALSE(v4 == v1);
    EXPECT_FALSE(v4 == v2);
    EXPECT_FALSE(v4 == v3);
    EXPECT_TRUE(v4 == v4);
}

} // namespace
