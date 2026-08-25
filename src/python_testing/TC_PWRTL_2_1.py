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

        Verifies the endpoint list attributes: the max-20 constraint on each,
        ActiveEndpoints being a subset of AvailableEndpoints, and that both are
        read-only. Also verifies ElectricalCircuitNodes presence against the
        CIRC feature.

        Feature-choice conformance and AttributeList composition are not checked
        here; TC_IDM_10_2 evaluates both from the data model.
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.PowerTopology
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature

        self.step(1, "Commissioning, already done", is_commissioning=True)

        self.step(2, "TH reads AvailableEndpoints from DUT")
        avail_eps = None
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.AvailableEndpoints):
            avail_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=cluster,
                attribute=attributes.AvailableEndpoints
            )
            log.info("AvailableEndpoints: %s", avail_eps)
            asserts.assert_less_equal(len(avail_eps), 20,
                                      "AvailableEndpoints exceeds its max 20 constraint")
        else:
            self.mark_current_step_skipped()

        self.step(3, "TH reads ActiveEndpoints and verifies it is a subset of AvailableEndpoints")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ActiveEndpoints):
            active_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=cluster,
                attribute=attributes.ActiveEndpoints
            )
            log.info("ActiveEndpoints: %s", active_eps)
            asserts.assert_less_equal(len(active_eps), 20,
                                      "ActiveEndpoints exceeds its max 20 constraint")
            # ActiveEndpoints SHALL be a subset of AvailableEndpoints (11.8.6.2). DYPF
            # implies SET, so AvailableEndpoints is present whenever ActiveEndpoints is.
            asserts.assert_is_not_none(avail_eps,
                                       "ActiveEndpoints present without AvailableEndpoints")
            for ep in active_eps:
                asserts.assert_in(ep, avail_eps,
                                  f"ActiveEndpoint {ep} is not in AvailableEndpoints")
        else:
            self.mark_current_step_skipped()

        self.step(4, "TH writes each endpoint list attribute - expect UNSUPPORTED_WRITE")
        wrote_any = False
        for attribute in (attributes.AvailableEndpoints, attributes.ActiveEndpoints):
            if not await self.attribute_guard(endpoint=endpoint, attribute=attribute):
                continue
            status = await self.write_single_attribute(
                attribute_value=attribute([]),
                endpoint_id=endpoint,
                expect_success=False)
            asserts.assert_equal(status, Status.UnsupportedWrite,
                                 f"Write to {attribute.__name__} should return UNSUPPORTED_WRITE")
            wrote_any = True
        if not wrote_any:
            self.mark_current_step_skipped()

        self.step(5, "TH verifies ElectricalCircuitNodes presence matches the CIRC feature")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.FeatureMap
        )
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attributes.AttributeList
        )
        log.info("FeatureMap: 0x%08X, AttributeList: %s", feature_map, attribute_list)
        has_circ = bool(feature_map & features.kElectricalCircuit)
        nodes_id = attributes.ElectricalCircuitNodes.attribute_id
        if has_circ:
            asserts.assert_in(nodes_id, attribute_list,
                              "ElectricalCircuitNodes must be present when the CIRC feature is set")
        else:
            asserts.assert_not_in(nodes_id, attribute_list,
                                  "ElectricalCircuitNodes must be absent when the CIRC feature is not set")


if __name__ == "__main__":
    default_matter_test_main()
