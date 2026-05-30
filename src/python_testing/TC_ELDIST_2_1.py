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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

'''
Purpose
Verify the mandatory server attributes of the Electrical Distribution Cluster
(ELDIST, 0x00A2): each attribute is implemented with the correct data type,
satisfies its declared constraint, honors the Nullable (X) quality, and the
cluster correctly rejects writes to its read-only attributes.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/electrical_distribution.adoc#tc-eldist-2-1
'''

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_asserts import (assert_int_in_range, assert_valid_bool, assert_valid_enum, assert_valid_int64,
                                           assert_valid_uint16)
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalDistribution


class TC_ELDIST_2_1(MatterBaseTest):

    def desc_TC_ELDIST_2_1(self) -> str:
        return "[TC-ELDIST-2.1] Attributes with DUT as Server"

    def pics_TC_ELDIST_2_1(self) -> list[str]:
        return ["ELDIST.S"]

    def steps_TC_ELDIST_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "TH reads MaxContinuousCurrent attribute",
                     "DUT returns amperage-mA (int64) satisfying min 1, OR null."),
            TestStep(3, "TH reads MaxVoltage attribute",
                     "DUT returns voltage-mV (int64) satisfying min 1, OR null."),
            TestStep(4, "TH reads NumberOfPoles attribute",
                     "DUT returns uint16 in inclusive range 1..4, OR null."),
            TestStep(5, "TH reads EndOfLife attribute",
                     "DUT returns a valid EndOfLifeEnum value (None/Damaged/Degraded/Expired), OR null."),
            TestStep(6, "TH reads ServiceEntranceRated attribute",
                     "DUT returns a bool, OR null."),
            TestStep(7, "TH attempts to write MaxContinuousCurrent attribute",
                     "DUT rejects the write with UNSUPPORTED_WRITE (read-only access)."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalDistribution))
    async def test_TC_ELDIST_2_1(self):
        endpoint = self.get_endpoint()

        self.step(1)
        # Commissioning is handled by the test runner.

        self.step(2)
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxContinuousCurrent)
        if val is not NullValue:
            assert_valid_int64(val, 'MaxContinuousCurrent must be an int64')
            asserts.assert_greater_equal(val, 1, 'MaxContinuousCurrent must satisfy min 1')

        self.step(3)
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxVoltage)
        if val is not NullValue:
            assert_valid_int64(val, 'MaxVoltage must be an int64')
            asserts.assert_greater_equal(val, 1, 'MaxVoltage must satisfy min 1')

        self.step(4)
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfPoles)
        if val is not NullValue:
            assert_valid_uint16(val, 'NumberOfPoles must be a uint16')
            assert_int_in_range(val, 1, 4, 'NumberOfPoles must be in inclusive range 1..4')

        self.step(5)
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EndOfLife)
        if val is not NullValue:
            assert_valid_enum(val, 'EndOfLife must be a valid EndOfLifeEnum',
                              cluster.Enums.EndOfLifeEnum)

        self.step(6)
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ServiceEntranceRated)
        if val is not NullValue:
            assert_valid_bool(val, 'ServiceEntranceRated must be a bool')

        self.step(7)
        status = await self.write_single_attribute(
            attribute_value=cluster.Attributes.MaxContinuousCurrent(1),
            endpoint_id=endpoint)
        asserts.assert_equal(status, Status.UnsupportedWrite,
                             'Write to read-only MaxContinuousCurrent must return UNSUPPORTED_WRITE')


if __name__ == "__main__":
    default_matter_test_main()
