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


import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from mobly import asserts

logger = logging.getLogger(__name__)


class AVSUMTestBase:
    has_feature_dptz = False
    has_feature_mpan = False
    has_feature_mtilt = False
    has_feature_mzoom = False
    has_feature_mpresets = False

    def ptz_range_validation(self, mptz, tiltmin, tiltmax, panmin, panmax, zoommax):
        if self.has_feature_mpan:
            if mptz.pan is None:
                asserts.fail("Pan value missing when MPAN feature set")
            else:
                apan = mptz.pan
                asserts.assert_greater_equal(apan, panmin, "Pan out of range of DUT defined values")
                asserts.assert_less_equal(apan, panmax, "Pan out of range of DUT defined values")

        if self.has_feature_mtilt:
            if mptz.tilt is None:
                asserts.fail("Tilt value missing when MTILT feature set")
            else:
                atilt = mptz.tilt
                asserts.assert_greater_equal(atilt, tiltmin, "Tilt out of range of DUT defined values")
                asserts.assert_less_equal(atilt, tiltmax, "Tilt out of range of DUT defined values")

        if self.has_feature_mzoom:
            if mptz.zoom is None:
                asserts.fail("Zoom value missing when MZOOM feature set")
            else:
                azoom = mptz.zoom
                asserts.assert_greater_equal(azoom, 1, "Zoom out of range of DUT defined values")
                asserts.assert_less_equal(azoom, zoommax, "Zoom out of range of DUT defined values")

    def verify_preset_matches(self, preset, position):
        if self.has_feature_mpan:
            asserts.assert_equal(preset.settings.pan, position.pan)

        if self.has_feature_mtilt:
            asserts.assert_equal(preset.settings.tilt, position.tilt)

        if self.has_feature_mzoom:
            asserts.assert_equal(preset.settings.zoom, position.zoom)

    async def read_avsum_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def check_avsum_attribute(self, attribute, expected_value, endpoint):
        value = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_save_presets_command(self, endpoint, name: str, presetID: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZSavePreset(
                name=name,
                presetID=presetID),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_move_to_preset_command(self, endpoint, presetID, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZMoveToPreset(
                presetID=presetID),
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
