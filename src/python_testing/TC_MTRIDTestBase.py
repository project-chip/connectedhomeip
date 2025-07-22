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
import time

from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts

logger = logging.getLogger(__name__)


class MeterIdentificationTestBaseHelper:
    """This class contains supporting methods for the MeterIdentification test cases."""

    # TestEventTriggers IDs
    test_event_fake_data = 0x0B06000000000000
    test_event_clear = 0x0B06000000000001

    async def send_test_event_trigger_fake_data(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_fake_data)
        time.sleep(t_wait)

    async def send_test_event_clear(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear)
        time.sleep(t_wait)

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
