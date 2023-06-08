/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/StringSplitter.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestStrdupSplitter(nlTestSuite * inSuite, void * inContext)
{
    // empty string handling
    {
        StrdupStringSplitter splitter("", ',');

        // next stays at nullptr
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // single item
    {
        StrdupStringSplitter splitter("single", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "single") == 0);

        // next stays at nullptr also after valid data
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // multi-item
    {
        StrdupStringSplitter splitter("one,two,three", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "one") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "two") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "three") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // mixed
    {
        StrdupStringSplitter splitter("a**bc*d,e*f", '*');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "a") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "bc") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "d,e") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "f") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // some edge cases
    {
        StrdupStringSplitter splitter(",", ',');
        // Note that even though "" is nullptr right away, "," becomes two empty strings
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
    {
        StrdupStringSplitter splitter("log,", ',');
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "log") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
    {
        StrdupStringSplitter splitter(",log", ',');
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "log") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
}

void TestFixedStringSplitter(nlTestSuite * inSuite, void * inContext)
{
    {
        FixedStringSplitter<128> splitter("a,b,c", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "a") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "b") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "c") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    {
        FixedStringSplitter<128> splitter(",a", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "a") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
    {
        FixedStringSplitter<128> splitter("a,", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "a") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
    {
        FixedStringSplitter<128> splitter(",,,", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "") == 0);
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // overflow
    {
        FixedStringSplitter<4> splitter("a,b,c,d", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "a") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "b") == 0);
        // out of space
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    // overflow
    {
        FixedStringSplitter<7> splitter("one,two,three", ',');

        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "one") == 0);
        NL_TEST_ASSERT(inSuite, strcmp(splitter.Next(), "tw") == 0);
        // out of space
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
}

void TestNullResilience(nlTestSuite * inSuite, void * inContext)
{
    {
        StringSplitter splitter(nullptr, ',');
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    {
        StrdupStringSplitter splitter(nullptr, ',');
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }

    {
        FixedStringSplitter<32> splitter(nullptr, ',');
        NL_TEST_ASSERT(inSuite, splitter.Next() == nullptr);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestStrdupSplitter", TestStrdupSplitter),           //
    NL_TEST_DEF("TestFixedStringSplitter", TestFixedStringSplitter), //
    NL_TEST_DEF("TestNullResilience", TestNullResilience),           //
    NL_TEST_SENTINEL()                                               //
};

} // namespace

int TestStringSplitter()
{
    nlTestSuite theSuite = { "StringSplitter", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStringSplitter)
