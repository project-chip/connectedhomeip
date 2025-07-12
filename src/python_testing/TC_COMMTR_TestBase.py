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

import chip.clusters as Clusters
from chip.testing import matter_asserts

logger = logging.getLogger(__name__)


class CommodityMeteringTestBaseHelper:

    test_event_fake_data = 0x0b07000000000000
    test_event_clear = 0x0b07000000000001

    async def send_test_event_trigger_fake_value_update(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_fake_data)
        time.sleep(t_wait)

    async def send_test_event_trigger_clear(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear)
        time.sleep(t_wait)

    async def checkMeteredQuantityStruct(self, struct: Clusters.CommodityMetering.Structs.MeteredQuantityStruct = None):
        """Supporting function to check MeteredQuantityStruct."""

        matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list", max_length=128)
        matter_asserts.assert_list_element_type(
            struct.tariffComponentIDs, int, "TariffComponentIDs attribute must contain int elements")
        matter_asserts.assert_valid_int64(struct.quantity, 'Quantity')
