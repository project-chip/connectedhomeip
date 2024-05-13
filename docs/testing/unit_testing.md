# Unit testing

## Why?

-   MUCH faster than integration tests.
-   Runs as part of build process.
-   Allows testing specific error conditions that are difficult to trigger under
    normal operating conditions.
    -   e.g. out of memory errors etc.
-   Allows testing different device compositions without defining multiple
    example applications.
    -   e.g. feature combinations not in example apps.

## Unit testing in the SDK - nlUnitTest

The following example gives a small demonstration of how to use nlUnitTest to
write a unit test

```
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

class YourTestContext : public Test::AppContext {
   ...
};


static void TestName(nlTestSuite * apSuite, void * apContext) {
    // If you register the test suite with a context, cast
    // apContext as appropriate
    YourTestContext * ctx = static_cast<YourTestContext *>(aContext);

    // Do some test things here, then check the results using NL_TEST_ASSERT
    NL_TEST_ASSERT(apSuite, <boolean condition>)
}

static const nlTest sTests[] =
{
    NL_TEST_DEF("TestName", TestName), // Can have multiple of these
    NL_TEST_SENTINEL() // If you forget this, you’re going to have a bad time
};

nlTestSuite sSuite =
{
    "TheNameOfYourTestSuite", // Test name
    &sTests[0], // The list of tests to run
    TestContext::Initialize, // Runs before all the tests (can be nullptr)
    TestContext::Finalize // Runs after all the tests (can be nullptr)
};

int YourTestSuiteName()
{
    return chip::ExecuteTestsWithContext<YourTestContext>(&sSuite); // or “without”
}

CHIP_REGISTER_TEST_SUITE(YourTestSuiteName)
```

Each test gets an nlTestSuite object (apSuite) that is passed into the test
assertions, and a void\* context (apContext) that is yours to do with as you
require.

The apContext should be derived from
[Test::AppContext](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/AppTestContext.h)

See
[TestSpan.cpp](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/tests/TestSpan.cpp)
for a great example of a good unit test.

## nlUnitTest - Compiling and running

-   Add to src/some_directory/tests/BUILD.gn
    -   chip_test_suite_using_nltest("tests")
        -   See for example
            [src/lib/support/tests/BUILD.gn](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/tests/BUILD.gn)
-   [./gn_build.sh](https://github.com/project-chip/connectedhomeip/blob/master/gn_build.sh)
    will build and run all tests
    -   CI runs this, so any unit tests that get added will automatically be
        added to the CI
-   Test binaries are compiled into:
    -   out/debug/<host_compiler>/tests
    -   e.g. out/debug/linux_x64_clang/tests
-   Tests are run when ./gn_build.sh runs, but you can run them individually in
    a debugger from their location.

## Debugging unit tests

-   After running ./gn_build.sh, test binaries are compiled into
    -   out/debug/<host_compiler>/tests
    -   e.g. out/debug/linux_x64_clang/tests
-   Individual binaries, can be run through regular tools:
    -   gdb
    -   valgrind
    -   Your favorite tool that you tell everyone about.

## Utilities

We have a small number of unit testing utilities that should be used in unit
tests.

Consider adding more utilities for general use if you require them for your
tests.

### Mock clock

The mock clock is located in
[src/system/SystemClock.h](https://github.com/project-chip/connectedhomeip/blob/master/src/system/SystemClock.h)
as `System::Clock::Internal::MockClock`.

To use the mock clock, use the `chip::System::SystemClock()` function as normal.
In the test, instantiate a MockClock and use the `SetSystemClockForTesting` to
inject the clock. The Set and Advance functions in the MockClock can then be
used to set exact times for testing. This allows testing specific edge
conditions in tests, helps reduce test flakiness due to race conditions, and
reduces the time required for testing as tests no long require real-time waits.

### TestPersistentStorageDelegate

The TestPersistentStorageDelegate is an in-memory version of storage that easily
allows removal of keys, presence checks, etc. It is available at
[src/lib/support/TestPersistentStorageDelegate.h](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/TestPersistentStorageDelegate.h)
