#include "UnitTest.h"

#include "pw_unit_test/framework.h"
#include "pw_unit_test/logging_event_handler.h"

namespace chip {
namespace test {

int RunAllTests()
{
    testing::InitGoogleTest(nullptr, static_cast<char **>(nullptr));
    pw::unit_test::LoggingEventHandler handler;
    pw::unit_test::RegisterEventHandler(&handler);
    return RUN_ALL_TESTS();
}

} // namespace test
} // namespace chip
