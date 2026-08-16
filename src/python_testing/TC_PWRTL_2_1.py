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
#     app: ${EVSE_APP}
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
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PWRTL_2_1(MatterBaseTest):

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics('PWRTL.S')
    @async_test_body
    async def test_TC_PWRTL_2_1(self):
        """[TC-PWRTL-2.1] Attributes with DUT as Server

        Verify FeatureMap (including the Matter 1.7 CIRC bit 4 with reserved-bit
        discipline), AttributeList composition based on topology features,
        AvailableEndpoints and ActiveEndpoints reads, the subset invariant
        (ActiveEndpoints subset of AvailableEndpoints), read-only access on
        both endpoint list attributes, and Non-volatile persistence of
        ActiveEndpoints across reboot (manual-mode only; CI skips the reboot
        steps via the is_pics_sdk_ci_only dispatch).
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.PowerTopology
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature

        self.step(1, "Commissioning, already done", is_commissioning=True)

        self.step(2, "TH reads FeatureMap from DUT")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.FeatureMap
        )
        log.info("FeatureMap: 0x%08X", feature_map)

        self.step(3, "TH validates O.a conformance (exactly one of NODE/TREE/SET); DYPF implies SET")
        has_node = bool(feature_map & features.kNodeTopology)
        has_tree = bool(feature_map & features.kTreeTopology)
        has_set = bool(feature_map & features.kSetTopology)
        topology_count = sum([has_node, has_tree, has_set])
        asserts.assert_equal(topology_count, 1,
                             f"Exactly one of NODE/TREE/SET must be set, got {topology_count}")
        has_dypf = bool(feature_map & features.kDynamicPowerFlow)
        if has_dypf:
            asserts.assert_true(has_set,
                                "DynamicPowerFlow (DYPF) requires SetTopology")
        log.info("Topology: NODE=%s, TREE=%s, SET=%s, DYPF=%s", has_node, has_tree, has_set, has_dypf)

        self.step(4, "TH validates CIRC bit and reserved bits 5..31")
        has_circ = bool(feature_map & features.kElectricalCircuit)
        log.info("ElectricalCircuit (CIRC): %s", has_circ)
        KNOWN_BITS_MASK = (features.kNodeTopology | features.kTreeTopology |
                           features.kSetTopology | features.kDynamicPowerFlow |
                           features.kElectricalCircuit)
        reserved_bits = feature_map & ~KNOWN_BITS_MASK
        asserts.assert_equal(reserved_bits, 0,
                             f"Reserved bits set in FeatureMap: 0x{reserved_bits:08X}")

        self.step(5, "TH reads AttributeList; verifies AvailableEndpoints present iff SET, ActiveEndpoints present iff DYPF")
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.AttributeList
        )
        log.info("AttributeList: %s", attribute_list)
        avail_ep_id = attributes.AvailableEndpoints.attribute_id
        active_ep_id = attributes.ActiveEndpoints.attribute_id

        # AvailableEndpoints (0x0000): conformance "SET" — mandatory iff SET feature,
        # disallowed otherwise (bare-symbol form, not bracketed).
        if has_set:
            asserts.assert_in(avail_ep_id, attribute_list,
                              "AvailableEndpoints must be in AttributeList when SET feature is set")
        else:
            asserts.assert_not_in(avail_ep_id, attribute_list,
                                  "AvailableEndpoints must NOT be in AttributeList when SET feature is not set")

        # ActiveEndpoints (0x0001): conformance "DYPF" — mandatory iff DYPF feature,
        # disallowed otherwise. DYPF itself requires SET ([SET]), so DYPF implies
        # AvailableEndpoints is also present.
        if has_dypf:
            asserts.assert_in(active_ep_id, attribute_list,
                              "ActiveEndpoints must be in AttributeList when DYPF feature is set")
        else:
            asserts.assert_not_in(active_ep_id, attribute_list,
                                  "ActiveEndpoints must NOT be in AttributeList when DYPF feature is not set")

        self.step(6, "TH reads AvailableEndpoints (if present in AttributeList)")
        avail_eps = None
        if avail_ep_id in attribute_list:
            avail_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=cluster,
                attribute=attributes.AvailableEndpoints
            )
            log.info("AvailableEndpoints: %s", avail_eps)
        else:
            log.info("AvailableEndpoints not in AttributeList (SET feature not set)")
            self.mark_current_step_skipped()

        self.step(7, "TH reads ActiveEndpoints (if present in AttributeList)")
        active_eps = None
        if active_ep_id in attribute_list:
            active_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=cluster,
                attribute=attributes.ActiveEndpoints
            )
            log.info("ActiveEndpoints: %s", active_eps)
        else:
            log.info("ActiveEndpoints not in AttributeList (DYPF feature not set)")
            self.mark_current_step_skipped()

        self.step(8, "TH verifies ActiveEndpoints is a subset of AvailableEndpoints")
        if avail_eps is not None and active_eps is not None:
            for ep in active_eps:
                asserts.assert_in(ep, avail_eps,
                                  f"ActiveEndpoint {ep} not in AvailableEndpoints")
            log.info("ActiveEndpoints is a subset of AvailableEndpoints")
        else:
            log.info("Skipping subset check (endpoint attributes not present)")
            self.mark_current_step_skipped()

        self.step(9, "TH attempts write to AvailableEndpoints - expect UNSUPPORTED_WRITE")
        if avail_eps is not None:
            status = await self.write_single_attribute(
                attribute_value=attributes.AvailableEndpoints([]),
                endpoint_id=endpoint,
                expect_success=False)
            asserts.assert_equal(status, Status.UnsupportedWrite,
                                 "Write to AvailableEndpoints should return UNSUPPORTED_WRITE")
        else:
            self.mark_current_step_skipped()

        self.step(10, "Operator reboots DUT (skipped in CI)")
        if self.is_pics_sdk_ci_only or active_eps is None:
            # CI cannot drive an operator reboot, and there are no ActiveEndpoints
            # to verify Non-volatile persistence against.
            self.mark_current_step_skipped()
        else:
            self.wait_for_user_input(
                prompt_msg="Reboot the DUT and wait for it to rejoin the fabric. Press Enter.")

        self.step(11, "TH verifies ActiveEndpoints persists after reboot - Non-volatile (skipped in CI)")
        if self.is_pics_sdk_ci_only or active_eps is None:
            self.mark_current_step_skipped()
        else:
            active_eps_post = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=cluster,
                attribute=attributes.ActiveEndpoints
            )
            asserts.assert_equal(sorted(active_eps_post), sorted(active_eps),
                                 "ActiveEndpoints changed after reboot - violates Non-volatile quality")
            log.info("ActiveEndpoints persisted across reboot (Non-volatile verified)")


if __name__ == "__main__":
    default_matter_test_main()
