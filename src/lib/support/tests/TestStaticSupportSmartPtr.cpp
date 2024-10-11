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

#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Scoped.h>
#include <lib/support/static_support_smart_ptr.h>

using namespace chip;
namespace {

struct TestClass
{
    const char * str;
    int num;

    TestClass(const char * s, int n) : str(s), num(n) {}
};

TestClass gTestClass("abc", 123);
} // namespace

namespace chip {

template <>
TestClass * GlobalInstanceProvider<TestClass>::InstancePointer()
{
    return &gTestClass;
}

} // namespace chip

namespace {

TEST(TestStaticSupportSmartPtr, TestCheckedGlobalInstanceReference)
{
    CheckedGlobalInstanceReference<TestClass> ref(&gTestClass);

    // We expect that sizes of global references is minimal
    EXPECT_EQ(sizeof(ref), 1u);

    ASSERT_TRUE(ref);
    EXPECT_EQ(ref->num, 123);
    EXPECT_STREQ(ref->str, "abc");

    {
        ScopedChange<int> change1(gTestClass.num, 100);
        ScopedChange<const char *> change2(ref->str, "xyz");

        EXPECT_EQ(ref->num, 100);
        EXPECT_STREQ(gTestClass.str, "xyz");
    }

    CheckedGlobalInstanceReference<TestClass> ref2(&gTestClass);

    ASSERT_TRUE(ref2);
    EXPECT_EQ(ref->num, ref2->num);
    EXPECT_STREQ(ref->str, ref2->str);

    {
        ScopedChange<int> change1(gTestClass.num, 321);
        ScopedChange<const char *> change2(ref->str, "test");

        EXPECT_EQ(ref->num, ref2->num);
        EXPECT_STREQ(ref->str, ref2->str);

        EXPECT_EQ(ref2->num, 321);
        EXPECT_STREQ(ref2->str, "test");
    }

    // Check default constructed CheckedGlobalInstanceReference
    CheckedGlobalInstanceReference<TestClass> ref_default;
    ASSERT_TRUE(ref_default);
}

TEST(TestStaticSupportSmartPtr, TestSimpleInstanceReference)
{
    TestClass a("abc", 123);
    TestClass b("xyz", 100);

    SimpleInstanceReference ref_a(&a);
    SimpleInstanceReference ref_b(&b);

    // overhead of simple references should be a simple pointer
    EXPECT_LE(sizeof(ref_a), sizeof(void *));

    ASSERT_TRUE(ref_a);
    ASSERT_TRUE(ref_b);

    EXPECT_EQ(ref_a->num, 123);
    EXPECT_EQ(ref_b->num, 100);

    ref_a->num = 99;
    b.num      = 30;

    EXPECT_EQ(a.num, 99);
    EXPECT_EQ(ref_b->num, 30);

    // Check default constructed SimpleInstanceReference
    SimpleInstanceReference<TestClass> ref_default;
    ASSERT_FALSE(ref_default);
}

} // namespace
