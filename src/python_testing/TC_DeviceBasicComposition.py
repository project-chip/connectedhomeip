#
#    Copyright (c) 2023 Project CHIP Authors
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

import base64
import copy
import functools
import json
import logging
import pathlib
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pprint import pprint
from typing import Any, Callable, Optional

import chip.clusters as Clusters
import chip.clusters.ClusterObjects
import chip.tlv
from chip.clusters.Attribute import ValueDecodeFailure
from matter_testing_support import AttributePathLocation, MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


def MatterTlvToJson(tlv_data: dict[int, Any]) -> dict[str, Any]:
    """Given TLV data for a specific cluster instance, convert to the Matter JSON format."""

    matter_json_dict = {}

    key_type_mappings = {
        chip.tlv.uint: "UINT",
        int: "INT",
        bool: "BOOL",
        list: "ARRAY",
        dict: "STRUCT",
        chip.tlv.float32: "FLOAT",
        float: "DOUBLE",
        bytes: "BYTES",
        str: "STRING",
        ValueDecodeFailure: "ERROR",
        type(None): "NULL",
    }

    def ConvertValue(value) -> Any:
        if isinstance(value, ValueDecodeFailure):
            raise ValueError(f"Bad Value: {str(value)}")

        if isinstance(value, bytes):
            return base64.b64encode(value).decode("UTF-8")
        elif isinstance(value, list):
            value = [ConvertValue(item) for item in value]
        elif isinstance(value, dict):
            value = MatterTlvToJson(value)

        return value

    for key in tlv_data:
        value_type = type(tlv_data[key])
        value = copy.deepcopy(tlv_data[key])

        element_type: str = key_type_mappings[value_type]
        sub_element_type = ""

        try:
            new_value = ConvertValue(value)
        except ValueError as e:
            new_value = str(e)

        if element_type:
            if element_type == "ARRAY":
                if len(new_value):
                    sub_element_type = key_type_mappings[type(tlv_data[key][0])]
                else:
                    sub_element_type = "?"

        new_key = ""
        if element_type:
            if sub_element_type:
                new_key = f"{str(key)}:{element_type}-{sub_element_type}"
            else:
                new_key = f"{str(key)}:{element_type}"
        else:
            new_key = str(key)

        matter_json_dict[new_key] = new_value

    return matter_json_dict


@dataclass
class TagProblem:
    root: int
    missing_attribute: bool
    missing_feature: bool
    duplicates: set[int]
    same_tag: set[int] = field(default_factory=set)


def check_int_in_range(min_value: int, max_value: int, allow_null: bool = False) -> Callable:
    """Returns a checker for whether `obj` is an int that fits in a range."""
    def int_in_range_checker(obj: Any):
        """Inner checker logic for check_int_in_range

        Checker validates that `obj` must have decoded as an integral value in range [min_value, max_value].

        On failure, a ValueError is raised with a diagnostic message.
        """
        if obj is None and allow_null:
            return

        if not isinstance(obj, int) and not isinstance(obj, chip.tlv.uint):
            raise ValueError(f"Value {str(obj)} is not an integer or uint (decoded type: {type(obj)})")
        int_val = int(obj)
        if (int_val < min_value) or (int_val > max_value):
            raise ValueError(
                f"Value {int_val} (0x{int_val:X}) not in range [{min_value}, {max_value}] ([0x{min_value:X}, 0x{max_value:X}])")

    return int_in_range_checker


def check_list_of_ints_in_range(min_value: int, max_value: int, min_size: int = 0, max_size: int = 65535, allow_null: bool = False) -> Callable:
    """Returns a checker for whether `obj` is a list of ints that fit in a range."""
    def list_of_ints_in_range_checker(obj: Any):
        """Inner checker for check_list_of_ints_in_range.

        Checker validates that `obj` must have decoded as a list of integral values in range [min_value, max_value].
        The length of the list must be between [min_size, max_size].

        On failure, a ValueError is raised with a diagnostic message.
        """
        if obj is None and allow_null:
            return

        if not isinstance(obj, list):
            raise ValueError(f"Value {str(obj)} is not a list, but a list was expected (decoded type: {type(obj)})")

        if len(obj) < min_size or len(obj) > max_size:
            raise ValueError(
                f"Value {str(obj)} is a list of size {len(obj)}, but expected a list with size in range [{min_size}, {max_size}]")

        for val_idx, val in enumerate(obj):
            if not isinstance(val, int) and not isinstance(val, chip.tlv.uint):
                raise ValueError(
                    f"At index {val_idx} in {str(obj)}, value {val} is not an int/uint, but an int/uint was expected (decoded type: {type(val)})")

            int_val = int(val)
            if not ((int_val >= min_value) and (int_val <= max_value)):
                raise ValueError(
                    f"At index {val_idx} in {str(obj)}, value {int_val} (0x{int_val:X}) not in range [{min_value}, {max_value}] ([0x{min_value:X}, 0x{max_value:X}])")

    return list_of_ints_in_range_checker


