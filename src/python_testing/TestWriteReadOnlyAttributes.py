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
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import cast

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TestWriteReadOnlyAttributes(MatterBaseTest):

    @async_test_body
    async def test_invalid_endpoint_command(self):
        self.print_step(0, "Commissioning - already done")

        self.print_step(1, "Find an invalid endpoint id")
        root_parts = cast(
            list[int],
            await self.read_single_attribute_check_success(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.PartsList,
                endpoint=0,
            ),
        )
        endpoints = set(root_parts)
        invalid_endpoint_id = 1
        while invalid_endpoint_id in endpoints:
            invalid_endpoint_id += 1
        non_root_endpoint = endpoints.pop()

        # The test assumes the following known ids:
        #   invalid_endpoint_id - is NOT a valid endpoint in general
        #   non_root_endpoint - a VALID endpoint id, but it is not Endpoint0
        #      In particular, root endpoint clusters should not exist there
        #      like AccessControl. So `non_root_endpoint/AccessControl::ClusterRevision/attributeList/...` is
        #      a invalid ReadOnly path
        read_only_attributes = [
            Clusters.AccessControl.Attributes.ClusterRevision(123),
            Clusters.AccessControl.Attributes.AcceptedCommandList([1, 2, 3]),
            Clusters.AccessControl.Attributes.GeneratedCommandList([2, 3]),
            Clusters.AccessControl.Attributes.AttributeList([100, 200]),
        ]

        self.print_step(2, "Write read only attributes on a VALID cluster")
        for attr in read_only_attributes:
            asserts.assert_equal(
                await self.write_single_attribute(
                    attr, endpoint_id=0, expect_success=False
                ),
                Status.UnsupportedWrite,
            )

        self.print_step(
            3, "Write read only attributes on a Invalid cluster for that endpoint"
        )
        for attr in read_only_attributes:
            asserts.assert_equal(
                await self.write_single_attribute(
                    attr, endpoint_id=non_root_endpoint, expect_success=False
                ),
                Status.UnsupportedCluster,
            )

        self.print_step(4, "Write read only attributes on a Invalid endpoint")
        for attr in read_only_attributes:
            asserts.assert_equal(
                await self.write_single_attribute(
                    attr, endpoint_id=invalid_endpoint_id, expect_success=False
                ),
                Status.UnsupportedEndpoint,
            )


if __name__ == "__main__":
    default_matter_test_main()
