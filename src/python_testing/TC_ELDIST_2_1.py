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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, pics, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_ELDIST_2_1(MatterBaseTest):

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics('ELDIST.S')
    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalDistribution))
    async def test_TC_ELDIST_2_1(self):
        """[TC-ELDIST-2.1] Attributes with Server as DUT

        Verify the non-global attributes of the Electrical Distribution Cluster
        server: MaxContinuousCurrent, MaxVoltage, NumberOfPoles, EndOfLife,
        ServiceEntranceRated. All attributes carry Fixed-by-manufacturer (X)
        quality.
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalDistribution
        attributes = cluster.Attributes

        self.step(1, "Commissioning, already done", is_commissioning=True)

        self.step(2, "TH reads MaxContinuousCurrent (amperage-mA, min 1)")
        max_continuous_current = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.MaxContinuousCurrent
        )
        asserts.assert_is_not_none(max_continuous_current,
                                   "MaxContinuousCurrent should not be None")
        if max_continuous_current is not Clusters.Types.NullValue:
            asserts.assert_greater_equal(max_continuous_current, 1,
                                         "MaxContinuousCurrent must be >= 1 mA")
        log.info(f"MaxContinuousCurrent: {max_continuous_current} mA")

        self.step(3, "TH reads MaxVoltage (voltage-mV, min 1)")
        max_voltage = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.MaxVoltage
        )
        asserts.assert_is_not_none(max_voltage, "MaxVoltage should not be None")
        if max_voltage is not Clusters.Types.NullValue:
            asserts.assert_greater_equal(max_voltage, 1,
                                         "MaxVoltage must be >= 1 mV")
        log.info(f"MaxVoltage: {max_voltage} mV")

        self.step(4, "TH reads NumberOfPoles (uint16, 1-4)")
        number_of_poles = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.NumberOfPoles
        )
        asserts.assert_is_not_none(number_of_poles, "NumberOfPoles should not be None")
        if number_of_poles is not Clusters.Types.NullValue:
            asserts.assert_greater_equal(number_of_poles, 1,
                                         "NumberOfPoles must be >= 1")
            asserts.assert_less_equal(number_of_poles, 4,
                                      "NumberOfPoles must be <= 4")
        log.info(f"NumberOfPoles: {number_of_poles}")

        self.step(5, "TH reads EndOfLife (EndOfLifeEnum)")
        end_of_life = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.EndOfLife
        )
        asserts.assert_is_not_none(end_of_life, "EndOfLife should not be None")
        if end_of_life is not Clusters.Types.NullValue:
            valid_values = [e.value for e in cluster.Enums.EndOfLifeEnum
                            if e != cluster.Enums.EndOfLifeEnum.kUnknownEnumValue]
            asserts.assert_in(end_of_life, valid_values,
                              f"EndOfLife must be a valid EndOfLifeEnum value, got {end_of_life}")
        log.info(f"EndOfLife: {end_of_life}")

        self.step(6, "TH reads ServiceEntranceRated (bool)")
        service_entrance_rated = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.ServiceEntranceRated
        )
        asserts.assert_is_not_none(service_entrance_rated,
                                   "ServiceEntranceRated should not be None")
        if service_entrance_rated is not Clusters.Types.NullValue:
            asserts.assert_true(isinstance(service_entrance_rated, bool),
                                "ServiceEntranceRated must be bool")
        log.info(f"ServiceEntranceRated: {service_entrance_rated}")

        self.step(7, "TH attempts write to MaxContinuousCurrent - expect UNSUPPORTED_WRITE")
        status = await self.write_single_attribute(
            attribute_value=attributes.MaxContinuousCurrent(50000),
            endpoint_id=endpoint,
            expect_success=False)
        asserts.assert_equal(status, Status.UnsupportedWrite,
                             "Write to MaxContinuousCurrent should return UNSUPPORTED_WRITE")


if __name__ == "__main__":
    default_matter_test_main()
