#
#    Copyright (c) 2024 Project CHIP Authors
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
import typing

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from mobly import asserts

logger = logging.getLogger(__name__)


class EWATERHTRBase:

    async def read_whm_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        cluster = Clusters.Objects.WaterHeaterManagement
        full_attr = getattr(cluster.Attributes, attribute)
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_whm_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_whm_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_boost_command(self, duration: int, one_shot: typing.Optional[bool] = None, emergency_boost: typing.Optional[bool] = None,
                                 temporary_setpoint: typing.Optional[int] = None, target_percentage: typing.Optional[int] = None, target_reheat: typing.Optional[int] = None,
                                 endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                 expected_status: Status = Status.Success):
        try:
            boostInfo = Clusters.WaterHeaterManagement.Structs.WaterHeaterBoostInfoStruct(duration=duration,
                                                                                          oneShot=one_shot,
                                                                                          emergencyBoost=emergency_boost,
                                                                                          temporarySetpoint=temporary_setpoint,
                                                                                          targetPercentage=target_percentage,
                                                                                          targetReheat=target_reheat)

            await self.send_single_cmd(cmd=Clusters.WaterHeaterManagement.Commands.Boost(boostInfo=boostInfo),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_cancel_boost_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                        expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.WaterHeaterManagement.Commands.CancelBoost(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_test_event_trigger_basic_installation_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000000)

    async def send_test_event_trigger_basic_installation_test_event_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000001)

    async def send_test_event_trigger_water_temperature20C_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000002)

    async def send_test_event_trigger_water_temperature61C_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000003)

    async def send_test_event_trigger_water_temperature66C_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000004)

    async def send_test_event_trigger_manual_mode_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000005)

    async def send_test_event_trigger_off_mode_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000006)

    async def send_test_event_trigger_draw_off_hot_water_test_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0094000000000007)
