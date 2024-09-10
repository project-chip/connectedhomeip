/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <tracing/backend.h>
#include <tracing/metric_event.h>

#include <algorithm>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Tracing;

namespace chip {
namespace Tracing {

static bool operator==(const MetricEvent & lhs, const MetricEvent & rhs)
{
    if (&lhs == &rhs)
    {
        return true;
    }

    if (lhs.type() == rhs.type() && std::string(lhs.key()) == std::string(rhs.key()) && lhs.ValueType() == rhs.ValueType())
    {
        switch (lhs.ValueType())
        {
        case MetricEvent::Value::Type::kInt32:
            return lhs.ValueInt32() == rhs.ValueInt32();

        case MetricEvent::Value::Type::kUInt32:
            return lhs.ValueUInt32() == rhs.ValueUInt32();

        case MetricEvent::Value::Type::kChipErrorCode:
            return lhs.ValueErrorCode() == rhs.ValueErrorCode();

        case MetricEvent::Value::Type::kUndefined:
            return true;
        }
    }
    return false;
}
} // namespace Tracing
} // namespace chip

namespace {

// This keeps a log of all received trace items
class MetricEventBackend : public Backend
{
public:
    MetricEventBackend() {}
    const std::vector<MetricEvent> & GetMetricEvents() const { return mMetricEvents; }

