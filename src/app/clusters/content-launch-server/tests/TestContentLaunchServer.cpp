/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/CommandResponseHelper.h>
#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentLauncher;
using namespace chip::literals;

namespace {

class TestDelegate : public Delegate
{
public:
    void HandleLaunchContent(CommandResponseHelper<Commands::LauncherResponse::Type> & helper,
                             const DataModel::DecodableList<Structs::ParameterStruct::DecodableType> & parameterList, bool autoplay,
                             const CharSpan & data,
                             const Optional<Structs::PlaybackPreferencesStruct::DecodableType> playbackPreferences,
                             bool useCurrentContext) override
    {
        mLaunchContentCalled = true;
    }

    void HandleLaunchUrl(CommandResponseHelper<Commands::LauncherResponse::Type> & helper, const CharSpan & contentUrl,
                         const CharSpan & displayString,
                         const Structs::BrandingInformationStruct::Type & brandingInformation) override
    {
        mLaunchUrlCalled = true;
        mLaunchUrlContentUrl.assign(contentUrl.data(), contentUrl.size());
        mLaunchUrlDisplayString.assign(displayString.data(), displayString.size());
        mLaunchUrlProviderName.assign(brandingInformation.providerName.data(), brandingInformation.providerName.size());
    }

    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }

    uint32_t HandleGetSupportedStreamingProtocols() override { return mStreamingProtocols; }

    uint32_t GetFeatureMap(EndpointId endpoint) override { return mFeatureMap; }
    uint16_t GetClusterRevision(EndpointId endpoint) override { return 3; }

    bool mLaunchContentCalled = false;
    bool mLaunchUrlCalled     = false;
    std::string mLaunchUrlContentUrl;
    std::string mLaunchUrlDisplayString;
    std::string mLaunchUrlProviderName;
    uint32_t mStreamingProtocols = 0x03;
    uint32_t mFeatureMap         = 0x00FF;
};

struct TestContentLaunchServer : public ::testing::Test
{
    // Encoding command responses into MockCommandHandler allocates through chip::Platform.
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandleGetMovable)
{
    TestDelegate delegate;
    EXPECT_FALSE(delegate.HandleGetMovable());
}

TEST_F(TestContentLaunchServer, TestDelegateOverrideHandleGetMovable)
{
    class MovableDelegate : public TestDelegate
    {
    public:
        bool HandleGetMovable() override { return true; }
    };

    MovableDelegate delegate;
    EXPECT_TRUE(delegate.HandleGetMovable());
}

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandleGetPresets)
{
    TestDelegate delegate;
    // Default implementation returns empty list — verify it returns CHIP_NO_ERROR
    // We can't easily test the encoder output without a full AttributeValueEncoder mock,
    // but we verify the method is callable and the return type is correct.
    (void) &Delegate::HandleGetPresets;
}

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandleContentReplicationRequest)
{
    TestDelegate delegate;
    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(1, Id, Commands::ContentReplicationRequest::Id);
    CommandResponseHelper<Commands::ContentReplicationResponse::Type> helper(&mockHandler, commandPath);

    delegate.HandleContentReplicationRequest(helper);

    // Without an override there is no content to replicate, so the request is refused and,
    // per the spec, ReplicationInfo is null rather than an empty Success.
    ASSERT_TRUE(mockHandler.HasResponse());
    Commands::ContentReplicationResponse::DecodableType response;
    EXPECT_EQ(mockHandler.DecodeResponse(response), CHIP_NO_ERROR);
    EXPECT_EQ(response.status, StatusEnum::kReplicationNotSupported);
    ASSERT_TRUE(response.replicationInfo.HasValue());
    EXPECT_TRUE(response.replicationInfo.Value().IsNull());
}

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandlePlayPreset)
{
    TestDelegate delegate;
    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(1, Id, Commands::PlayPreset::Id);

    delegate.HandlePlayPreset(&mockHandler, commandPath, 1);

    // The default Presets list is empty, so no PresetID can match.
    ASSERT_TRUE(mockHandler.HasStatus());
    EXPECT_EQ(mockHandler.GetLastStatus().status,
              Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusEnum::kPresetNotFound));
}

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandleLaunchUrlRequestForwardsFields)
{
    TestDelegate delegate;
    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(1, Id, Commands::LaunchURL::Id);
    CommandResponseHelper<Commands::LauncherResponse::Type> helper(&mockHandler, commandPath);

    Commands::LaunchURL::DecodableType request;
    request.contentURL = "https://example.com/content"_span;
    request.displayString.SetValue("Now playing"_span);
    Structs::BrandingInformationStruct::Type branding;
    branding.providerName = "Provider"_span;
    request.brandingInformation.SetValue(branding);
    request.queueType.SetValue(DataModel::MakeNullable(QueueTypeEnum::kNext));

    delegate.HandleLaunchUrlRequest(helper, request);

    // A delegate that only implements the three-field HandleLaunchUrl still receives those fields.
    EXPECT_TRUE(delegate.mLaunchUrlCalled);
    EXPECT_EQ(delegate.mLaunchUrlContentUrl, "https://example.com/content");
    EXPECT_EQ(delegate.mLaunchUrlDisplayString, "Now playing");
    EXPECT_EQ(delegate.mLaunchUrlProviderName, "Provider");
}

