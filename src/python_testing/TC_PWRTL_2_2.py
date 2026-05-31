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
Verify the writable, fabric-scoped ElectricalCircuitNodes attribute (0x0002)
of the Power Topology Cluster on a DUT that supports the ElectricalCircuit
(CIRC, FeatureMap bit 4) feature, new in Matter 1.7.

The test exercises:
  * Successful writes + read-back of a list of CircuitNodeStruct entries.
  * List length constraint (max 50 entries) and Label field constraint
    (max 128 characters), including the negative paths that MUST return
    CONSTRAINT_ERROR.
  * Fabric-Scoped (F) per-fabric isolation (TH1 / TH2).
  * Persistence across DUT reboot (Non-Volatile quality).
  * ACL-restricted access (TH3 with View-only privilege).
  * Subscription reporting.

NOTE: Steps 7-14 (multi-fabric, reboot, ACL, subscription) are scaffolded
as self.skip_step() calls pending the test-harness primitives. Steps 1-6
(single-fabric writes + constraint negatives) are implemented for real.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/power_topology.adoc#tc-pwrtl-2-2
'''

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.PowerTopology


class TC_PWRTL_2_2(MatterBaseTest):

    def desc_TC_PWRTL_2_2(self) -> str:
        return "[TC-PWRTL-2.2] ElectricalCircuitNodes (CIRC feature) with DUT as Server"

    def pics_TC_PWRTL_2_2(self) -> list[str]:
        return ["PWRTL.S", "PWRTL.S.F04"]

    def steps_TC_PWRTL_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "TH1 writes ElectricalCircuitNodes with a valid list of 2 CircuitNodeStruct entries",
                     "DUT returns SUCCESS."),
            TestStep(3, "TH1 reads ElectricalCircuitNodes",
                     "DUT returns the 2-entry list written in step 2."),
            TestStep(4, "TH1 writes a list of exactly 50 entries (max)",
                     "DUT returns SUCCESS."),
            TestStep(5, "TH1 writes a list of 51 entries (over max)",
                     "DUT returns CONSTRAINT_ERROR (0x87); attribute unchanged."),
            TestStep(6, "TH1 writes a single entry with Label of exactly 128 chars (max)",
                     "DUT returns SUCCESS."),
            TestStep(7, "TH1 writes a single entry with Label of 129 chars (over max)",
                     "DUT returns CONSTRAINT_ERROR (0x87); attribute unchanged."),
            TestStep(8, "TH2 (second fabric) reads ElectricalCircuitNodes",
                     "DUT returns fabric-filtered list (empty or TH2's own entries only)."),
            TestStep(9, "TH2 writes one entry; TH1 reads",
                     "TH1 sees only its own entries (per-fabric isolation)."),
            TestStep(10, "Reboot DUT; TH1 reads ElectricalCircuitNodes",
                     "Persisted value from prior write is returned (Non-Volatile quality)."),
            TestStep(11, "TH1 writes ACL granting TH3 View-only privilege on this cluster",
                     "DUT returns SUCCESS."),
            TestStep(12, "TH3 attempts to write ElectricalCircuitNodes",
                     "DUT returns UNSUPPORTED_ACCESS (0x7e)."),
            TestStep(13, "TH3 reads ElectricalCircuitNodes",
                     "DUT returns SUCCESS (View privilege is sufficient for reads)."),
            TestStep(14, "TH1 establishes subscription; subsequently writes",
                     "Subscription priming report received; write produces subscription report reflecting update."),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.PowerTopology, Clusters.PowerTopology.Bitmaps.Feature.kElectricalCircuit))
    async def test_TC_PWRTL_2_2(self):
        endpoint = self.get_endpoint()
        attr = cluster.Attributes.ElectricalCircuitNodes
        CircuitNodeStruct = cluster.Structs.CircuitNodeStruct

        self.step(1)
        # Commissioning is handled by the test runner.

        self.step(2)
        entries_2 = [
            CircuitNodeStruct(node=0x000000000000B001),
            CircuitNodeStruct(node=0x000000000000B002, endpoint=1, label="circuit-A"),
        ]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_2), endpoint_id=endpoint)
        asserts.assert_equal(status, Status.Success, 'Write of 2-entry list must succeed')

        self.step(3)
        read_back = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        asserts.assert_equal(len(read_back), 2, 'Read-back length must match the 2 entries written')
        # TODO: deep-compare struct fields once helper is in place.

        self.step(4)
        entries_50 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(50)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_50), endpoint_id=endpoint)
        asserts.assert_equal(status, Status.Success, 'Write of 50-entry list (at max) must succeed')

        self.step(5)
        entries_51 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(51)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_51), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write of 51-entry list (over max) must return CONSTRAINT_ERROR')

        self.step(6)
        label_128 = "x" * 128
        entries_label_max = [CircuitNodeStruct(node=0x000000000000B001, label=label_128)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_label_max), endpoint_id=endpoint)
        asserts.assert_equal(status, Status.Success, 'Write with 128-char Label (at max) must succeed')

        self.step(7)
        label_129 = "x" * 129
        entries_label_over = [CircuitNodeStruct(node=0x000000000000B001, label=label_129)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_label_over), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write with 129-char Label (over max) must return CONSTRAINT_ERROR')

        # Steps 8-9: Fabric-scoping. Requires a second fabric (TH2).
        # TODO: open commissioning window, commission TH2 to a second fabric,
        # perform reads/writes as TH2, confirm per-fabric isolation.
        # See TC_ACL_2_*.py for the fabric-add pattern.
        self.skip_step(8)
        self.skip_step(9)

        # Step 10: Persistence. Requires DUT reboot via TestEventTrigger or similar.
        # TODO: invoke GeneralDiagnostics.TestEventTrigger with reboot trigger,
        # wait for DUT to come back, re-establish session, read attribute.
        self.skip_step(10)

        # Steps 11-13: ACL gating. TH3 = third controller with View-only access.
        # TODO: commission TH3 onto TH1's fabric, install ACL entry granting
        # View only on this cluster, attempt TH3 write (expect UNSUPPORTED_ACCESS)
        # and read (expect SUCCESS).
        self.skip_step(11)
        self.skip_step(12)
        self.skip_step(13)

        # Step 14: Subscription. Establish, write, confirm report.
        # TODO: use self.default_controller.ReadAttribute with reportInterval
        # to establish subscription, then perform write and await report.
        self.skip_step(14)


if __name__ == "__main__":
    default_matter_test_main()
