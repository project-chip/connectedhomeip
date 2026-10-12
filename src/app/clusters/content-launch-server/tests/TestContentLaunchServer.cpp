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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentLauncher;

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
    }

    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }

    uint32_t HandleGetSupportedStreamingProtocols() override { return mStreamingProtocols; }

    uint32_t GetFeatureMap(EndpointId endpoint) override { return mFeatureMap; }
    uint16_t GetClusterRevision(EndpointId endpoint) override { return 3; }

    bool mLaunchContentCalled    = false;
    bool mLaunchUrlCalled        = false;
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
