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
 *      Regression tests for src/inet/TCPEndPointImplLwIP.cpp.
 *
 *      Built for ESP32 / FreeRTOS+LwIP so the test runs under the
 *      esp32-qemu-tests CI image.
 */

#include <pw_unit_test/framework.h>

#include <lwip/opt.h>
#include <lwip/tcp.h>

#include <inet/IPAddress.h>
#include <inet/InetConfig.h>
#include <inet/TCPEndPoint.h>
#include <inet/TCPEndPointImplLwIP.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT

namespace chip {
namespace Inet {

// Friend hook into TCPEndPoint (mState) and TCPEndPointImplLwIP (mTCP) so the
// test can simulate the post-LwIPHandleError state without performing a real
// TCP three-way handshake. The matching `friend class TCPTest;` declarations
// live in TCPEndPoint.h and TCPEndPointImplLwIP.h.
class TCPTest
{
public:
    // Drive the endpoint into the exact race window that the fix targets:
    // mState is still kConnected (so IsConnected() passes the gate at the top
    // of every affected method) but mTCP has already been cleared by
    // LwIPHandleError running on the LwIP TCP/IP task.
    static void SimulatePCBClearedWhileConnected(const TCPEndPointHandle & ep)
    {
        auto * impl  = static_cast<TCPEndPointImplLwIP *>(ep.operator->());
        impl->mState = TCPEndPoint::State::kConnected;
        impl->mTCP   = nullptr;
    }

    // Restore mState to kReady so the destructor's Free() path traverses a
    // straightforward kReady->kClosed transition. mTCP is still nullptr, so
    // DoCloseImpl() is a no-op either way; this just makes teardown intent
    // explicit.
    static void ResetToReadyForTeardown(const TCPEndPointHandle & ep)
    {
        auto * impl  = static_cast<TCPEndPointImplLwIP *>(ep.operator->());
        impl->mState = TCPEndPoint::State::kReady;
    }

    // DriveSendingImpl() is private on TCPEndPointImplLwIP; expose it through
    // the friend so the test can invoke it directly without the surrounding
    // DriveSending() wrapper that would otherwise call DoClose() on error.
    static CHIP_ERROR CallDriveSendingImpl(const TCPEndPointHandle & ep)
    {
        auto * impl = static_cast<TCPEndPointImplLwIP *>(ep.operator->());
        return impl->DriveSendingImpl();
    }
};

} // namespace Inet
} // namespace chip

namespace {

using namespace chip;
using namespace chip::Inet;

class TestTCPEndPointLwIP : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

        // PlatformMgr().InitChipStack() brings up esp_netif (and therefore the
        // LwIP TCP/IP thread + LWIP_TCPIP_CORE_LOCKING mutex used by
        // RunOnTCPIPRet) and initializes the Inet TCPEndPointManager.
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }
};

// Regression test for "[Inet] Fix LwIP TCP endpoint against PCB races".
//
// Background
// ----------
// mTCP is shared between the chip stack thread and the LwIP TCP/IP task.
// LwIPHandleError() runs on the TCP/IP task and clears mTCP synchronously when
// LwIP frees the PCB. Pre-fix, GetPeerInfo / GetLocalInfo / EnableNoDelay /
// EnableKeepAlive / DisableKeepAlive / AckReceive / DriveSendingImpl
// null-checked mTCP on the chip stack thread before scheduling the dereference
// onto the TCP/IP task, leaving a TOCTOU window in which the lambda would
// dereference a freed PCB and crash.
//
// Post-fix, the null-check moved inside RunOnTCPIPRet() so the check and the
// dereference both happen under the LwIP core lock, and each method reports
// CHIP_ERROR_CONNECTION_ABORTED when mTCP is gone.
//
// Test strategy
// -------------
// Reproduce the exact race state without needing a real peer: drive the
// endpoint into kConnected (so IsConnected() passes the gate at the top of
// each method) but leave mTCP null (the post-LwIPHandleError state, before
// the chip-stack-thread HandleError() lambda has run). Pre-fix this would
// dereference null inside the LwIP-task lambda; post-fix every method must
// cleanly return CHIP_ERROR_CONNECTION_ABORTED.
TEST_F(TestTCPEndPointLwIP, AbortedAccessReturnsConnectionAborted)
{
    DeviceLayer::StackLock lock;

    TCPEndPointHandle ep;
    ASSERT_EQ(DeviceLayer::TCPEndPointManager()->NewEndPoint(ep), CHIP_NO_ERROR);
    ASSERT_FALSE(ep.IsNull());

    TCPTest::SimulatePCBClearedWhileConnected(ep);
    ASSERT_TRUE(ep->IsConnected());

    IPAddress addr;
    uint16_t port = 0;

    EXPECT_EQ(ep->GetPeerInfo(&addr, &port), CHIP_ERROR_CONNECTION_ABORTED);
    EXPECT_EQ(ep->GetLocalInfo(&addr, &port), CHIP_ERROR_CONNECTION_ABORTED);
    EXPECT_EQ(ep->EnableNoDelay(), CHIP_ERROR_CONNECTION_ABORTED);
    EXPECT_EQ(ep->AckReceive(1), CHIP_ERROR_CONNECTION_ABORTED);

#if LWIP_TCP_KEEPALIVE
    // When LwIP is built without keepalive support these methods short-circuit
    // to CHIP_ERROR_NOT_IMPLEMENTED before reaching the (un)fixed code path,
    // so they only carry signal when keepalive is compiled in.
    EXPECT_EQ(ep->EnableKeepAlive(/*interval=*/10, /*timeoutCount=*/3), CHIP_ERROR_CONNECTION_ABORTED);
    EXPECT_EQ(ep->DisableKeepAlive(), CHIP_ERROR_CONNECTION_ABORTED);
#endif // LWIP_TCP_KEEPALIVE

    // DriveSendingImpl() does not gate on IsConnected(); it always interacts
    // with the PCB. With mTCP null, the very first RunOnTCPIPRet() inside it
    // (the tcp_sndbuf() call) is the one that used to crash.
    EXPECT_EQ(TCPTest::CallDriveSendingImpl(ep), CHIP_ERROR_CONNECTION_ABORTED);

    TCPTest::ResetToReadyForTeardown(ep);
    ep.Release();
}

} // namespace

#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
