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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include "DataModelFixtures.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeValueDecoder.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
#include <app/tests/AppTestContext.h>
#include <controller/WriteInteraction.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitTesting;
using namespace chip::app::DataModelTests;
using namespace chip::Protocols;
using namespace chip::Test;

namespace {

const MockNodeConfig & TestMockNodeConfig()
{
    using namespace Clusters::Globals::Attributes;

    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kRootEndpointId, {
            MockClusterConfig(Clusters::IcdManagement::Id, {
                ClusterRevision::Id, FeatureMap::Id,
                Clusters::IcdManagement::Attributes::OperatingMode::Id,
            }),
        }),
        MockEndpointConfig(kTestEndpointId, {
            MockClusterConfig(Clusters::UnitTesting::Id, {
                ClusterRevision::Id, FeatureMap::Id,
                Clusters::UnitTesting::Attributes::Boolean::Id,
                Clusters::UnitTesting::Attributes::Int16u::Id,
                Clusters::UnitTesting::Attributes::Int8u::Id,
                Clusters::UnitTesting::Attributes::ListFabricScoped::Id,
                Clusters::UnitTesting::Attributes::ListStructOctetString::Id,
            }),
        }),
        MockEndpointConfig(kMockEndpoint1, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }, {
                MockEventId(1), MockEventId(2),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
        }),
        MockEndpointConfig(kMockEndpoint2, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
            }),
        }),
        MockEndpointConfig(kMockEndpoint3, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(4), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
        }),
    });
    // clang-format on
    return config;
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

class TestWrite : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        chip::Test::AppContext::SetUp();
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&CustomDataModel::Instance());
        chip::Test::SetMockNodeConfig(TestMockNodeConfig());
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::Test::ResetMockNodeConfig();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        chip::Test::AppContext::TearDown();
    }

    void ResetCallback() { mSingleWriteCallback.reset(); }

    void PrepareWriteCallback(ConcreteAttributePath path) { mSingleWriteCallback = std::make_unique<SingleWriteCallback>(path); }

    SingleWriteCallback * GetWriteCallback() { return mSingleWriteCallback.get(); }

protected:
    std::unique_ptr<SingleWriteCallback> mSingleWriteCallback;
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

TEST_F(TestWrite, TestDataResponse)
{
    auto sessionHandle      = GetSessionBobToAlice();
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

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendAttributeSuccess);

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

    DrainAndServiceIO();

    EXPECT_TRUE(onSuccessCbInvoked);
    EXPECT_FALSE(onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestDataResponseWithAcceptedDataVersion)
{
    auto sessionHandle      = GetSessionBobToAlice();
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

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendAttributeSuccess);

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

    DrainAndServiceIO();

    EXPECT_TRUE(onSuccessCbInvoked && !onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestDataResponseWithRejectedDataVersion)
{
    auto sessionHandle      = GetSessionBobToAlice();
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

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendAttributeSuccess);

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

    DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestAttributeError)
{
    auto sessionHandle      = GetSessionBobToAlice();
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

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendAttributeError);

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

    DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestFabricScopedAttributeWithoutFabricIndex)
{
    auto sessionHandle      = GetSessionBobToAlice();
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

    DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessCbInvoked && onFailureCbInvoked);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestMultipleSuccessResponses)
{
    auto sessionHandle  = GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendMultipleSuccess);

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    DrainAndServiceIO();

    EXPECT_EQ(successCalls, 1u);
    EXPECT_EQ(failureCalls, 0u);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestMultipleFailureResponses)
{
    auto sessionHandle  = GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendMultipleErrors);

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    DrainAndServiceIO();

    EXPECT_EQ(successCalls, 0u);
    EXPECT_EQ(failureCalls, 1u);
    EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestWrite, TestWriteClusterSpecificStatuses)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Cluster-specific success code case
    {
        ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendClusterSpecificSuccess);

        this->ResetCallback();
        this->PrepareWriteCallback(
            ConcreteAttributePath{ kTestEndpointId, Clusters::UnitTesting::Id, Clusters::UnitTesting::Attributes::Int8u::Id });

        SingleWriteCallback * writeCb = this->GetWriteCallback();

        WriteClient writeClient(&GetExchangeManager(), this->GetWriteCallback(), Optional<uint16_t>::Missing());
        AttributePathParams attributePath{ kTestEndpointId, Clusters::UnitTesting::Id,
                                           Clusters::UnitTesting::Attributes::Int8u::Id };
        constexpr uint8_t attributeValue = 1u;
        ASSERT_EQ(writeClient.EncodeAttribute(attributePath, attributeValue), CHIP_NO_ERROR);
        ASSERT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(writeCb->WasDone());
        EXPECT_TRUE(writeCb->PathWasResponded());
        EXPECT_EQ(writeCb->GetLastChipError(), CHIP_NO_ERROR);

        StatusIB pathStatus = writeCb->GetPathStatus();
        EXPECT_EQ(pathStatus.mStatus, Protocols::InteractionModel::Status::Success);
        ASSERT_TRUE(pathStatus.mClusterStatus.HasValue());
        EXPECT_EQ(pathStatus.mClusterStatus.Value(), kExampleClusterSpecificSuccess);

        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    // Cluster-specific failure code case
    {
        ScopedChange directive(gWriteResponseDirective, WriteResponseDirective::kSendClusterSpecificFailure);

        this->ResetCallback();
        this->PrepareWriteCallback(
            ConcreteAttributePath{ kTestEndpointId, Clusters::UnitTesting::Id, Clusters::UnitTesting::Attributes::Int8u::Id });

        SingleWriteCallback * writeCb = this->GetWriteCallback();

        WriteClient writeClient(&GetExchangeManager(), this->GetWriteCallback(), Optional<uint16_t>::Missing());
        AttributePathParams attributePath{ kTestEndpointId, Clusters::UnitTesting::Id,
                                           Clusters::UnitTesting::Attributes::Int8u::Id };

        constexpr uint8_t attributeValue = 2u;
        ASSERT_EQ(writeClient.EncodeAttribute(attributePath, attributeValue), CHIP_NO_ERROR);
        ASSERT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(writeCb->WasDone());
        EXPECT_TRUE(writeCb->PathWasResponded());
        EXPECT_EQ(writeCb->GetLastChipError(), CHIP_NO_ERROR);

        StatusIB pathStatus = writeCb->GetPathStatus();
        EXPECT_EQ(pathStatus.mStatus, Protocols::InteractionModel::Status::Failure);
        ASSERT_TRUE(pathStatus.mClusterStatus.HasValue());
        EXPECT_EQ(pathStatus.mClusterStatus.Value(), kExampleClusterSpecificFailure);

        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}

} // namespace
