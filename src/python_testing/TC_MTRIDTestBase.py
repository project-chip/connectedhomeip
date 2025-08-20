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


import asyncio
import logging

from mobly import asserts

from matter.clusters import ClusterObjects, Globals, MeterIdentification
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class MeterIdentificationTestBaseHelper(MatterBaseTest):
    """This class contains supporting methods for the MeterIdentification test cases."""

    # TestEventTriggers IDs
    test_event_fake_data = 0x0B06000000000000
    test_event_clear = 0x0B06000000000001

    async def send_test_event_trigger_fake_data(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_fake_data)
        await asyncio.sleep(t_wait)

    async def send_test_event_clear(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear)
        await asyncio.sleep(t_wait)

    async def checkPowerThresholdStruct(self, struct: Globals.Structs.PowerThresholdStruct = None):
        """PowerThresholdStruct type validator."""
        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, "PowerThreshold")
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, "ApparentPowerThreshold")
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.powerThresholdSource,
                "PowerThresholdSource attribute must return a Globals.Enums.PowerThresholdSourceEnum",
                Globals.Enums.PowerThresholdSourceEnum,
            )

    async def check_meter_type_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterType
            )
        if attribute_value is not NullValue:
            matter_asserts.assert_valid_enum(
                attribute_value,
                "MeterType attribute must return a Clusters.MeterIdentification.Enums.MeterTypeEnum",
                MeterIdentification.Enums.MeterTypeEnum,
            )
            matter_asserts.assert_int_in_range(attribute_value, 0, 2, "MeterType must be in range 0 - 2")

    async def check_point_of_delivery_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
            )
        if attribute_value is not NullValue:
            matter_asserts.assert_is_string(attribute_value, "PointOfDelivery must be a string")
            asserts.assert_less_equal(len(attribute_value), 64, "PointOfDelivery must have length at most 64!")

    async def check_meter_serial_number_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
            )
        if attribute_value is not NullValue:
            matter_asserts.assert_is_string(attribute_value, "MeterSerialNumber must be a string")
            asserts.assert_less_equal(len(attribute_value), 64, "MeterSerialNumber must have length at most 64!")

    async def check_protocol_version_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.ProtocolVersion):
                attribute_value = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
                )
        if attribute_value is not NullValue and attribute_value is not None:
            matter_asserts.assert_is_string(attribute_value, "ProtocolVersion must be a string")
            asserts.assert_less_equal(len(attribute_value), 64, "ProtocolVersion must have length at most 64!")

    async def check_power_threshold_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
                attribute_value = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
                )
        if attribute_value is not NullValue:
            asserts.assert_true(
                isinstance(attribute_value, Globals.Structs.PowerThresholdStruct),
                "PowerThreshold must be of type Globals.Structs.PowerThresholdStruct",
            )
            await self.checkPowerThresholdStruct(struct=attribute_value)

    async def verify_reporting(self, reports: dict, attribute: ClusterObjects.ClusterAttributeDescriptor, attribute_name: str, saved_value) -> None:

        try:
            asserts.assert_not_equal(reports[attribute][0].value, saved_value,
                                     "Reported value should be different from saved value")
        except KeyError as err:
            asserts.fail(f"There is not reports for attribute {attribute_name}:\n{err}")
