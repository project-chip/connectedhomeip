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
#     app-ready-pattern: "APP STATUS: Starting event loop"
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
import random

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.PowerTopology


class TC_PWRTL_2_2(MatterBaseTest):

    def _assert_nodes_equal(self, actual, expected, what: str) -> None:
        """Compare every field of every CircuitNodeStruct, not just the list length.

        Checking length alone lets a wrong node, endpoint or label pass, and lets a
        difference in any entry after the first go unnoticed.
        """
        asserts.assert_equal(len(actual), len(expected),
                             f'{what}: list length must match')
        for i, (got, want) in enumerate(zip(actual, expected)):
            asserts.assert_equal(got.node, want.node, f'{what}[{i}].node must match')
            asserts.assert_equal(got.endpoint, want.endpoint, f'{what}[{i}].endpoint must match')
            asserts.assert_equal(got.label, want.label, f'{what}[{i}].label must match')

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kElectricalCircuit))
    async def test_TC_PWRTL_2_2(self):
        """[TC-PWRTL-2.2] ElectricalCircuitNodes (CIRC feature) with Server as DUT

        This test case verifies the behavior of the writable, fabric-scoped
        ElectricalCircuitNodes (0x0002) attribute on a DUT that supports the CIRC feature.
        """
        endpoint = self.get_endpoint()
        attr = cluster.Attributes.ElectricalCircuitNodes
        CircuitNodeStruct = cluster.Structs.CircuitNodeStruct

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH1 writes ElectricalCircuitNodes with a valid list of 2 CircuitNodeStruct entries",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00).")
        entries_2 = [
            CircuitNodeStruct(node=0x000000000000B001),
            CircuitNodeStruct(node=0x000000000000B002, endpoint=1, label="circuit-A"),
        ]
        # write_single_attribute asserts SUCCESS internally when expect_success is left at
        # its default (True), so the success-path writes below do not need an extra
        # assert_equal on the returned status.
        await self.write_single_attribute(
            attribute_value=attr(entries_2), endpoint_id=endpoint)

        self.step(3, "TH1 reads ElectricalCircuitNodes",
                  expectation="Verify DUT responds with a list[CircuitNodeStruct] equal to the value written in step 2.")
        read_back = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        asserts.assert_equal(len(read_back), 2, 'Read-back length must match the 2 entries written')
        self._assert_nodes_equal(read_back, entries_2, 'read-back')

        self.step(4, "TH1 writes ElectricalCircuitNodes with a list of exactly 50 CircuitNodeStruct entries",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). 50 is the spec-defined maximum list length.")
        entries_50 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(50)]
        await self.write_single_attribute(
            attribute_value=attr(entries_50), endpoint_id=endpoint)

        self.step(5, "TH1 writes ElectricalCircuitNodes with a list of 51 CircuitNodeStruct entries, one over the maximum",
                  expectation="Verify DUT responds w/ status CONSTRAINT_ERROR(0x87); the attribute value is unchanged "
                  "from step 4.")
        entries_51 = [CircuitNodeStruct(node=0x000000000000B000 + i) for i in range(51)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_51), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write of 51-entry list (over max) must return CONSTRAINT_ERROR')
        after_over_max = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        self._assert_nodes_equal(after_over_max, entries_50, 'after the rejected 51-entry write')

        self.step(6, "TH1 writes ElectricalCircuitNodes with a single entry whose Label is exactly 128 characters",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00); a subsequent read returns the entry with "
                  "the full 128-character Label.")
        label_128 = "x" * 128
        entries_label_max = [CircuitNodeStruct(node=0x000000000000B001, label=label_128)]
        await self.write_single_attribute(
            attribute_value=attr(entries_label_max), endpoint_id=endpoint)
        read_label_max = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        asserts.assert_equal(len(read_label_max), 1, 'Read-back must return the single entry written')
        asserts.assert_equal(read_label_max[0].label, label_128,
                             'Label must round-trip at the full 128 characters')

        self.step(7, "TH1 writes ElectricalCircuitNodes with a single entry whose Label is 129 characters, one over the "
                     "maximum",
                  expectation="Verify DUT responds w/ status CONSTRAINT_ERROR(0x87); the attribute value from step 6 is "
                  "unchanged.")
        label_129 = "x" * 129
        entries_label_over = [CircuitNodeStruct(node=0x000000000000B001, label=label_129)]
        status = await self.write_single_attribute(
            attribute_value=attr(entries_label_over), endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(status, Status.ConstraintError,
                             'Write with 129-char Label (over max) must return CONSTRAINT_ERROR')
        after_over_label = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        self._assert_nodes_equal(after_over_label, entries_label_max,
                                 'after the rejected 129-character Label write')

        self.step(8, "Open a commissioning window on DUT and commission DUT to the TH2 fabric. As TH2, TH reads "
                     "ElectricalCircuitNodes",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00) with an empty list, or with only the TH2 "
                  "entries. The attribute is Fabric-Scoped (F), so the TH1 entries are not visible to TH2.")
        th1 = self.default_controller
        discriminator = random.randint(0, 4095)
        params = await th1.OpenCommissioningWindow(
            nodeId=self.dut_node_id, timeout=900, iteration=10000, discriminator=discriminator, option=1)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=th1.fabricId + 1)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        await th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        th2_view = await self.read_single_attribute_check_success(
            dev_ctrl=th2, endpoint=endpoint, cluster=cluster, attribute=attr)
        asserts.assert_equal(th2_view, [],
                             'A Fabric-Scoped attribute must not expose the TH1 entries to TH2')

        self.step(9, "As TH2, TH writes ElectricalCircuitNodes with one CircuitNodeStruct entry, then as TH1 reads the "
                     "attribute",
                  expectation="The TH2 write returns SUCCESS. The TH1 read returns only the TH1 entries, with the TH2 "
                  "entry not visible, confirming per-fabric isolation.")
        # write_single_attribute always writes as the default controller, so the TH2 write goes
        # through the controller directly.
        th2_entries = [CircuitNodeStruct(node=0x000000000000B0F2, label="th2-only")]
        result = await th2.WriteAttribute(self.dut_node_id, [(endpoint, attr(th2_entries))])
        asserts.assert_equal(result[0].Status, Status.Success, 'The TH2 write must succeed on its own fabric')
        th1_view = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr)
        self._assert_nodes_equal(th1_view, entries_label_max, 'TH1 view after the TH2 write')

        self.step(10, "As TH1 and as TH2, establish a subscription to ElectricalCircuitNodes on the test endpoint",
                  expectation="Both subscriptions are established successfully; TH awaits a subscription report on "
                  "each, carrying that fabric's own list value.")
        # fabric_filtered defaults to False on the handler, which would surface the other fabric's
        # entries alongside each subscriber's own. Each fabric's own view is what is under test.
        # start() awaits ReadAttribute, which returns only once the priming report has arrived, and
        # installs the update callback after that. Each priming value is therefore in its own
        # transaction cache and never reaches the handler's queue; only later updates do.
        th1_reports = AttributeSubscriptionHandler(cluster, attr)
        th1_subscription = await th1_reports.start(th1, self.dut_node_id, endpoint=endpoint, fabric_filtered=True,
                                                   min_interval_sec=0, max_interval_sec=30)
        th2_reports = AttributeSubscriptionHandler(cluster, attr)
        th2_subscription = await th2_reports.start(th2, self.dut_node_id, endpoint=endpoint, fabric_filtered=True,
                                                   min_interval_sec=0, max_interval_sec=30)
        self._assert_nodes_equal(th1_subscription.GetAttributes()[endpoint][cluster][attr], entries_label_max,
                                 'TH1 priming report')
        self._assert_nodes_equal(th2_subscription.GetAttributes()[endpoint][cluster][attr], th2_entries,
                                 'TH2 priming report')

        self.step(11, "As TH2, TH writes ElectricalCircuitNodes with a new valid list of 2 entries",
                  expectation="Write returns SUCCESS; TH awaits a subscription report on TH2's subscription "
                  "reflecting the updated list. TH1 is also reported to, since dirtiness is per attribute path "
                  "and not per fabric, but its report still carries only its own entries.")
        try:
            entries_final = [
                CircuitNodeStruct(node=0x000000000000B010),
                CircuitNodeStruct(node=0x000000000000B011, endpoint=1, label="circuit-B"),
            ]
            result = await th2.WriteAttribute(self.dut_node_id, [(endpoint, attr(entries_final))])
            asserts.assert_equal(result[0].Status, Status.Success, 'The TH2 write must succeed on its own fabric')
            report = th2_reports.wait_for_attribute_report()
            self._assert_nodes_equal(report.value, entries_final, "TH2's report after its own write")
            # TH1 is reported to as well: the IM marks an attribute dirty per path, and
            # AttributePathParams carries no fabric, so there is no way to dirty an attribute for one
            # fabric only. What must not happen is TH2's entries reaching TH1, so the content is what
            # is checked rather than the absence of a report.
            th1_report = th1_reports.wait_for_attribute_report()
            self._assert_nodes_equal(th1_report.value, entries_label_max,
                                     "TH1's report must still carry only TH1's own entries")
        finally:
            th1_reports.cancel()
            th2_reports.cancel()

if __name__ == "__main__":
    default_matter_test_main()
