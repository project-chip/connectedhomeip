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

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status

log = logging.getLogger(__name__)


class CHIMETestBase:

    async def read_chime_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Chime
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_chime_attribute_expect_success(self, endpoint, attribute, value):
        return await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint)

    async def write_chime_attribute_expect_failure(self, endpoint, attribute, value, status):
        response = await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint, expect_success=False)

        asserts.assert_equal(response, status, "Unexpected error returned")

    async def send_play_chime_sound_command(self, endpoint, chimeID: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.Chime.Commands.PlayChimeSound(
                chimeID=chimeID),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def get_child_speaker_endpoint(self, endpoint):
        parts_list = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.PartsList
        )
        for child_endpoint in parts_list:
            device_type_list = await self.read_single_attribute_check_success(
                endpoint=child_endpoint,
                cluster=Clusters.Objects.Descriptor,
                attribute=Clusters.Objects.Descriptor.Attributes.DeviceTypeList
            )
            for dt in device_type_list:
                if dt.deviceType == 0x0022:  # Speaker device type ID
                    return child_endpoint
        return None

    async def unmute_speaker_if_present(self, speaker_endpoint):
        if speaker_endpoint is not None:
            await self.send_single_cmd(
                cmd=Clusters.OnOff.Commands.On(),
                endpoint=speaker_endpoint
            )
        else:
            log.info("No child Speaker endpoint found, skipping unmute initialization step")

    async def set_speaker_volume_high_if_supported(self, speaker_endpoint):
        if speaker_endpoint is not None:
            server_list = await self.read_single_attribute_check_success(
                endpoint=speaker_endpoint,
                cluster=Clusters.Objects.Descriptor,
                attribute=Clusters.Objects.Descriptor.Attributes.ServerList
            )
            has_level_control = 0x0008 in server_list
            if has_level_control:
                await self.send_single_cmd(
                    cmd=Clusters.LevelControl.Commands.MoveToLevel(level=200, transitionTime=0, optionsMask=0, optionsOverride=0),
                    endpoint=speaker_endpoint
                )
            else:
                log.info("Level Control not supported on Speaker endpoint, skipping volume initialization step")
        else:
            log.info("No child Speaker endpoint found, skipping volume initialization step")

