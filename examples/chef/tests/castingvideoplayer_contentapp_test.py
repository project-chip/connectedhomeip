#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_CASTINGVIDEOPLAYER(MatterBaseTest):
    """Tests for chef castingvideoplayer device."""

    CASTINGVIDEOPLAYER_ENDPOINT = 1

    def desc_TC_CASTINGVIDEOPLAYER(self) -> str:
        return "[TC_CASTINGVIDEOPLAYER] chef castingvideoplayer functionality test."

    def steps_TC_CASTINGVIDEOPLAYER(self):
        return [TestStep(1, "[TC_CASTINGVIDEOPLAYER] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_CASTINGVIDEOPLAYER] Test media playback."),
                TestStep(3, "[TC_CASTINGVIDEOPLAYER] Test wake on lan."),
                TestStep(4, "[TC_CASTINGVIDEOPLAYER] Test channel."),
                TestStep(5, "[TC_CASTINGVIDEOPLAYER] Test on/off."),
                TestStep(6, "[TC_CASTINGVIDEOPLAYER] Test content launcher.")]

    async def _read_on_off(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_media_playback_current_state(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.MediaPlayback, attribute=Clusters.Objects.MediaPlayback.Attributes.CurrentState)

    async def _read_media_playback_playback_speed(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.MediaPlayback, attribute=Clusters.Objects.MediaPlayback.Attributes.PlaybackSpeed)

    async def _read_media_playback_sampled_position(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.MediaPlayback, attribute=Clusters.Objects.MediaPlayback.Attributes.SampledPosition)

    async def _read_wake_on_lan_mac_address(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.WakeOnLan, attribute=Clusters.Objects.WakeOnLan.Attributes.MACAddress)

    async def _read_channel_current_channel(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.Channel, attribute=Clusters.Objects.Channel.Attributes.CurrentChannel)

    async def _read_channel_channel_list(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.Channel, attribute=Clusters.Objects.Channel.Attributes.ChannelList)

    async def on_off_test(self, endpoint):
        # Set initial state to On
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.On(),
            endpoint=endpoint,
        )
        asserts.assert_true(await self._read_on_off(endpoint), "OnOff attribute should be True after On command.")

        # Send Off command and verify
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.Off(),
            endpoint=endpoint,
        )
        asserts.assert_false(await self._read_on_off(endpoint), "OnOff attribute should be False after Off command.")

        # Send On command and verify
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.On(),
            endpoint=endpoint,
        )
        asserts.assert_true(await self._read_on_off(endpoint), "OnOff attribute should be True after On command.")

    async def media_playback_test(self, endpoint):
        # Play
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Play(),
            endpoint=endpoint,
        )
        asserts.assert_equal(await self._read_media_playback_current_state(endpoint), Clusters.Objects.MediaPlayback.Enums.PlaybackStateEnum.kPlaying)
        asserts.assert_equal(await self._read_media_playback_playback_speed(endpoint), 1.0)

        # Pause
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Pause(),
            endpoint=endpoint,
        )
        asserts.assert_equal(await self._read_media_playback_current_state(endpoint), Clusters.Objects.MediaPlayback.Enums.PlaybackStateEnum.kPaused)
        asserts.assert_equal(await self._read_media_playback_playback_speed(endpoint), 0.0)

        # Stop
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Play(),
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Stop(),
            endpoint=endpoint,
        )
        asserts.assert_equal(await self._read_media_playback_current_state(endpoint), Clusters.Objects.MediaPlayback.Enums.PlaybackStateEnum.kNotPlaying)
        asserts.assert_equal(await self._read_media_playback_playback_speed(endpoint), 0.0)

        # StartOver
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.StartOver(),
            endpoint=endpoint,
        )
        pos = await self._read_media_playback_sampled_position(endpoint)
        asserts.assert_equal(pos.position, 0)

        # Previous
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Previous(),
            endpoint=endpoint,
        )

        # Next
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Next(),
            endpoint=endpoint,
        )

        # Rewind
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Play(),
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Rewind(),
            endpoint=endpoint,
        )
        asserts.assert_equal(await self._read_media_playback_playback_speed(endpoint), -1.0)

        # FastForward
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Play(),
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.FastForward(),
            endpoint=endpoint,
        )
        asserts.assert_equal(await self._read_media_playback_playback_speed(endpoint), 2.0)

        # SkipForward/Backward/Seek
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Pause(),
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.StartOver(),
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.SkipForward(deltaPositionMilliseconds=1000),
            endpoint=endpoint,
        )
        pos = await self._read_media_playback_sampled_position(endpoint)
        asserts.assert_equal(pos.position, 1000)

        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.SkipBackward(deltaPositionMilliseconds=1000),
            endpoint=endpoint,
        )
        pos = await self._read_media_playback_sampled_position(endpoint)
        asserts.assert_equal(pos.position, 0)

        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.Seek(position=5000),
            endpoint=endpoint,
        )
        pos = await self._read_media_playback_sampled_position(endpoint)
        asserts.assert_equal(pos.position, 5000)

    async def wake_on_lan_test(self, endpoint):
        mac = await self._read_wake_on_lan_mac_address(endpoint)
        asserts.assert_true(mac, "Mac address is empty.")

    async def channel_test(self, endpoint):
        current_channel = await self._read_channel_current_channel(endpoint)
        channels_list = await self._read_channel_channel_list(endpoint)

        for next_channel in channels_list:
            if current_channel == next_channel:
                continue

            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.ChangeChannel(match=f"{next_channel.majorNumber}.{next_channel.minorNumber}"),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber, next_channel.majorNumber)
            asserts.assert_equal(current_channel.minorNumber, next_channel.minorNumber)

        for next_channel in channels_list:
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.ChangeChannelByNumber(
                    majorNumber=next_channel.majorNumber, minorNumber=next_channel.minorNumber),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber, next_channel.majorNumber)
            asserts.assert_equal(current_channel.minorNumber, next_channel.minorNumber)

        if len(channels_list) > 1:
            # SkipChannel
            num_channels = len(channels_list)
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.ChangeChannel(
                    match=f"{channels_list[0].majorNumber}.{channels_list[0].minorNumber}"),
                endpoint=endpoint,
            )
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.SkipChannel(count=num_channels - 1),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber, channels_list[-1].majorNumber)

            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.SkipChannel(count=1 - num_channels),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber, channels_list[0].majorNumber)

    async def content_launcher_interstellar_test(self, endpoint):
        # Launch Interstellar
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kType, value="Movie"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kVideo, value="Interstellar"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kDirector, value="Christopher Nolan"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kActor, value="Matthew McConaughey"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kActor, value="Anne Hathaway"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kGenre, value="Sci-Fi"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kSuccess)

    async def content_launcher_man_united_match_test(self, endpoint):
        # Launch Man United Match
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kType, value="SportsEvent"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kLeague, value="Premier League"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kSportsTeam, value="Manchester United"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kSport, value="Football"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kSuccess)

    async def content_launcher_unavailable_test(self, endpoint):
        # Try to launch non-existent content
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kVideo, value="NonExistentMovie"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kURLNotAvailable)

    @async_test_body
    async def test_TC_CASTINGVIDEOPLAYER(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        await self.media_playback_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(3)
        await self.wake_on_lan_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(4)
        await self.channel_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(5)
        await self.on_off_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(6)
        await self.content_launcher_interstellar_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_man_united_match_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_unavailable_test(self.CASTINGVIDEOPLAYER_ENDPOINT)


if __name__ == "__main__":
    default_matter_test_main()
