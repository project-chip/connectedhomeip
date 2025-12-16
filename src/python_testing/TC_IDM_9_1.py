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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.global_attribute_ids import GlobalAttributeIds, is_standard_attribute_id, is_standard_cluster_id
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.tlv import uint

log = logging.getLogger(__name__)


def checkable_attributes(cluster_id, cluster, xml_cluster) -> list[uint]:
    """Get list of attributes that exist on the DUT and have spec/codegen data available."""
    all_attrs = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]

    checkable_attrs = []
    for attr_id in all_attrs:
        if not is_standard_attribute_id(attr_id):
            continue

        if attr_id not in xml_cluster.attributes:
            continue

        if attr_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
            continue

        checkable_attrs.append(attr_id)

    return checkable_attrs


class TC_IDM_9_1(BasicCompositionTests):
    def _get_xml_path(self, cluster_id: int) -> Optional[str]:
        """Get the XML file path for a cluster."""
        xml_cluster = self.xml_clusters.get(cluster_id)
        if not xml_cluster:
            return None

        # Get spec version from data model
        dm = self._get_dm()
        spec_version = dm.dirname

        # Special mappings for non-standard XML filenames
        special_mappings = {
            "On/Off": "OnOff.xml",
            "Time Format Localization": "LocalizationTimeFormat.xml",
            "Unit Localization": "LocalizationUnit.xml",
            "Access Control": "ACL-Cluster.xml",
            "OTA Software Update Requestor": "OTARequestor.xml",
            "Pump Configuration and Control": "PumpConfigurationControl.xml",
            "Valve Configuration and Control": "ValveConfigurationControl.xml",
            "HEPA Filter Monitoring": "ResourceMonitoring.xml",
            "Activated Carbon Filter Monitoring": "ResourceMonitoring.xml",
        }

        # Build list of possible filenames
        if xml_cluster.name in special_mappings:
            possible_names = [special_mappings[xml_cluster.name]]
        else:
            cluster_name_no_spaces = xml_cluster.name.replace(" ", "")
            cluster_name_with_dashes = xml_cluster.name.replace(" ", "-")
            possible_names = [
                f"{cluster_name_no_spaces}Cluster.xml",
                f"{cluster_name_no_spaces}.xml",
                f"{cluster_name_with_dashes}-Cluster.xml",
                f"{cluster_name_no_spaces}-Cluster.xml",
            ]

        # Find XML file dynamically from script location
        script_dir = Path(__file__).parent
        repo_root = script_dir.parent.parent
        xml_dir = repo_root / 'data_model' / spec_version / 'clusters'

        for possible_name in possible_names:
            file_path = xml_dir / possible_name
            if file_path.exists():
                return str(file_path)

        return None

    def _parse_constraint_references(self, elem, constraint_dict: dict, prefix: str):
        """Parse dynamic constraint references (attribute/field references)."""
        attr_ref = elem.find('./attribute')
        if attr_ref is not None and 'name' in attr_ref.attrib:
            ref_attr_name = attr_ref.attrib['name']
            field_ref = attr_ref.find('./field')

            if field_ref is not None and 'name' in field_ref.attrib:
                constraint_dict[f'{prefix}AttributeRef'] = {
                    'attribute': ref_attr_name,
                    'field': field_ref.attrib['name']
                }
            else:
                constraint_dict[f'{prefix}AttributeRef'] = {'attribute': ref_attr_name}

    def _parse_attribute_constraints(self, cluster_id: int, attribute_id: int) -> dict:
        """Parse constraint information from XML for a specific attribute."""
        xml_path = self._get_xml_path(cluster_id)
        if not xml_path:
            return {}

        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()

            # Find the attribute element - try different hex formats
            attr_elem = None
            search_ids = [
                f"0x{attribute_id:04X}",
                f"0x{attribute_id:X}",
            ]

            for search_id in search_ids:
                attr_elem = root.find(f".//attribute[@id='{search_id}']")
                if attr_elem is not None:
                    break

            if not attr_elem:
                return {}

            # Parse constraint element
            constraint_elem = attr_elem.find('./constraint')
            if constraint_elem is None:
                return {}

            constraints = {}

            # Parse min/max/between constraints
            if constraint_elem.find('./min') is not None:
                min_elem = constraint_elem.find('./min')
                if 'value' in min_elem.attrib:
                    constraints['min'] = int(min_elem.attrib['value'])
                else:
                    self._parse_constraint_references(min_elem, constraints, 'min')

            if constraint_elem.find('./max') is not None:
                max_elem = constraint_elem.find('./max')
                if 'value' in max_elem.attrib:
                    constraints['max'] = int(max_elem.attrib['value'])
                else:
                    self._parse_constraint_references(max_elem, constraints, 'max')

            between_elem = constraint_elem.find('./between')
            if between_elem is not None:
                from_elem = between_elem.find('./from')
                to_elem = between_elem.find('./to')

                if from_elem is not None:
                    if 'value' in from_elem.attrib:
                        constraints['min'] = int(from_elem.attrib['value'])
                    else:
                        self._parse_constraint_references(from_elem, constraints, 'min')

                if to_elem is not None:
                    if 'value' in to_elem.attrib:
                        constraints['max'] = int(to_elem.attrib['value'])
                    else:
                        self._parse_constraint_references(to_elem, constraints, 'max')

            # Parse string length constraints
            for constraint_type in ['minLength', 'maxLength']:
                elem = constraint_elem.find(f'./{constraint_type}')
                if elem is not None and 'value' in elem.attrib:
                    constraints[constraint_type] = int(elem.attrib['value'])

            # Parse list count constraints
            for constraint_type, xml_tag in [('minCount', 'minCount'), ('maxCount', 'maxCount')]:
                elem = constraint_elem.find(f'./{xml_tag}')
                if elem is not None:
                    if 'value' in elem.attrib:
                        constraints[constraint_type] = int(elem.attrib['value'])
                    else:
                        self._parse_constraint_references(elem, constraints, constraint_type.replace('Count', ''))

            return constraints

        except Exception as e:
            log.warning(f"Exception parsing constraints for cluster 0x{cluster_id:04X} attr 0x{attribute_id:04X}: {e}")
            return {}

    async def _resolve_dynamic_constraint(self, cluster_class, endpoint_id: int, ref_dict: dict) -> Optional[int]:
        """Resolve a dynamic constraint reference by reading the attribute value."""
        ref_attr_name = ref_dict['attribute']
        ref_field_name = ref_dict.get('field')

        ref_attr = getattr(cluster_class.Attributes, ref_attr_name, None)
        if not ref_attr:
            return None

        ref_value = await self.read_single_attribute_check_success(
            endpoint=endpoint_id,
            cluster=cluster_class,
            attribute=ref_attr
        )

        if ref_field_name:
            python_field_name = ref_field_name[0].lower() + ref_field_name[1:]
            if hasattr(ref_value, python_field_name):
                return getattr(ref_value, python_field_name)
            return None

        return ref_value if isinstance(ref_value, (int, float)) else None

    def _generate_constraint_violation(self, attr_info: dict, constraints: dict):
        """Generate a test value that violates the given constraints."""
        datatype = attr_info['datatype']

        # String constraints
        if 'string' in datatype or 'octstr' in datatype:
            if 'maxLength' in constraints:
                return 'x' * (constraints['maxLength'] + 1)
            if 'minLength' in constraints:
                return 'x' * max(0, constraints['minLength'] - 1)

        # List constraints
        if 'list' in datatype:
            if 'maxCount' in constraints:
                return [{}] * (constraints['maxCount'] + 1)
            if 'minCount' in constraints:
                count = max(0, constraints['minCount'] - 1)
                return [{}] * count if count > 0 else []

        # Numeric-like constraints (int, uint, percent, elapsed-s, temperature, etc.)
        if 'max' in constraints:
            return constraints['max'] + 1
        if 'min' in constraints:
            return max(0, constraints['min'] - 1)

        return None

    async def _test_attribute_constraint(self, attr_info: dict, constraints: dict) -> bool:
        """Test a single attribute's constraint. Returns True if test passed, False otherwise."""
        # Resolve dynamic constraints if present
        if 'minAttributeRef' in constraints or 'maxAttributeRef' in constraints:
            cluster_class = attr_info['cluster_class']

            if 'minAttributeRef' in constraints:
                constraints['min'] = await self._resolve_dynamic_constraint(
                    cluster_class, attr_info['endpoint_id'], constraints['minAttributeRef']
                )

            if 'maxAttributeRef' in constraints:
                constraints['max'] = await self._resolve_dynamic_constraint(
                    cluster_class, attr_info['endpoint_id'], constraints['maxAttributeRef']
                )

        # Generate constraint violation
        test_value = self._generate_constraint_violation(attr_info, constraints)
        if test_value is None:
            return None  # Unsupported constraint type

        # Read original value
        original_value = await self.read_single_attribute_check_success(
            endpoint=attr_info['endpoint_id'],
            cluster=attr_info['cluster_class'],
            attribute=attr_info['attribute']
        )

        # Attempt to write violating value
        attr_obj = attr_info['attribute'](test_value)
        write_result = await self.default_controller.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(attr_info['endpoint_id'], attr_obj)]
        )
        result_status = write_result[0].Status

        if result_status == Status.ConstraintError:
            # Verify value wasn't set to the violating value
            new_value = await self.read_single_attribute_check_success(
                endpoint=attr_info['endpoint_id'],
                cluster=attr_info['cluster_class'],
                attribute=attr_info['attribute']
            )

            if new_value == test_value:
                log.error(f"FAIL: {attr_info['cluster_name']}.{attr_info['attribute_name']} "
                          f"was set to invalid value {test_value} despite CONSTRAINT_ERROR")
                return False

            log.info(f"PASS: {attr_info['cluster_name']}.{attr_info['attribute_name']} "
                     f"constraint properly enforced (original={original_value}, rejected={test_value})")
            return True

        log.error(f"FAIL: {attr_info['cluster_name']}.{attr_info['attribute_name']} "
                  f"got {result_status} instead of CONSTRAINT_ERROR for value {test_value}")
        return False

    def steps_TC_IDM_9_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "Send a command with a uint16 parameter value out of range in the Invoke Request message to the DUT from the TH. Example: Command - ColorControl.MoveToColorTemperature, field - ColorTemperatureMireds, Constraint Max Value - 65279, Test Value - 65280",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1b", "Set the data field of a command of data type octstr to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the octstr to a value that is larger than the constraint allowed. Example: Command - SignVIDVerificationRequest, data field - ClientChallenge, Constraint - 32 bytes exactly, Test Value - 33 bytes",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1c", "Set the data field of a command of data type octstr to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the octstr to a value that is smaller than the constraint allowed. Example: Command - SignVIDVerificationRequest, data field - ClientChallenge, Constraint - 32 bytes exactly, Test Value - 31 bytes",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1d", "Set the data field of a command of data type string to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the string to a value that is larger than the maximum length allowed. Example: Command - SetRegulatoryConfig, data field - CountryCode, Constraint - 2",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1e", "Set the data field of a command of data type string to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the string to a value that is smaller than the minimum length allowed. Example: Command - SetRegulatoryConfig, data field - CountryCode, Constraint - 2",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep(2, "Read every attribute that is writable with bounds from all the clusters from all the endpoints. For every writable attribute read, set the data field  to an out of bounds value in the Write Request message to the DUT from the TH.",
                     "Verify on the TH that the DUT sends a Status Response Action with a CONSTRAINT_ERROR Status Code."),
        ]

    @async_test_body
    async def test_TC_IDM_9_1(self):
        self.step(0)
        await self.setup_class_helper(allow_pase=False)
        self.build_spec_xmls()
        self.endpoint = MatterBaseTest.get_endpoint(self)

        # Step 1a: Test uint16 constraint validation using ColorControl clusters MoveToColorTemperature command
        self.step("1a")
        if await MatterBaseTest.command_guard(self, endpoint=self.endpoint, command=Clusters.ColorControl.Commands.MoveToColorTemperature):
            try:
                cmd = Clusters.ColorControl.Commands.MoveToColorTemperature(
                    colorTemperatureMireds=65280,  # Max value is 65279 for this uint16 attribute according to XML spec
                    transitionTime=0,
                    optionsMask=0,
                    optionsOverride=0
                )
                await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=self.endpoint, payload=cmd)
                asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR, but got {e.status}")

        # Step 1b: Test octstr max length constraint violation using OperationalCredentials clusters SignVIDVerificationRequest command
        self.step("1b")
        if await MatterBaseTest.command_guard(self, endpoint=self.endpoint, command=Clusters.OperationalCredentials.Commands.SignVIDVerificationRequest):
            # ClientChallenge is octstr with constraint = 32 (must be 32 bytes)
            try:
                cmd = Clusters.OperationalCredentials.Commands.SignVIDVerificationRequest(
                    fabricIndex=1,
                    clientChallenge=b'x' * 33
                )
                await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=self.endpoint, payload=cmd)
                asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError,
                                    f"Expected CONSTRAINT_ERROR, but got {e.status}")

        # Step 1c: Test octstr min length constraint violation using OperationalCredentials clusters SignVIDVerificationRequest command
        self.step("1c")
        if await MatterBaseTest.command_guard(self, endpoint=self.endpoint, command=Clusters.OperationalCredentials.Commands.SignVIDVerificationRequest):
            # ClientChallenge is octstr with constraint = 32 (must be 32 bytes)
            try:
                cmd = Clusters.OperationalCredentials.Commands.SignVIDVerificationRequest(
                    fabricIndex=1,
                    clientChallenge=b'x' * 31
                )
                await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=self.endpoint, payload=cmd)
                asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError,
                                    f"Expected CONSTRAINT_ERROR, but got {e.status}")

        # Step 1d: Test string max length constraint violation using GeneralCommissioning clusters SetRegulatoryConfig command
        self.step("1d")
        try:
            # CountryCode field is string with constraint length=2
            cmd = Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
                newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor,
                countryCode="USA",  # Out of range (must be 2 chars)
                breadcrumb=0
            )
            await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR, got {e.status}")

        # Step 1e: Test string min length constraint violation
        self.step("1e")
        try:
            # CountryCode field is string with constraint length=2
            cmd = Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
                newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor,
                countryCode="U",  # Out of range (must be 2 chars)
                breadcrumb=0
            )
            await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR, got {e.status}")

        # Step 2: Test all writable attributes with constraints
        self.step(2)
        log.info("Testing writable attributes for constraint errors")

        # Collect writable attributes from DUT
        writable_attributes = []
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, device_cluster_data in endpoint.items():
                if not is_standard_cluster_id(cluster_id):
                    continue
                if cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                    continue

                xml_cluster = self.xml_clusters[cluster_id]
                cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

                for attribute_id in checkable_attributes(cluster_id, device_cluster_data, xml_cluster):
                    xml_attr = xml_cluster.attributes[attribute_id]

                    if xml_attr.write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                        writable_attributes.append({
                            'endpoint_id': endpoint_id,
                            'cluster_id': cluster_id,
                            'cluster_name': xml_cluster.name,
                            'attribute_id': attribute_id,
                            'attribute_name': xml_attr.name,
                            'attribute': Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id],
                            'cluster_class': cluster_class,
                            'datatype': xml_attr.datatype
                        })

        log.info(f"Found {len(writable_attributes)} writable attributes on DUT")

        # Test attributes with constraints
        tested_count = 0
        skipped_count = 0
        failed_attributes = []

        for attr_info in writable_attributes:
            constraints = self._parse_attribute_constraints(attr_info['cluster_id'], attr_info['attribute_id'])

            if not constraints:
                skipped_count += 1
                continue

            try:
                result = await self._test_attribute_constraint(attr_info, constraints)
                if result is None:
                    skipped_count += 1
                elif result is False:
                    failed_attributes.append(f"{attr_info['cluster_name']}.{attr_info['attribute_name']}")
                    tested_count += 1
                else:
                    tested_count += 1
            except Exception as e:
                log.warning(f"Exception testing {attr_info['cluster_name']}.{attr_info['attribute_name']}: {e}")
                skipped_count += 1

        log.info(f"Step 2 complete: Tested {tested_count} attributes, skipped {skipped_count}")

        if failed_attributes:
            log.error(f"Failed attributes constraints not enforced: {', '.join(failed_attributes)}")
            asserts.fail("Failed attributes constraints not enforced")


if __name__ == "__main__":
    default_matter_test_main()