def check_non_empty_list_of_ints_in_range(min_value: int, max_value: int, max_size: int = 65535, allow_null: bool = False) -> Callable:
    """Returns a checker for whether `obj` is a non-empty list of ints that fit in a range."""
    return check_list_of_ints_in_range(min_value, max_value, min_size=1, max_size=max_size, allow_null=allow_null)


def check_no_duplicates(obj: Any) -> None:
    if not isinstance(obj, list):
        raise ValueError(f"Value {str(obj)} is not a list, but a list was expected (decoded type: {type(obj)})")
    if len(set(obj)) != len(obj):
        raise ValueError(f"Value {str(obj)} contains duplicate values")


def separate_endpoint_types(endpoint_dict: dict[int, Any]) -> tuple[list[int], list[int]]:
    """Returns a tuple containing the list of flat endpoints and a list of tree endpoints"""
    flat = []
    tree = []
    for endpoint_id, endpoint in endpoint_dict.items():
        if endpoint_id == 0:
            continue
        aggregator_id = 0x000e
        device_types = [d.deviceType for d in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]]
        if aggregator_id in device_types:
            flat.append(endpoint_id)
        else:
            tree.append(endpoint_id)
    return (flat, tree)


def get_all_children(endpoint_id, endpoint_dict: dict[int, Any]) -> set[int]:
    """Returns all the children (include subchildren) of the given endpoint
       This assumes we've already checked that there are no cycles, so we can do the dumb things and just trace the tree
    """
    children = set()

    def add_children(endpoint_id, children):
        immediate_children = endpoint_dict[endpoint_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        if not immediate_children:
            return
        children.update(set(immediate_children))
        for child in immediate_children:
            add_children(child, children)

    add_children(endpoint_id, children)
    return children


def find_tree_roots(tree_endpoints: list[int], endpoint_dict: dict[int, Any]) -> set[int]:
    """Returns a set of all the endpoints in tree_endpoints that are roots for a tree (not include singletons)"""
    tree_roots = set()

    def find_tree_root(current_id):
        for endpoint_id, endpoint in endpoint_dict.items():
            if endpoint_id not in tree_endpoints:
                continue
            if current_id in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
                # this is not the root, move up
                return find_tree_root(endpoint_id)
        return current_id

    for endpoint_id in tree_endpoints:
        root = find_tree_root(endpoint_id)
        if root != endpoint_id:
            tree_roots.add(root)
    return tree_roots


def parts_list_cycles(tree_endpoints: list[int], endpoint_dict: dict[int, Any]) -> list[int]:
    """Returns a list of all the endpoints in the tree_endpoints list that contain cycles"""
    def parts_list_cycle_detect(visited: set, current_id: int) -> bool:
        if current_id in visited:
            return True
        visited.add(current_id)
        for child in endpoint_dict[current_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
            child_has_cycles = parts_list_cycle_detect(visited, child)
            if child_has_cycles:
                return True
        return False

    cycles = []
    # This is quick enough that we can do all the endpoints wihtout searching for the roots
    for endpoint_id in tree_endpoints:
        visited = set()
        if parts_list_cycle_detect(visited, endpoint_id):
            cycles.append(endpoint_id)
    return cycles


def create_device_type_lists(roots: list[int], endpoint_dict: dict[int, Any]) -> dict[int, dict[int, set[int]]]:
    """Returns a list of endpoints per device type for each root in the list"""
    device_types = {}
    for root in roots:
        tree_device_types = defaultdict(set)
        eps = get_all_children(root, endpoint_dict)
        eps.add(root)
        for ep in eps:
            for d in endpoint_dict[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]:
                tree_device_types[d.deviceType].add(ep)
        device_types[root] = tree_device_types

    return device_types


def cmp_tag_list(a: Clusters.Descriptor.Structs.SemanticTagStruct, b: Clusters.Descriptor.Structs.SemanticTagStruct):
    if a.mfgCode != b.mfgCode:
        return -1 if a.mfgCode < b.mfgCode else 1
    if a.namespaceID != b.namespaceID:
        return -1 if a.namespaceID < b.namespaceID else 1
    if a.tag != b.tag:
        return -1 if a.tag < b.tag else 1
    if a.label != b.label:
        return -1 if a.label < b.label else 1
    return 0


def find_tag_list_problems(roots: list[int], device_types: dict[int, dict[int, set[int]]], endpoint_dict: dict[int, Any]) -> dict[int, TagProblem]:
    """Checks for non-spec compliant tag lists"""
    tag_problems = {}
    for root in roots:
        for _, endpoints in device_types[root].items():
            if len(endpoints) < 2:
                continue
            for endpoint in endpoints:
                missing_feature = not bool(endpoint_dict[endpoint][Clusters.Descriptor]
                                           [Clusters.Descriptor.Attributes.FeatureMap] & Clusters.Descriptor.Bitmaps.Feature.kTagList)
                if Clusters.Descriptor.Attributes.TagList not in endpoint_dict[endpoint][Clusters.Descriptor] or endpoint_dict[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] == []:
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=True,
                                                        missing_feature=missing_feature, duplicates=endpoints)
                    continue
                # Check that this tag isn't the same as the other tags in the endpoint list
                duplicate_tags = set()
                for other in endpoints:
                    if other == endpoint:
                        continue
                    # The OTHER endpoint is missing a tag list attribute - ignore this here, we'll catch that when we assess this endpoint as the primary
                    if Clusters.Descriptor.Attributes.TagList not in endpoint_dict[other][Clusters.Descriptor]:
                        continue

                    if sorted(endpoint_dict[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList], key=functools.cmp_to_key(cmp_tag_list)) == sorted(endpoint_dict[other][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList], key=functools.cmp_to_key(cmp_tag_list)):
                        duplicate_tags.add(other)
                if len(duplicate_tags) != 0:
                    duplicate_tags.add(endpoint)
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=False, missing_feature=missing_feature,
                                                        duplicates=endpoints, same_tag=duplicate_tags)
                    continue
                if missing_feature:
                    tag_problems[endpoint] = TagProblem(root=root, missing_attribute=False,
                                                        missing_feature=missing_feature, duplicates=endpoints)

    return tag_problems


