/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("single")));

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
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("one")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("two")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("three")));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data() == nullptr);
    }

    // mixed
    {
        StringSplitter splitter("a**bc*d,e*f", '*');

        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("a")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("bc")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("d,e")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("f")));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }

    // some edge cases
    {
        StringSplitter splitter(",", ',');
        // Note that even though "" is nullptr right away, "," becomes two empty strings
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter("log,", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("log")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter(",log", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("log")));
        NL_TEST_ASSERT(inSuite, !splitter.Next(out));
    }
    {
        StringSplitter splitter(",,,", ',');
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
        NL_TEST_ASSERT(inSuite, splitter.Next(out));
        NL_TEST_ASSERT(inSuite, out.data_equal(CharSpan::fromCharString("")));
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
