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
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from mobly import asserts

logger = logging.getLogger(__name__)


class ElectricalGridConditionsTestBaseHelper:

    async def test_ForecastConditions(self,
                                      endpoint: int = None,
                                      cluster: Clusters.ElectricalGridConditions = None,
                                      forecastConditions: list = None):
        matter_asserts.assert_list(forecastConditions, "ForecastConditions must be a list")
        matter_asserts.assert_list_element_type(
            forecastConditions, cluster.Structs.ElectricalGridConditionsStruct,
            "ForecastConditions attribute must contain ElectricalGridConditionsStruct elements",
            allow_empty=True)

        asserts.assert_less_equal(len(forecastConditions),
                                  56, "ForecastConditions list must be less than 56 entries")
        for item in forecastConditions:
            await self.test_checkElectricalGridConditionsStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def test_checkElectricalGridConditionsStruct(self,
                                                       endpoint: int = None,
                                                       cluster: Clusters.ElectricalGridConditions = None,
                                                       struct: Clusters.ElectricalGridConditions.Structs.ElectricalGridConditionsStruct = None):
        matter_asserts.assert_valid_uint32(struct.periodStart, 'PeriodStart')
        if struct.periodEnd is not NullValue:
            matter_asserts.assert_valid_uint32(struct.periodEnd, 'PeriodEnd')
        matter_asserts.assert_valid_int16(struct.gridCarbonIntensity, 'GridCarbonIntensity')
        matter_asserts.assert_valid_enum(
            struct.gridCarbonLevel, "GridCarbonLevel attribute must return a ThreeLevelEnum", cluster.Enums.ThreeLevelEnum)
        matter_asserts.assert_valid_int16(struct.localCarbonIntensity, 'LocalCarbonIntensity')
        matter_asserts.assert_valid_enum(
            struct.localCarbonLevel, "LocalCarbonLevel attribute must return a ThreeLevelEnum", cluster.Enums.ThreeLevelEnum)

    async def send_test_event_trigger_current_conditions_update(self):
        await self.send_test_event_triggers(eventTrigger=0x00A0000000000000)

    async def send_test_event_trigger_forecast_conditions_update(self):
        await self.send_test_event_triggers(eventTrigger=0x00A0000000000001)

    def convert_epoch_s_to_time(self, epoch_s, tz=timezone.utc):
        if epoch_s is not NullValue:
            delta_from_epoch = timedelta(seconds=epoch_s)
            matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, tz)

            return matter_epoch + delta_from_epoch
        else:
            return "None"
