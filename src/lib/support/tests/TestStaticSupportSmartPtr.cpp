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
#include <lib/support/static_support_smart_ptr.h>

#include <lib/support/Scoped.h>
#include <lib/support/UnitTestRegistration.h>

#include <cstring>

#include <nlunit-test.h>

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

void TestCheckedGlobalInstanceReference(nlTestSuite * inSuite, void * inContext)
{
    CheckedGlobalInstanceReference<TestClass> ref(&gTestClass);

    // We expect that sizes of global references is minimal
    NL_TEST_ASSERT(inSuite, sizeof(ref) == 1);

    NL_TEST_ASSERT(inSuite, ref->num == 123);
    NL_TEST_ASSERT(inSuite, strcmp(ref->str, "abc") == 0);

    {
        ScopedChange<int> change1(gTestClass.num, 100);
        ScopedChange<const char *> change2(ref->str, "xyz");

        NL_TEST_ASSERT(inSuite, ref->num == 100);
        NL_TEST_ASSERT(inSuite, strcmp(gTestClass.str, "xyz") == 0);
    }

    CheckedGlobalInstanceReference<TestClass> ref2(&gTestClass);

    NL_TEST_ASSERT(inSuite, ref->num == ref2->num);
    NL_TEST_ASSERT(inSuite, strcmp(ref->str, ref2->str) == 0);

    {
        ScopedChange<int> change1(gTestClass.num, 321);
        ScopedChange<const char *> change2(ref->str, "test");

        NL_TEST_ASSERT(inSuite, ref->num == ref2->num);
        NL_TEST_ASSERT(inSuite, strcmp(ref->str, ref2->str) == 0);

        NL_TEST_ASSERT(inSuite, ref2->num == 321);
        NL_TEST_ASSERT(inSuite, strcmp(ref2->str, "test") == 0);
    }
}

void TestSimpleInstanceReference(nlTestSuite * inSuite, void * inContext)
{
    TestClass a("abc", 123);
    TestClass b("xyz", 100);

    SimpleInstanceReference ref_a(&a);
    SimpleInstanceReference ref_b(&b);

    // overhead of simple references should be a simple pointer
    NL_TEST_ASSERT(inSuite, sizeof(ref_a) <= sizeof(void *));

    NL_TEST_ASSERT(inSuite, ref_a->num == 123);
    NL_TEST_ASSERT(inSuite, ref_b->num == 100);

    ref_a->num = 99;
    b.num      = 30;

    NL_TEST_ASSERT(inSuite, a.num == 99);
    NL_TEST_ASSERT(inSuite, ref_b->num == 30);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestCheckedGlobalInstanceReference),
    NL_TEST_DEF_FN(TestSimpleInstanceReference),
    NL_TEST_SENTINEL(),
};

} // namespace

int TestStaticSupportSmartPtr()
{
    nlTestSuite theSuite = { "CHIP Static support smart pointers", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStaticSupportSmartPtr)
