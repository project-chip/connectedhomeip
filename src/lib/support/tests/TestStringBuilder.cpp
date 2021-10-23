/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/support/StringBuilder.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestStringBuilder(nlTestSuite * inSuite, void * inContext)
{

    StringBuilder<64> builder;

    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "") == 0);

    builder.Add("foo");
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);

    builder.Add("bar");
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foobar") == 0);
}

void TestIntegerAppend(nlTestSuite * inSuite, void * inContext)
{

    StringBuilder<64> builder;

    builder.Add("nr: ").Add(1234);
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "nr: 1234") == 0);

    builder.Add(", ").Add(-22);
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "nr: 1234, -22") == 0);
}

void TestOverflow(nlTestSuite * inSuite, void * inContext)
{

    {
        StringBuilder<4> builder;

        builder.Add("foo");
        NL_TEST_ASSERT(inSuite, builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);

        builder.Add("bar");
        NL_TEST_ASSERT(inSuite, !builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);
    }

    {
        StringBuilder<7> builder;

        builder.Add("x: ").Add(12345);
        NL_TEST_ASSERT(inSuite, !builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "x: 123") == 0);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestStringBuilder", TestStringBuilder), //
    NL_TEST_DEF("TestIntegerAppend", TestIntegerAppend), //
    NL_TEST_DEF("TestOverflow", TestOverflow),           //
    NL_TEST_SENTINEL()                                   //
};

} // namespace

int TestStringBuilder(void)
{
    nlTestSuite theSuite = { "StringBuilder", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStringBuilder)
