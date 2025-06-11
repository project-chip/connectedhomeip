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


class CHIMETestBase:

    async def read_chime_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Chime
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_chime_attribute_expect_success(self, endpoint, attribute, value):
        return await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint)

    async def write_chime_attribute_expect_failure(self, endpoint, attribute, value, status):
        response = await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint, expect_success=False)

        asserts.assert_equal(response, status, "Unexpected error returned")

    async def send_play_chime_sound_command(self, endpoint, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.Chime.Commands.PlayChimeSound(),
                                       endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
