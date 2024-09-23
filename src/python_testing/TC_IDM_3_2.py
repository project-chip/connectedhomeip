from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
import chip.clusters as Clusters

from basic_composition_support import BasicCompositionTests
from mobly import asserts
from spec_parsing_support import build_xml_clusters

class TC_IDM_3_2(MatterBaseTest, BasicCompositionTests):

    @async_test_body
    async def test_TC_IDM_3_2(self):

        # Step 1

        # TH selects a writeable attribute that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint.
        #  It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated
        #  value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.
        self.user_params["use_pase_only"] = False
        super().setup_class()
        await self.setup_class_helper()
        self.xml_clusters, self.problems = build_xml_clusters()
        writable_attributes = []
        for cluster_id in self.xml_clusters:
            xml_cluster = self.xml_clusters[cluster_id]
            attributes = xml_cluster.attributes
            for attribute_id in attributes:
                xml_attribute = attributes[attribute_id]
                write_access = xml_attribute.write_access
                if write_access:
                    if cluster_id in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                        try:
                            if hasattr(Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id], 'value'):
                                writable_attributes.append((cluster_id, attribute_id, write_access))
                        except KeyError: # Some XML clusters aren't present in ALL_CLUSTERS or ALL_ATTRIBUTES
                            pass
        chosen_writable_attribute_tuple = writable_attributes[0]
        chosen_writable_attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[chosen_writable_attribute_tuple[0]][chosen_writable_attribute_tuple[1]]
        chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute_tuple[0]]
        
        output_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [chosen_writable_attribute])
        endpoint = next(iter(output_1))
        await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, chosen_writable_attribute(value=123456))])
        output_3 = await self.default_controller.ReadAttribute(self.dut_node_id, [chosen_writable_attribute])
        
        asserts.assert_not_equal(output_1[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                 output_3[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                 "Output did not change")

# Step 2

# TH selects a writeable attribute that is present on at least one endpoint.
# If no such attribute exists, skip this step.
# TH reads the attribute value and saves as original. TH sends a WriteRequestMessage to the DUT to write to the attribute on all endpoints. It should set a value that is different than original.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 3

# TH selects a writeable attribute of type bool that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 4

# TH selects a writeable attribute of type string that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 5

# TH selects a writeable attribute of type unsigned integer that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 6

# TH selects a writeable attribute of type signed integer that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 7

# TH selects a writeable attribute of type floating point that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 8

# TH selects a writeable attribute of type octet string that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 9

# TH selects a writeable attribute of type struct that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 10

# TH selects a writeable attribute of type list that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 11

# TH selects a writeable attribute of type enum that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 12

# TH selects a writeable attribute of type bitmap that is present on at least one endpoint.
# If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 13

# TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported Endpoint.
# Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT

# Step 14

# TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported cluster.
# Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER

# Step 15

# TH sends the WriteRequestMessage to the DUT to write an unsupported attribute

# Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE

# Step 16

# TH selects a writeable, nullable attribute that is present on at least one endpoint.
# If no such attribute exists, skip this step.
# TH reads the attribute value and saves as original.
# TH sends a WriteRequestMessage to the DUT to write to the attribute to null.

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 17

# TH sends a ReadRequest message to the DUT to read any writeable attribute on any cluster.
# If no such attribute exists, skip this step.
# DUT returns with a report data action with the attribute values and the dataversion of the cluster.
# TH sends a WriteRequestMessage to the DUT to modify the value of the selected attribute with the DataVersion field set to the one received in the prior step. +

# Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

# Step 18

# TH sends a ReadRequest message to the DUT to read any writeable attribute on any cluster.
# If no such attribute exists, skip this step.
# DUT returns with a report data action with the attribute values and the dataversion of the cluster.
# TH sends a WriteRequestMessage to the DUT to modify the value of the selected attribute no DataVersion indicated.
# TH sends a second WriteRequestMessage to the DUT to modify the value of an attribute with the dataversion field set to the value received earlier.

# Verify that the DUT sends a Write Response message with the error DATA_VERSION_MISMATCH for the second Write request.

# Step 19

# TH sends the WriteRequestMessage to the DUT to modify the value of a specific attribute data that needs Timed Write transaction to write and this action is not part of a Timed Write transaction.
# If no such attribute exists, skip this step.

# On the TH verify that the DUT sends a status code NEEDS_TIMED_INTERACTION.

if __name__ == "__main__":
    default_matter_test_main()