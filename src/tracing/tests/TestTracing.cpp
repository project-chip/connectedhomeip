/*
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
#include <lib/support/UnitTestRegistration.h>
#include <tracing/backend.h>
#include <tracing/macros.h>

#include <nlunit-test.h>

#include <algorithm>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Tracing;

namespace {

// This keeps a log of all received trace items
class LoggingTraceBackend : public Backend
{
public:
    LoggingTraceBackend() {}
    const std::vector<std::string> & traces() const { return mTraces; }

    // Implementation
    void TraceBegin(const char * label, const char * group) override
    {
        mTraces.push_back(std::string("BEGIN:") + group + ":" + label);
    }

    void TraceEnd(const char * label, const char * group) override { mTraces.push_back(std::string("END:") + group + ":" + label); }

    void TraceInstant(const char * label, const char * group) override
    {
        mTraces.push_back(std::string("INSTANT:") + group + ":" + label);
    }

private:
    std::vector<std::string> mTraces;
};

void TestBasicTracing(nlTestSuite * inSuite, void * inContext)
{
    LoggingTraceBackend backend;

    {
        ScopedRegistration scope(backend);

        MATTER_TRACE_SCOPE("A", "Group");
        {
            MATTER_TRACE_SCOPE("B", "Group");

            // direct scope begin/end (not usual, but should work)
            MATTER_TRACE_BEGIN("C", "Group");
            MATTER_TRACE_BEGIN("D", "Group");
            MATTER_TRACE_END("D", "Group");
            MATTER_TRACE_INSTANT("FOO", "Group");
            MATTER_TRACE_END("C", "Group");
        }
        {
            MATTER_TRACE_SCOPE("E", "Group");
        }
    }

    std::vector<std::string> expected = {
        "BEGIN:Group:A", "BEGIN:Group:B", "BEGIN:Group:C", "BEGIN:Group:D", "END:Group:D", "INSTANT:Group:FOO",
        "END:Group:C",   "END:Group:B",   "BEGIN:Group:E", "END:Group:E",   "END:Group:A",
    };

    NL_TEST_ASSERT(inSuite, backend.traces().size() == expected.size());
    NL_TEST_ASSERT(inSuite, std::equal(backend.traces().begin(), backend.traces().end(), expected.begin(), expected.end()));
}

void TestMultipleBackends(nlTestSuite * inSuite, void * inContext)
{
    LoggingTraceBackend b1;
    LoggingTraceBackend b2;
    LoggingTraceBackend b3;

    {
        ScopedRegistration register1(b1);
        MATTER_TRACE_SCOPE("1", "G");

        {
            ScopedRegistration register2(b2);
            MATTER_TRACE_SCOPE("2", "G");

            {
                ScopedRegistration register3(b3);
                MATTER_TRACE_SCOPE("3", "G");
            }
            {
                MATTER_TRACE_SCOPE("4", "G");
            }
        }
    }

    std::vector<std::string> expected1 = {
        "BEGIN:G:1", "BEGIN:G:2", "BEGIN:G:3", "END:G:3", "BEGIN:G:4", "END:G:4", "END:G:2", "END:G:1",
    };

    NL_TEST_ASSERT(inSuite, b1.traces().size() == expected1.size());
    NL_TEST_ASSERT(inSuite, std::equal(b1.traces().begin(), b1.traces().end(), expected1.begin(), expected1.end()));

    std::vector<std::string> expected2 = {
        "BEGIN:G:2", "BEGIN:G:3", "END:G:3", "BEGIN:G:4", "END:G:4", "END:G:2",
    };

    NL_TEST_ASSERT(inSuite, b2.traces().size() == expected2.size());
    NL_TEST_ASSERT(inSuite, std::equal(b2.traces().begin(), b2.traces().end(), expected2.begin(), expected2.end()));

    std::vector<std::string> expected3 = {
        "BEGIN:G:3",
        "END:G:3",
    };

    NL_TEST_ASSERT(inSuite, b3.traces().size() == expected3.size());
    NL_TEST_ASSERT(inSuite, std::equal(b3.traces().begin(), b3.traces().end(), expected3.begin(), expected3.end()));
}

const nlTest sTests[] = {
    NL_TEST_DEF("BasicTracing", TestBasicTracing),              //
    NL_TEST_DEF("BasicMultipleBackends", TestMultipleBackends), //
    NL_TEST_SENTINEL()                                          //
};

} // namespace

int TestTracing()
{
    nlTestSuite theSuite = { "Tracing tests", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTracing)
