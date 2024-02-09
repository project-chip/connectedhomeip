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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from mobly import asserts

logger = logging.getLogger(__name__)


class EnergyReportingBaseTestHelper:

    async def read_epm_attribute_expect_success(self, attribute: str = "", endpoint: int = None, ):
        cluster = Clusters.Objects.ElectricalPowerMeasurement
        full_attr = getattr(cluster.Attributes, attribute)
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def read_eem_attribute_expect_success(self, attribute: str = "", endpoint: int = None):
        cluster = Clusters.Objects.ElectricalEnergyMeasurement
        full_attr = getattr(cluster.Attributes, attribute)
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    def check_value_in_range(self, attribute: str, value: int, lower_value: int, upper_value: int):
        asserts.assert_greater_equal(value, lower_value,
                                     f"Unexpected '{attribute}' value - expected {lower_value}, was {value}")
        asserts.assert_less_equal(value, upper_value,
                                  f"Unexpected '{attribute}' value - expected {upper_value}, was {value}")

    async def check_epm_attribute_in_range(self, attribute, lower_value: int, upper_value: int, endpoint: int = None, allow_null: bool = False):
        value = await self.read_epm_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        if allow_null and value is NullValue:
            # skip the range check
            logger.info("value is NULL - OK")
            return value

        self.check_value_in_range(attribute, value, lower_value, upper_value)
        return value

    async def check_eem_attribute_in_range(self, attribute, lower_value: int, upper_value: int, endpoint: int = None, allow_null: bool = False):
        value = await self.read_eem_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        if allow_null and value is NullValue:
            # skip the range check
            logger.info("value is NULL - OK")
            return value

        self.check_value_in_range(attribute, value, lower_value, upper_value)
        return value

    async def get_supported_epm_attributes(self, endpoint: int = None):
        return await self.read_epm_attribute_expect_success("AttributeList", endpoint)

    async def get_supported_eem_attributes(self, endpoint: int = None):
        return await self.read_eem_attribute_expect_success("AttributeList", endpoint)

    async def send_test_event_trigger_stop_fake_readings(self):
        await self.send_test_event_triggers(eventTrigger=0x0091000000000000)

    async def send_test_event_trigger_start_fake_1kw_load_2s(self):
        await self.send_test_event_triggers(eventTrigger=0x0091000000000001)

    async def send_test_event_trigger_start_fake_3kw_generator_5s(self):
        await self.send_test_event_triggers(eventTrigger=0x0091000000000002)
