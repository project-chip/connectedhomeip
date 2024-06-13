/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app-common/zap-generated/ids/Clusters.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
#include <app/tests/AppTestContext.h>
#include <controller/WriteInteraction.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/interaction_model/Constants.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitTesting;
using namespace chip::Protocols;

namespace {

constexpr EndpointId kTestEndpointId       = 1;
constexpr DataVersion kRejectedDataVersion = 1;
constexpr DataVersion kAcceptedDataVersion = 5;

constexpr uint8_t kExampleClusterSpecificSuccess = 11u;
constexpr uint8_t kExampleClusterSpecificFailure = 12u;

enum ResponseDirective
{
    kSendAttributeSuccess,
    kSendAttributeError,
    kSendMultipleSuccess,
    kSendMultipleErrors,
    kSendClusterSpecificSuccess,
    kSendClusterSpecificFailure,
};

ResponseDirective gResponseDirective;

} // namespace

namespace chip {
namespace app {

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    static ListIndex listStructOctetStringElementCount = 0;

    if (aPath.mDataVersion.HasValue() && aPath.mDataVersion.Value() == kRejectedDataVersion)
    {
        return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::DataVersionMismatch);
    }

    if (aPath.mClusterId == Clusters::UnitTesting::Id &&
        aPath.mAttributeId == Attributes::ListStructOctetString::TypeInfo::GetAttributeId())
    {
        if (gResponseDirective == kSendAttributeSuccess)
        {
            if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
            {

                Attributes::ListStructOctetString::TypeInfo::DecodableType value;

                ReturnErrorOnFailure(DataModel::Decode(aReader, value));

                auto iter                         = value.begin();
                listStructOctetStringElementCount = 0;
                while (iter.Next())
                {
                    auto & item = iter.GetValue();

                    VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                    listStructOctetStringElementCount++;
                }

                aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
            }
            else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
            {
                Structs::TestListStructOctet::DecodableType item;
                ReturnErrorOnFailure(DataModel::Decode(aReader, item));
                VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                listStructOctetStringElementCount++;

                aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
            }
            else
            {
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }
        }
        else
        {
            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Failure);
        }

        return CHIP_NO_ERROR;
    }
    if (aPath.mClusterId == Clusters::UnitTesting::Id && aPath.mAttributeId == Attributes::ListFabricScoped::Id)
    {
        // Mock a invalid SubjectDescriptor
        AttributeValueDecoder decoder(aReader, Access::SubjectDescriptor());
        if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            Attributes::ListFabricScoped::TypeInfo::DecodableType value;

            ReturnErrorOnFailure(decoder.Decode(value));

            auto iter = value.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                (void) item;
            }

            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
        }
        else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            Structs::TestFabricScoped::DecodableType item;
            ReturnErrorOnFailure(decoder.Decode(item));

            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        return CHIP_NO_ERROR;
    }

    // Boolean attribute of unit testing cluster triggers "multiple errors" case.
    if (aPath.mClusterId == Clusters::UnitTesting::Id && aPath.mAttributeId == Attributes::Boolean::TypeInfo::GetAttributeId())
    {
        InteractionModel::ClusterStatusCode status{ Protocols::InteractionModel::Status::InvalidValue };

        if (gResponseDirective == kSendMultipleSuccess)
        {
            status = InteractionModel::Status::Success;
        }
        else if (gResponseDirective == kSendMultipleErrors)
        {
            status = InteractionModel::Status::Failure;
        }
        else
        {
            VerifyOrDie(false);
        }

        for (size_t i = 0; i < 4; ++i)
        {
            aWriteHandler->AddStatus(aPath, status);
        }

        return CHIP_NO_ERROR;
    }

    if (aPath.mClusterId == Clusters::UnitTesting::Id && aPath.mAttributeId == Attributes::Int8u::TypeInfo::GetAttributeId())
    {
        InteractionModel::ClusterStatusCode status{ Protocols::InteractionModel::Status::InvalidValue };
        if (gResponseDirective == kSendClusterSpecificSuccess)
        {
            status = InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(kExampleClusterSpecificSuccess);
        }
        else if (gResponseDirective == kSendClusterSpecificFailure)
        {
            status = InteractionModel::ClusterStatusCode::ClusterSpecificFailure(kExampleClusterSpecificFailure);
        }
        else
        {
            VerifyOrDie(false);
        }

        aWriteHandler->AddStatus(aPath, status);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

class SingleWriteCallback : public WriteClient::Callback
{
public:
    explicit SingleWriteCallback(ConcreteAttributePath path) : mPath(path) {}

    void OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & aPath, StatusIB attributeStatus) override
    {

        if (aPath.MatchesConcreteAttributePath(mPath))
        {
            mPathWasReponded = true;
            mPathStatus      = attributeStatus;
        }
    }

    void OnError(const WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        (void) apWriteClient;
        mLastChipError = aError;
    }

    void OnDone(WriteClient * apWriteClient) override
    {
        (void) apWriteClient;
        mOnDoneCalled = true;
    }

    bool WasDone() const { return mOnDoneCalled; }
    bool PathWasResponded() const { return mOnDoneCalled; }
    CHIP_ERROR GetLastChipError() const { return mLastChipError; }
    StatusIB GetPathStatus() const { return mPathStatus; }

private:
    ConcreteAttributePath mPath;
    bool mOnDoneCalled        = false;
    CHIP_ERROR mLastChipError = CHIP_NO_ERROR;
    bool mPathWasReponded     = false;
    StatusIB mPathStatus;
};

} // namespace app
} // namespace chip

