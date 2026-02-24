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
    APP_A_ENDPOINT = 2
    APP_A_VENDOR_ID = 0xFFF1
    APP_A_ID = "Application_A_ID"

    def desc_TC_CASTINGVIDEOPLAYER(self) -> str:
        return "[TC_CASTINGVIDEOPLAYER] chef castingvideoplayer functionality test."

    def steps_TC_CASTINGVIDEOPLAYER(self):
        return [TestStep(1, "[TC_CASTINGVIDEOPLAYER] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_MEDIA_PLAYBACK] Test media playback."),
                TestStep(3, "[TC_WAKE_ON_LAN] Test wake on lan."),
                TestStep(4, "[TC_CHANNEL] Test channel."),
                TestStep(5, "[TC_ON_OFF] Test on/off."),
                TestStep(6, "[TC_CONTENT_LAUNCHER] Test content launcher."),
                TestStep(7, "[TC_APPLICATION_BASIC] Test application basic."),
                TestStep(
                    8, "[APP_LAUNCH_CUJ] Launch and stop app A using platform endpoint."),
                TestStep(
                    9, "[APP_LAUNCH_CUJ] Launch, hide and stop app A using platform endpoint."),
                TestStep(
                    10, "[APP_LAUNCH_CUJ] Launch and stop app A using app endpoint."),
                TestStep(
                    11, "[APP_LAUNCH_CUJ] Launch, hide and stop app A using app endpoint."),
                TestStep(12, "[TC_TARGET_NAVIGATOR] Test target navigator.")]

    async def _read_application_launcher_current_app(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationLauncher, attribute=Clusters.Objects.ApplicationLauncher.Attributes.CurrentApp)

    async def _read_application_basic_status(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.Status)

    async def cuj_launch_and_stop_app_a(self, command_on_endpoint):
        app_a = Clusters.Objects.ApplicationLauncher.Structs.ApplicationStruct(
            catalogVendorID=self.APP_A_VENDOR_ID, applicationID=self.APP_A_ID)

        # 1. Launch App A via Platform
        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ApplicationLauncher.Commands.LaunchApp(
                application=app_a),
            endpoint=command_on_endpoint
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ApplicationLauncher.Enums.StatusEnum.kSuccess)

        # 2. Verify Status on App A Endpoint is ActiveVisibleFocus
        status = await self._read_application_basic_status(self.APP_A_ENDPOINT)
        asserts.assert_equal(
            status, Clusters.Objects.ApplicationBasic.Enums.ApplicationStatusEnum.kActiveVisibleFocus)

        # 3. Verify CurrentApp on Platform Endpoint
        current_app = await self._read_application_launcher_current_app(self.CASTINGVIDEOPLAYER_ENDPOINT)
        asserts.assert_equal(
            current_app.application.catalogVendorID, self.APP_A_VENDOR_ID)
        asserts.assert_equal(
            current_app.application.applicationID, self.APP_A_ID)

        # 4. Stop App A via Platform
        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ApplicationLauncher.Commands.StopApp(
                application=app_a),
            endpoint=command_on_endpoint
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ApplicationLauncher.Enums.StatusEnum.kSuccess)

        # 5. Verify CurrentApp on Platform Endpoint is Null
        current_app = await self._read_application_launcher_current_app(self.CASTINGVIDEOPLAYER_ENDPOINT)
        asserts.assert_equal(current_app, Clusters.Types.NullValue)

        # 6. Verify Status on App A Endpoint is Stopped
        status = await self._read_application_basic_status(self.APP_A_ENDPOINT)
        asserts.assert_equal(
            status, Clusters.Objects.ApplicationBasic.Enums.ApplicationStatusEnum.kStopped)

    async def cuj_launch_hide_and_stop_app_a(self, command_on_endpoint):
        app_a = Clusters.Objects.ApplicationLauncher.Structs.ApplicationStruct(
            catalogVendorID=self.APP_A_VENDOR_ID, applicationID=self.APP_A_ID)

        # 1. Launch App A via Platform
        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ApplicationLauncher.Commands.LaunchApp(
                application=app_a),
            endpoint=command_on_endpoint
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ApplicationLauncher.Enums.StatusEnum.kSuccess)

        # 2. Verify Status on App A Endpoint is ActiveVisibleFocus
        status = await self._read_application_basic_status(self.APP_A_ENDPOINT)
        asserts.assert_equal(
            status, Clusters.Objects.ApplicationBasic.Enums.ApplicationStatusEnum.kActiveVisibleFocus)

        # 3. Verify CurrentApp on Platform Endpoint
        current_app = await self._read_application_launcher_current_app(self.CASTINGVIDEOPLAYER_ENDPOINT)
        asserts.assert_equal(
            current_app.application.catalogVendorID, self.APP_A_VENDOR_ID)
        asserts.assert_equal(
            current_app.application.applicationID, self.APP_A_ID)

        # 4. Hide App A via Platform
        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ApplicationLauncher.Commands.HideApp(
                application=app_a),
            endpoint=command_on_endpoint
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ApplicationLauncher.Enums.StatusEnum.kSuccess)

        # 5. Verify CurrentApp on Platform Endpoint is Null
        current_app = await self._read_application_launcher_current_app(self.CASTINGVIDEOPLAYER_ENDPOINT)
        asserts.assert_equal(current_app, Clusters.Types.NullValue)

        # 6. Verify Status on App A Endpoint is Stopped
        status = await self._read_application_basic_status(self.APP_A_ENDPOINT)
        asserts.assert_equal(
            status, Clusters.Objects.ApplicationBasic.Enums.ApplicationStatusEnum.kActiveHidden)

        # 7. Stop App A via Platform
        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ApplicationLauncher.Commands.StopApp(
                application=app_a),
            endpoint=command_on_endpoint
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ApplicationLauncher.Enums.StatusEnum.kSuccess)

        # 8. Verify CurrentApp on Platform Endpoint is Null
        current_app = await self._read_application_launcher_current_app(self.CASTINGVIDEOPLAYER_ENDPOINT)
        asserts.assert_equal(current_app, Clusters.Types.NullValue)

        # 9. Verify Status on App A Endpoint is Stopped
        status = await self._read_application_basic_status(self.APP_A_ENDPOINT)
        asserts.assert_equal(
            status, Clusters.Objects.ApplicationBasic.Enums.ApplicationStatusEnum.kStopped)

    async def _read_on_off(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_application_basic_vendor_name(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.VendorName)

    async def _read_application_basic_vendor_id(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.VendorID)

    async def _read_application_basic_application_name(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.ApplicationName)

    async def _read_application_basic_product_id(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.ProductID)

    async def _read_application_basic_application(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.Application)

    async def _read_application_basic_application_version(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.ApplicationVersion)

    async def _read_application_basic_allowed_vendor_list(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ApplicationBasic, attribute=Clusters.Objects.ApplicationBasic.Attributes.AllowedVendorList)

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

    async def _read_content_launcher_accept_header(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ContentLauncher, attribute=Clusters.Objects.ContentLauncher.Attributes.AcceptHeader)

    async def _read_content_launcher_supported_streaming_protocols(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ContentLauncher, attribute=Clusters.Objects.ContentLauncher.Attributes.SupportedStreamingProtocols)

    async def _read_content_launcher_feature_map(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.ContentLauncher, attribute=Clusters.Objects.ContentLauncher.Attributes.FeatureMap)

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
            cmd=Clusters.Objects.MediaPlayback.Commands.SkipForward(
                deltaPositionMilliseconds=1000),
            endpoint=endpoint,
        )
        pos = await self._read_media_playback_sampled_position(endpoint)
        asserts.assert_equal(pos.position, 1000)

        await self.send_single_cmd(
            cmd=Clusters.Objects.MediaPlayback.Commands.SkipBackward(
                deltaPositionMilliseconds=1000),
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
                cmd=Clusters.Objects.Channel.Commands.ChangeChannel(
                    match=f"{next_channel.majorNumber}.{next_channel.minorNumber}"),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber,
                                 next_channel.majorNumber)
            asserts.assert_equal(current_channel.minorNumber,
                                 next_channel.minorNumber)

        for next_channel in channels_list:
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.ChangeChannelByNumber(
                    majorNumber=next_channel.majorNumber, minorNumber=next_channel.minorNumber),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber,
                                 next_channel.majorNumber)
            asserts.assert_equal(current_channel.minorNumber,
                                 next_channel.minorNumber)

        if len(channels_list) > 1:
            # SkipChannel
            num_channels = len(channels_list)
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.ChangeChannel(
                    match=f"{channels_list[0].majorNumber}.{channels_list[0].minorNumber}"),
                endpoint=endpoint,
            )
            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.SkipChannel(
                    count=num_channels - 1),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber,
                                 channels_list[-1].majorNumber)

            await self.send_single_cmd(
                cmd=Clusters.Objects.Channel.Commands.SkipChannel(
                    count=1 - num_channels),
                endpoint=endpoint,
            )
            current_channel = await self._read_channel_current_channel(endpoint)
            asserts.assert_equal(current_channel.majorNumber,
                                 channels_list[0].majorNumber)

    async def content_launcher_interstellar_test(self, endpoint):
        # Launch Interstellar
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kType, value="Movie"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kVideo, value="Interstellar"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kDirector, value="Christopher Nolan"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kActor, value="Matthew McConaughey"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kActor, value="Anne Hathaway"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kGenre, value="Sci-Fi"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(
            parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(
                search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kSuccess)

    async def content_launcher_man_united_match_test(self, endpoint):
        # Launch Man United Match
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kType, value="SportsEvent"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kLeague, value="Premier League"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kSportsTeam, value="Manchester United"),
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kSport, value="Football"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(
            parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(
                search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kSuccess)

    async def content_launcher_unavailable_test(self, endpoint):
        # Try to launch non-existent content
        params = [
            Clusters.Objects.ContentLauncher.Structs.ParameterStruct(
                type=Clusters.Objects.ContentLauncher.Enums.ParameterEnum.kVideo, value="NonExistentMovie"),
        ]

        search = Clusters.Objects.ContentLauncher.Structs.ContentSearchStruct(
            parameterList=params)

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchContent(
                search=search, autoPlay=True),
            endpoint=endpoint,
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kURLNotAvailable)

    async def content_launcher_launch_url_test(self, endpoint):
        # Launch URL
        url = "https://www.google.com"
        display_string = "Google"

        response = await self.send_single_cmd(
            cmd=Clusters.Objects.ContentLauncher.Commands.LaunchURL(
                contentURL=url, displayString=display_string),
            endpoint=endpoint,
        )
        asserts.assert_equal(
            response.status, Clusters.Objects.ContentLauncher.Enums.StatusEnum.kSuccess)
        asserts.assert_equal(response.data, url)

    async def content_launcher_attribute_test(self, endpoint):
        # Test AcceptHeader
        expected_accept_headers = [
            "application/dash+xml",
            "application/vnd.apple.mpegurl",
            "application/x-mpegurl",
            "video/mp4",
            "video/webm",
            "audio/mp4",
            "audio/mpeg"
        ]
        accept_headers = await self._read_content_launcher_accept_header(endpoint)
        asserts.assert_equal(accept_headers, expected_accept_headers)

        # Test SupportedStreamingProtocols
        supported_protocols = await self._read_content_launcher_supported_streaming_protocols(endpoint)
        asserts.assert_equal(supported_protocols, 0)

        # Test FeatureMap
        feature_map = await self._read_content_launcher_feature_map(endpoint)
        asserts.assert_equal(feature_map, 3)

    async def _read_target_navigator_target_list(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.TargetNavigator, attribute=Clusters.Objects.TargetNavigator.Attributes.TargetList)

    async def _read_target_navigator_current_target(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.TargetNavigator, attribute=Clusters.Objects.TargetNavigator.Attributes.CurrentTarget)

    async def target_navigator_test(self, endpoint):
        target_list = await self._read_target_navigator_target_list(endpoint)
        asserts.assert_true(len(target_list) > 0,
                            "Target list should not be empty.")

        current_target = await self._read_target_navigator_current_target(endpoint)

        for target in target_list:
            if current_target == target.identifier:
                continue

            response = await self.send_single_cmd(
                cmd=Clusters.Objects.TargetNavigator.Commands.NavigateTarget(
                    target=target.identifier, data="test data"),
                endpoint=endpoint,
            )
            asserts.assert_equal(
                response.status, Clusters.Objects.TargetNavigator.Enums.StatusEnum.kSuccess)

            current_target = await self._read_target_navigator_current_target(endpoint)
            asserts.assert_equal(current_target, target.identifier)

    async def application_basic_test(self, endpoint):
        # Test VendorName
        vendor_name = await self._read_application_basic_vendor_name(endpoint)
        asserts.assert_equal(vendor_name, "TEST_VENDOR")

        # Test VendorID
        vendor_id = await self._read_application_basic_vendor_id(endpoint)
        asserts.assert_equal(vendor_id, 0xFFF1)

        # Test ApplicationName
        app_name = await self._read_application_basic_application_name(endpoint)
        asserts.assert_equal(app_name, "Application_A_Name")

        # Test ProductID
        product_id = await self._read_application_basic_product_id(endpoint)
        asserts.assert_equal(product_id, 32768)

        # Test Application
        application = await self._read_application_basic_application(endpoint)
        asserts.assert_equal(application.catalogVendorID, 0xFFF1)
        asserts.assert_equal(application.applicationID, "Application_A_ID")

        # Test ApplicationVersion
        app_version = await self._read_application_basic_application_version(endpoint)
        asserts.assert_equal(app_version, "Version_1")

        # Test AllowedVendorList
        allowed_vendors = await self._read_application_basic_allowed_vendor_list(endpoint)
        asserts.assert_equal(allowed_vendors, [0xFFF1])

    @async_test_body
    async def test_TC_CASTINGVIDEOPLAYER(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        await self.media_playback_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.media_playback_test(self.APP_A_ENDPOINT)

        self.step(3)
        await self.wake_on_lan_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(4)
        await self.channel_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.channel_test(self.APP_A_ENDPOINT)

        self.step(5)
        await self.on_off_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(6)
        await self.content_launcher_interstellar_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_man_united_match_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_unavailable_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_launch_url_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.content_launcher_attribute_test(self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(7)
        await self.application_basic_test(2)

        self.step(8)
        await self.cuj_launch_and_stop_app_a(command_on_endpoint=self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(9)
        await self.cuj_launch_hide_and_stop_app_a(command_on_endpoint=self.CASTINGVIDEOPLAYER_ENDPOINT)

        self.step(10)
        await self.cuj_launch_and_stop_app_a(command_on_endpoint=self.APP_A_ENDPOINT)

        self.step(11)
        await self.cuj_launch_hide_and_stop_app_a(command_on_endpoint=self.APP_A_ENDPOINT)

        self.step(12)
        await self.target_navigator_test(self.CASTINGVIDEOPLAYER_ENDPOINT)
        await self.target_navigator_test(self.APP_A_ENDPOINT)


if __name__ == "__main__":
    default_matter_test_main()
