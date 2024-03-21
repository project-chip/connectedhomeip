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
from mobly import asserts

logger = logging.getLogger(__name__)


class DEMBaseTestHelper:

    async def read_dem_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        cluster = Clusters.Objects.DeviceEnergyManagement
        full_attr = getattr(cluster.Attributes, attribute)
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_dem_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_dem_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_power_adjustment_command(self, power: int, duration: int,
                                            cause: Clusters.Objects.DeviceEnergyManagement.Enums.CauseEnum,
                                            endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                            expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.PowerAdjustRequest(
                power=power,
                duration=duration,
                cause=cause),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_start_time_adjust_request_command(self, requestedStartTime: int,
                                            cause: Clusters.Objects.DeviceEnergyManagement.Enums.CauseEnum,
                                            endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                            expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.StartTimeAdjustRequest(
                requestedStartTime=requestedStartTime,
                cause=cause),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_test_event_trigger_power_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000000)

    async def send_test_event_trigger_power_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000001)

    async def send_test_event_trigger_user_opt_out_local(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000002)

    async def send_test_event_trigger_user_opt_out_grid(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000003)

    async def send_test_event_trigger_user_opt_out_clear_all(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000004)

    async def send_test_event_trigger_start_time_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000005)

    async def send_test_event_trigger_start_time_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000006)

    async def send_test_event_trigger_pauseable(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000007)

    async def send_test_event_trigger_pauseable_next_slot(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000008)

    async def send_test_event_trigger_pauseable_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000009)
