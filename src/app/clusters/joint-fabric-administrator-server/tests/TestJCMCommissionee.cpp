/*
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

#include <app/clusters/joint-fabric-administrator-server/JCMCommissionee.h>
#include <app/tests/AppTestContext.h>
#include <credentials/jcm/TrustVerification.h>
#include <lib/core/CHIPError.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <access/SubjectDescriptor.h>
#include <app/CommandHandler.h>

#include <utility>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::JointFabricAdministrator;
using namespace chip::Credentials::JCM;

namespace {

class FakeCommandHandler : public CommandHandler
{
public:
    explicit FakeCommandHandler(FabricIndex fabricIndex = FabricIndex{ 1 }) : mFabricIndex(fabricIndex)
    {
        mSubjectDescriptor.fabricIndex = fabricIndex;
    }

    void SetExchangeContext(Messaging::ExchangeContext * exchange) { mExchangeContext = exchange; }
    void SetAccessingFabricIndex(FabricIndex fabricIndex)
    {
        mFabricIndex                   = fabricIndex;
        mSubjectDescriptor.fabricIndex = fabricIndex;
    }

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath &, const Protocols::InteractionModel::ClusterStatusCode &,
                                 const char *) override
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath &, const Protocols::InteractionModel::ClusterStatusCode &, const char *) override {}

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath &, ClusterStatus) override { return CHIP_NO_ERROR; }

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath &, ClusterStatus) override { return CHIP_NO_ERROR; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath &, CommandId, const DataModel::EncodableToTLV &) override
    {
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath &, CommandId, const DataModel::EncodableToTLV &) override {}

    bool IsTimedInvoke() const override { return false; }

    void FlushAcksRightAwayOnSlowCommand() override {}

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    Access::SubjectDescriptor GetSubjectDescriptor() const override { return mSubjectDescriptor; }

    Messaging::ExchangeContext * GetExchangeContext() const override { return mExchangeContext; }

private:
    FabricIndex mFabricIndex;
    Messaging::ExchangeContext * mExchangeContext = nullptr;
    Access::SubjectDescriptor mSubjectDescriptor;
};

class TestableJCMCommissionee : public JCMCommissionee
{
public:
    using Base             = JCMCommissionee;
    using OnCompletionFunc = Base::OnCompletionFunc;

    TestableJCMCommissionee(CommandHandler::Handle & handle, EndpointId endpointId, OnCompletionFunc onCompletion) :
        Base(handle, endpointId, std::move(onCompletion))
    {}

    TrustVerificationStage NextStage(TrustVerificationStage currentStage) { return GetNextTrustVerificationStage(currentStage); }

    void BeginVerification() { StartTrustVerification(); }

    void FinishStage(TrustVerificationStage stage, TrustVerificationError error) { TrustVerificationStageFinished(stage, error); }

    const std::vector<TrustVerificationStage> & ObservedStages() const { return mObservedStages; }

    const std::vector<TrustVerificationError> & CompletionEvents() const { return mCompletionEvents; }

protected:
    void PerformTrustVerificationStage(const TrustVerificationStage & nextStage) override { mObservedStages.push_back(nextStage); }

    void OnTrustVerificationComplete(TrustVerificationError error) override
    {
        mCompletionEvents.push_back(error);
        Base::OnTrustVerificationComplete(error);
    }

private:
    std::vector<TrustVerificationStage> mObservedStages;
    std::vector<TrustVerificationError> mCompletionEvents;
};

class TestJCMCommissionee : public chip::Test::AppContext
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        chip::Test::AppContext::SetUpTestSuite();
    }

    static void TearDownTestSuite()
    {
        AppContext::TearDownTestSuite();
        Platform::MemoryShutdown();
    }

protected:
    void SetUp() override { AppContext::SetUp(); }
    void TearDown() override { AppContext::TearDown(); }

    void NextStageFollowsExpectedOrder();
    void SuccessfulProgressionAdvancesAllStages();
    void ErrorDuringStagePropagatesToCompletion();
};

} // namespace

TEST_F_FROM_FIXTURE(TestJCMCommissionee, NextStageFollowsExpectedOrder)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_ERROR_INTERNAL;

    TestableJCMCommissionee commissionee(handle, EndpointId{ 1 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kIdle), TrustVerificationStage::kStoringEndpointID);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kStoringEndpointID),
              TrustVerificationStage::kReadingCommissionerAdminFabricIndex);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kReadingCommissionerAdminFabricIndex),
              TrustVerificationStage::kPerformingVendorIDVerification);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kPerformingVendorIDVerification),
              TrustVerificationStage::kCrossCheckingAdministratorIds);
    EXPECT_EQ(commissionee.NextStage(TrustVerificationStage::kCrossCheckingAdministratorIds), TrustVerificationStage::kComplete);

    EXPECT_FALSE(completionCalled);
    EXPECT_EQ(completionError, CHIP_ERROR_INTERNAL);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, SuccessfulProgressionAdvancesAllStages)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_ERROR_INTERNAL;

    TestableJCMCommissionee commissionee(handle, EndpointId{ 2 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    commissionee.BeginVerification();

    ASSERT_EQ(commissionee.ObservedStages().size(), 1u);
    EXPECT_EQ(commissionee.ObservedStages()[0], TrustVerificationStage::kStoringEndpointID);

    commissionee.FinishStage(TrustVerificationStage::kStoringEndpointID, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 2u);
    EXPECT_EQ(commissionee.ObservedStages()[1], TrustVerificationStage::kReadingCommissionerAdminFabricIndex);

    commissionee.FinishStage(TrustVerificationStage::kReadingCommissionerAdminFabricIndex, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 3u);
    EXPECT_EQ(commissionee.ObservedStages()[2], TrustVerificationStage::kPerformingVendorIDVerification);

    commissionee.FinishStage(TrustVerificationStage::kPerformingVendorIDVerification, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 4u);
    EXPECT_EQ(commissionee.ObservedStages()[3], TrustVerificationStage::kCrossCheckingAdministratorIds);

    commissionee.FinishStage(TrustVerificationStage::kCrossCheckingAdministratorIds, TrustVerificationError::kSuccess);
    ASSERT_EQ(commissionee.ObservedStages().size(), 5u);
    EXPECT_EQ(commissionee.ObservedStages()[4], TrustVerificationStage::kComplete);

    commissionee.FinishStage(TrustVerificationStage::kComplete, TrustVerificationError::kSuccess);

    ASSERT_EQ(commissionee.CompletionEvents().size(), 1u);
    EXPECT_EQ(commissionee.CompletionEvents()[0], TrustVerificationError::kSuccess);
    EXPECT_TRUE(completionCalled);
    EXPECT_EQ(completionError, CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE(TestJCMCommissionee, ErrorDuringStagePropagatesToCompletion)
{
    FakeCommandHandler commandHandler;
    CommandHandler::Handle handle(&commandHandler);

    bool completionCalled      = false;
    CHIP_ERROR completionError = CHIP_NO_ERROR;

    TestableJCMCommissionee commissionee(handle, EndpointId{ 3 }, [&](CHIP_ERROR err) {
        completionCalled = true;
        completionError  = err;
    });

    commissionee.BeginVerification();
    ASSERT_EQ(commissionee.ObservedStages().size(), 1u);

    commissionee.FinishStage(TrustVerificationStage::kStoringEndpointID, TrustVerificationError::kInternalError);

    ASSERT_EQ(commissionee.CompletionEvents().size(), 1u);
    EXPECT_EQ(commissionee.CompletionEvents()[0], TrustVerificationError::kInternalError);

    EXPECT_TRUE(completionCalled);
    EXPECT_EQ(completionError, CHIP_ERROR_INTERNAL);
}
