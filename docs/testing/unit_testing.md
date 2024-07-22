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

## Unit testing in the SDK using pw_unit_test

This SDK uses Pigweed unit test (pw_unit_test), which is an implementation of
GoogleTest. For more information see the
[pw_unit_test documentation](https://pigweed.dev/pw_unit_test/) or the
[GoogleTest documentation](https://google.github.io/googletest/).

### Simple unit tests

The following example demonstrates how to use pw_unit_test to write a simple
unit test. Each test function is defined using `TEST(NameOfFunction)`. The set
of test functions in a given source file is called a "suite".

```
#include <pw_unit_test/framework.h>

TEST(YourTestFunction1)
{
    // Do some test things here, then check the results using EXPECT_*
    SomeTypeX foo;
    foo.DoSomething();
    EXPECT_EQ(foo.GetResultCount(), 7);
    foo.DoSomethingElse();
    EXPECT_EQ(foo.GetResultCount(), 5);

    // If you want to abort the rest of the test upon failure, use ASSERT_*
    SomeTypeY * ptr = foo.GetSomePointer();
    ASSERT_NE(ptr, nullptr);
    ptr->DoTheThing();  // Won't reach here if the ASSERT failed.
}

TEST(YourTestFunction2)
{
    // Do some test things here, then check the results using EXPECT_*
    SomeTypeZ foo;
    foo.DoSomething();
    EXPECT_EQ(foo.GetResultCount(), 3);
}
```

See
[TestSpan.cpp](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/tests/TestSpan.cpp)
for an example of a simple unit test.

In the above example there are no fixtures or setup/teardown behavior.

### Test fixtures and setup/teardown behavior

If your tests need fixtures or some kind of setup/teardown you will need to
define a test context that derives from `::testing::Test`. Each of your test
functions will be defined with `TEST_F(NameOfTestContext, NameOfFunction)`. The
following example demonstrates how to use pw_unit_test to write a unit test that
uses fixtures and setup/teardown behavior.

```
#include <pw_unit_test/framework.h>

class YourTestContext : public ::testing::Test
{
public:
    // Performs shared setup for all tests in the test suite.  Run once for the whole suite.
    static void SetUpTestSuite()
    {
        // Your per-suite setup goes here:
        sPerSuiteFixture.Init();
        ASSERT_TRUE(sPerSuiteFixture.WorkingGreat());
    }

    // Performs shared teardown for all tests in the test suite.  Run once for the whole suite.
    static void TearDownTestSuite()
    {
        // Your per-suite teardown goes here:
        sPerSuiteFixture.Shutdown();
    }

protected:
    // Performs setup for each test in the suite.  Run once for each test function.
    void SetUp()
    {
        // Your per-test setup goes here:
        mPerTestFixture.Init();
        ASSERT_TRUE(mPerTestFixture.WorkingGreat());
    }

    // Performs teardown for each test in the suite.  Run once for each test function.
    void TearDown()
    {
        // Your per-test teardown goes here:
        mPerTestFixture.Shutdown();
    }

private:
    // Your per-suite and per-test fixtures are declared here:
    static SomeTypeA sPerSuiteFixture;
    SomeTypeB mPerTestFixture;
};
// Your per-suite fixtures are defined here:
SomeTypeA YourTestContext::sPerSuiteFixture;

TEST_F(YourTestContext, YourTestFunction1)
{
    // Do some test things here, then check the results using EXPECT_*
    mPerTestFixture.DoSomething();
    EXPECT_EQ(mPerTestFixture.GetResultCount(), 7);
    sPerSuiteFixture.DoSomething();
    EXPECT_EQ(sPerSuiteFixture.GetResultCount(), 5);

    // If you want to abort the rest of the test upon failure, use ASSERT_*
    SomeTypeC * ptr = mPerTestFixture.GetSomePointer();
    ASSERT_NE(ptr, nullptr);
    ptr->DoTheThing();  // Won't reach here if the ASSERT failed.
}

TEST_F(YourTestContext, YourTestFunction2)
{
    // Do some test things here, then check the results using EXPECT_*
    mPerTestFixture.DoSomethingElse();
    EXPECT_EQ(mPerTestFixture.GetResultCount(), 9);
}
```

### A loopback messaging context for convenience

If you need messaging, there is a convenience class
[Test::AppContext](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/AppTestContext.h)
that you can derive your test context from. It provides a network layer and a
system layer and two secure sessions connected with each other. The following
example demonstrates this.

```
#include <app/tests/AppTestContext.h>

class YourTestContext : public Test::AppContext
{
public:
    // Performs shared setup for all tests in the test suite.  Run once for the whole suite.
    static void SetUpTestSuite()
    {
        AppContext::SetUpTestSuite();  // Call parent.
        VerifyOrReturn(!HasFailure());  // Stop if parent had a failure.

        // Your per-suite setup goes here:
        sPerSuiteFixture.Init();
        ASSERT_TRUE(sPerSuiteFixture.WorkingGreat());
    }

    // Performs shared teardown for all tests in the test suite.  Run once for the whole suite.
    static void TearDownTestSuite()
    {
        // Your per-suite teardown goes here:
        sPerSuiteFixture.Shutdown();

        AppContext::TearDownTestSuite();  // Call parent.
    }

protected:
    // Performs setup for each test in the suite.  Run once for each test function.
    void SetUp()
    {
        AppContext::SetUp();  // Call parent.
        VerifyOrReturn(!HasFailure());  // Stop if parent had a failure.

        // Your per-test setup goes here:
        mPerTestFixture.Init();
        ASSERT_TRUE(mPerTestFixture.WorkingGreat());
    }

    // Performs teardown for each test in the suite.  Run once for each test function.
    void TearDown()
    {
        // Your per-test teardown goes here:
        mPerTestFixture.Shutdown();

        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();  // Call parent.
    }

private:
    // Your per-suite and per-test fixtures are declared here:
    static SomeTypeA sPerSuiteFixture;
    SomeTypeB mPerTestFixture;
};
// Your per-suite fixtures are defined here:
SomeTypeA YourTestContext::sPerSuiteFixture;

TEST_F(YourTestContext, YourTestFunction1)
{
    // Do some test things here, then check the results using EXPECT_*
}

TEST_F(YourTestContext, YourTestFunction2)
{
    // Do some test things here, then check the results using EXPECT_*
}
```

You don't have to override all 4 functions `SetUpTestsuite`,
`TearDownTestSuite`, `SetUp`, `TearDown`. If you don't need any custom behavior
in one of those functions just omit it.

If you override one of the setup/teardown functions make sure to invoke the
parent's version of the function as well. `AppContext::SetUpTestSuite` and
`AppContext::SetUp` may generate fatal failures, so after you call these from
your overriding function make sure to check `HasFailure()` and return if the
parent function failed.

If you don't override any of the setup/teardown functions, you can simply make a
type alias: `using YourTestContext = Test::AppContext;` instead of defining your
own text context class.

## Best practices

-   Try to use as specific an assertion as possible. For example use these

    ```
    EXPECT_EQ(result, 3);
    EXPECT_GT(result, 1);
    EXPECT_STREQ(myString, "hello");
    ```

    instead of these

    ```
    EXPECT_TRUE(result == 3);
    EXPECT_TRUE(result > 1);
    EXPECT_EQ(strcmp(myString, "hello"), 0);
    ```

-   If you want a test to abort when an assertion fails, use `ASSERT_*` instead
    of `EXPECT_*`. This will cause the current function to return.

-   If a test calls a subroutine which exits due to an ASSERT failing, execution
    of the test will continue after the subroutine call. This is because ASSERT
    causes the subroutine to return (as opposed to throwing an exception). If
    you want to prevent the test from continuing, check the value of
    `HasFailure()` and stop execution if true. Example:

    ```
    void Subroutine()
    {
        ASSERT_EQ(1, 2);  // Fatal failure.
    }

    TEST(YourTestContext, YourTestFunction1)
    {
        Subroutine();  // A fatal failure happens in this subroutine...
        // ... however execution still continues.
        print("This gets executed");
        VerifyOrReturn(!HasFailure());
        print("This does not get executed");
    }
    ```

-   If you want to force a fatal failure use `FAIL()`, which will record a fatal
    failure and exit the current function. This is similar to using
    `ASSERT_TRUE(false)`. If you want to force a non-fatal failure use
    `ADD_FAILURE()`, which will record a non-fatal failure and continue
    executing the current function. This is similar to using
    `EXPECT_TRUE(false)`.

-   `ASSERT_*` and `FAIL` will only work in functions with void return type,
    since they generate a `return;` statement. If you must use these in a
    non-void function, instead use `EXPECT_*` or `ADD_FAILURE` and then check
    `HasFailure()` afterward and return if needed.

-   If your test requires access to private/protected members of the underlying
    class you're testing, you'll need to create an accessor class that performs
    these operations and is friended to the underlying class. Please name the
    class `chip::Test::SomethingTestAccess` where `Something` is the name of the
    underlying class whose private/protected members you're trying to access.
    Then add `friend class chip::Test::SomethingTestAccess;` to the underlying
    class. Make sure your test's BUILD.gn file contains
    `sources = [ "SomethingTestAccess.h" ]`. Before creating a new TestAccess
    class, check if one already exists. If it does exist but doesn't expose the
    member you need, you can add a function to that class to do so. Note that
    you should make these functions as minimal as possible, with no logic
    besides just exposing the private/protected member.
    -   For an example see `ICDConfigurationDataTestAccess` which is defined in
        [ICDConfigurationDataTestAccess.h](https://github.com/project-chip/connectedhomeip/blob/master/src/app/icd/server/tests/ICDConfigurationDataTestAccess.h),
        friends the underlying class in
        [ICDConfigurationData.h](https://github.com/project-chip/connectedhomeip/blob/master/src/app/icd/server/ICDConfigurationData.h),
        is included as a source in
        [BUILD.gn](https://github.com/project-chip/connectedhomeip/blob/master/src/app/icd/server/tests/BUILD.gn),
        and is used by a test in
        [TestICDManager.cpp](https://github.com/project-chip/connectedhomeip/blob/master/src/app/icd/server/tests/TestICDManager.cpp).
    -   For another example see `TCPBaseTestAccess` which is defined in
        [TCPBaseTestAccess.h](https://github.com/project-chip/connectedhomeip/blob/master/src/transport/raw/tests/TCPBaseTestAccess.h),
        friends the underlying class in
        [TCP.h](https://github.com/project-chip/connectedhomeip/blob/master/src/transport/raw/TCP.h),
        is included as a source in
        [BUILD.gn](https://github.com/project-chip/connectedhomeip/blob/master/src/transport/raw/tests/BUILD.gn),
        and is used by a test in
        [TestTCP.cpp](https://github.com/project-chip/connectedhomeip/blob/master/src/transport/raw/tests/TestTCP.cpp).

## Compiling and running

-   Add to `src/some_directory/tests/BUILD.gn`

    -   Example

        ```
        chip_test_suite("tests") {
            output_name = "libSomethingTests"

            test_sources = [
                "TestSuite1.cpp",
                "TestSuite2.cpp",
                // Other test source files go here.
            ]

            sources = [
                // Non-test source files go here.
            ]

            cflags = [ "-Wconversion" ]

            public_deps = [
                // Dependencies go here.
            ]
        }
        ```

    -   Another example:
        [src/lib/support/tests/BUILD.gn](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/tests/BUILD.gn)

-   Build and run all tests with
    [./gn_build.sh](https://github.com/project-chip/connectedhomeip/blob/master/gn_build.sh)
    -   CI runs this, so any unit tests that get added will automatically be
        added to the CI.
-   Test binaries are compiled into:
    -   `out/debug/<host_compiler>/tests`
    -   e.g. `out/debug/linux_x64_clang/tests`
-   Tests are run when `./gn_build.sh` runs, but you can run them individually
    in a debugger from their location.

## Debugging unit tests

-   After running `./gn_build.sh`, test binaries are compiled into
    -   `out/debug/<host_compiler>/tests`
    -   e.g. `out/debug/linux_x64_clang/tests`
-   Individual binaries can be run through regular tools:
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
