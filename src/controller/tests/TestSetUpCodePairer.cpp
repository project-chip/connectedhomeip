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

#include <pw_unit_test/framework.h>

#include <controller/CHIPDeviceController.h>
#include <controller/SetUpCodePairer.h>
#include <controller/tests/SetUpCodePairerTestAccess.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;
using namespace chip::Controller;
using PairerAccess = chip::Testing::SetUpCodePairerTestAccess;

namespace {

class TestSetUpCodePairer : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    DeviceCommissioner commissioner;
    SetUpCodePairer pairer{ &commissioner };
    PairerAccess access{ &pairer };
};

// When the discovery timeout fires while a PASE attempt is in progress,
// DNS-SD should be stopped (it runs indefinitely) but other transports
// (BLE, Wi-Fi PAF, NFC) should be left alone since they self-terminate.
TEST_F(TestSetUpCodePairer, TimeoutDuringPASE_StopsDNSSD_PreservesOtherTransports)
{
    access.SetRemoteId(1);
    access.SetWaitingForPASE(true);
    access.SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    access.SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    access.FireTimeoutCallback();

    // DNS-SD must be stopped to prevent DiscoveryInProgress() from being stuck true.
    EXPECT_FALSE(access.GetWaitingForDiscovery(PairerAccess::kIPTransport));
    // BLE must be preserved — it may still discover a commissionee.
    EXPECT_TRUE(access.GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // PASE state must not be disturbed.
    EXPECT_TRUE(access.GetWaitingForPASE());
}

// When the discovery timeout fires with no PASE in progress,
// all transports should be stopped and failure reported.
TEST_F(TestSetUpCodePairer, TimeoutNoPASE_StopsAllTransports)
{
    access.SetRemoteId(1);
    access.SetWaitingForPASE(false);
    access.SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    access.SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    access.FireTimeoutCallback();

    EXPECT_FALSE(access.GetWaitingForDiscovery(PairerAccess::kIPTransport));
    EXPECT_FALSE(access.GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // StopPairingIfTransportsExhausted should have reported failure and cleared mRemoteId.
    EXPECT_EQ(access.GetRemoteId(), kUndefinedNodeId);
}

// When a PASE attempt fails (OnPairingComplete with error) and DNS-SD
// is still running, DNS-SD should NOT be stopped.  Keeping DNS-SD alive
// keeps DiscoveryInProgress() true, which suppresses premature failure
// through OnStatusUpdate, TryNextRendezvousParameters, and
// StopPairingIfTransportsExhausted.
TEST_F(TestSetUpCodePairer, PASEFailure_DNSSDStaysAlive)
{
    access.SetRemoteId(1);
    access.SetWaitingForDiscovery(PairerAccess::kIPTransport, true);
    access.SetWaitingForDiscovery(PairerAccess::kBLETransport, true);

    // Simulate a UDP PASE attempt: set mCurrentPASEParameters and enter
    // the PASE-waiting state (as ConnectToDiscoveredDevice would).
    SetUpCodePairerParameters udpParams;
    udpParams.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
    access.SetCurrentPASEParameters(udpParams);
    access.ExpectPASEEstablishment();

    // PASE fails.
    access.CallOnPairingComplete(CHIP_ERROR_TIMEOUT);

    // DNS-SD must still be running — the 30s timeout is its natural upper bound.
    EXPECT_TRUE(access.GetWaitingForDiscovery(PairerAccess::kIPTransport));
    // BLE must still be running.
    EXPECT_TRUE(access.GetWaitingForDiscovery(PairerAccess::kBLETransport));
    // The pairer should still be active (not yet reported failure).
    EXPECT_NE(access.GetRemoteId(), kUndefinedNodeId);
    // The error should be saved for later.
    EXPECT_EQ(access.GetLastPASEError(), CHIP_ERROR_TIMEOUT);
    // mCurrentPASEParameters should have been cleared.
    EXPECT_FALSE(access.HasCurrentPASEParameters());
}

// When a PASE attempt fails synchronously in ConnectToDiscoveredDevice,
// mCurrentPASEParameters must be cleared so a later non-UDP PASE
// completion cannot incorrectly use stale parameters for ReconfirmRecord.
TEST_F(TestSetUpCodePairer, SyncPASEFailure_ClearsCurrentPASEParameters)
{
    access.SetRemoteId(1);
    access.SetWaitingForDiscovery(PairerAccess::kIPTransport, true);

    // Set up stale UDP parameters (as if ConnectToDiscoveredDevice had set
    // them before calling PairDevice, which then failed synchronously).
    SetUpCodePairerParameters udpParams;
    udpParams.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
    access.SetCurrentPASEParameters(udpParams);

    // Now simulate a subsequent non-UDP PASE completion (e.g., BLE PASE
    // succeeds or fails).  ExpectPASEEstablishment + OnPairingComplete
    // with success exercises ResetDiscoveryState which clears everything.
    // Instead, verify directly that after a PASE failure the stale
    // parameters don't persist: simulate a PASE completion with error.
    access.ExpectPASEEstablishment();
    access.CallOnPairingComplete(CHIP_ERROR_CONNECTION_ABORTED);

    // mCurrentPASEParameters must be cleared — stale UDP params must not
    // survive to confuse a later ReconfirmRecord check.
    EXPECT_FALSE(access.HasCurrentPASEParameters());
}

} // namespace
