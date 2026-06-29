/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file implements unit tests for the Server port retry functionality.
 */

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>

#include <functional>
#include <limits>

namespace chip {
namespace app {
namespace {

/**
 * Helper function to check if an error is an "address already in use" error
 * (Copied from Server.cpp for testing)
 */
bool IsAddressInUseError(CHIP_ERROR err)
{
    return (err == CHIP_ERROR_POSIX(EADDRINUSE));
}

/**
 * Helper function to safely increment a port number with overflow protection
 * (Copied from Server.cpp for testing)
 */
bool SafePortIncrement(uint16_t basePort, uint16_t increment, uint16_t & outPort)
{
    if (increment > (UINT16_MAX - basePort))
    {
        return false;
    }
    outPort = static_cast<uint16_t>(basePort + increment);
    return true;
}

/**
 * Helper function to initialize a transport with automatic port selection and retry logic
 * (Copied from Server.cpp for testing)
 */
CHIP_ERROR InitTransportWithPortRetry(uint16_t basePort, uint16_t maxRetries, const char * componentName,
                                      std::function<CHIP_ERROR(uint16_t)> initFunction, std::function<void()> closeFunction,
                                      uint16_t & outBoundPort)
{
    uint16_t portToTry     = basePort;
    uint16_t attemptNumber = 0;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    for (;;)
    {
        if (attemptNumber > 0)
        {
            if (!SafePortIncrement(basePort, attemptNumber, portToTry))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        err = initFunction(portToTry);

        if (err == CHIP_NO_ERROR)
        {
            outBoundPort = portToTry;
            break;
        }

        if (IsAddressInUseError(err) && attemptNumber < maxRetries)
        {
            closeFunction();
            attemptNumber++;
            continue;
        }

        break;
    }

    return err;
}

// Test fixture for Server port retry tests
class TestServerPortRetry : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

// Test IsAddressInUseError function
TEST_F(TestServerPortRetry, TestIsAddressInUseError)
{
    // Test that EADDRINUSE is correctly identified
    EXPECT_TRUE(IsAddressInUseError(CHIP_ERROR_POSIX(EADDRINUSE)));

    // Test that other errors are not identified as address in use
    EXPECT_FALSE(IsAddressInUseError(CHIP_NO_ERROR));
    EXPECT_FALSE(IsAddressInUseError(CHIP_ERROR_INVALID_ARGUMENT));
    EXPECT_FALSE(IsAddressInUseError(CHIP_ERROR_POSIX(ECONNREFUSED)));
    EXPECT_FALSE(IsAddressInUseError(CHIP_ERROR_POSIX(ETIMEDOUT)));
}

// Test SafePortIncrement with normal values
TEST_F(TestServerPortRetry, TestSafePortIncrementNormal)
{
    uint16_t outPort = 0;

    // Test normal increment
    EXPECT_TRUE(SafePortIncrement(5000, 1, outPort));
    EXPECT_EQ(outPort, 5001);

    // Test larger increment
    EXPECT_TRUE(SafePortIncrement(5000, 100, outPort));
    EXPECT_EQ(outPort, 5100);

    // Test zero increment
    EXPECT_TRUE(SafePortIncrement(5000, 0, outPort));
    EXPECT_EQ(outPort, 5000);
}

// Test SafePortIncrement with overflow conditions
TEST_F(TestServerPortRetry, TestSafePortIncrementOverflow)
{
    uint16_t outPort = 0;

    // Test overflow at max value
    EXPECT_FALSE(SafePortIncrement(UINT16_MAX, 1, outPort));

    // Test overflow near max value
    EXPECT_FALSE(SafePortIncrement(UINT16_MAX - 5, 10, outPort));

    // Test exact boundary (should succeed)
    EXPECT_TRUE(SafePortIncrement(UINT16_MAX - 10, 10, outPort));
    EXPECT_EQ(outPort, UINT16_MAX);

    // Test one past boundary (should fail)
    EXPECT_FALSE(SafePortIncrement(UINT16_MAX - 10, 11, outPort));
}

// Test SafePortIncrement with edge cases
TEST_F(TestServerPortRetry, TestSafePortIncrementEdgeCases)
{
    uint16_t outPort = 0;

    // Test from port 0
    EXPECT_TRUE(SafePortIncrement(0, 1, outPort));
    EXPECT_EQ(outPort, 1);

    // Test maximum safe increment from 0
    EXPECT_TRUE(SafePortIncrement(0, UINT16_MAX, outPort));
    EXPECT_EQ(outPort, UINT16_MAX);

    // Test increment that would overflow from 0
    EXPECT_FALSE(SafePortIncrement(1, UINT16_MAX, outPort));
}

// Test InitTransportWithPortRetry with successful first attempt
TEST_F(TestServerPortRetry, TestInitTransportSuccessFirstAttempt)
{
    uint16_t boundPort    = 0;
    int initCallCount     = 0;
    int closeCallCount    = 0;
    uint16_t expectedPort = 5000;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        initCallCount++;
        EXPECT_EQ(port, expectedPort);
        return CHIP_NO_ERROR;
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, 3, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(boundPort, 5000);
    EXPECT_EQ(initCallCount, 1);
    EXPECT_EQ(closeCallCount, 0); // Should not call close on success
}

// Test InitTransportWithPortRetry with retry on address in use
TEST_F(TestServerPortRetry, TestInitTransportRetryOnAddressInUse)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    int closeCallCount = 0;
    int successAttempt = 2; // Succeed on third attempt (0-indexed)

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        int currentAttempt = initCallCount++;
        if (currentAttempt < successAttempt)
        {
            return CHIP_ERROR_POSIX(EADDRINUSE);
        }
        return CHIP_NO_ERROR;
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, 5, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(boundPort, 5002);   // Should bind to port 5002 (base + 2)
    EXPECT_EQ(initCallCount, 3);  // Should have tried 3 times
    EXPECT_EQ(closeCallCount, 2); // Should have closed 2 times (after first two failures)
}

// Test InitTransportWithPortRetry with max retries exceeded
TEST_F(TestServerPortRetry, TestInitTransportMaxRetriesExceeded)
{
    uint16_t boundPort  = 0;
    int initCallCount   = 0;
    int closeCallCount  = 0;
    uint16_t maxRetries = 3;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        initCallCount++;
        return CHIP_ERROR_POSIX(EADDRINUSE); // Always fail
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, maxRetries, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_ERROR_POSIX(EADDRINUSE));
    EXPECT_EQ(initCallCount, maxRetries + 1); // Should try maxRetries + 1 times (initial + retries)
    EXPECT_EQ(closeCallCount, maxRetries);    // Should close maxRetries times
}

// Test InitTransportWithPortRetry with non-retryable error
TEST_F(TestServerPortRetry, TestInitTransportNonRetryableError)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    int closeCallCount = 0;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        initCallCount++;
        return CHIP_ERROR_INVALID_ARGUMENT; // Non-retryable error
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, 5, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(initCallCount, 1);  // Should only try once
    EXPECT_EQ(closeCallCount, 0); // Should not close on non-retryable error
}

// Test InitTransportWithPortRetry with port overflow
TEST_F(TestServerPortRetry, TestInitTransportPortOverflow)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    int closeCallCount = 0;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        initCallCount++;
        return CHIP_ERROR_POSIX(EADDRINUSE);
    };

    auto closeFunc = [&]() { closeCallCount++; };

    // Start near max port value
    CHIP_ERROR err = InitTransportWithPortRetry(UINT16_MAX - 1, 5, "test", initFunc, closeFunc, boundPort);

    // Should fail with INVALID_ARGUMENT when port increment would overflow
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(initCallCount, 2);  // Should try twice before overflow
    EXPECT_EQ(closeCallCount, 2); // Should close twice (after each failure before overflow)
}

// Test InitTransportWithPortRetry with zero retries
TEST_F(TestServerPortRetry, TestInitTransportZeroRetries)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    int closeCallCount = 0;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        initCallCount++;
        return CHIP_ERROR_POSIX(EADDRINUSE);
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, 0, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_ERROR_POSIX(EADDRINUSE));
    EXPECT_EQ(initCallCount, 1);  // Should only try once
    EXPECT_EQ(closeCallCount, 0); // Should not close when no retries allowed
}

// Test InitTransportWithPortRetry with alternating errors
TEST_F(TestServerPortRetry, TestInitTransportAlternatingErrors)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    int closeCallCount = 0;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        int attempt = initCallCount++;
        if (attempt == 0)
        {
            return CHIP_ERROR_POSIX(EADDRINUSE); // Retryable
        }
        if (attempt == 1)
        {
            return CHIP_ERROR_INVALID_ARGUMENT; // Non-retryable
        }
        return CHIP_NO_ERROR;
    };

    auto closeFunc = [&]() { closeCallCount++; };

    CHIP_ERROR err = InitTransportWithPortRetry(5000, 5, "test", initFunc, closeFunc, boundPort);

    // Should stop on non-retryable error
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(initCallCount, 2);  // Should try twice
    EXPECT_EQ(closeCallCount, 1); // Should close once after first failure
}

// Test InitTransportWithPortRetry verifies correct port increments
TEST_F(TestServerPortRetry, TestInitTransportPortIncrements)
{
    uint16_t boundPort = 0;
    int initCallCount  = 0;
    uint16_t basePort  = 5000;
    std::vector<uint16_t> attemptedPorts;

    auto initFunc = [&](uint16_t port) -> CHIP_ERROR {
        attemptedPorts.push_back(port);
        int attempt = initCallCount++;
        if (attempt < 3)
        {
            return CHIP_ERROR_POSIX(EADDRINUSE);
        }
        return CHIP_NO_ERROR;
    };

    auto closeFunc = [&]() {};

    CHIP_ERROR err = InitTransportWithPortRetry(basePort, 5, "test", initFunc, closeFunc, boundPort);

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(boundPort, 5003);
    EXPECT_EQ(attemptedPorts.size(), 4u);
    EXPECT_EQ(attemptedPorts[0], 5000);
    EXPECT_EQ(attemptedPorts[1], 5001);
    EXPECT_EQ(attemptedPorts[2], 5002);
    EXPECT_EQ(attemptedPorts[3], 5003);
}

} // namespace
} // namespace app
} // namespace chip
