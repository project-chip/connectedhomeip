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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ELECTRICAL_PROTECTION_APP}
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
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_ELDIST_2_1(MatterBaseTest):

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics('ELDIST.S')
    @async_test_body
    async def test_TC_ELDIST_2_1(self):
        """[TC-ELDIST-2.1] Attributes with Server as DUT

        Verify the non-global attributes of the Electrical Distribution Cluster
        server: MaxContinuousCurrent, MaxVoltage, NumberOfPoles, EndOfLife,
        ServiceEntranceRated. Every attribute is Nullable (X), so null is a
        spec-valid response to each read.
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalDistribution
        attributes = cluster.Attributes

        self.step(1, "Commissioning, already done", is_commissioning=True)

        self.step(2, "TH reads from the DUT the MaxContinuousCurrent attribute.",
                  expectation="Verify that the DUT response contains a value of type `amperage-mA` (int64) "
                              "satisfying the constraint `min 1`, OR `null` (attribute is Nullable).")
        max_continuous_current = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.MaxContinuousCurrent
        )
        if max_continuous_current is not Clusters.Types.NullValue:
            matter_asserts.assert_valid_int64(max_continuous_current, "MaxContinuousCurrent")
            asserts.assert_greater_equal(max_continuous_current, 1,
                                         "MaxContinuousCurrent must be >= 1 mA")
        log.info("MaxContinuousCurrent: %s mA", max_continuous_current)

        self.step(3, "TH reads from the DUT the MaxVoltage attribute.",
                  expectation="Verify that the DUT response contains a value of type `voltage-mV` (int64) "
                              "satisfying the constraint `min 1`, OR `null` (attribute is Nullable).")
        max_voltage = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.MaxVoltage
        )
        if max_voltage is not Clusters.Types.NullValue:
            matter_asserts.assert_valid_int64(max_voltage, "MaxVoltage")
            asserts.assert_greater_equal(max_voltage, 1,
                                         "MaxVoltage must be >= 1 mV")
        log.info("MaxVoltage: %s mV", max_voltage)

        self.step(4, "TH reads from the DUT the NumberOfPoles attribute.",
                  expectation="Verify that the DUT response contains a `uint16` value within the inclusive "
                              "range 1 to 4, OR `null` (attribute is Nullable).")
        number_of_poles = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.NumberOfPoles
        )
        if number_of_poles is not Clusters.Types.NullValue:
            matter_asserts.assert_int_in_range(number_of_poles, 1, 4, "NumberOfPoles")
        log.info("NumberOfPoles: %s", number_of_poles)

        self.step(5, "TH reads from the DUT the EndOfLife attribute.",
                  expectation="Verify that the DUT response contains a valid `EndOfLifeEnum` value "
                              "(one of: 0=None, 1=Damaged, 2=Degraded, 3=Expired), OR `null` "
                              "(attribute is Nullable).")
        end_of_life = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.EndOfLife
        )
        if end_of_life is not Clusters.Types.NullValue:
            matter_asserts.assert_valid_enum(end_of_life, "EndOfLife", cluster.Enums.EndOfLifeEnum)
        log.info("EndOfLife: %s", end_of_life)

        self.step(6, "TH reads from the DUT the ServiceEntranceRated attribute.",
                  expectation="Verify that the DUT response contains a `bool` value (true or false), "
                              "OR `null` (attribute is Nullable).")
        service_entrance_rated = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.ServiceEntranceRated
        )
        if service_entrance_rated is not Clusters.Types.NullValue:
            matter_asserts.assert_valid_bool(service_entrance_rated, "ServiceEntranceRated")
        log.info("ServiceEntranceRated: %s", service_entrance_rated)


if __name__ == "__main__":
    default_matter_test_main()
