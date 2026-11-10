/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/keypad-input-server/KeypadInputCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/KeypadInput/Commands.h>
#include <clusters/KeypadInput/Enums.h>
#include <clusters/KeypadInput/Metadata.h>

#include <optional>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadInput;
using namespace chip::Testing;

using Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockKeypadInputDelegate : public KeypadInput::Delegate
{
public:
    enum class Reply
    {
        kResponse,
        kStatus,
        kNothing,
    };

    void HandleSendKey(CommandResponseHelper<Commands::SendKeyResponse::Type> & helper, const CECKeyCodeEnum & keyCode) override
    {
        sendKeyCount++;
        lastKeyCode = keyCode;

        switch (reply)
        {
        case Reply::kResponse: {
            Commands::SendKeyResponse::Type response;
            response.status = responseStatus;
            EXPECT_EQ(helper.Success(response), CHIP_NO_ERROR);
            break;
        }
        case Reply::kStatus:
            EXPECT_EQ(helper.Failure(failureStatus), CHIP_NO_ERROR);
            break;
        case Reply::kNothing:
            break;
        }
    }

    uint32_t GetFeatureMap(EndpointId endpoint) override
    {
        lastFeatureMapEndpoint = endpoint;
        return featureMap;
    }

    Reply reply                       = Reply::kResponse;
    StatusEnum responseStatus         = StatusEnum::kSuccess;
    Status failureStatus              = Status::Failure;
    uint32_t featureMap               = 0;
    size_t sendKeyCount               = 0;
    CECKeyCodeEnum lastKeyCode        = CECKeyCodeEnum::kUnknownEnumValue;
    EndpointId lastFeatureMapEndpoint = kInvalidEndpointId;
};

struct TestKeypadInputCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override { mCluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    ClusterTester::InvokeResult<Commands::SendKeyResponse::DecodableType> SendKey(CECKeyCodeEnum keyCode)
    {
        Commands::SendKey::Type request;
        request.keyCode = keyCode;
        return mClusterTester.Invoke(request);
    }

    MockKeypadInputDelegate mDelegate;
    KeypadInputCluster mCluster{ kTestEndpointId, mDelegate };
    ClusterTester mClusterTester{ mCluster };
};

TEST_F(TestKeypadInputCluster, AttributeListContainsOnlyGlobalAttributes)
{
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, std::vector<DataModel::AttributeEntry>{}));
}

TEST_F(TestKeypadInputCluster, CommandLists)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster, { Commands::SendKey::kMetadataEntry }));
    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(mCluster, { Commands::SendKeyResponse::Id }));
}

TEST_F(TestKeypadInputCluster, ReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, KeypadInput::kRevision);
}

TEST_F(TestKeypadInputCluster, ReadFeatureMapFromDelegate)
{
    const uint32_t kAllFeatures =
        to_underlying(Feature::kNavigationKeyCodes) | to_underlying(Feature::kLocationKeys) | to_underlying(Feature::kNumberKeys);

    const uint32_t kExpectedFeatureMaps[] = { 0, to_underlying(Feature::kLocationKeys), kAllFeatures };

    for (uint32_t expected : kExpectedFeatureMaps)
    {
        mDelegate.featureMap = expected;

        uint32_t featureMap = 0xFFFFFFFF;
        ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, expected);
        EXPECT_EQ(mDelegate.lastFeatureMapEndpoint, kTestEndpointId);
    }
}

TEST_F(TestKeypadInputCluster, DelegateHasFeatureUsesFeatureMap)
{
    mDelegate.featureMap = to_underlying(Feature::kNumberKeys);

    EXPECT_TRUE(mDelegate.HasFeature(kTestEndpointId, Feature::kNumberKeys));
    EXPECT_FALSE(mDelegate.HasFeature(kTestEndpointId, Feature::kNavigationKeyCodes));
    EXPECT_FALSE(mDelegate.HasFeature(kTestEndpointId, Feature::kLocationKeys));
}

TEST_F(TestKeypadInputCluster, SendKeyForwardsKeyCodeAndReturnsDelegateResponse)
{
    for (StatusEnum status : { StatusEnum::kSuccess, StatusEnum::kUnsupportedKey, StatusEnum::kInvalidKeyInCurrentState })
    {
        mDelegate.responseStatus = status;

        auto result = SendKey(CECKeyCodeEnum::kUp);
        ASSERT_TRUE(result.IsSuccess());
        if (result.response.has_value())
        {
            EXPECT_EQ(result.response->status, status);
        }
        EXPECT_EQ(mDelegate.lastKeyCode, CECKeyCodeEnum::kUp);
    }
    EXPECT_EQ(mDelegate.sendKeyCount, 3u);
}

TEST_F(TestKeypadInputCluster, SendKeyReturnsDelegateStatus)
{
    mDelegate.reply         = MockKeypadInputDelegate::Reply::kStatus;
    mDelegate.failureStatus = Status::InvalidInState;

    auto result = SendKey(CECKeyCodeEnum::kSelect);
    EXPECT_EQ(result.GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));
    EXPECT_FALSE(mClusterTester.GetCommandHandler().HasResponse());
    EXPECT_EQ(mClusterTester.GetCommandHandler().GetStatuses().size(), 1u);
}

// A delegate that returns without answering must still produce exactly one status for the command.
TEST_F(TestKeypadInputCluster, SendKeyWithoutDelegateAnswerReturnsFailure)
{
    mDelegate.reply = MockKeypadInputDelegate::Reply::kNothing;

    auto result = SendKey(CECKeyCodeEnum::kNumbers5);
    EXPECT_EQ(result.GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::Failure));
    EXPECT_FALSE(mClusterTester.GetCommandHandler().HasResponse());
    EXPECT_FALSE(mClusterTester.GetCommandHandler().HasStatus());
    EXPECT_EQ(mDelegate.sendKeyCount, 1u);
    EXPECT_EQ(mDelegate.lastKeyCode, CECKeyCodeEnum::kNumbers5);
}

} // namespace
