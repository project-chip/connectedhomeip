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
    CharSpan out;

    // empty string handling
    {
        StringSplitter splitter("", ',');

        // next stays at nullptr
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
    }

    // single item
    {
        StringSplitter splitter("single", ',');

        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("single"_span));

        // next stays at nullptr also after valid data
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
    }

    // multi-item
    {
        StringSplitter splitter("one,two,three", ',');

        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("one"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("two"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("three"_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
    }

    // mixed
    {
        StringSplitter splitter("a**bc*d,e*f", '*');

        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("a"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("bc"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("d,e"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("f"_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }

    // some edge cases
    {
        StringSplitter splitter(",", ',');
        // Note that even though "" is nullptr right away, "," becomes two empty strings
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter("log,", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("log"_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter(",log", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal("log"_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter(",,,", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(""_span));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
}

void TestNullResilience(nlTestSuite * inSuite, void * inContext)
{
    {
        StringSplitter splitter(nullptr, ',');
        CharSpan span;
        NL_TEST_ASSERT(inSuite, !splitter.Next(span));
        NL_TEST_ASSERT(inSuite, span.data() == nullptr);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSplitter", TestStrdupSplitter),       //
    NL_TEST_DEF("TestNullResilience", TestNullResilience), //
    NL_TEST_SENTINEL()                                     //
};

} // namespace

int TestStringSplitter()
{
    nlTestSuite theSuite = { "StringSplitter", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStringSplitter)
