// Unify bridge components

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

static void TestExample(nlTestSuite * inSuite, void * aContext)
{
    NL_TEST_ASSERT(inSuite, true);
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
    uint32_t mNumTimersHandled;
};

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Example::TestExample",             TestExample),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "ExampleTests",
    &sTests[0],
    nullptr,
    nullptr
};

int TestExampleSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestExampleSuite)