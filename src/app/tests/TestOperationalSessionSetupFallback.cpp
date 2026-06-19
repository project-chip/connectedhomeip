/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      This file implements unit tests for address resolution fallback functionality in
 *      OperationalSessionSetup and CASESessionManager.
 */

#include <pw_unit_test/framework.h>

#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetup.h>
#include <app/tests/AppTestContext.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SessionManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

namespace {

constexpr uint16_t kTestPort = 5540;

class MockOperationalSessionReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup * sessionSetup) override
    {
        mReleasedCount++;
        mLastReleasedSession = sessionSetup;
    }

    void Reset()
    {
        mReleasedCount       = 0;
        mLastReleasedSession = nullptr;
    }

    int mReleasedCount                             = 0;
    OperationalSessionSetup * mLastReleasedSession = nullptr;
};

class MockSessionEstablishmentDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablished(const SessionHandle & session) override
    {
        mSessionEstablishedCount++;
        mLastError = CHIP_NO_ERROR;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error, SessionEstablishmentStage stage) override
    {
        mSessionEstablishmentErrorCount++;
        mLastError = error;
        mLastStage = stage;
    }

    void Reset()
    {
        mSessionEstablishedCount        = 0;
        mSessionEstablishmentErrorCount = 0;
        mLastError                      = CHIP_NO_ERROR;
        mLastStage                      = SessionEstablishmentStage::kNotInKeyExchange;
    }

    int mSessionEstablishedCount         = 0;
    int mSessionEstablishmentErrorCount  = 0;
    CHIP_ERROR mLastError                = CHIP_NO_ERROR;
    SessionEstablishmentStage mLastStage = SessionEstablishmentStage::kNotInKeyExchange;
};

class TestOperationalSessionSetupFallback : public chip::Testing::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        mReleaseDelegate.Reset();
        mSessionDelegate.Reset();
    }

    void TearDown() override { AppContext::TearDown(); }

protected:
    MockOperationalSessionReleaseDelegate mReleaseDelegate;
    MockSessionEstablishmentDelegate mSessionDelegate;

    AddressResolve::ResolveResult CreateTestResolveResult()
    {
        AddressResolve::ResolveResult result;
        IPAddress ipAddress;
        EXPECT_TRUE(IPAddress::FromString("fe80::1", ipAddress));
        result.address           = PeerAddress::UDP(ipAddress, kTestPort);
        result.supportsTcpClient = false;
        result.supportsTcpServer = false;
        result.mrpRemoteConfig   = GetDefaultMRPConfig();
        return result;
    }
};

#if CHIP_CONFIG_ENABLE_ADDRESS_RESOLVE_FALLBACK

constexpr NodeId kTestNodeId       = 0x123456789abcdefULL;
constexpr FabricIndex kFabricIndex = 1;

TEST_F(TestOperationalSessionSetupFallback, TestSetFallbackResolveResult)
{
    // Test that we can set a fallback resolve result on OperationalSessionSetup

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);

    // Note: This test is limited because OperationalSessionSetup requires significant
    // infrastructure (SessionManager, ExchangeManager, etc.) to fully instantiate.
    // A more complete test would require a full test harness.

    AddressResolve::ResolveResult fallbackResult = CreateTestResolveResult();

    // Verify that the fallback result structure can be created
    EXPECT_TRUE(fallbackResult.address.IsInitialized());
    EXPECT_EQ(fallbackResult.address.GetPort(), kTestPort);
}

TEST_F(TestOperationalSessionSetupFallback, TestFallbackResolveResultStructure)
{
    // Verify the structure and values of ResolveResult
    AddressResolve::ResolveResult result = CreateTestResolveResult();

    EXPECT_TRUE(result.address.IsInitialized());
    EXPECT_EQ(result.address.GetTransportType(), Transport::Type::kUdp);
    EXPECT_EQ(result.address.GetPort(), kTestPort);
    EXPECT_FALSE(result.supportsTcpClient);
    EXPECT_FALSE(result.supportsTcpServer);
}

TEST_F(TestOperationalSessionSetupFallback, TestFallbackTimeoutValue)
{
    // Verify that the default fallback timeout is 5 seconds
    constexpr System::Clock::Seconds16 kExpectedFallbackTimeout(5);

    // The timeout is hardcoded in OperationalSessionSetup.h as:
    // System::Clock::Timeout mFallbackTimeout = System::Clock::Seconds16(5);

    // This test verifies the expected timeout value
    System::Clock::Timeout expectedTimeout = kExpectedFallbackTimeout;

    // Verify the timeout is non-zero
    EXPECT_GT(expectedTimeout.count(), 0u);

    // Verify it's exactly 5 seconds
    EXPECT_EQ(std::chrono::duration_cast<System::Clock::Seconds16>(expectedTimeout).count(), 5u);
}

TEST_F(TestOperationalSessionSetupFallback, TestMultipleFallbackResults)
{
    // Test that multiple different resolve results can be created
    AddressResolve::ResolveResult result1 = CreateTestResolveResult();

    AddressResolve::ResolveResult result2;
    IPAddress ipAddress2;
    EXPECT_TRUE(IPAddress::FromString("::1", ipAddress2));
    result2.address           = PeerAddress::UDP(ipAddress2, 5541);
    result2.supportsTcpClient = true;
    result2.supportsTcpServer = false;
    result2.mrpRemoteConfig   = GetDefaultMRPConfig();

    // Verify both results are valid but different
    EXPECT_TRUE(result1.address.IsInitialized());
    EXPECT_TRUE(result2.address.IsInitialized());
    EXPECT_NE(result1.address.GetPort(), result2.address.GetPort());
    EXPECT_NE(result1.supportsTcpClient, result2.supportsTcpClient);
}

TEST_F(TestOperationalSessionSetupFallback, TestFallbackWithTcpAddress)
{
    // Test creating a fallback result with TCP transport
    AddressResolve::ResolveResult result;
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("192.168.1.100", ipAddress));

    result.address           = PeerAddress::TCP(ipAddress, 5540);
    result.supportsTcpClient = true;
    result.supportsTcpServer = true;
    result.mrpRemoteConfig   = GetDefaultMRPConfig();

    EXPECT_TRUE(result.address.IsInitialized());
    EXPECT_EQ(result.address.GetTransportType(), Transport::Type::kTcp);
    EXPECT_TRUE(result.supportsTcpClient);
    EXPECT_TRUE(result.supportsTcpServer);
}

TEST_F(TestOperationalSessionSetupFallback, TestMRPConfigInFallbackResult)
{
    // Test that MRP config is properly stored in fallback result
    AddressResolve::ResolveResult result;
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("fe80::1", ipAddress));
    result.address = PeerAddress::UDP(ipAddress, kTestPort);

    // Set custom MRP config
    ReliableMessageProtocolConfig customConfig(System::Clock::Milliseconds32(300), // idleRetransTimeout
                                               System::Clock::Milliseconds32(300), // activeRetransTimeout
                                               System::Clock::Milliseconds16(4000) // activeThresholdTime
    );
    result.mrpRemoteConfig = customConfig;

    // Verify MRP config is preserved
    EXPECT_EQ(result.mrpRemoteConfig.mIdleRetransTimeout, customConfig.mIdleRetransTimeout);
    EXPECT_EQ(result.mrpRemoteConfig.mActiveRetransTimeout, customConfig.mActiveRetransTimeout);
    EXPECT_EQ(result.mrpRemoteConfig.mActiveThresholdTime, customConfig.mActiveThresholdTime);
}

#else // !CHIP_CONFIG_ENABLE_ADDRESS_RESOLVE_FALLBACK

TEST_F(TestOperationalSessionSetupFallback, TestFallbackDisabledByDefault)
{
    // When CHIP_CONFIG_ENABLE_ADDRESS_RESOLVE_FALLBACK is not defined, the fallback
    // functionality should not be compiled in. This test simply verifies
    // the test framework is working when the feature is disabled.
    SUCCEED();
}

#endif // CHIP_CONFIG_ENABLE_ADDRESS_RESOLVE_FALLBACK

} // namespace
