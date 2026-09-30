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

#include <pw_unit_test/framework.h>

#include <app/DeviceProxy.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/DevicePairingDelegate.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <lib/core/StringBuilderAdapters.h>
#include <platform/CHIPDeviceLayer.h>

#include <clusters/IcdManagement/Commands.h>

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

class MockPairingDelegate : public DevicePairingDelegate
{
public:
    void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) override
    {
        mStatusUpdateCount++;
        mLastStageCompleted = stageCompleted;
        mLastError          = error;
    }

    void OnICDRegistrationComplete(ScopedNodeId icdNodeId, uint32_t icdCounter) override
    {
        mRegistrationCompleteCount++;
        mLastICDCounter = icdCounter;
    }

    void OnICDStayActiveComplete(ScopedNodeId icdNodeId, uint32_t promisedActiveDurationMsec) override
    {
        mStayActiveCompleteCount++;
        mLastPromisedActiveDuration = promisedActiveDurationMsec;
    }

    int mStatusUpdateCount                 = 0;
    CommissioningStage mLastStageCompleted = CommissioningStage::kError;
    CHIP_ERROR mLastError                  = CHIP_NO_ERROR;
    int mRegistrationCompleteCount         = 0;
    uint32_t mLastICDCounter               = 0;
    int mStayActiveCompleteCount           = 0;
    uint32_t mLastPromisedActiveDuration   = 0;
};

class TestICDManagementResponses : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    FakeDeviceProxy mDevice;
    MockPairingDelegate mDelegate;
    DeviceCommissioner mCommissioner{};
};

// A response landing in a stage it does not belong to must be ignored: it says nothing about
// the stage actually in progress, so completing that stage (with any error) would be wrong.
TEST_F(TestICDManagementResponses, StayActiveResponseInWrongStageIsIgnored)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDRegistration);
    access.SetDeviceBeingCommissioned(&mDevice);

    IcdManagement::Commands::StayActiveResponse::DecodableType data;
    data.promisedActiveDuration = 1000u;
    DeviceCommissionerTestAccess::OnICDManagementStayActiveResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mStayActiveCompleteCount, 0);
    EXPECT_EQ(mDelegate.mStatusUpdateCount, 0);
}

TEST_F(TestICDManagementResponses, RegisterClientResponseInWrongStageIsIgnored)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDSendStayActive);
    access.SetDeviceBeingCommissioned(&mDevice);

    IcdManagement::Commands::RegisterClientResponse::DecodableType data;
    data.ICDCounter = 77u;
    DeviceCommissionerTestAccess::OnICDManagementRegisterClientResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mRegistrationCompleteCount, 0);
    EXPECT_EQ(mDelegate.mStatusUpdateCount, 0);
}

TEST_F(TestICDManagementResponses, StayActiveResponseInCorrectStageCompletesStage)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDSendStayActive);
    access.SetDeviceBeingCommissioned(&mDevice);

    IcdManagement::Commands::StayActiveResponse::DecodableType data;
    data.promisedActiveDuration = 1234u;
    DeviceCommissionerTestAccess::OnICDManagementStayActiveResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mStayActiveCompleteCount, 1);
    EXPECT_EQ(mDelegate.mLastPromisedActiveDuration, 1234u);
    ASSERT_EQ(mDelegate.mStatusUpdateCount, 1);
    EXPECT_EQ(mDelegate.mLastStageCompleted, CommissioningStage::kICDSendStayActive);
    EXPECT_EQ(mDelegate.mLastError, CHIP_NO_ERROR);
}

TEST_F(TestICDManagementResponses, RegisterClientResponseInCorrectStageCompletesStage)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDRegistration);
    access.SetDeviceBeingCommissioned(&mDevice);

    IcdManagement::Commands::RegisterClientResponse::DecodableType data;
    data.ICDCounter = 77u;
    DeviceCommissionerTestAccess::OnICDManagementRegisterClientResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mRegistrationCompleteCount, 1);
    EXPECT_EQ(mDelegate.mLastICDCounter, 77u);
    ASSERT_EQ(mDelegate.mStatusUpdateCount, 1);
    EXPECT_EQ(mDelegate.mLastStageCompleted, CommissioningStage::kICDRegistration);
    EXPECT_EQ(mDelegate.mLastError, CHIP_NO_ERROR);
}

// Before the handlers checked-then-returned, the paths below crashed: the shared exit still
// called CommissioningStageComplete, which dies on a null mDeviceBeingCommissioned (and the
// null-context case dereferenced the null commissioner outright).
TEST_F(TestICDManagementResponses, StayActiveResponseWithNoDeviceIsIgnored)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDSendStayActive);
    access.SetDeviceBeingCommissioned(nullptr);

    IcdManagement::Commands::StayActiveResponse::DecodableType data;
    DeviceCommissionerTestAccess::OnICDManagementStayActiveResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mStayActiveCompleteCount, 0);
    EXPECT_EQ(mDelegate.mStatusUpdateCount, 0);
}

TEST_F(TestICDManagementResponses, RegisterClientResponseWithNoDeviceIsIgnored)
{
    DeviceCommissionerTestAccess access(&mCommissioner);
    mCommissioner.RegisterPairingDelegate(&mDelegate);
    access.SetCommissioningStage(CommissioningStage::kICDRegistration);
    access.SetDeviceBeingCommissioned(nullptr);

    IcdManagement::Commands::RegisterClientResponse::DecodableType data;
    DeviceCommissionerTestAccess::OnICDManagementRegisterClientResponse(&mCommissioner, data);

    EXPECT_EQ(mDelegate.mRegistrationCompleteCount, 0);
    EXPECT_EQ(mDelegate.mStatusUpdateCount, 0);
}

TEST_F(TestICDManagementResponses, StayActiveResponseWithNullContextIsIgnored)
{
    IcdManagement::Commands::StayActiveResponse::DecodableType data;
    DeviceCommissionerTestAccess::OnICDManagementStayActiveResponse(nullptr, data);
}

TEST_F(TestICDManagementResponses, RegisterClientResponseWithNullContextIsIgnored)
{
    IcdManagement::Commands::RegisterClientResponse::DecodableType data;
    DeviceCommissionerTestAccess::OnICDManagementRegisterClientResponse(nullptr, data);
}

} // namespace
