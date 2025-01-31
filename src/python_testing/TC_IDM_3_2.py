import inspect
from enum import IntFlag
# from typing import Union, get_args, get_origin
from typing import get_args

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.ClusterObjects import ClusterObject
from chip.clusters.enum import MatterIntEnum
from chip.clusters.Types import Nullable, NullValue
# from chip.clusters.Types import 
from chip.interaction_model import InteractionModelError, Status
from chip.testing import global_attribute_ids
from chip.tlv import uint
from matter_testing_infrastructure.chip.testing.basic_composition import BasicCompositionTests
from matter_testing_infrastructure.chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from matter_testing_infrastructure.chip.testing.spec_parsing import build_xml_clusters
from mobly import asserts


class TC_IDM_3_2(MatterBaseTest, BasicCompositionTests):

    async def all_type_attributes_for_cluster(self, cluster: ClusterObjects.Cluster, desired_type: type) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        all_attributes = [attribute for attribute in cluster.Attributes.__dict__.values() if inspect.isclass(
            attribute) and issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor)]

        if desired_type == MatterIntEnum:
            all_attributes_of_type = [attribute for attribute in all_attributes if type(
                attribute.attribute_type.Type) == type(ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type).Type)]

        elif desired_type == IntFlag:
            if hasattr(cluster, 'Attributes'):
                attributes_class = getattr(cluster, 'Attributes')
                if hasattr(attributes_class, 'FeatureMap'):
                    all_attributes_of_type = [cluster.Attributes.FeatureMap]
                else:
                    raise Exception(f'Cluster {cluster} lacks a FeatureMap')
            else:
                raise Exception(f'Cluster {cluster} lacks attributes')
        else:
            all_attributes_of_type = [attribute for attribute in all_attributes if attribute.attribute_type ==
                                      ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type)]

        return all_attributes_of_type

    def all_device_clusters(self) -> set:
        device_clusters = set()
        for endpoint in self.endpoints:
            device_clusters |= set(self.endpoints[endpoint].keys())
        return device_clusters

    def all_device_attributes(self) -> set:
        device_attributes = set()
        for endpoint in self.endpoints:
            for cluster in self.endpoints[endpoint]:
                device_attributes |= self.endpoints[endpoint][cluster].keys()
        return device_attributes

    def pick_writable_value(self, attribute):
        attribute_type = attribute.attribute_type.Type
        types_in_union = get_args(attribute_type)

        if uint in types_in_union or attribute_type == uint:
            value = 123456
        elif int in types_in_union or attribute_type == int:
            value = -123456
        elif str in types_in_union or attribute_type == str:
            value = "Hello World"
        elif bool in types_in_union or attribute_type == bool:
            value = True  # Todo: This value isn't always supported for every attribute (such as Network Commissioning)
        elif float in types_in_union or attribute_type == float:
            value = 1.1
        elif bytes in types_in_union or attribute_type == bytes:
            value = bytes("Hello World", "utf-8")
        elif IntFlag in types_in_union or attribute_type == IntFlag:
            value = None  # Todo: Fill in
        elif list in types_in_union or attribute_type == list:
            value = [1, 2, 3, 4, 5, 6]
        elif ClusterObjects.ClusterObjectFieldDescriptor(Type=MatterIntEnum).Type in types_in_union or attribute_type == ClusterObjects.ClusterObjectFieldDescriptor(Type=MatterIntEnum).Type:
            value = None  # Todo: Fill in
        elif ClusterObject in types_in_union or attribute_type == ClusterObject:
            value = None  # Todo: Fill in
        else:
            value = None
        return value

    async def check_attribute_write_for_type(self, desired_attribute_type: type) -> None:
        for endpoint in self.endpoints:

            for cluster, clusterdata in self.endpoints[endpoint].items():
                all_types = await self.all_type_attributes_for_cluster(cluster, desired_attribute_type)
                all_types = list(set(all_types) & set(self.writable_attributes))
                attributes_of_type = set(all_types)
                attributes_of_type_on_device = attributes_of_type.intersection(set(clusterdata.keys()))

                if attributes_of_type_on_device or attributes_of_type:
                    print(f'attributes_of_type_on_device: {attributes_of_type_on_device}, attributes_of_type: {attributes_of_type}')
                    chosen_attribute = next(iter(attributes_of_type_on_device))
                    value = self.pick_writable_value(chosen_attribute)
                    await self.write_single_attribute(
                        # output = await self.write_single_attribute(
                        attribute_value=chosen_attribute(value=value),
                        endpoint_id=endpoint,
                    )

                if attributes_of_type_on_device:
                    return True
        return False

    def generate_nullable_attributes(self) -> None:
        self.nullable_attributes = [a for a in self.writable_attributes if isinstance(a.value, Nullable)]
        self.nullable_attributes_iter = iter(self.nullable_attributes)
        return

    def generate_writable_attributes(self):
        writable_attributes = []
        command_map = {}
        for cluster_id in self.xml_clusters:
            xml_cluster = self.xml_clusters[cluster_id]
            attributes = xml_cluster.attributes
            for attribute_id, xml_attribute in attributes.items():
                write_access = xml_attribute.write_access
                if not write_access:
                    continue
                if write_access is not None and write_access < Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate:
                    continue
                if cluster_id in Clusters.ClusterObjects.ALL_ATTRIBUTES and attribute_id in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                    attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]
                    if hasattr(attribute, 'value') and write_access >= Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate:

                        writable_attributes.append(attribute)
                        if cluster_id not in command_map:
                            command_map[cluster_id] = self.xml_clusters[cluster_id].command_map

        writable_clusters = []

        for writable_attribute in writable_attributes:
            cluster = Clusters.ClusterObjects.ALL_CLUSTERS[writable_attribute.cluster_id]
            if cluster not in writable_clusters:
                writable_clusters.append(cluster)
        self.writable_attributes = writable_attributes
        self.command_map = command_map
        self.writable_attributes_iter = iter(self.writable_attributes)
        return

    @async_test_body
    async def test_TC_IDM_3_2(self):
        super().setup_class()
        await self.setup_class_helper()
        self.xml_clusters, self.problems = build_xml_clusters()
        all_clusters = [cluster for cluster in Clusters.ClusterObjects.ALL_ATTRIBUTES]
        self.generate_writable_attributes()
        self.generate_nullable_attributes()
        # expected_descriptor_attributes = ClusterObjects.ALL_ATTRIBUTES[Clusters.Objects.Descriptor.id]

        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Objects.Descriptor)])
        # returned_attributes = [a for a in read_request[0][Clusters.Objects.Descriptor].keys() if a !=
        #                        Clusters.Attribute.DataVersion]

        self.device_clusters = self.all_device_clusters()
        self.device_attributes = self.all_device_attributes()
        self.all_supported_clusters = [cluster for cluster in Clusters.__dict__.values(
        ) if inspect.isclass(cluster) and issubclass(cluster, ClusterObjects.Cluster)]

        # Step 1

        # TH selects a writeable attribute that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint.
        #  It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated
        #  value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.
        writable_attributes = []
        command_map = {}
        for cluster_id in self.xml_clusters:
            xml_cluster = self.xml_clusters[cluster_id]
            attributes = xml_cluster.attributes
            for attribute_id, xml_attribute in attributes.items():
                write_access = xml_attribute.write_access
                if not write_access:
                    continue
                if write_access is not None and write_access < Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate:
                    continue
                if cluster_id in Clusters.ClusterObjects.ALL_ATTRIBUTES and attribute_id in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                    attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]
                    if hasattr(attribute, 'value') and write_access >= Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate:

                        writable_attributes.append(attribute)
                        if cluster_id not in command_map:
                            command_map[cluster_id] = self.xml_clusters[cluster_id].command_map

        writable_clusters = []

        for writable_attribute in writable_attributes:
            cluster = Clusters.ClusterObjects.ALL_CLUSTERS[writable_attribute.cluster_id]
            if cluster not in writable_clusters:
                writable_clusters.append(cluster)

        writable_attributes_iter = iter(writable_attributes)

        chosen_writable_attribute = next(writable_attributes_iter)
        chosen_writable_attribute = next(self.writable_attributes_iter)

        chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute.cluster_id]
        if chosen_writable_cluster in self.device_clusters and chosen_writable_attribute in self.device_attributes:

            output_1 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])

            if output_1:  # Skip if no output -- e.g., happens with Objects.PumpConfigurationAndControl.Attributes.LifetimeRunningHours

                endpoint = next(iter(output_1.attributes))

                original_value = output_1.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute]

                value = self.pick_writable_value(chosen_writable_attribute)
                await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, chosen_writable_attribute(value=value))])
                output_2 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])

                asserts.assert_not_equal(output_1.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                         output_2.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                         "Output did not change")
                asserts.assert_equal(output_2.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute], value)
                write_output = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, chosen_writable_attribute(value=original_value))])
                asserts.assert_equal(write_output[0].Status, 0, "Write failed")

                output_3 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
                asserts.assert_equal(output_3.attributes[endpoint][chosen_writable_cluster]
                                     [chosen_writable_attribute], original_value, "Failure writing back to original value")

        # Step 2

        # TH selects a writeable attribute that is present on at least one endpoint.
        # If no such attribute exists, skip this step.
        # TH reads the attribute value and saves as original. TH sends a WriteRequestMessage to the DUT to write to the attribute on all endpoints. It should set a value that is different than original.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        chosen_writable_attribute = next(self.writable_attributes_iter)
        chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute.cluster_id]
        if chosen_writable_cluster in self.device_clusters and chosen_writable_attribute in self.device_attributes:
            output_1 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
            if output_1:
                endpoint = next(iter(output_1.attributes))
                value = self.pick_writable_value(chosen_writable_attribute)
                await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, chosen_writable_attribute(value=value))])
                output_2 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
                asserts.assert_not_equal(output_1.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                         output_2.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute],
                                         "Output did not change")

        # Step 3

        # TH selects a writeable attribute of type bool that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(bool)

        # Step 4

        # TH selects a writeable attribute of type string that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(str)

        # Step 5

        # TH selects a writeable attribute of type unsigned integer that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(uint)

        # Step 6

        # TH selects a writeable attribute of type signed integer that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(int)

        # Step 7

        # TH selects a writeable attribute of type floating point that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(float)

        # Step 8

        # TH selects a writeable attribute of type octet string that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(bytes)

        # Step 9

        # TH selects a writeable attribute of type struct that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(ClusterObject)

        # Step 10

        # TH selects a writeable attribute of type list that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(list)

        # Step 11

        # TH selects a writeable attribute of type enum that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(MatterIntEnum)

        # Step 12

        # TH selects a writeable attribute of type bitmap that is present on at least one endpoint.
        # If no such attribute exists, skip this step. TH reads the attribute value and saves as `original`. TH sends a WriteRequestMessage to the DUT to write to the attribute on one endpoint. It should set a value that is different than `original`.

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        await self.check_attribute_write_for_type(IntFlag)

        # Step 13

        # TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported Endpoint.
        # Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT

        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints)+2))
        unsupported = list(all_endpoints - supported_endpoints)
        result = await self.read_single_attribute_expect_error(endpoint=unsupported[0], cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.FeatureMap, error=Status.UnsupportedEndpoint)
        await self.read_single_attribute_expect_error(endpoint=unsupported[0], cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.FeatureMap, error=Status.UnsupportedEndpoint)
        chosen_writable_attribute = next(self.writable_attributes_iter)
        value = self.pick_writable_value(chosen_writable_attribute)
        await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)
        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)

        # Step 14

        # TH sends the WriteRequestMessage to the DUT to write any attribute on an unsupported cluster.
        # Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER

        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_clusters = set(list(ClusterObjects.ALL_CLUSTERS.keys()))
                dut_clusters = set(list(x.id for x in endpoint.keys()))

                unsupported = [id for id in list(all_clusters - dut_clusters) if global_attribute_ids.attribute_id_type(id)
                               == global_attribute_ids.AttributeIdType.kStandardNonGlobal]

                if unsupported:
                    unsupported_attribute = (ClusterObjects.ALL_ATTRIBUTES[unsupported[0]])[0]
                    chosen_writable_attribute = next(self.writable_attributes_iter)
                    chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute.cluster_id]
                    value = self.pick_writable_value(chosen_writable_attribute)
                    if value is not None:
                        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)
                        result = await self.read_single_attribute_expect_error(endpoint=endpoint_id, cluster=ClusterObjects.ALL_CLUSTERS[unsupported[0]], attribute=unsupported_attribute, error=Status.UnsupportedCluster)
                        asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                            msg="Unexpected success writing invalid cluster")

        # Step 15

        # TH sends the WriteRequestMessage to the DUT to write an unsupported attribute

        # Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE

        found_unsupported = False
        for endpoint_id, endpoint in self.endpoints.items():

            if found_unsupported:
                break
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(list(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys()))
                dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

                unsupported = [id for id in list(all_attrs - dut_attrs) if global_attribute_ids.attribute_id_type(id)
                               == global_attribute_ids.AttributeIdType.kStandardNonGlobal]

                if unsupported:
                    chosen_writable_attribute = next(self.writable_attributes_iter)
                    value = self.pick_writable_value(chosen_writable_attribute)
                    if value is not None:
                        output_1 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
                        original_value = output_1.attributes[endpoint][chosen_writable_cluster][chosen_writable_attribute]
                        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)
                        asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                            msg="Unexpected success writing invalid attribute")
                    found_unsupported = True
                    break
        write_output = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, chosen_writable_attribute(value=original_value))])
        asserts.assert_equal(output_3.attributes[endpoint][chosen_writable_cluster]
                             [chosen_writable_attribute], original_value, "Failure writing back to original value")

        # Step 16

        # TH selects a writeable, nullable attribute that is present on at least one endpoint.
        # If no such attribute exists, skip this step.
        # TH reads the attribute value and saves as original.
        # TH sends a WriteRequestMessage to the DUT to write to the attribute to null.

        chosen_nullable_attribute = next(self.nullable_attributes_iter)
        # This picks, e.g. Clusters.ThreadNetworkDirectory.Attributes.PreferredExtendedPanID on linux-x64-lock/chip-lock-app,
        # which contains an entry for value=Null
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [chosen_nullable_attribute]
        )
        # read_request returns {}?
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, chosen_nullable_attribute(value=NullValue))])
        # Why is this exception happening?: ValueError: Field . expected <class 'bytes'>, but got <class 'type'>
        
        read_request_2 = await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [chosen_nullable_attribute]
            )
        # TODO: Parse once ValueError exception is fixed

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        # Step 17

        # TH sends a ReadRequest message to the DUT to read any writeable attribute on any cluster.
        # If no such attribute exists, skip this step.
        # DUT returns with a report data action with the attribute values and the dataversion of the cluster.
        # TH sends a WriteRequestMessage to the DUT to modify the value of the selected attribute with the DataVersion field set to the one received in the prior step. +

        # Verify that the DUT sends a WriteResponseMessage with any status except UNSUPPORTED_WRITE or DATA_VERSION_MISMATCH. If the Status is SUCCESS, verify the updated value by sending a ReadRequestMessage for all affected paths. If the status is SUCCESS, send a WriteRequestMessage to set the value back to `original`.

        chosen_writable_attribute = next(self.writable_attributes_iter)
        chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute.cluster_id]
        output_1 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
        endpoint = next(iter(output_1.attributes))
        value = self.pick_writable_value(chosen_writable_attribute)
        # data_version = output_1.attributes[endpoint][Clusters.Thermostat][Clusters.Attribute.DataVersion]
        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)

        asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                            msg="Unexpected success writing invalid attribute")

        # Step 18

        # TH sends a ReadRequest message to the DUT to read any writeable attribute on any cluster.
        # If no such attribute exists, skip this step.
        # DUT returns with a report data action with the attribute values and the dataversion of the cluster.
        # TH sends a WriteRequestMessage to the DUT to modify the value of the selected attribute no DataVersion indicated.
        # TH sends a second WriteRequestMessage to the DUT to modify the value of an attribute with the dataversion field set to the value received earlier.
        # Verify that the DUT sends a Write Response message with the error DATA_VERSION_MISMATCH for the second Write request.

        chosen_writable_attribute = next(self.writable_attributes_iter)
        chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_writable_attribute.cluster_id]
        output_1 = await self.default_controller.Read(self.dut_node_id, [chosen_writable_attribute])
        endpoint = next(iter(output_1.attributes))

        value = self.pick_writable_value(chosen_writable_attribute)
        # data_version = output_1.attributes[endpoint][Clusters.Thermostat][Clusters.Attribute.DataVersion]
        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)
        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value + 1), endpoint_id=endpoint)

        asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                            msg="Unexpected success writing invalid attribute")

        # Step 19

        # TH sends the WriteRequestMessage to the DUT to modify the value of a specific attribute data that needs Timed Write transaction to write and this action is not part of a Timed Write transaction.
        # If no such attribute exists, skip this step.

        # On the TH verify that the DUT sends a status code NEEDS_TIMED_INTERACTION.

        command_map_iter = iter(command_map)

        while True:

            cluster_id = next(command_map_iter)
            command_dict = command_map[cluster_id]
            chosen_writable_cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]
            if command_dict and chosen_writable_cluster in writable_clusters:
                break

        for writable_attribute in writable_attributes:
            if writable_attribute.cluster_id == chosen_writable_cluster.id:
                chosen_writable_attribute = writable_attribute
                break

        chosen_command_str = next(iter(command_dict))
        value = self.pick_writable_value(chosen_writable_attribute)
        chosen_command = getattr(chosen_writable_cluster.Commands, chosen_command_str)

        await self.send_single_cmd(timedRequestTimeoutMs=1000, cmd=chosen_command(is_client=False))
        value = self.pick_writable_value(chosen_writable_attribute)
        result = await self.write_single_attribute(attribute_value=chosen_writable_attribute(value=value), endpoint_id=endpoint)
        # In one particular example, chosen_writable_attribute = Clusters.HepaFilterMonitoring.Attributes.LastChangedTime,
        # chosen_writable_cluster = Clusters.HepaFilterMonitoring, and
        # chosen_command = Clusters.HepaFilterMonitoring.Commands.ResetCondition
        # Yet the only members of this command are cluster_id, command_id, descriptor, is_client, and response_type.
        # Passing any of these (e.g. Clusters.HepaFilterMonitoring.Commands.ResetCondition(is_client=False) gives a TypeError with an
        # unexpected argument, so passing this to await self.send_single_cmd(cmd=cmd) can't be done. How to filter out commands that work?)


if __name__ == "__main__":
    default_matter_test_main()
