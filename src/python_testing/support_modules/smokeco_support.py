#
#    Copyright (c) 2026 Project CHIP Authors
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
from datetime import datetime, timezone

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_asserts import assert_valid_bool, assert_valid_uint32
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.timeoperations import utc_datetime_from_matter_epoch_us

log = logging.getLogger(__name__)


class SmokeCoBaseTest(MatterBaseTest):

    smokeco_cluster = Clusters.SmokeCoAlarm
    smokeco_enums = Clusters.SmokeCoAlarm.Enums

    async def read_smokeco_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(cluster=self.smokeco_cluster, endpoint=self.get_endpoint(), attribute=attribute)

    async def read_smokeco_event(self, smokeco_event):
        urgent = 1
        events_response = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(self.get_endpoint(), smokeco_event, urgent)],
            fabricFiltered=True
        )
        log.info(f"Events gathered {events_response}")
        if len(events_response) == 0:
            asserts.fail("Failed to read events")
        smoke_alarm_event_data = None
        # Read and match events
        for event in events_response:
            if event.Header.EventId == smokeco_event.event_id:
                log.info(f"Event retrieved {event}")
                smoke_alarm_event_data = event.Data
        if smoke_alarm_event_data is None:
            asserts.fail(f"Failed to retrieve event for {smokeco_event}")
        return smoke_alarm_event_data

    async def read_attribute_check_range(self, attribute, enum):
        """Reads an attribute from the SmokeCluster and validate against a range."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        is_valid = any(attr == item.value and str(item.name).lower() != "unknown" for item in enum)
        asserts.assert_true(is_valid, f"Value {attr} is not in the range for the Enum {enum}")

    async def read_attribute_check_bool(self, attribute):
        """Reads an attribute from the SmokeCluster and validate against a boolean value."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        assert_valid_bool(value=attr, description=f"Attribute {attribute} is not a bool instance {attr}")

    async def read_attribute_check_epoch(self, attribute):
        """Reads an attribute from the SmokeCluster and validate is a int value represeting the seconds of matter epoch."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        log.info(f"Reading attribte with value {attr} and checking the matter epoch ")
        # Number of seconds representing the matter epoch
        assert_valid_uint32(attr, "Attribute is not in uint range")

    async def is_valid_expired_date(self) -> bool:
        """Check if the device alarm  has expired using the ExpiryDate.

        Returns:
            bool: Status of Expiration
        """
        expiry_date = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpiryDate)
        # Convert the epoch time from the device into UTC to compare it to current date
        device_utc_datetime = utc_datetime_from_matter_epoch_us(expiry_date * 1000000)
        current_date = datetime.now(tz=timezone.utc)
        log.info(f"Current device Expire Date  {device_utc_datetime}")
        return device_utc_datetime > current_date
