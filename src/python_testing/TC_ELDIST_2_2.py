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
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ELDIST_2_2(MatterBaseTest):
    """TC-ELDIST-2.2: Fixed-by-Manufacturer Quality Verification with Server as DUT

    Verify that all five mandatory attributes of the Electrical Distribution
    Cluster maintain their values across reboot (Fixed-by-manufacturer quality X).
    """

    def desc_TC_ELDIST_2_2(self) -> str:
        return "[TC-ELDIST-2.2] Fixed-by-Manufacturer Quality Verification with Server as DUT"

    def pics_TC_ELDIST_2_2(self) -> list[str]:
        return ["ELDIST.S"]

    def steps_TC_ELDIST_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads all five mandatory attributes and records values"),
            TestStep(3, "TH re-reads MaxContinuousCurrent to confirm stability within single boot"),
            TestStep(4, "TH attempts write to MaxVoltage - expect UNSUPPORTED_WRITE"),
            TestStep(5, "TH attempts write to NumberOfPoles - expect UNSUPPORTED_WRITE"),
            TestStep(6, "TH attempts write to ServiceEntranceRated - expect UNSUPPORTED_WRITE"),
            TestStep(7, "Operator reboots DUT"),
            TestStep(8, "TH re-reads all five attributes and verifies values unchanged"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalDistribution))
    async def test_TC_ELDIST_2_2(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ElectricalDistribution
        attributes = cluster.Attributes

        self.step(1)

        # Step 2: Read all attributes and record
        self.step(2)
        mcc_1 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.MaxContinuousCurrent)
        mv_1 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.MaxVoltage)
        nop_1 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.NumberOfPoles)
        eol_1 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.EndOfLife)
        ser_1 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.ServiceEntranceRated)

        log.info(f"Pre-reboot values: MCC={mcc_1}, MV={mv_1}, "
                 f"NOP={nop_1}, EOL={eol_1}, SER={ser_1}")

        # Step 3: Re-read MaxContinuousCurrent within same boot
        self.step(3)
        mcc_2 = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.MaxContinuousCurrent)
        asserts.assert_equal(mcc_2, mcc_1,
                             "MaxContinuousCurrent changed within single boot - "
                             "violates Fixed-by-manufacturer quality")

        # Step 4: Write to MaxVoltage
        self.step(4)
        try:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attributes.MaxVoltage(230000))]
            )
            asserts.fail("Write to MaxVoltage should have failed")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedWrite,
                                 "Write to MaxVoltage should return UNSUPPORTED_WRITE")

        # Step 5: Write to NumberOfPoles
        self.step(5)
        try:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attributes.NumberOfPoles(2))]
            )
            asserts.fail("Write to NumberOfPoles should have failed")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedWrite,
                                 "Write to NumberOfPoles should return UNSUPPORTED_WRITE")

        # Step 6: Write to ServiceEntranceRated
        self.step(6)
        try:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attributes.ServiceEntranceRated(False))]
            )
            asserts.fail("Write to ServiceEntranceRated should have failed")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedWrite,
                                 "Write to ServiceEntranceRated should return UNSUPPORTED_WRITE")

        # Step 7: Reboot DUT
        self.step(7)
        self.wait_for_user_input(
            prompt_msg="Reboot the DUT and wait for it to rejoin the fabric. Press Enter.")

        # Step 8: Re-read and verify persistence
        self.step(8)
        mcc_post = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.MaxContinuousCurrent)
        mv_post = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.MaxVoltage)
        nop_post = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.NumberOfPoles)
        eol_post = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.EndOfLife)
        ser_post = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=attributes.ServiceEntranceRated)

        asserts.assert_equal(mcc_post, mcc_1,
                             "MaxContinuousCurrent changed after reboot")
        asserts.assert_equal(mv_post, mv_1,
                             "MaxVoltage changed after reboot")
        asserts.assert_equal(nop_post, nop_1,
                             "NumberOfPoles changed after reboot")
        asserts.assert_equal(eol_post, eol_1,
                             "EndOfLife changed after reboot")
        asserts.assert_equal(ser_post, ser_1,
                             "ServiceEntranceRated changed after reboot")

        log.info("All Fixed-by-manufacturer attributes verified stable across reboot")


if __name__ == "__main__":
    default_matter_test_main()
