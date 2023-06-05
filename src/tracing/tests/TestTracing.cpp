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
#include <tracing/scope.h>

#include <nlunit-test.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::Tracing;

namespace {

// This keeps a log of all received trace items
class LoggingTraceBackend : public Backend
{
public:
    enum class TraceEventType
    {
        BEGIN,
        END
    };

    struct ReceivedTraceEvent
    {
        TraceEventType type;
        Scope scope;

        bool operator==(const ReceivedTraceEvent & other) const { return (type == other.type) && (scope == other.scope); }
    };

    LoggingTraceBackend() {}
    const std::vector<ReceivedTraceEvent> & traces() const { return mTraces; }

    // Implementation
    void TraceBegin(Scope scope) override { mTraces.push_back(ReceivedTraceEvent{ TraceEventType::BEGIN, scope }); }

    void TraceEnd(Scope scope) override { mTraces.push_back(ReceivedTraceEvent{ TraceEventType::END, scope }); }

    void TraceInstant(Instant instant) override
    {
        // NOT SUPPORTED HERE
    }

private:
    std::vector<ReceivedTraceEvent> mTraces;
};

void TestBasicTracing(nlTestSuite * inSuite, void * inContext)
{
    LoggingTraceBackend backend;

    {
        ScopedRegistration scope(backend);

        MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma1);
        {
            MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma2);

            // direct scope begin/end (not usual, but should work)
            MATTER_TRACE_BEGIN(Scope::OperationalCredentials_AddNOC);
            MATTER_TRACE_BEGIN(Scope::OperationalCredentials_UpdateNOC);
            MATTER_TRACE_END(Scope::OperationalCredentials_UpdateNOC);
            MATTER_TRACE_END(Scope::OperationalCredentials_AddNOC);
        }
        {
            MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma3);
        }
    }

    std::vector<LoggingTraceBackend::ReceivedTraceEvent> expected = {
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma1 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma2 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::OperationalCredentials_UpdateNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::OperationalCredentials_UpdateNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma2 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma1 },
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
        MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma1);

        {
            ScopedRegistration register2(b2);
            MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma2);

            {
                ScopedRegistration register3(b3);
                MATTER_TRACE_SCOPE(Scope::CASESession_SendSigma3);
            }
            {
                MATTER_TRACE_SCOPE(Scope::OperationalCredentials_AddNOC);
            }
        }
    }

    std::vector<LoggingTraceBackend::ReceivedTraceEvent> expected1 = {
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma1 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma2 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma2 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma1 },
    };

    NL_TEST_ASSERT(inSuite, b1.traces().size() == expected1.size());
    NL_TEST_ASSERT(inSuite, std::equal(b1.traces().begin(), b1.traces().end(), expected1.begin(), expected1.end()));

    std::vector<LoggingTraceBackend::ReceivedTraceEvent> expected2 = {
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma2 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::OperationalCredentials_AddNOC },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma2 },
    };

    NL_TEST_ASSERT(inSuite, b2.traces().size() == expected2.size());
    NL_TEST_ASSERT(inSuite, std::equal(b2.traces().begin(), b2.traces().end(), expected2.begin(), expected2.end()));

    std::vector<LoggingTraceBackend::ReceivedTraceEvent> expected3 = {
        { LoggingTraceBackend::TraceEventType::BEGIN, Scope::CASESession_SendSigma3 },
        { LoggingTraceBackend::TraceEventType::END, Scope::CASESession_SendSigma3 },
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

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTracing)
