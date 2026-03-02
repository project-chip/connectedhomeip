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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device contact-sensor --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Attribute import AttributePath
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

MAX_NUM_PATHS_IN_MTU = 50


class TC_IDM_2_3(BasicCompositionTests):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.endpoint = 0

    @property
    def default_timeout(self) -> int:
        return 5 * 60

    def steps_TC_IDM_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "TH reads from the DUT the CapabilityMinima attribute from the Basic Information Cluster.", is_commissioning=True),
            TestStep(2, "TH reads from the DUT all attributes from all clusters on all endpoints, to collect valid AttributePaths."),
            TestStep(3, "TH sends a Read Request Message to the DUT with a number of paths up to the ReadPathsSupported value."),
            TestStep(4, "TH sends a Subscribe Request Message to the DUT with a number of paths up to the SubscribePathsSupported value. TH then modifies one of the subscribed attributes and waits for a Report Data Action."),
        ]

    def verify_paths_in_response(self, requested_paths, response_tlv):
        """Helper to verify that all requested paths are present in the response TLV data."""
        for path in requested_paths:
            endpoint_id = path.EndpointId
            cluster_id = path.ClusterId
            attribute_id = path.AttributeId

            asserts.assert_in(endpoint_id, response_tlv,
                              f"Endpoint {endpoint_id} missing in response")
            asserts.assert_in(cluster_id, response_tlv[endpoint_id],
                              f"Cluster {cluster_id} missing in response for endpoint {endpoint_id}")
            asserts.assert_in(attribute_id, response_tlv[endpoint_id][cluster_id],
                              f"Attribute {attribute_id} missing in response for endpoint {endpoint_id}, cluster {cluster_id}")

    def get_paths(self, count, all_paths):
        path_list = []
        num_available_paths = len(all_paths)
        for i in range(count):
            # Use modulo to wrap around if count > num_available_paths
            path_index = i % num_available_paths
            path_list.append(all_paths[path_index])
        return path_list

    @async_test_body
    async def test_TC_IDM_2_3(self):
        # Setup class helper performs wildcard read and populates self.endpoints_tlv
        await self.setup_class_helper(allow_pase=False)

        # Step 1: CapabilityMinima
        self.step(1)
        cluster_revision = await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.ClusterRevision
        )

        # Default values for number of read paths and subscribe paths
        num_read_paths_supported = 9
        num_subscribe_paths_supported = 3

        capability_minima = await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.CapabilityMinima
        )

        if cluster_revision >= 6:
            asserts.assert_is_not_none(capability_minima.readPathsSupported,
                                       "ReadPathsSupported should be present when ClusterRevision >= 6")
            asserts.assert_is_not_none(capability_minima.subscribePathsSupported,
                                       "SubscribePathsSupported should be present when ClusterRevision >= 6")

            # Extract values, providing defaults if optional fields are missing
            num_read_paths_supported = capability_minima.readPathsSupported
            num_subscribe_paths_supported = capability_minima.subscribePathsSupported
        else:
            log.info("Basic Information Cluster revision is less than 6, read paths and subscribe paths are not part of CapabilityMinima struct.")

        log.info(f"CapabilityMinima: readPathsSupported={num_read_paths_supported}, "
                 f"subscribePathsSupported={num_subscribe_paths_supported}")

        # Step 2: Collect available paths
        self.step(2)
        all_paths = []
        for endpoint_id, clusters in self.endpoints_tlv.items():
            for cluster_id, cluster_data in clusters.items():
                if global_attribute_ids.cluster_id_type(cluster_id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue
                # Use Global Attribute List to find valid attributes
                attr_list = cluster_data.get(global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)
                if attr_list:
                    for attr_id in attr_list:
                        all_paths.append(AttributePath(EndpointId=endpoint_id, ClusterId=cluster_id, AttributeId=attr_id))

        if not all_paths:
            # Fallback for empty devices (unlikely)
            all_paths = [AttributePath(EndpointId=0, ClusterId=Clusters.BasicInformation.id,
                                       AttributeId=Clusters.BasicInformation.Attributes.NodeLabel.attribute_id)]

        # Step 3: Read max number of paths
        self.step(3)
        read_paths = self.get_paths(num_read_paths_supported, all_paths)

        async def read_request(paths):
            log.info("Conducting read request")
            return await self.default_controller.Read(self.dut_node_id, paths)

        # Read requests must fit into 1 MTU, as reads cannot be chained across multiple packets. If a device reports
        # a number of read paths (or subscription paths) larger than what is possible on the controller side, we need to
        # reduce the number of paths here to be as much as can fit in the request.
        async def conduct_request_with_potential_path_size_reduction(paths, num_paths, request_function):
            # TODO: The maximum here should be adjusted and be based upon the max size
            # of an AttributePath, as well as the size of the payload for the MTU. See Issue #43083
            if num_paths > MAX_NUM_PATHS_IN_MTU:
                paths[:] = paths[:MAX_NUM_PATHS_IN_MTU]
                log.info(f"Reduced number of paths used from {num_paths} to {MAX_NUM_PATHS_IN_MTU}")
            return await request_function(paths)

        read_response = await conduct_request_with_potential_path_size_reduction(read_paths, num_read_paths_supported, read_request)
        asserts.assert_is_not_none(
            read_response, "No response returned from read request. Ensure the number of paths in request is valid.")
        self.verify_paths_in_response(read_paths, read_response.tlvAttributes)
        log.info("Successfully completed read request")

        # Step 4: Subscribe with max paths in a single request
        self.step(4)

        initial_node_label = await self.read_single_attribute_check_success(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.NodeLabel
        )

        # TODO: Should have resilience to a missing node label
        sub_paths = self.get_paths(num_subscribe_paths_supported, all_paths)
        sub_paths[0] = AttributePath(EndpointId=0, ClusterId=Clusters.BasicInformation.id,
                                     AttributeId=Clusters.BasicInformation.Attributes.NodeLabel.attribute_id)

        handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.NodeLabel
        )

        async def subscribe_request(paths):
            log.info("Conducting subscribe request")
            return await self.default_controller.Read(
                self.dut_node_id,
                paths,
                reportInterval=(1, 1000),
                keepSubscriptions=False
            )

        sub_transaction = await conduct_request_with_potential_path_size_reduction(sub_paths, num_subscribe_paths_supported, subscribe_request)
        asserts.assert_is_not_none(
            sub_transaction, "No response returned from subscribe request. Ensure the number of paths in request is valid.")
        log.info("Successfully completed subscribe request")
        sub_transaction.SetAttributeUpdateCallback(handler)

        # Write new NodeLabel to trigger a subscription report
        # Generate a new label guaranteed different from initial state.
        new_label = initial_node_label
        while new_label == initial_node_label:
            new_label = str(random.randint(0, 999999))
        await self.write_single_attribute(
            Clusters.BasicInformation.Attributes.NodeLabel(value=new_label),
            endpoint_id=0
        )

        # Verify change of NodeLabel received.
        handler.await_sequence_of_reports(
            attribute=Clusters.BasicInformation.Attributes.NodeLabel,
            sequence=[new_label],
            timeout_sec=10
        )

        log.info("Successfully subscribed and verified report sequence.")


if __name__ == "__main__":
    default_matter_test_main()
