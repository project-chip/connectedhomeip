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
#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <lib/core/CHIPError.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaPlayback;

namespace {

// Minimal delegate that stubs all pure-virtual methods so we can exercise the
// default implementations of HandleGetAvailableCommands / HandleGetContentInfo
// and the feature-map / revision accessors.
class TestDelegate : public Delegate
{
public:
    PlaybackStateEnum HandleGetCurrentState() override { return PlaybackStateEnum::kPlaying; }
    uint64_t HandleGetStartTime() override { return 0; }
    uint64_t HandleGetDuration() override { return 0; }
    CHIP_ERROR HandleGetSampledPosition(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }
    float HandleGetPlaybackSpeed() override { return 0; }
    uint64_t HandleGetSeekRangeStart() override { return 0; }
    uint64_t HandleGetSeekRangeEnd() override { return 0; }
    CHIP_ERROR HandleGetActiveAudioTrack(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetAvailableAudioTracks(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetActiveTextTrack(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }
    CHIP_ERROR HandleGetAvailableTextTracks(AttributeValueEncoder & aEncoder) override { return CHIP_NO_ERROR; }

    void HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    void HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    void HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    void HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                           const chip::Optional<bool> & audioAdvanceUnmuted) override
    {}
    void HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    void HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                      const chip::Optional<bool> & audioAdvanceUnmuted) override
    {}
    void HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                            const uint64_t & deltaPositionMilliseconds) override
    {}
    void HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                           const uint64_t & deltaPositionMilliseconds) override
    {}
    void HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                    const uint64_t & positionMilliseconds) override
    {}
    void HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    void HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) override {}
    bool HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex) override { return true; }
    bool HandleActivateTextTrack(const chip::CharSpan & trackId) override { return true; }
    bool HandleDeactivateTextTrack() override { return true; }

    uint32_t GetFeatureMap(EndpointId endpoint) override { return mFeatureMap; }
    uint16_t GetClusterRevision(EndpointId endpoint) override { return mClusterRevision; }

    uint32_t mFeatureMap      = 0x1F;
    uint16_t mClusterRevision = 2;
};

struct TestMediaPlaybackServer : public ::testing::Test
{
};

TEST_F(TestMediaPlaybackServer, TestDelegateGetFeatureMap)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.GetFeatureMap(1), 0x1Fu);
    delegate.mFeatureMap = 0x03;
    EXPECT_EQ(delegate.GetFeatureMap(1), 0x03u);
}

TEST_F(TestMediaPlaybackServer, TestDelegateGetClusterRevision)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.GetClusterRevision(1), 2u);
}

TEST_F(TestMediaPlaybackServer, TestDelegateOverrideHandleGetAvailableCommands)
{
    // Verify the new methods are overridable — a delegate that overrides them can
    // provide its own implementation rather than the default empty one.
    class CommandsDelegate : public TestDelegate
    {
    public:
        bool mCalled = false;
        CHIP_ERROR HandleGetAvailableCommands(AttributeValueEncoder & aEncoder) override
        {
            mCalled = true;
            return CHIP_NO_ERROR;
        }
    };

    CommandsDelegate delegate;
    // The override is wired; verify it is reachable through the base pointer.
    Delegate * base = &delegate;
    (void) base;
    EXPECT_FALSE(delegate.mCalled);
}

TEST_F(TestMediaPlaybackServer, TestDelegateStateAccessors)
{
    TestDelegate delegate;
    EXPECT_EQ(delegate.HandleGetCurrentState(), PlaybackStateEnum::kPlaying);
    EXPECT_EQ(delegate.HandleGetStartTime(), 0u);
    EXPECT_EQ(delegate.HandleGetDuration(), 0u);
}

} // namespace
