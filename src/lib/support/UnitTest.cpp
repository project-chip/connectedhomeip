#include "UnitTest.h"

#include "pw_unit_test/framework.h"
#include "pw_unit_test/googletest_style_event_handler.h"
#include "pw_unit_test/logging_event_handler.h"
#include <pw_log/log.h>

#include <system/SystemClock.h>

namespace chip {
namespace test {

class SuccessTimingLoggingEventHandler : public pw::unit_test::LoggingEventHandler
{
public:
    SuccessTimingLoggingEventHandler() : LoggingEventHandler(false), mTestStartTime(0) {}

    void TestCaseStart(const pw::unit_test::TestCase & test_case) override
    {
        // Store the start time
        mTestStartTime = System::Clock::Milliseconds32(System::SystemClock().GetMonotonicTimestamp()).count();

        // Call the parent implementation to keep default behavior
        LoggingEventHandler::TestCaseStart(test_case);
    }

    void TestCaseEnd(const pw::unit_test::TestCase & test_case, pw::unit_test::TestResult result) override
    {
        // Calculate elapsed time
        uint32_t endTime   = System::Clock::Milliseconds32(System::SystemClock().GetMonotonicTimestamp()).count();
        uint32_t elapsedMs = endTime - mTestStartTime;

        if (result == pw::unit_test::TestResult::kSuccess)
        {
            PW_LOG_INFO("[       OK ] %s.%s (%" PRIu32 " ms)", test_case.suite_name, test_case.test_name, elapsedMs);
            return;
        }

        LoggingEventHandler::TestCaseEnd(test_case, result);
    }

private:
    uint32_t mTestStartTime;
};

int RunAllTests()
{
    testing::InitGoogleTest(nullptr, static_cast<char **>(nullptr));
    SuccessTimingLoggingEventHandler handler;
    pw::unit_test::RegisterEventHandler(&handler);
    return RUN_ALL_TESTS();
}

} // namespace test
} // namespace chip
