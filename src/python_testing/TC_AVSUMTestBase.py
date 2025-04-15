#
#    Copyright (c) 2023 Project CHIP Authors
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


import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts

logger = logging.getLogger(__name__)


class AVSUMTestBase:

    async def read_avsum_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        full_attr = getattr(cluster.Attributes, attribute)
        logging.info(f"endpoint {endpoint} full_attr {full_attr}")
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_avsum_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_save_presets_command(self, endpoint, name: str, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZSavePreset(
                name=name),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_mptz_set_pan_position_command(self, endpoint, pan, expected_status: Status = Status.Success):
        tilt = zoom = None
        await self.send_mptz_set_position_command(endpoint, pan, tilt, zoom, expected_status)

    async def send_mptz_set_tilt_position_command(self, endpoint, tilt, expected_status: Status = Status.Success):
        pan = zoom = None
        await self.send_mptz_set_position_command(endpoint, pan, tilt, zoom, expected_status)

    async def send_mptz_set_zoom_position_command(self, endpoint, zoom, expected_status: Status = Status.Success):
        pan = tilt = None
        await self.send_mptz_set_position_command(endpoint, pan, tilt, zoom, expected_status)

    async def send_mptz_set_position_command(self, endpoint, pan, tilt, zoom, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZSetPosition(
                pan=pan, tilt=tilt, zoom=zoom),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_mptz_relative_move_pan_command(self, endpoint, panDelta, expected_status: Status = Status.Success):
        tiltDelta = zoomDelta = None
        await self.send_mptz_relative_move_command(endpoint, panDelta, tiltDelta, zoomDelta, expected_status)

    async def send_mptz_relative_move_tilt_command(self, endpoint, tiltDelta, expected_status: Status = Status.Success):
        panDelta = zoomDelta = None
        await self.send_mptz_relative_move_command(endpoint, panDelta, tiltDelta, zoomDelta, expected_status)

    async def send_mptz_relative_move_zoom_command(self, endpoint, zoomDelta, expected_status: Status = Status.Success):
        panDelta = tiltDelta = None
        await self.send_mptz_relative_move_command(endpoint, panDelta, tiltDelta, zoomDelta, expected_status)

    async def send_mptz_relative_move_command(self, endpoint, panDelta, tiltDelta, zoomDelta, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZRelativeMove(
                panDelta=panDelta, tiltDelta=tiltDelta, zoomDelta=zoomDelta),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
