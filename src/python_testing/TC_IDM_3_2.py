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
from typing import Any, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
# from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_IDM_3_2(MatterBaseTest, BasicCompositionTests):
    """Test case for IDM-3.2: Write Response Action from DUT to TH. [{DUT_Server}]"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.endpoint = 0

    async def find_timed_write_attribute(
        self, endpoints_data: dict[int, Any]
    ) -> tuple[Optional[int], Optional[type[ClusterObjects.ClusterAttributeDescriptor]]]:
        """
        Find an attribute that requires timed write on the actual device
        Uses the wildcard read data that's already in endpoints_data
        """
        logging.info(f"Searching for timed write attributes across {len(endpoints_data)} endpoints")

        for endpoint_id, endpoint in endpoints_data.items():
            for cluster_type, cluster_data in endpoint.items():
                cluster_id = cluster_type.id

                cluster_type_enum = global_attribute_ids.cluster_id_type(cluster_id)
                # If debugging, please uncomment the following line to add Unit Testing clusters to the search and comment out the line below it.
                if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard and cluster_type_enum != global_attribute_ids.ClusterIdType.kTest:
                    # if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                for attr_type in cluster_data:
                    # Check if this is an attribute descriptor class
                    if (isinstance(attr_type, type) and
                            issubclass(attr_type, ClusterObjects.ClusterAttributeDescriptor)):
                        # Check if this attribute requires timed write using the must_use_timed_write class property
                        if attr_type.must_use_timed_write:
                            logging.info(f"Found timed write attribute: {attr_type.__name__} "
                                         f"in cluster {cluster_type.__name__} on endpoint {endpoint_id}")
                            return endpoint_id, attr_type

        logging.warning("No timed write attributes found on device")
        return None, None

    def steps_TC_IDM_3_2(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported Endpoint. DUT responds with the Write Response action",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT"),
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

    @async_test_body
    async def test_TC_IDM_3_2(self):
        self.step(0)

        # Test Setup with robust endpoint/cluster discovery
        await self.setup_class_helper(allow_pase=False)

        self.step(1)
        '''
        Write any attribute on an unsupported endpoint to DUT
        Find an unsupported endpoint
        '''
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
        '''
        Write all attributes on an unsupported cluster to DUT
        Find an unsupported cluster
        '''
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

        # Use the first unsupported cluster
        unsupported_cluster_id = next(iter(unsupported_cluster_ids))
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
            logging.warning("No unsupported attributes found - this may be OK for non-commissionable devices")
        else:
            write_status2 = await self.write_single_attribute(
                attribute_value=unsupported_attribute(0),
                endpoint_id=unsupported_endpoint,
                expect_success=False
            )
            logging.info(f"Writing unsupported attribute: {unsupported_attribute}")
            asserts.assert_equal(write_status2, Status.UnsupportedAttribute,
                                 f"Write to unsupported attribute should return UNSUPPORTED_ATTRIBUTE, got {write_status2}")

        self.skip_step(4)
        # Currently skipping step 4 as we have removed support in the python framework for this functionality currently.
        # This is now contained in the SuppressResponse test module PR referenced below for TC_IDM_3_2, once this test module merges that PR can then be merged
        # and this test step will become valid after issues with SuppressResponse mentioned in issue https://github.com/project-chip/connectedhomeip/issues/41227.
        # SuppressResponse PR Reference: https://github.com/project-chip/connectedhomeip/pull/41590
        # TODO: Once the SuppressResponse test module PR merges, uncomment the following code and remove the skip_step line above.

        """
        self.step(4)
        # Check if NodeLabel attribute exists for step 4 (SuppressResponse tests)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=Clusters.BasicInformation.Attributes.NodeLabel):
            '''
            TH sends the WriteRequestMessage to the DUT to modify the value of one attribute and Set SuppressResponse to True.

            NOTE: Per Issue #41227, the current spec does not strictly enforce that devices must suppress the response.
            For now, we just ensure the device doesn't crash. The device MAY respond or may not - either is acceptable.
            Future spec revisions will enforce no response behavior.
            Reference: https://github.com/project-chip/connectedhomeip/issues/41227
            '''

            test_attribute = Clusters.BasicInformation.Attributes.NodeLabel
            test_value = "SuppressResponse-Test"

            logging.info("Testing SuppressResponse functionality with NodeLabel attribute")
            logging.info("NOTE: Device may or may not respond - both behaviors are acceptable for now per Issue #41227")

            # Send write request with suppressResponse=True
            # Device may respond or not - we just ensure it doesn't crash
            try:
                res = await self.default_controller.WriteAttribute(
                    nodeid=self.dut_node_id,
                    attributes=[(self.endpoint, test_attribute(test_value))],
                    suppressResponse=True
                )
                logging.info(f"Device responded to suppressResponse=True request: {res}")
            except Exception as e:
                # Device didn't respond (timeout or other error) - this is also acceptable
                logging.info(f"Device did not respond or encountered error: {e}")

            # Verify the write operation succeeded by reading back the value
            logging.info("Verifying that the write operation succeeded")
            actual_value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint,
                cluster=Clusters.BasicInformation,
                attribute=test_attribute
            )

            asserts.assert_equal(actual_value, test_value,
                                 f"Attribute should be written. Expected {test_value}, got {actual_value}")
        """
        # Check if NodeLabel attribute exists for steps 5 and 6 (DataVersion test steps)
        self.step(5)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=Clusters.BasicInformation.Attributes.NodeLabel):
            '''
            TH sends a ReadRequest message to the DUT to read any attribute on any cluster.
            DUT returns with a report data action with the attribute values and the dataversion of the cluster.
            TH sends a WriteRequestMessage to the DUT to modify the value of one attribute with the DataVersion field set to the one received in the prior step.
            '''
            test_cluster = Clusters.BasicInformation
            test_attribute = Clusters.BasicInformation.Attributes.NodeLabel
            new_value0 = "New-Label-Step5"

            # Read an attribute to get the current DataVersion
            read_result = await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [(self.endpoint, test_cluster, test_attribute)]
            )

            # Get the current DataVersion
            current_data_version = read_result[self.endpoint][test_cluster][Clusters.Attribute.DataVersion]
            logging.info(f"Current DataVersion for cluster {test_cluster.id}: {current_data_version}")

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
        if await self.attribute_guard(endpoint=self.endpoint, attribute=Clusters.BasicInformation.Attributes.NodeLabel):
            '''
            TH sends a ReadRequest message to the DUT to read any attribute on any cluster.
            DUT returns with a report data action with the attribute values and the dataversion of the cluster.
            TH sends a WriteRequestMessage to the DUT to modify the value of one attribute no DataVersion indicated.
            TH sends a second WriteRequestMessage to the DUT to modify the value of an attribute with the dataversion field set to the value received earlier.
            '''

            # First, read to get the initial DataVersion
            initial_read = await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [(self.endpoint, test_cluster, test_attribute)]
            )

            initial_data_version = initial_read[self.endpoint][test_cluster][Clusters.Attribute.DataVersion]
            logging.info(f"Initial DataVersion for step 6: {initial_data_version}")

            # Write without DataVersion (this should succeed and increment the DataVersion)
            new_value1 = "New-Label-Step6"
            write_status = await self.write_single_attribute(
                attribute_value=test_attribute(new_value1),
                endpoint_id=self.endpoint,
                expect_success=True
            )

            # Now try to write with the old DataVersion (this should fail with DATA_VERSION_MISMATCH)
            new_value2 = "New-Label-Step6-2"
            write_result_old_version = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(self.endpoint, test_attribute(new_value2), initial_data_version)]
            )

            # Verify we get DATA_VERSION_MISMATCH error
            asserts.assert_equal(write_result_old_version[0].Status, Status.DataVersionMismatch,
                                 f"Write with old DataVersion should return DATA_VERSION_MISMATCH, got {write_result_old_version[0].Status}")

        else:
            # NodeLabel doesn't exist - skip these steps for now
            # Created following follow-up task for the event that the node label attribute does not exist
            # TODO: https://github.com/project-chip/matter-test-scripts/issues/693
            logging.info("NodeLabel not found - this may be a non-commissionable device")

        endpoint_id, timed_attr = await self.find_timed_write_attribute(self.endpoints)
        if timed_attr:
            self.step(7)
            '''
            TH sends the WriteRequestMessage to the DUT to modify the value of a specific attribute data that needs
            timed write transaction to write and this action is not part of a timed write transaction.

            This step tests the following 3 timed write error scenarios:
            1. NEEDS_TIMED_INTERACTION: Writing timed-write-required attribute without timed transaction
            2. TIMED_REQUEST_MISMATCH: Writing with TimedRequest flag but no actual timed transaction
                                        (Timed Request ACTION = No, TimedRequest FLAG = True)
            3. TIMED_REQUEST_MISMATCH: Writing with timed action performed but TimedRequest flag set to false
                                        (Timed Request ACTION = Yes, TimedRequest FLAG = False)

            Understanding the distinction:
            - TIMED REQUEST ACTION: The TimedRequest protocol message sent BEFORE the WriteRequest
            - TIMEDREQUEST FLAG: A boolean field IN the WriteRequest message itself

            Normal timed write: Action=Yes, Flag=True (both must match)
            '''

            # Test with the real timed-write attribute found on the device
            logging.info(f"Testing timed write attribute: {timed_attr}")

            # Test NEEDS_TIMED_INTERACTION - Writing timed-write-required attribute without timed transaction
            # Found below logic in /home/ubuntu/connectedhomeapi/connectedhomeip/src/controller/python/tests/scripts/cluster_objects.py and TC_IDM_1_2 test logic.
            logging.info("Writing timed-write-required attribute without timedRequestTimeoutMs should be rejected")
            try:
                await self.default_controller.WriteAttribute(
                    self.dut_node_id,
                    attributes=[(endpoint_id, timed_attr(True))]
                )
                asserts.fail("The write request should be rejected due to InteractionModelError: NeedsTimedInteraction (0xc6).")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.NeedsTimedInteraction,
                                     f"WriteAttribute should return NeedsTimedInteraction, got {e.status}")

            # TIMED_REQUEST_MISMATCH - Writing with TimedRequest flag but no actual timed transaction
            # Thanks to Cecille for the guidance on the test step logic and plumbing for this to function below.
            logging.info("Writing with TimedRequest flag but no timed transaction should return TIMED_REQUEST_MISMATCH")
            try:
                await self.default_controller.TestOnlyWriteAttributeWithMismatchedTimedRequestField(
                    self.dut_node_id,
                    timedRequestTimeoutMs=0,  # No timed action
                    timedRequestFieldValue=True,  # But field=true
                    attributes=[(endpoint_id, timed_attr(False))]
                )
                asserts.fail("The write request should be rejected due to InteractionModelError: TimedRequestMismatch (0xc9).")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.TimedRequestMismatch,
                                     f"WriteAttribute should return TimedRequestMismatch, got {e.status}")

            # TIMED_REQUEST_MISMATCH - Writing with timed action performed but TimedRequest flag set to false
            logging.info("Writing with timed action but TimedRequest flag=false should return TIMED_REQUEST_MISMATCH")
            try:
                await self.default_controller.TestOnlyWriteAttributeWithMismatchedTimedRequestField(
                    self.dut_node_id,
                    timedRequestTimeoutMs=1000,  # Timed action performed
                    timedRequestFieldValue=False,  # But field=false
                    attributes=[(endpoint_id, timed_attr(False))]
                )
                asserts.fail("The write request should be rejected due to InteractionModelError: TimedRequestMismatch (0xc9).")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.TimedRequestMismatch,
                                     f"WriteAttribute should return TimedRequestMismatch, got {e.status}")

        else:
            self.skip_step(7)


if __name__ == "__main__":
    default_matter_test_main()