namespace {

class TestWrite : public ::testing::Test
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        if (mpContext == nullptr)
        {
            mpContext = new TestContext();
            ASSERT_NE(mpContext, nullptr);
        }
        mpContext->SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        mpContext->TearDownTestSuite();
        if (mpContext != nullptr)
        {
            delete mpContext;
            mpContext = nullptr;
        }
    }

    void ResetCallback() { mSingleWriteCallback.reset(); }

    void PrepareWriteCallback(ConcreteAttributePath path) { mSingleWriteCallback = std::make_unique<SingleWriteCallback>(path); }

    SingleWriteCallback * GetWriteCallback() { return mSingleWriteCallback.get(); }

protected:
    // Performs setup for each individual test in the test suite
    void SetUp() { mpContext->SetUp(); }

    // Performs teardown for each individual test in the test suite
    void TearDown() { mpContext->TearDown(); }

    static TestContext * mpContext;

    std::unique_ptr<SingleWriteCallback> mSingleWriteCallback;
};

TestContext * TestWrite::mpContext = nullptr;

TEST_F(TestWrite, TestDataResponse)
{
    auto sessionHandle      = mpContext->GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    gResponseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessCbInvoked && !onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestDataResponseWithAcceptedDataVersion)
{
    auto sessionHandle      = mpContext->GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    gResponseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Optional<chip::DataVersion> dataVersion;
    dataVersion.SetValue(kAcceptedDataVersion);
    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb, nullptr, dataVersion);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessCbInvoked && !onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestDataResponseWithRejectedDataVersion)
{
    auto sessionHandle      = mpContext->GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    gResponseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Optional<chip::DataVersion> dataVersion(kRejectedDataVersion);
    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb, nullptr, dataVersion);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestAttributeError)
{
    auto sessionHandle      = mpContext->GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Attributes::ListStructOctetString::TypeInfo::Type value;
    Structs::TestListStructOctet::Type valueBuf[4];

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    gResponseDirective = kSendAttributeError;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        EXPECT_TRUE(attributePath != nullptr);
        onFailureCbInvoked = true;
    };

    Controller::WriteAttribute<Attributes::ListStructOctetString::TypeInfo>(sessionHandle, kTestEndpointId, value, onSuccessCb,
                                                                            onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestFabricScopedAttributeWithoutFabricIndex)
{
    auto sessionHandle      = mpContext->GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestFabricScoped::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListFabricScoped::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.fabricIndex = i;
        i++;
    }

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        EXPECT_EQ(aError, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListFabricScoped::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestMultipleSuccessResponses)
{
    auto sessionHandle  = mpContext->GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    gResponseDirective = kSendMultipleSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_EQ(successCalls, 1u);
    EXPECT_EQ(failureCalls, 0u);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestMultipleFailureResponses)
{
    auto sessionHandle  = mpContext->GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    gResponseDirective = kSendMultipleErrors;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_EQ(successCalls, 0u);
    EXPECT_EQ(failureCalls, 1u);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestWriteClusterSpecificStatuses)
{
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    // Cluster-specific success code case
    {
        gResponseDirective = kSendClusterSpecificSuccess;

        this->ResetCallback();
        this->PrepareWriteCallback(
            ConcreteAttributePath{ kTestEndpointId, Clusters::UnitTesting::Id, Clusters::UnitTesting::Attributes::Int8u::Id });

        SingleWriteCallback * writeCb = this->GetWriteCallback();

        WriteClient writeClient(&mpContext->GetExchangeManager(), this->GetWriteCallback(), Optional<uint16_t>::Missing());
        AttributePathParams attributePath{ kTestEndpointId, Clusters::UnitTesting::Id,
                                           Clusters::UnitTesting::Attributes::Int8u::Id };
        constexpr uint8_t attributeValue = 1u;
        ASSERT_EQ(writeClient.EncodeAttribute(attributePath, attributeValue), CHIP_NO_ERROR);
        ASSERT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);

        mpContext->DrainAndServiceIO();

        EXPECT_TRUE(writeCb->WasDone());
        EXPECT_TRUE(writeCb->PathWasResponded());
        EXPECT_EQ(writeCb->GetLastChipError(), CHIP_NO_ERROR);

        StatusIB pathStatus = writeCb->GetPathStatus();
        EXPECT_EQ(pathStatus.mStatus, Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(pathStatus.mClusterStatus.HasValue());
        EXPECT_EQ(pathStatus.mClusterStatus.Value(), kExampleClusterSpecificSuccess);

        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
        EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    // Cluster-specific failure code case
    {
        gResponseDirective = kSendClusterSpecificFailure;

        this->ResetCallback();
        this->PrepareWriteCallback(
            ConcreteAttributePath{ kTestEndpointId, Clusters::UnitTesting::Id, Clusters::UnitTesting::Attributes::Int8u::Id });

        SingleWriteCallback * writeCb = this->GetWriteCallback();

        WriteClient writeClient(&mpContext->GetExchangeManager(), this->GetWriteCallback(), Optional<uint16_t>::Missing());
        AttributePathParams attributePath{ kTestEndpointId, Clusters::UnitTesting::Id,
                                           Clusters::UnitTesting::Attributes::Int8u::Id };

        constexpr uint8_t attributeValue = 2u;
        ASSERT_EQ(writeClient.EncodeAttribute(attributePath, attributeValue), CHIP_NO_ERROR);
        ASSERT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);

        mpContext->DrainAndServiceIO();

        EXPECT_TRUE(writeCb->WasDone());
        EXPECT_TRUE(writeCb->PathWasResponded());
        EXPECT_EQ(writeCb->GetLastChipError(), CHIP_NO_ERROR);

        StatusIB pathStatus = writeCb->GetPathStatus();
        EXPECT_EQ(pathStatus.mStatus, Protocols::InteractionModel::Status::Failure);
        ASSERT_TRUE(pathStatus.mClusterStatus.HasValue());
        EXPECT_EQ(pathStatus.mClusterStatus.Value(), kExampleClusterSpecificFailure);

        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
        EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}

} // namespace