class TC_DeviceBasicComposition(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        dev_ctrl = self.default_controller
        self.problems = []

        do_test_over_pase = self.user_params.get("use_pase_only", True)
        dump_device_composition_path: Optional[str] = self.user_params.get("dump_device_composition_path", None)

        if do_test_over_pase:
            info = self.get_setup_payload_info()

            commissionable_nodes = dev_ctrl.DiscoverCommissionableNodes(
                info.filter_type, info.filter_value, stopOnFirst=True, timeoutSecond=15)
            logging.info(f"Commissionable nodes: {commissionable_nodes}")
            # TODO: Support BLE
            if commissionable_nodes is not None and len(commissionable_nodes) > 0:
                commissionable_node = commissionable_nodes[0]
                instance_name = f"{commissionable_node.instanceName}._matterc._udp.local"
                vid = f"{commissionable_node.vendorId}"
                pid = f"{commissionable_node.productId}"
                address = f"{commissionable_node.addresses[0]}"
                logging.info(f"Found instance {instance_name}, VID={vid}, PID={pid}, Address={address}")

                node_id = 1
                dev_ctrl.EstablishPASESessionIP(address, info.passcode, node_id)
            else:
                asserts.fail("Failed to find the DUT according to command line arguments.")
        else:
            # Using the already commissioned node
            node_id = self.dut_node_id

        wildcard_read = (await dev_ctrl.Read(node_id, [()]))
        endpoints_tlv = wildcard_read.tlvAttributes

        node_dump_dict = {endpoint_id: MatterTlvToJson(endpoints_tlv[endpoint_id]) for endpoint_id in endpoints_tlv}
        logging.info(f"Raw TLV contents of Node: {json.dumps(node_dump_dict, indent=2)}")

        if dump_device_composition_path is not None:
            with open(pathlib.Path(dump_device_composition_path).with_suffix(".json"), "wt+") as outfile:
                json.dump(node_dump_dict, outfile, indent=2)
            with open(pathlib.Path(dump_device_composition_path).with_suffix(".txt"), "wt+") as outfile:
                pprint(wildcard_read.attributes, outfile, indent=1, width=200, compact=True)

        logging.info("###########################################################")
        logging.info("Start of actual tests")
        logging.info("###########################################################")

        # ======= State kept for use by all tests =======

        # All endpoints in "full object" indexing format
        self.endpoints = wildcard_read.attributes

        # All endpoints in raw TLV format
        self.endpoints_tlv = wildcard_read.tlvAttributes

    def get_test_name(self) -> str:
        """Return the function name of the caller. Used to create logging entries."""
        return sys._getframe().f_back.f_code.co_name

    def fail_current_test(self, msg: Optional[str] = None):
        if not msg:
            # Without a message, just log the last problem seen
            asserts.fail(msg=self.problems[-1].problem)
        else:
            asserts.fail(msg)

    # ======= START OF ACTUAL TESTS =======
    def test_TC_SM_1_1(self):
        ROOT_NODE_DEVICE_TYPE = 0x16
        self.print_step(1, "Perform a wildcard read of attributes on all endpoints - already done")
        self.print_step(2, "Verify that endpoint 0 exists")
        if 0 not in self.endpoints:
            self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=0),
                              problem="Did not find Endpoint 0.", spec_location="Endpoint Composition")
            self.fail_current_test()

        self.print_step(3, "Verify that endpoint 0 descriptor cluster includes the root node device type")
        if Clusters.Descriptor not in self.endpoints[0]:
            self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=0),
                              problem="No descriptor cluster on Endpoint 0", spec_location="Root node device type")
            self.fail_current_test()

        listed_device_types = [i.deviceType for i in self.endpoints[0]
                               [Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]]
        if ROOT_NODE_DEVICE_TYPE not in listed_device_types:
            self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=0),
                              problem="Root node device type not listed on endpoint 0", spec_location="Root node device type")
            self.fail_current_test()

        self.print_step(4, "Verify that the root node device type does not appear in any of the non-zero endpoints")
        for endpoint_id, endpoint in self.endpoints.items():
            if endpoint_id == 0:
                continue
            listed_device_types = [i.deviceType for i in endpoint[Clusters.Descriptor]
                                   [Clusters.Descriptor.Attributes.DeviceTypeList]]
            if ROOT_NODE_DEVICE_TYPE in listed_device_types:
                self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=endpoint_id),
                                  problem=f'Root node device type listed on endpoint {endpoint_id}', spec_location="Root node device type")
                self.fail_current_test()

        self.print_step(5, "Verify the existence of all the root node clusters on EP0")
        root = self.endpoints[0]
        required_clusters = [Clusters.BasicInformation, Clusters.AccessControl, Clusters.GroupKeyManagement,
                             Clusters.GeneralCommissioning, Clusters.AdministratorCommissioning, Clusters.OperationalCredentials, Clusters.GeneralDiagnostics]
        for c in required_clusters:
            if c not in root:
                self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=0),
                                  problem=f'Root node does not contain required cluster {c}', spec_location="Root node device type")
                self.fail_current_test()

    def test_DT_1_1(self):
        self.print_step(1, "Perform a wildcard read of attributes on all endpoints - already done")
        self.print_step(2, "Verify that each endpoint includes a descriptor cluster")
        success = True
        for endpoint_id, endpoint in self.endpoints.items():
            has_descriptor = (Clusters.Descriptor in endpoint)
            logging.info(f"Checking descriptor on Endpoint {endpoint_id}: {'found' if has_descriptor else 'not_found'}")
            if not has_descriptor:
                self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=endpoint_id, cluster_id=Clusters.Descriptor.id),
                                  problem=f"Did not find a descriptor on endpoint {endpoint_id}", spec_location="Base Cluster Requirements for Matter")
                success = False

        if not success:
            self.fail_current_test("At least one endpoint was missing the descriptor cluster.")

    def test_IDM_10_1(self):
        self.print_step(1, "Perform a wildcard read of attributes on all endpoints - already done")

        self.print_step(2, "Validate all global attributes are present")

        @dataclass
        class RequiredMandatoryAttribute:
            id: int
            name: str
            validators: list[Callable]

        ATTRIBUTE_LIST_ID = 0xFFFB

        ATTRIBUTES_TO_CHECK = [
            RequiredMandatoryAttribute(id=0xFFFD, name="ClusterRevision", validators=[check_int_in_range(1, 0xFFFF)]),
            RequiredMandatoryAttribute(id=0xFFFC, name="FeatureMap", validators=[check_int_in_range(0, 0xFFFF_FFFF)]),
            RequiredMandatoryAttribute(id=0xFFFB, name="AttributeList",
                                       validators=[check_non_empty_list_of_ints_in_range(0, 0xFFFF_FFFF), check_no_duplicates]),
            # TODO: Check for EventList
            # RequiredMandatoryAttribute(id=0xFFFA, name="EventList", validator=check_list_of_ints_in_range(0, 0xFFFF_FFFF)),
            RequiredMandatoryAttribute(id=0xFFF9, name="AcceptedCommandList",
                                       validators=[check_list_of_ints_in_range(0, 0xFFFF_FFFF), check_no_duplicates]),
            RequiredMandatoryAttribute(id=0xFFF8, name="GeneratedCommandList",
                                       validators=[check_list_of_ints_in_range(0, 0xFFFF_FFFF), check_no_duplicates]),
        ]

        self.print_step(3, "Validate all reported attributes match AttributeList")
        success = True
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                for req_attribute in ATTRIBUTES_TO_CHECK:
                    attribute_string = self.cluster_mapper.get_attribute_string(cluster_id, req_attribute.id)

                    has_attribute = (req_attribute.id in cluster)
                    location = AttributePathLocation(endpoint_id, cluster_id, req_attribute.id)
                    logging.info(
                        f"Checking for mandatory global {attribute_string} on {location.as_cluster_string(self.cluster_mapper)}: {'found' if has_attribute else 'not_found'}")

                    # Check attribute is actually present
                    if not has_attribute:
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f"Did not find mandatory global {attribute_string} on {location.as_cluster_string(self.cluster_mapper)}", spec_location="Global Elements")
                        success = False
                        continue

                    # Validate attribute value based on the provided validators.
                    for validator in req_attribute.validators:
                        try:
                            validator(cluster[req_attribute.id])
                        except ValueError as e:
                            self.record_error(self.get_test_name(), location=location,
                                              problem=f"Failed validation of value on {location.as_string(self.cluster_mapper)}: {str(e)}", spec_location="Global Elements")
                            success = False
                            continue

        # Validate there are attributes in the global range that are not in the required list
        allowed_globals = [a.id for a in ATTRIBUTES_TO_CHECK]
        # also allow event list because it's not disallowed
        event_list_id = 0xFFFA
        allowed_globals.append(event_list_id)
        global_range_min = 0x0000_F000
        standard_range_max = 0x000_4FFF
        mei_range_min = 0x0001_0000
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                globals = [a for a in cluster[ATTRIBUTE_LIST_ID] if a >= global_range_min and a < mei_range_min]
                unexpected_globals = sorted(list(set(globals) - set(allowed_globals)))
                for unexpected in unexpected_globals:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=unexpected)
                    self.record_error(self.get_test_name(), location=location,
                                      problem=f"Unexpected global attribute {unexpected} in cluster {cluster_id}", spec_location="Global elements")
                    success = False

        # validate that all the returned attributes in the standard clusters contain only known attribute ids
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                if cluster_id not in chip.clusters.ClusterObjects.ALL_ATTRIBUTES:
                    # Skip clusters that are not part of the standard generated corpus (e.g. MS clusters)
                    continue
                standard_attributes = [a for a in cluster[ATTRIBUTE_LIST_ID] if a <= standard_range_max]
                allowed_standard_attributes = chip.clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]
                unexpected_standard_attributes = sorted(list(set(standard_attributes) - set(allowed_standard_attributes)))
                for unexpected in unexpected_standard_attributes:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=unexpected)
                    self.record_error(self.get_test_name(), location=location,
                                      problem=f"Unexpected standard attribute {unexpected} in cluster {cluster_id}", spec_location=f"Cluster {cluster_id}")
                    success = False

        # validate there are no attributes in the range between standard and global
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                bad_range_values = [a for a in cluster[ATTRIBUTE_LIST_ID] if a > standard_range_max and a < global_range_min]
                for bad in bad_range_values:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=bad)
                    self.record_error(self.get_test_name(), location=location,
                                      problem=f"Attribute in undefined range {bad} in cluster {cluster_id}", spec_location=f"Cluster {cluster_id}")
                    success = False

        # Validate that any attribute in the manufacturer prefix range is in the standard suffix range.
        suffix_mask = 0x000_FFFF
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                manufacturer_range_values = [a for a in cluster[ATTRIBUTE_LIST_ID] if a > mei_range_min]
                for manufacturer_value in manufacturer_range_values:
                    suffix = manufacturer_value & suffix_mask
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                     attribute_id=manufacturer_value)
                    if suffix > standard_range_max and suffix < global_range_min:
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f"Manufacturer attribute in undefined range {manufacturer_value} in cluster {cluster_id}",
                                          spec_location=f"Cluster {cluster_id}")
                        success = False
                    elif suffix >= global_range_min:
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f"Manufacturer attribute in global range {manufacturer_value} in cluster {cluster_id}",
                                          spec_location=f"Cluster {cluster_id}")
                        success = False

        # TODO: maybe while we're at it, we should check that the command list doesn't contain unexpected commands.

        # Validate presence of claimed attributes
        if success:
            # TODO: Also check the reverse: that each attribute appears in the AttributeList.
            logging.info(
                "Validating that a wildcard read on each cluster provided all attributes claimed in AttributeList mandatory global attribute")

            for endpoint_id, endpoint in self.endpoints_tlv.items():
                for cluster_id, cluster in endpoint.items():
                    attribute_list = cluster[ATTRIBUTE_LIST_ID]
                    for attribute_id in attribute_list:
                        location = AttributePathLocation(endpoint_id, cluster_id, attribute_id)
                        has_attribute = attribute_id in cluster

                        attribute_string = self.cluster_mapper.get_attribute_string(cluster_id, attribute_id)
                        logging.info(
                            f"Checking presence of claimed supported {attribute_string} on {location.as_cluster_string(self.cluster_mapper)}: {'found' if has_attribute else 'not_found'}")

                        # Check attribute is actually present.
                        if not has_attribute:
                            # TODO: Handle detecting write-only attributes from schema.
                            if "WriteOnly" in attribute_string:
                                continue

                            self.record_error(self.get_test_name(), location=location,
                                              problem=f"Did not find {attribute_string} on {location.as_cluster_string(self.cluster_mapper)} when it was claimed in AttributeList ({attribute_list})", spec_location="AttributeList Attribute")
                            success = False
                            continue

                        attribute_value = cluster[attribute_id]
                        if isinstance(attribute_value, ValueDecodeFailure):
                            self.record_warning(self.get_test_name(), location=location,
                                                problem=f"Found a failure to read/decode {attribute_string} on {location.as_cluster_string(self.cluster_mapper)} when it was claimed as supported in AttributeList ({attribute_list}): {str(attribute_value)}", spec_location="AttributeList Attribute")
                            # Warn only for now
                            # TODO: Fail in the future
                            continue
                    for attribute_id in cluster:
                        if attribute_id not in attribute_list:
                            attribute_string = self.cluster_mapper.get_attribute_string(cluster_id, attribute_id)
                            location = AttributePathLocation(endpoint_id, cluster_id, attribute_id)
                            self.record_error(self.get_test_name(), location=location,
                                              problem=f'Found attribute {attribute_string} on {location.as_cluster_string(self.cluster_mapper)} not listed in attribute list', spec_location="AttributeList Attribute")
                            success = False

        if not success:
            self.fail_current_test(
                "At least one cluster was missing a mandatory global attribute or had differences between claimed attributes supported and actual.")

    def test_IDM_11_1(self):
        success = True
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                for attribute_id, attribute in cluster.items():
                    if cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES or attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                        continue
                    if Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id].attribute_type.Type is not str:
                        continue
                    try:
                        cluster[attribute_id].encode('utf-8', errors='strict')
                    except UnicodeError:
                        location = AttributePathLocation(endpoint_id, cluster_id, attribute_id)
                        attribute_string = self.cluster_mapper.get_attribute_string(cluster_id, attribute_id)
                        self.record_error(self.get_test_name(
                        ), location=location, problem=f'Attribute {attribute_string} on {location.as_cluster_string(self.cluster_mapper)} is invalid UTF-8', spec_location="Data types - Character String")
                        success = False
        if not success:
            self.fail_current_test("At least one attribute string was not valid UTF-8")

    def test_all_event_strings_valid(self):
        asserts.skip("TODO: Validate every string in the read events is valid UTF-8 and has no nulls")

    def test_all_schema_scalars(self):
        asserts.skip("TODO: Validate all int/uint are in range of the schema (or null if nullable) for known attributes")

    def test_all_commands_reported_are_executable(self):
        asserts.skip("TODO: Validate all commands reported in AcceptedCommandList are actually executable")

    def test_dump_all_pics_for_all_endpoints(self):
        asserts.skip("TODO: Make a test that generates the basic PICS list for each endpoint based on actually reported contents")

    def test_all_schema_mandatory_elements_present(self):
        asserts.skip(
            "TODO: Make a test that ensures every known cluster has the mandatory elements present (commands, attributes) based on features")

    def test_all_endpoints_have_valid_composition(self):
        asserts.skip(
            "TODO: Make a test that verifies each endpoint has valid set of device types, and that the device type conformance is respected for each")

    def test_TC_SM_1_2(self):
        self.print_step(1, "Wildcard read of device - already done")

        self.print_step(2, "Verify the Descriptor cluster PartsList on endpoint 0 exactly lists all the other (non-0) endpoints on the DUT")
        parts_list_0 = self.endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        cluster_id = Clusters.Descriptor.id
        attribute_id = Clusters.Descriptor.Attributes.PartsList.attribute_id
        location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=attribute_id)
        if len(self.endpoints.keys()) != len(set(self.endpoints.keys())):
            self.record_error(self.get_test_name(), location=location,
                              problem='duplicate endpoint ids found in the returned data', spec_location="PartsList Attribute")
            self.fail_current_test()

        if len(parts_list_0) != len(set(parts_list_0)):
            self.record_error(self.get_test_name(), location=location,
                              problem='Duplicate endpoint ids found in the parts list on ep0', spec_location="PartsList Attribute")
            self.fail_current_test()

        expected_parts = set(self.endpoints.keys())
        expected_parts.remove(0)
        if set(parts_list_0) != expected_parts:
            self.record_error(self.get_test_name(), location=location,
                              problem='EP0 Descriptor parts list does not match the set of returned endpoints', spec_location="PartsList Attribute")
            self.fail_current_test()

        self.print_step(
            3, "For each endpoint on the DUT (including EP 0), verify the PartsList in the Descriptor cluster on that endpoint does not include itself")
        for endpoint_id, endpoint in self.endpoints.items():
            if endpoint_id in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=f"Endpoint {endpoint_id} parts list includes itself", spec_location="PartsList Attribute")
                self.fail_current_test()

        self.print_step(4, "Separate endpoints into flat and tree style")
        flat, tree = separate_endpoint_types(self.endpoints)

        self.print_step(5, "Check for cycles in the tree endpoints")
        cycles = parts_list_cycles(tree, self.endpoints)
        if len(cycles) != 0:
            for id in cycles:
                location = AttributePathLocation(endpoint_id=id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=f"Endpoint {id} parts list includes a cycle", spec_location="PartsList Attribute")
            self.fail_current_test()

        self.print_step(6, "Check flat lists include all sub ids")
        ok = True
        for endpoint_id in flat:
            # ensure that every sub-id in the parts list is included in the parent
            sub_children = []
            for child in self.endpoints[endpoint_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]:
                sub_children.update(get_all_children(child))
            if not all(item in sub_children for item in self.endpoints[endpoint_id][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]):
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem='Flat parts list does not include all the sub-parts', spec_location='Endpoint composition')
                ok = False
        if not ok:
            self.fail_current_test()

    def test_TC_PS_3_1(self):
        BRIDGED_NODE_DEVICE_TYPE_ID = 0x13
        success = True
        self.print_step(1, "Wildcard read of device - already done")

        self.print_step(2, "Verify that all endpoints listed in the EndpointList are valid")
        attribute_id = Clusters.PowerSource.Attributes.EndpointList.attribute_id
        cluster_id = Clusters.PowerSource.id
        attribute_string = self.cluster_mapper.get_attribute_string(cluster_id, attribute_id)
        for endpoint_id, endpoint in self.endpoints.items():
            if Clusters.PowerSource not in endpoint:
                continue
            location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
            cluster_revision = Clusters.PowerSource.Attributes.ClusterRevision
            if cluster_revision not in endpoint[Clusters.PowerSource]:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                 attribute_id=cluster_revision.attribute_id)
                self.record_error(self.get_test_name(
                ), location=location, problem=f'Did not find Cluster revision on {location.as_cluster_string(self.cluster_mapper)}', spec_location='Global attributes')
            if endpoint[Clusters.PowerSource][cluster_revision] < 2:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                 attribute_id=cluster_revision.attribute_id)
                self.record_note(self.get_test_name(), location=location,
                                 problem='Power source ClusterRevision is < 2, skipping remainder of test for this endpoint')
                continue
            if Clusters.PowerSource.Attributes.EndpointList not in endpoint[Clusters.PowerSource]:
                self.record_error(self.get_test_name(), location=location,
                                  problem=f'Did not find {attribute_string} on {location.as_cluster_string(self.cluster_mapper)}', spec_location="EndpointList Attribute")
                success = False
                continue

            endpoint_list = endpoint[Clusters.PowerSource][Clusters.PowerSource.Attributes.EndpointList]
            non_existent = set(endpoint_list) - set(self.endpoints.keys())
            if non_existent:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=f'{attribute_string} lists a non-existent endpoint', spec_location="EndpointList Attribute")
                success = False

        self.print_step(3, "Verify that all Bridged Node endpoint lists are correct")
        device_types = {}
        parts_list = {}
        for endpoint_id, endpoint in self.endpoints.items():
            if Clusters.PowerSource not in endpoint or Clusters.PowerSource.Attributes.EndpointList not in endpoint[Clusters.PowerSource]:
                continue

            def GetPartValidityProblem(endpoint):
                if Clusters.Descriptor not in endpoint:
                    return "Missing cluster descriptor"
                if Clusters.Descriptor.Attributes.PartsList not in endpoint[Clusters.Descriptor]:
                    return "Missing PartList in descriptor cluster"
                if Clusters.Descriptor.Attributes.DeviceTypeList not in endpoint[Clusters.Descriptor]:
                    return "Missing DeviceTypeList in descriptor cluster"
                return None

            problem = GetPartValidityProblem(endpoint)
            if problem:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=Clusters.Descriptor.id,
                                                 attribute_id=Clusters.Descriptor.Attributes.PartsList.id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=problem, spec_location="PartsList Attribute")
                success = False
                continue

            device_types[endpoint_id] = [i.deviceType for i in endpoint[Clusters.Descriptor]
                                         [Clusters.Descriptor.Attributes.DeviceTypeList]]
            parts_list[endpoint_id] = endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]

        bridged_nodes = [id for (id, dev_type) in device_types.items() if BRIDGED_NODE_DEVICE_TYPE_ID in dev_type]

        for endpoint_id in bridged_nodes:
            if Clusters.PowerSource not in self.endpoints[endpoint_id]:
                continue
            # using a list because we do want to preserve duplicates and error on those.
            desired_endpoint_list = parts_list[endpoint_id].copy()
            desired_endpoint_list.append(endpoint_id)
            desired_endpoint_list.sort()
            ep_list = self.endpoints[endpoint_id][Clusters.PowerSource][Clusters.PowerSource.Attributes.EndpointList]
            ep_list.sort()
            if ep_list != desired_endpoint_list:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=f'Power source EndpointList on bridged node endpoint {endpoint_id} is not as expected. Desired: {desired_endpoint_list} Actual: {ep_list}', spec_location="EndpointList Attribute")
                success = False

        self.print_step(4, "Verify that all Bridged Node children endpoint lists are correct")
        children = []
        # note, this doesn't handle the full tree structure, single layer only
        for endpoint_id in bridged_nodes:
            children = children + parts_list[endpoint_id]

        for endpoint_id in children:
            if Clusters.PowerSource not in self.endpoints[endpoint_id]:
                continue
            desired_endpoint_list = [endpoint_id]
            ep_list = self.endpoints[endpoint_id][Clusters.PowerSource][Clusters.PowerSource.Attributes.EndpointList]
            ep_list.sort()
            if ep_list != desired_endpoint_list:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                self.record_error(self.get_test_name(), location=location,
                                  problem=f'Power source EndpointList on bridged child endpoint {endpoint_id} is not as expected. Desired: {desired_endpoint_list} Actual: {ep_list}', spec_location="EndpointList Attribute")
                success = False

        if not success:
            self.fail_current_test("power source EndpointList attribute is incorrect")

    def test_DESC_2_2(self):
        self.print_step(1, "Wildcard read of device - already done")

        self.print_step(2, "Identify all endpoints that are roots of a tree-composition")
        _, tree = separate_endpoint_types(self.endpoints)
        roots = find_tree_roots(tree, self.endpoints)

        self.print_step(
            3, "For each tree root, go through each of the children and add their endpoint IDs to a list of device types based on the DeviceTypes list")
        device_types = create_device_type_lists(roots, self.endpoints)

        self.print_step(
            4, "For device types with more than one endpoint listed, ensure each of the listed endpoints has a tag attribute and the tag attributes are not the same")
        problems = find_tag_list_problems(roots, device_types, self.endpoints)

        for ep, problem in problems.items():
            location = AttributePathLocation(endpoint_id=ep, cluster_id=Clusters.Descriptor.id,
                                             attribute_id=Clusters.Descriptor.Attributes.TagList.attribute_id)
            msg = f'problem on ep {ep}: missing feature = {problem.missing_feature}, missing attribute = {problem.missing_attribute}, duplicates = {problem.duplicates}, same_tags = {problem.same_tag}'
            self.record_error(self.get_test_name(), location=location, problem=msg, spec_location="Descriptor TagList")

        if problems:
            self.fail_current_test("Problems with tags lists")


if __name__ == "__main__":
    default_matter_test_main()
