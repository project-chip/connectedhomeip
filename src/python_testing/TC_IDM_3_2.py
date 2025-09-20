#
#    Copyright (c) 2024 Project CHIP Authors
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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.interaction_model import Status
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_IDM_3_2(MatterBaseTest, BasicCompositionTests):
    """Test case for IDM-3.2: Write Response Action from DUT to TH. [{DUT_Server}]"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.endpoint = 0

    def steps_TC_IDM_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported Endpoint. DUT responds with the Write Response action",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT", is_commissioning=True),
            TestStep(2, "TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported cluster. DUT responds with the Write Response action",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER"),
            TestStep(3, "TH sends the WriteRequestMessage to the DUT to write an unsupported attribute. DUT responds with the Write Response action",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE"),
            TestStep(4, "TH sends the WriteRequestMessage to the DUT to modify the value of one attribute and Set SuppressResponse to True.",
                     "On the TH verify that the DUT does not send a Write Response message with a success back to the TH."),
            TestStep(5, "TH sends a ReadRequest message to the DUT to read any attribute on any cluster. DUT returns with a report data action with the attribute values and the dataversion of the cluster. TH sends a WriteRequestMessage to the DUT to modify the value of one attribute with the DataVersion field set to the one received in the prior step.",
                     "Verify that the DUT sends a Write Response message with a success back to the TH. Verify by sending a ReadRequest that the Write Action on DUT was successful."),
            TestStep(6, "TH sends a ReadRequest message to the DUT to read any attribute on any cluster. DUT returns with a report data action with the attribute values and the dataversion of the cluster. TH sends a WriteRequestMessage to the DUT to modify the value of one attribute no DataVersion indicated. TH sends a second WriteRequestMessage to the DUT to modify the value of an attribute with the dataversion field set to the value received earlier.",
                     "Verify that the DUT sends a Write Response message with the error DATA_VERSION_MISMATCH for the second Write request."),
            TestStep(7, "TH sends the WriteRequestMessage to the DUT to modify the value of a specific attribute data that needs Timed Write transaction to write and this action is not part of a Timed Write transaction.",
                     "On the TH verify that the DUT sends a status code NEEDS_TIMED_INTERACTION."),
        ]
        return steps

    @async_test_body
    async def test_TC_IDM_3_2(self):
        # Test Setup with robust endpoint/cluster discovery
        await self.setup_class_helper(allow_pase=False)

        self.step(1)
        # Write any attribute on an unsupported endpoint to DUT
        # Find an unsupported endpoint
        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints) + 2))
        unsupported_endpoints = list(all_endpoints - supported_endpoints)
        unsupported_endpoint = unsupported_endpoints[0]

        test_attribute = Clusters.Descriptor.Attributes.FeatureMap

        # Try to write to an unsupported endpoint using framework method
        write_status = await self.write_single_attribute(
            attribute_value=test_attribute(0),  
            endpoint_id=unsupported_endpoint,
            expect_success=False
        )

        # Verify we get UNSUPPORTED_ENDPOINT error
        asserts.assert_equal(write_status, Status.UnsupportedEndpoint,
                             f"Write to unsupported endpoint should return UNSUPPORTED_ENDPOINT, got {write_status}")

        self.step(2)
        # Write all attributes on an unsupported cluster to DUT
        # Find an unsupported cluster (reusing logic from _read_unsupported_cluster)
        supported_cluster_ids = set()
        for endpoint_clusters in self.endpoints.values():
            supported_cluster_ids.update({cluster.id for cluster in endpoint_clusters.keys(
            ) if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard})

        # Get all possible standard clusters
        all_standard_cluster_ids = {cluster_id for cluster_id in ClusterObjects.ALL_CLUSTERS.keys(
        ) if global_attribute_ids.cluster_id_type(cluster_id) == global_attribute_ids.ClusterIdType.kStandard}

        # Find unsupported clusters
        unsupported_cluster_ids = all_standard_cluster_ids - supported_cluster_ids

        if not unsupported_cluster_ids:
            self.skip_step("No unsupported standard clusters found to test")
            return

        # Use the first unsupported cluster
        unsupported_cluster_id = next(iter(unsupported_cluster_ids))
        unsupported_cluster = ClusterObjects.ALL_CLUSTERS[unsupported_cluster_id]

        # Get any writable attribute from this cluster
        cluster_attributes = ClusterObjects.ALL_ATTRIBUTES[unsupported_cluster_id]
        test_unsupported_attribute = next(iter(cluster_attributes.values()))

        write_status = await self.write_single_attribute(
            attribute_value=test_unsupported_attribute,
            endpoint_id=self.endpoint,
            expect_success=False
        )
        # Verify we get UNSUPPORTED_CLUSTER error
        asserts.assert_equal(write_status, Status.UnsupportedCluster,
                             f"Write to unsupported cluster should return UNSUPPORTED_CLUSTER, got {write_status}")

        self.step(3)
        # Write an unsupported attribute to DUT
        unsupported_endpoint = None
        unsupported_attribute = None
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster_data in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys())
                dut_attrs = set(cluster_data.get(cluster_type.Attributes.AttributeList, []))

                unsupported = [
                    attr_id for attr_id in (all_attrs - dut_attrs)
                    if global_attribute_ids.attribute_id_type(attr_id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal
                ]
                if unsupported:
                    unsupported_attribute = ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]]
                    unsupported_endpoint = endpoint_id
                    logging.info(f"Found unsupported attribute: {unsupported_attribute} in cluster {cluster_type.id}")
                    break
            if unsupported_attribute:
                logging.info(f"Unsupported attribute: {unsupported_attribute}")
                break

        if not unsupported_attribute:
            asserts.fail("No unsupported attributes found - we must find at least one unsupported attribute")

        # Attempting to write to the device now
        write_status2 = await self.write_single_attribute(
            attribute_value=unsupported_attribute(0),
            endpoint_id=unsupported_endpoint,
            expect_success=False
        )
        logging.info(f"Writing unsupported attribute: {unsupported_attribute}")
        asserts.assert_equal(write_status2, Status.UnsupportedAttribute,
                             f"Write to unsupported attribute should return UNSUPPORTED_ATTRIBUTE, got {write_status2}")

        self.step(4)
        # TH sends the WriteRequestMessage to the DUT to modify the value of one attribute and Set SuppressResponse to True.
        # Currently there is an open issue for the SuppressResponse not being available currently and is mentioned in the yaml file for this test::
        # Issue Link: https://github.com/project-chip/connectedhomeip/issues/8043
        # Out of Scope
        # For now similar to the yaml file version, skipping this test step as the Python API doesn't expose SuppressResponse
        logging.info(
            "Step 4: SuppressResponse parameter not supported in current WriteAttribute API, please refer to the issue link for more details")

        self.step(5)
        # TH sends a ReadRequest message to the DUT to read any attribute on any cluster.
        # DUT returns with a report data action with the attribute values and the dataversion of the cluster.
        # TH sends a WriteRequestMessage to the DUT to modify the value of one attribute with the DataVersion field set to the one received in the prior step.

        # Read an attribute to get the current DataVersion
        test_cluster = Clusters.BasicInformation
        test_attribute = Clusters.BasicInformation.Attributes.NodeLabel
        read_result = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(self.endpoint, test_cluster, test_attribute)]
        )

        # Get the current DataVersion
        current_data_version = read_result[self.endpoint][test_cluster][Clusters.Attribute.DataVersion]
        logging.info(f"Current DataVersion for cluster {test_cluster.id}: {current_data_version}")

        new_value0 = "New-Label-Step5"
        write_result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(self.endpoint, test_attribute(new_value0), current_data_version)]
        )

        # Verify write was successful
        asserts.assert_equal(write_result[0].Status, Status.Success,
                             f"Write with correct DataVersion should succeed, got {write_result[0].Status}")

        # Verify the value was written by reading it back
        actual_value = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=test_cluster, attribute=test_attribute)

        asserts.assert_equal(actual_value, new_value0,
                             f"Read value {actual_value} should match written value {new_value0}")

        self.step(6)
        # TH sends a ReadRequest message to the DUT to read any attribute on any cluster.
        # DUT returns with a report data action with the attribute values and the dataversion of the cluster.
        # TH sends a WriteRequestMessage to the DUT to modify the value of one attribute no DataVersion indicated.
        # TH sends a second WriteRequestMessage to the DUT to modify the value of an attribute with the dataversion field set to the value received earlier.

        # First, read to get the initial DataVersion
        initial_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(self.endpoint, test_cluster, test_attribute)]
        )

        initial_data_version = initial_read[self.endpoint][test_cluster][Clusters.Attribute.DataVersion]
        logging.info(f"Initial DataVersion for step 6: {initial_data_version}")

        # Write without DataVersion using framework method (this should succeed and increment the DataVersion)
        new_value1 = "New-Label-Step6"
        write_status = await self.write_single_attribute(
            attribute_value=test_attribute(new_value1),
            endpoint_id=self.endpoint,
            expect_success=True
        )

        # Now try to write with the old DataVersion using direct WriteAttribute (this should fail with DATA_VERSION_MISMATCH)
        new_value2 = "New-Label-Step6-2"
        write_result_old_version = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(self.endpoint, test_attribute(new_value2), initial_data_version)]
        )

        # Verify we get DATA_VERSION_MISMATCH error
        asserts.assert_equal(write_result_old_version[0].Status, Status.DataVersionMismatch,
                             f"Write with old DataVersion should return DATA_VERSION_MISMATCH, got {write_result_old_version[0].Status}")

        self.step(7)
        # TH sends the WriteRequestMessage to the DUT to modify the value of a specific attribute data that needs Timed Write transaction to write and this action is not part of a Timed Write transaction.
        # This test is in the yaml script version, it requires this to be implemented on the DUT.
        # For now, skipping this test step as the Python API doesn't expose the timed write transaction
        logging.info("Step 7: Timed write transaction does not appear to be supported in the current Python API")


if __name__ == "__main__":
    default_matter_test_main()
