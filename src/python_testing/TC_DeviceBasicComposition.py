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
import json
import logging
import pathlib
import sys
from dataclasses import dataclass
from pprint import pprint
from typing import Any, Callable, Optional

import chip.clusters as Clusters
import chip.tlv
from chip import discovery
from chip.clusters.Attribute import ValueDecodeFailure
from chip.exceptions import ChipStackError
from chip.setup_payload import SetupPayload
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


class TC_DeviceBasicComposition(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        dev_ctrl = self.default_controller
        self.problems = []

        do_test_over_pase = self.user_params.get("use_pase_only", True)
        dump_device_composition_path: Optional[str] = self.user_params.get("dump_device_composition_path", None)

        if do_test_over_pase:
            if self.matter_test_config.qr_code_content is not None:
                qr_code = self.matter_test_config.qr_code_content
                try:
                    setup_payload = SetupPayload().ParseQrCode(qr_code)
                except ChipStackError:
                    asserts.fail(f"QR code '{qr_code} failed to parse properly as a Matter setup code.")

            elif self.matter_test_config.manual_code is not None:
                manual_code = self.matter_test_config.manual_code
                try:
                    setup_payload = SetupPayload().ParseManualPairingCode(manual_code)
                except ChipStackError:
                    asserts.fail(
                        f"Manual code code '{manual_code}' failed to parse properly as a Matter setup code. Check that all digits are correct and length is 11 or 21 characters.")
            else:
                asserts.fail("Require either --qr-code or --manual-code to proceed with PASE needed for test.")

            if setup_payload.short_discriminator is not None:
                filter_type = discovery.FilterType.SHORT_DISCRIMINATOR
                filter_value = setup_payload.short_discriminator
            else:
                filter_type = discovery.FilterType.LONG_DISCRIMINATOR
                filter_value = setup_payload.long_discriminator

            commissionable_nodes = dev_ctrl.DiscoverCommissionableNodes(
                filter_type, filter_value, stopOnFirst=True, timeoutSecond=15)
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
                dev_ctrl.EstablishPASESessionIP(address, setup_payload.setup_passcode, node_id)
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
    def test_endpoint_zero_present(self):
        logging.info("Validating that the Root Node endpoint is present (EP0)")
        if 0 not in self.endpoints:
            self.record_error(self.get_test_name(), location=AttributePathLocation(endpoint_id=0),
                              problem="Did not find Endpoint 0.", spec_location="Endpoint Composition")
            self.fail_current_test()

    def test_descriptor_present_on_each_endpoint(self):
        logging.info("Validating each endpoint has a descriptor cluster")

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

    def test_global_attributes_present_on_each_cluster(self):
        logging.info("Validating each cluster has the mandatory global attributes")

        @dataclass
        class RequiredMandatoryAttribute:
            id: int
            name: str
            validator: Callable

        ATTRIBUTE_LIST_ID = 0xFFFB

        ATTRIBUTES_TO_CHECK = [
            RequiredMandatoryAttribute(id=0xFFFD, name="ClusterRevision", validator=check_int_in_range(1, 0xFFFF)),
            RequiredMandatoryAttribute(id=0xFFFC, name="FeatureMap", validator=check_int_in_range(0, 0xFFFF_FFFF)),
            RequiredMandatoryAttribute(id=0xFFFB, name="AttributeList",
                                       validator=check_non_empty_list_of_ints_in_range(0, 0xFFFF_FFFF)),
            # TODO: Check for EventList
            # RequiredMandatoryAttribute(id=0xFFFA, name="EventList", validator=check_list_of_ints_in_range(0, 0xFFFF_FFFF)),
            RequiredMandatoryAttribute(id=0xFFF9, name="AcceptedCommandList",
                                       validator=check_list_of_ints_in_range(0, 0xFFFF_FFFF)),
            RequiredMandatoryAttribute(id=0xFFF8, name="GeneratedCommandList",
                                       validator=check_list_of_ints_in_range(0, 0xFFFF_FFFF)),
        ]

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

                    # Validate attribute value based on the provided validator.
                    try:
                        req_attribute.validator(cluster[req_attribute.id])
                    except ValueError as e:
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f"Failed validation of value on {location.as_string(self.cluster_mapper)}: {str(e)}", spec_location="Global Elements")
                        success = False
                        continue

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

        if not success:
            self.fail_current_test(
                "At least one cluster was missing a mandatory global attribute or had differences between claimed attributes supported and actual.")

    def test_all_attribute_strings_valid(self):
        asserts.skip("TODO: Validate every string in the attribute tree is valid UTF-8 and has no nulls")

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

    def test_topology_is_valid(self):
        asserts.skip("TODO: Make a test that verifies each endpoint only lists direct descendants, except Root Node and Aggregator endpoints that list all their descendants")


if __name__ == "__main__":
    default_matter_test_main()