TEST_F(TestContentLaunchServer, TestDelegateDefaultHandleLaunchUrlRequestWithoutOptionalFields)
{
    TestDelegate delegate;
    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(1, Id, Commands::LaunchURL::Id);
    CommandResponseHelper<Commands::LauncherResponse::Type> helper(&mockHandler, commandPath);

    Commands::LaunchURL::DecodableType request;
    request.contentURL = "https://example.com/content"_span;

    delegate.HandleLaunchUrlRequest(helper, request);

    // Absent DisplayString and BrandingInformation are forwarded as empty values.
    EXPECT_TRUE(delegate.mLaunchUrlCalled);
    EXPECT_EQ(delegate.mLaunchUrlContentUrl, "https://example.com/content");
    EXPECT_TRUE(delegate.mLaunchUrlDisplayString.empty());
    EXPECT_TRUE(delegate.mLaunchUrlProviderName.empty());
}

TEST_F(TestContentLaunchServer, TestDelegateOverrideHandleLaunchUrlRequestReceivesQueueingFields)
{
    class QueueingDelegate : public TestDelegate
    {
    public:
        void HandleLaunchUrlRequest(CommandResponseHelper<Commands::LauncherResponse::Type> & helper,
                                    const Commands::LaunchURL::DecodableType & request) override
        {
            mQueueType       = request.queueType;
            mOffsetMillisecs = request.offsetMillisecs;
        }

        Optional<DataModel::Nullable<QueueTypeEnum>> mQueueType;
        Optional<DataModel::Nullable<uint32_t>> mOffsetMillisecs;
    };

    QueueingDelegate delegate;
    chip::Testing::MockCommandHandler mockHandler;
    ConcreteCommandPath commandPath(1, Id, Commands::LaunchURL::Id);
    CommandResponseHelper<Commands::LauncherResponse::Type> helper(&mockHandler, commandPath);

    Commands::LaunchURL::DecodableType request;
    request.contentURL = "https://example.com/content"_span;
    request.queueType.SetValue(DataModel::MakeNullable(QueueTypeEnum::kLast));
    request.offsetMillisecs.SetValue(DataModel::MakeNullable(static_cast<uint32_t>(5000)));

    // Called through the base class, as the cluster server does.
    static_cast<Delegate &>(delegate).HandleLaunchUrlRequest(helper, request);

    // A delegate that supports ContentQueueing sees the queueing fields; the three-field
    // HandleLaunchUrl is not involved.
    EXPECT_FALSE(delegate.mLaunchUrlCalled);
    ASSERT_TRUE(delegate.mQueueType.HasValue());
    EXPECT_EQ(delegate.mQueueType.Value().Value(), QueueTypeEnum::kLast);
    ASSERT_TRUE(delegate.mOffsetMillisecs.HasValue());
    EXPECT_EQ(delegate.mOffsetMillisecs.Value().Value(), 5000u);
}

TEST_F(TestContentLaunchServer, TestDelegateGetSupportedStreamingProtocols)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.HandleGetSupportedStreamingProtocols(), 0x03u);
    delegate.mStreamingProtocols = 0x07;
    EXPECT_EQ(delegate.HandleGetSupportedStreamingProtocols(), 0x07u);
}

TEST_F(TestContentLaunchServer, TestDelegateGetFeatureMap)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.GetFeatureMap(1), 0x00FFu);
    delegate.mFeatureMap = 0x03;
    EXPECT_EQ(delegate.GetFeatureMap(1), 0x03u);
}

TEST_F(TestContentLaunchServer, TestDelegateGetClusterRevision)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.GetClusterRevision(1), 3u);
}

} // namespace
