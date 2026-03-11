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

#include <gtest/gtest.h>

#include <controller/CHIPDeviceController.h>
#include <controller/SetUpCodePairer.h>
#include <controller/tests/SetUpCodePairerTestAccess.h>
#include <lib/core/CHIPError.h>

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

} // namespace
