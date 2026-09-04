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

// How the commissioner interprets a NetworkConfigResponse. The stage it arrives in matters: the
// same networkingStatus that fails an AddOrUpdate can be the expected outcome of a RemoveNetwork.

#include <pw_unit_test/framework.h>

#include <app/DeviceProxy.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/DevicePairingDelegate.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>
#include <platform/CHIPDeviceLayer.h>

#include <clusters/NetworkCommissioning/Commands.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Testing;

namespace {

constexpr NodeId kTestNodeId = 0x12344321;

class FakeDeviceProxy : public DeviceProxy
{
public:
    void Disconnect() override {}
    NodeId GetDeviceId() const override { return kTestNodeId; }
    Messaging::ExchangeManager * GetExchangeManager() const override { return nullptr; }
    chip::Optional<SessionHandle> GetSecureSession() const override { return NullOptional; }

protected:
    bool IsSecureConnected() const override { return false; }
};

// The commissioner reports the outcome of every stage to the pairing delegate before handing over to
// the commissioning delegate, which is left unset here so the flow stops after we have observed it.
class MockPairingDelegate : public DevicePairingDelegate
{
public:
    void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) override
    {
        mStatusUpdateCount++;
        mLastStageCompleted = stageCompleted;
        mLastError          = error;
    }

    int mStatusUpdateCount                 = 0;
    CommissioningStage mLastStageCompleted = CommissioningStage::kError;
    CHIP_ERROR mLastError                  = CHIP_NO_ERROR;
};

class TestNetworkConfigResponses : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        mCommissioner.RegisterPairingDelegate(&mDelegate);
        mAccess.SetDeviceBeingCommissioned(&mDevice);
    }

    // Delivers a NetworkConfigResponse as though it had arrived during the given stage,
    // and returns the error the commissioner completed that stage with.
    CHIP_ERROR DeliverResponse(CommissioningStage stage, NetworkCommissioning::NetworkCommissioningStatusEnum status)
    {
        mAccess.SetCommissioningStage(stage);

        NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType data;
        data.networkingStatus = status;
        DeviceCommissionerTestAccess::OnNetworkConfigResponse(&mCommissioner, data);

        EXPECT_EQ(mDelegate.mStatusUpdateCount, 1);
        EXPECT_EQ(mDelegate.mLastStageCompleted, stage);
        return mDelegate.mLastError;
    }

    FakeDeviceProxy mDevice;
    MockPairingDelegate mDelegate;
    DeviceCommissioner mCommissioner{};
    DeviceCommissionerTestAccess mAccess{ &mCommissioner };
};

// Removing a network configuration the commissionee does not have achieves what the removal was
// asking for, so it must not fail the attempt. We rely on this when giving up on a network
// technology we never managed to configure, and when the NetworkID we name is stale.
TEST_F(TestNetworkConfigResponses, RemovingAnAbsentWiFiNetworkSucceeds)
{
    EXPECT_EQ(DeliverResponse(kRemoveWiFiNetworkConfig, NetworkCommissioning::NetworkCommissioningStatusEnum::kNetworkIDNotFound),
              CHIP_NO_ERROR);
}

TEST_F(TestNetworkConfigResponses, RemovingAnAbsentThreadNetworkSucceeds)
{
    EXPECT_EQ(DeliverResponse(kRemoveThreadNetworkConfig, NetworkCommissioning::NetworkCommissioningStatusEnum::kNetworkIDNotFound),
              CHIP_NO_ERROR);
}

// "not found" against an AddOrUpdate is still a failure
TEST_F(TestNetworkConfigResponses, NetworkIdNotFoundStillFailsNetworkSetup)
{
    EXPECT_NE(DeliverResponse(kWiFiNetworkSetup, NetworkCommissioning::NetworkCommissioningStatusEnum::kNetworkIDNotFound),
              CHIP_NO_ERROR);
}

// Only the "not found" status is forgiven, any other status is still an error.
TEST_F(TestNetworkConfigResponses, OtherFailuresStillFailARemoval)
{
    EXPECT_NE(DeliverResponse(kRemoveWiFiNetworkConfig, NetworkCommissioning::NetworkCommissioningStatusEnum::kUnknownError),
              CHIP_NO_ERROR);
}

TEST_F(TestNetworkConfigResponses, SuccessCompletesTheStage)
{
    EXPECT_EQ(DeliverResponse(kRemoveWiFiNetworkConfig, NetworkCommissioning::NetworkCommissioningStatusEnum::kSuccess),
              CHIP_NO_ERROR);
}
} // namespace