    // Implementation
    virtual void LogMetricEvent(const MetricEvent & event) { mMetricEvents.push_back(event); }

private:
    std::vector<MetricEvent> mMetricEvents;
};

TEST(TestMetricEvents, TestBasicMetricEvent)
{

    {
        MetricEvent event(MetricEvent::Type::kInstantEvent, "instant_event");
        EXPECT_EQ(event.type(), MetricEvent::Type::kInstantEvent);
        EXPECT_EQ(std::string(event.key()), std::string("instant_event"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kUndefined);
    }

    {
        MetricEvent event(MetricEvent::Type::kBeginEvent, "begin_event");
        EXPECT_EQ(event.type(), MetricEvent::Type::kBeginEvent);
        EXPECT_EQ(std::string(event.key()), std::string("begin_event"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kUndefined);
    }

    {
        MetricEvent event(MetricEvent::Type::kEndEvent, "end_event");
        EXPECT_EQ(event.type(), MetricEvent::Type::kEndEvent);
        EXPECT_EQ(std::string(event.key()), std::string("end_event"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kUndefined);
    }

    {
        MetricEvent event(MetricEvent::Type::kEndEvent, "end_event_with_int32_value", int32_t(42));
        EXPECT_EQ(event.type(), MetricEvent::Type::kEndEvent);
        EXPECT_EQ(std::string(event.key()), std::string("end_event_with_int32_value"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kInt32);
        EXPECT_EQ(event.ValueInt32(), 42);
    }

    {
        MetricEvent event(MetricEvent::Type::kEndEvent, "end_event_with_uint32_value", uint32_t(42));
        EXPECT_EQ(event.type(), MetricEvent::Type::kEndEvent);
        EXPECT_EQ(std::string(event.key()), std::string("end_event_with_uint32_value"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kUInt32);
        EXPECT_EQ(event.ValueUInt32(), 42u);
    }

    {
        MetricEvent event(MetricEvent::Type::kEndEvent, "end_event_with_error_value", CHIP_ERROR_BUSY);
        EXPECT_EQ(event.type(), MetricEvent::Type::kEndEvent);
        EXPECT_EQ(std::string(event.key()), std::string("end_event_with_error_value"));
        EXPECT_EQ(event.ValueType(), MetricEvent::Value::Type::kChipErrorCode);
        EXPECT_EQ(chip::ChipError(event.ValueErrorCode()), CHIP_ERROR_BUSY);
    }
}

TEST(TestMetricEvents, TestInstantMetricEvent)
{
    MetricEventBackend backend;

    {
        ScopedRegistration scope(backend);

        MATTER_LOG_METRIC("event1");
        MATTER_LOG_METRIC("event2");
        MATTER_LOG_METRIC("event3");
    }

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1"),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2"),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3"),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

TEST(TestMetricEvents, TestBeginEndMetricEvent)
{
    MetricEventBackend backend1;
    MetricEventBackend backend2;

    {
        ScopedRegistration scope1(backend1);
        {

            MATTER_LOG_METRIC_BEGIN("event1");
            MATTER_LOG_METRIC_BEGIN("event2");
            MATTER_LOG_METRIC_END("event2", 53);
            MATTER_LOG_METRIC_END("event1");
        }

        std::vector<MetricEvent> expected1 = {
            MetricEvent(MetricEvent::Type::kBeginEvent, "event1"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event2"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event2", 53),
            MetricEvent(MetricEvent::Type::kEndEvent, "event1"),
        };

        EXPECT_EQ(backend1.GetMetricEvents().size(), expected1.size());
        EXPECT_TRUE(
            std::equal(backend1.GetMetricEvents().begin(), backend1.GetMetricEvents().end(), expected1.begin(), expected1.end()));

        {
            ScopedRegistration scope2(backend2);

            MATTER_LOG_METRIC_BEGIN("event1");
            MATTER_LOG_METRIC_BEGIN("event2");
            MATTER_LOG_METRIC_BEGIN("event3");
            MATTER_LOG_METRIC_BEGIN("event4");
            MATTER_LOG_METRIC_END("event3", CHIP_ERROR_UNKNOWN_KEY_TYPE);
            MATTER_LOG_METRIC_END("event1", 91u);
            MATTER_LOG_METRIC_END("event2", 53);
            MATTER_LOG_METRIC_END("event4");
        }

        std::vector<MetricEvent> expected2 = {
            MetricEvent(MetricEvent::Type::kBeginEvent, "event1"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event2"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event3"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event4"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event3", CHIP_ERROR_UNKNOWN_KEY_TYPE),
            MetricEvent(MetricEvent::Type::kEndEvent, "event1", 91u),
            MetricEvent(MetricEvent::Type::kEndEvent, "event2", 53),
            MetricEvent(MetricEvent::Type::kEndEvent, "event4"),
        };

        EXPECT_EQ(backend2.GetMetricEvents().size(), expected2.size());
        EXPECT_TRUE(
            std::equal(backend2.GetMetricEvents().begin(), backend2.GetMetricEvents().end(), expected2.begin(), expected2.end()));
    }
}

TEST(TestMetricEvents, TestScopedMetricEvent)
{
    MetricEventBackend backend1;
    MetricEventBackend backend2;
    MetricEventBackend backend3;
    chip::ChipError err1 = CHIP_NO_ERROR;
    chip::ChipError err2 = CHIP_NO_ERROR;
    chip::ChipError err3 = CHIP_NO_ERROR;
    chip::ChipError err4 = CHIP_NO_ERROR;

    {
        ScopedRegistration scope1(backend1);
        {
            MATTER_LOG_METRIC_SCOPE("event1", err1);
            err1 = CHIP_ERROR_BUSY;
            {
                ScopedRegistration scope2(backend2);
                MATTER_LOG_METRIC_SCOPE("event2", err2);
                err2 = CHIP_ERROR_BAD_REQUEST;

                {
                    ScopedRegistration scope3(backend3);
                    MATTER_LOG_METRIC_SCOPE("event3", err3);
                    err3 = CHIP_ERROR_EVENT_ID_FOUND;
                }
                {
                    MATTER_LOG_METRIC_SCOPE("event4", err4);
                    err4 = CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
        }

        std::vector<MetricEvent> expected1 = {
            MetricEvent(MetricEvent::Type::kBeginEvent, "event1"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event2"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event3"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event3", CHIP_ERROR_EVENT_ID_FOUND),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event4"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event4", CHIP_ERROR_BUFFER_TOO_SMALL),
            MetricEvent(MetricEvent::Type::kEndEvent, "event2", CHIP_ERROR_BAD_REQUEST),
            MetricEvent(MetricEvent::Type::kEndEvent, "event1", CHIP_ERROR_BUSY),
        };

        EXPECT_EQ(backend1.GetMetricEvents().size(), expected1.size());
        EXPECT_TRUE(
            std::equal(backend1.GetMetricEvents().begin(), backend1.GetMetricEvents().end(), expected1.begin(), expected1.end()));

        std::vector<MetricEvent> expected2 = {
            MetricEvent(MetricEvent::Type::kBeginEvent, "event2"),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event3"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event3", CHIP_ERROR_EVENT_ID_FOUND),
            MetricEvent(MetricEvent::Type::kBeginEvent, "event4"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event4", CHIP_ERROR_BUFFER_TOO_SMALL),
            MetricEvent(MetricEvent::Type::kEndEvent, "event2", CHIP_ERROR_BAD_REQUEST),
        };

        EXPECT_EQ(backend2.GetMetricEvents().size(), expected2.size());
        EXPECT_TRUE(
            std::equal(backend2.GetMetricEvents().begin(), backend2.GetMetricEvents().end(), expected2.begin(), expected2.end()));

        std::vector<MetricEvent> expected3 = {
            MetricEvent(MetricEvent::Type::kBeginEvent, "event3"),
            MetricEvent(MetricEvent::Type::kEndEvent, "event3", CHIP_ERROR_EVENT_ID_FOUND),
        };

        EXPECT_EQ(backend3.GetMetricEvents().size(), expected3.size());
        EXPECT_TRUE(
            std::equal(backend3.GetMetricEvents().begin(), backend3.GetMetricEvents().end(), expected3.begin(), expected3.end()));
    }
}

static int DoubleOf(int input)
{
    return input * 2;
}

TEST(TestMetricEvents, TestVerifyOrExitWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);
    chip::ChipError err = CHIP_NO_ERROR;

    VerifyOrExitWithMetric("event0", DoubleOf(2) == 4, err = CHIP_ERROR_BAD_REQUEST);
    VerifyOrExitWithMetric("event1", DoubleOf(3) == 9, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_INCORRECT_STATE),
    };

    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

TEST(TestMetricEvents, TestSuccessOrExitWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);
    chip::ChipError err = CHIP_NO_ERROR;

    SuccessOrExitWithMetric("event1", err = CHIP_NO_ERROR);
    SuccessOrExitWithMetric("event2", err = CHIP_ERROR_BUSY);
    SuccessOrExitWithMetric("event3", err = CHIP_NO_ERROR);

exit:
    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_BUSY),
    };

    EXPECT_EQ(err, CHIP_ERROR_BUSY);
    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static CHIP_ERROR InvokeReturnErrorOnFailureWithMetric(MetricKey key, const CHIP_ERROR & error)
{
    ReturnErrorOnFailureWithMetric(key, error);
    return CHIP_NO_ERROR;
}

TEST(TestMetricEvents, TestReturnErrorOnFailureWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    CHIP_ERROR err = InvokeReturnErrorOnFailureWithMetric("event0", CHIP_NO_ERROR);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeReturnErrorOnFailureWithMetric("event1", CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    err = InvokeReturnErrorOnFailureWithMetric("event2", CHIP_ERROR_BAD_REQUEST);
    EXPECT_EQ(err, CHIP_ERROR_BAD_REQUEST);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_INCORRECT_STATE),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_BAD_REQUEST),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static CHIP_ERROR InvokeReturnLogErrorOnFailureWithMetric(MetricKey key, const CHIP_ERROR & error)
{
    ReturnLogErrorOnFailureWithMetric(key, error);
    return CHIP_NO_ERROR;
}

TEST(TestMetricEvents, TestReturnLogErrorOnFailureWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    CHIP_ERROR err = InvokeReturnLogErrorOnFailureWithMetric("event0", CHIP_NO_ERROR);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeReturnLogErrorOnFailureWithMetric("event1", CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    err = InvokeReturnLogErrorOnFailureWithMetric("event2", CHIP_ERROR_BAD_REQUEST);
    EXPECT_EQ(err, CHIP_ERROR_BAD_REQUEST);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_INCORRECT_STATE),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_BAD_REQUEST),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static void InvokeReturnOnFailureWithMetric(MetricKey key, const CHIP_ERROR & error)
{
    ReturnOnFailureWithMetric(key, error);
    return;
}

TEST(TestMetricEvents, TestReturnOnFailureWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    InvokeReturnOnFailureWithMetric("event0", CHIP_NO_ERROR);

    InvokeReturnOnFailureWithMetric("event1", CHIP_ERROR_INCORRECT_STATE);

    InvokeReturnOnFailureWithMetric("event2", CHIP_ERROR_BAD_REQUEST);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_INCORRECT_STATE),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_BAD_REQUEST),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static void InvokeVerifyOrReturnWithMetric(MetricKey key, bool result)
{
    VerifyOrReturnWithMetric(key, result);
    return;
}

TEST(TestMetricEvents, TestInvokeVerifyOrReturnWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    InvokeVerifyOrReturnWithMetric("event0", DoubleOf(2) == 4);
    InvokeVerifyOrReturnWithMetric("event1", DoubleOf(3) == 9);
    InvokeVerifyOrReturnWithMetric("event2", DoubleOf(4) == 8);
    InvokeVerifyOrReturnWithMetric("event3", DoubleOf(5) == 11);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", false),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", false),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static CHIP_ERROR InvokeVerifyOrReturnErrorWithMetric(MetricKey key, bool expr, const CHIP_ERROR & error)
{
    VerifyOrReturnErrorWithMetric(key, expr, error);
    return CHIP_NO_ERROR;
}

TEST(TestMetricEvents, TestVerifyOrReturnErrorWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    CHIP_ERROR err = InvokeVerifyOrReturnErrorWithMetric("event0", DoubleOf(2) == 4, CHIP_ERROR_BAD_REQUEST);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeVerifyOrReturnErrorWithMetric("event1", DoubleOf(3) == 9, CHIP_ERROR_ACCESS_DENIED);
    EXPECT_EQ(err, CHIP_ERROR_ACCESS_DENIED);

    err = InvokeVerifyOrReturnErrorWithMetric("event2", DoubleOf(4) == 8, CHIP_ERROR_BUSY);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeVerifyOrReturnErrorWithMetric("event3", DoubleOf(5) == 11, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(err, CHIP_ERROR_CANCELLED);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_ACCESS_DENIED),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", CHIP_ERROR_CANCELLED),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

template <typename return_code_type>
static return_code_type InvokeVerifyOrReturnValueWithMetric(MetricKey key, bool expr, return_code_type retval)
{
    VerifyOrReturnValueWithMetric(key, expr, retval);
    return return_code_type();
}

TEST(TestMetricEvents, TestVerifyOrReturnValueWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    auto retval = InvokeVerifyOrReturnValueWithMetric("event0", DoubleOf(2) == 4, 0);
    EXPECT_EQ(retval, 0);

    auto err = InvokeVerifyOrReturnValueWithMetric("event1", DoubleOf(3) == 9, CHIP_ERROR_ACCESS_DENIED);
    EXPECT_EQ(err, CHIP_ERROR_ACCESS_DENIED);

    err = InvokeVerifyOrReturnValueWithMetric("event2", DoubleOf(4) == 8, CHIP_ERROR_BAD_REQUEST);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    retval = InvokeVerifyOrReturnValueWithMetric("event3", DoubleOf(5) == 11, 16);
    EXPECT_EQ(retval, 16);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_ACCESS_DENIED),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", 16),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

static CHIP_ERROR InvokeVerifyOrReturnLogErrorWithMetric(MetricKey key, bool expr, const CHIP_ERROR & error)
{
    VerifyOrReturnLogErrorWithMetric(key, expr, error);
    return CHIP_NO_ERROR;
}

TEST(TestMetricEvents, TestVerifyOrReturnLogErrorWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    auto err = InvokeVerifyOrReturnLogErrorWithMetric("event0", DoubleOf(2) == 4, CHIP_NO_ERROR);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeVerifyOrReturnLogErrorWithMetric("event1", DoubleOf(3) == 9, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(err, CHIP_ERROR_CANCELLED);

    err = InvokeVerifyOrReturnLogErrorWithMetric("event2", DoubleOf(4) == 8, CHIP_NO_ERROR);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InvokeVerifyOrReturnLogErrorWithMetric("event3", DoubleOf(5) == 11, CHIP_ERROR_BAD_REQUEST);
    EXPECT_EQ(err, CHIP_ERROR_BAD_REQUEST);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event1", CHIP_ERROR_CANCELLED),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", CHIP_ERROR_BAD_REQUEST),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

template <typename return_code_type>
static return_code_type InvokeReturnErrorCodeWithMetricIf(MetricKey key, bool expr, const return_code_type & code)
{
    ReturnErrorCodeWithMetricIf(key, expr, code);
    return return_code_type();
}

TEST(TestMetricEvents, TestReturnErrorCodeWithMetricIf)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    auto err = InvokeReturnErrorCodeWithMetricIf("event0", DoubleOf(2) == 4, CHIP_ERROR_DUPLICATE_KEY_ID);
    EXPECT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);

    auto retval = InvokeReturnErrorCodeWithMetricIf("event1", DoubleOf(3) == 9, 11);
    EXPECT_EQ(retval, 0);

    retval = InvokeReturnErrorCodeWithMetricIf("event2", DoubleOf(4) == 8, 22);
    EXPECT_EQ(retval, 22);

    err = InvokeReturnErrorCodeWithMetricIf("event3", DoubleOf(5) == 11, CHIP_ERROR_ACCESS_DENIED);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event0", CHIP_ERROR_DUPLICATE_KEY_ID),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", 22),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

TEST(TestMetricEvents, TestExitNowWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);
    chip::ChipError err = CHIP_NO_ERROR;

    ExitNowWithMetric("event0", err = CHIP_ERROR_BUSY);

exit:
    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event0"),
    };

    EXPECT_EQ(err, CHIP_ERROR_BUSY);
    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

TEST(TestMetricEvents, TestLogErrorOnFailureWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    LogErrorOnFailureWithMetric("event0", CHIP_ERROR_BAD_REQUEST);
    LogErrorOnFailureWithMetric("event1", CHIP_NO_ERROR);
    LogErrorOnFailureWithMetric("event2", CHIP_ERROR_DATA_NOT_ALIGNED);
    LogErrorOnFailureWithMetric("event3", CHIP_ERROR_BUSY);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event0", CHIP_ERROR_BAD_REQUEST),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_DATA_NOT_ALIGNED),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", CHIP_ERROR_BUSY),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

TEST(TestMetricEvents, TestVerifyOrDoWithMetric)
{
    MetricEventBackend backend;
    ScopedRegistration scope(backend);

    VerifyOrDoWithMetric("event0", DoubleOf(2) == 5, 11);
    VerifyOrDoWithMetric("event1", DoubleOf(3) == 6, 12);
    VerifyOrDoWithMetric("event2", DoubleOf(4) == 7, CHIP_ERROR_BAD_REQUEST, MATTER_LOG_METRIC("event4", 10));
    VerifyOrDoWithMetric("event3", DoubleOf(5) == 8, 13);
    VerifyOrDoWithMetric("event5", DoubleOf(6) == 12, 14);

    std::vector<MetricEvent> expected = {
        MetricEvent(MetricEvent::Type::kInstantEvent, "event0", 11),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event2", CHIP_ERROR_BAD_REQUEST),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event4", 10),
        MetricEvent(MetricEvent::Type::kInstantEvent, "event3", 13),
    };

    EXPECT_EQ(backend.GetMetricEvents().size(), expected.size());
    EXPECT_TRUE(std::equal(backend.GetMetricEvents().begin(), backend.GetMetricEvents().end(), expected.begin(), expected.end()));
}

} // namespace
