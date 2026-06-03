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
#     app: ${EVSE_APP}
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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.PowerTopology


class TC_PWRTL_2_2(MatterBaseTest):

    @pics('PWRTL.S', 'PWRTL.S.F04')
    @async_test_body
    async def test_TC_PWRTL_2_2(self):
        """[TC-PWRTL-2.2] ElectricalCircuitNodes (CIRC feature) with DUT as Server

        Verify the writable, fabric-scoped ElectricalCircuitNodes attribute
        (0x0002) of the Power Topology Cluster on a DUT that supports the
        ElectricalCircuit (CIRC, FeatureMap bit 4) feature, new in Matter 1.7.

        Exercises: successful writes + read-back of a list of CircuitNodeStruct
        entries; list length constraint (max 50 entries) and Label field
        constraint (max 128 characters) including the CONSTRAINT_ERROR negative
        paths; Fabric-Scoped (F) per-fabric isolation (TH1/TH2); persistence
        across DUT reboot (Non-Volatile quality); ACL-restricted access (TH3
        View-only); and subscription reporting.

        Steps 7-14 (multi-fabric, reboot, ACL, subscription) are scaffolded
        as self.skip_step() calls pending the test-harness primitives.
        Steps 1-6 (single-fabric writes + constraint negatives) are
        implemented for real.

        Test Plan:
        https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/power_topology.adoc#tc-pwrtl-2-2
        """
        endpoint = self.get_endpoint()
        attr = cluster.Attributes.ElectricalCircuitNodes
        CircuitNodeStruct = cluster.Structs.CircuitNodeStruct

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH1 writes ElectricalCircuitNodes with a valid list of 2 CircuitNodeStruct entries")
        entries_2 = [
            CircuitNodeStruct(node=0x000000000000B001),
            CircuitNodeStruct(node=0x000000000000B002, endpoint=1, label="circuit-A"),
        ]
        # write_single_attribute asserts SUCCESS internally when expect_success is left at
        # its default (True), so the success-path writes below do not need an extra
        # assert_equal on the returned status.
        await self.write_single_attribute(
            attribute_value=attr(entries_2), endpoint_id=endpoint)

        self.step(3, "TH1 reads ElectricalCircuitNodes; verifies the 2-entry list is returned")
        read_back = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        asserts.assert_equal(len(read_back), 2, 'Read-back length must match the 2 entries written')
        # TODO: deep-compare struct fields once helper is in place.

        self.step(4, "TH1 writes a list of exactly 50 entries (max)")
        entries_50 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(50)]
        await self.write_single_attribute(
            attribute_value=attr(entries_50), endpoint_id=endpoint)

        self.step(5, "TH1 writes a list of 51 entries (over max) - expect CONSTRAINT_ERROR")
        entries_51 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(51)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_51), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write of 51-entry list (over max) must return CONSTRAINT_ERROR')

        self.step(6, "TH1 writes a single entry with Label of exactly 128 chars (max)")
        label_128 = "x" * 128
        entries_label_max = [CircuitNodeStruct(node=0x000000000000B001, label=label_128)]
        await self.write_single_attribute(
            attribute_value=attr(entries_label_max), endpoint_id=endpoint)

        self.step(7, "TH1 writes a single entry with Label of 129 chars (over max) - expect CONSTRAINT_ERROR")
        label_129 = "x" * 129
        entries_label_over = [CircuitNodeStruct(node=0x000000000000B001, label=label_129)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_label_over), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write with 129-char Label (over max) must return CONSTRAINT_ERROR')

        self.step(8, "TH2 (second fabric) reads ElectricalCircuitNodes - fabric-filtered list")
        # TODO: open commissioning window, commission TH2 to a second fabric,
        # perform reads/writes as TH2, confirm per-fabric isolation.
        # See TC_ACL_2_*.py for the fabric-add pattern.
        self.mark_current_step_skipped()

        self.step(9, "TH2 writes one entry; TH1 reads - sees only its own entries (per-fabric isolation)")
        self.mark_current_step_skipped()

        self.step(10, "Reboot DUT; TH1 reads ElectricalCircuitNodes - persisted value returned (Non-Volatile)")
        # TODO: invoke GeneralDiagnostics.TestEventTrigger with reboot trigger,
        # wait for DUT to come back, re-establish session, read attribute.
        self.mark_current_step_skipped()

        self.step(11, "TH1 writes ACL granting TH3 View-only privilege on this cluster")
        # TODO: commission TH3 onto TH1's fabric, install ACL entry granting
        # View only on this cluster.
        self.mark_current_step_skipped()

        self.step(12, "TH3 attempts to write ElectricalCircuitNodes - expect UNSUPPORTED_ACCESS")
        self.mark_current_step_skipped()

        self.step(13, "TH3 reads ElectricalCircuitNodes - expect SUCCESS (View privilege sufficient for reads)")
        self.mark_current_step_skipped()

        self.step(14, "TH1 establishes subscription; subsequently writes - subscription report reflects update")
        # TODO: use self.default_controller.ReadAttribute with reportInterval
        # to establish subscription, then perform write and await report.
        self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
