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

import xml.etree.ElementTree as ElementTree
import zipfile
from importlib.abc import Traversable

from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from chip.testing.problem_notices import NamespacePathLocation, ProblemNotice, ProblemSeverity
from chip.testing.spec_parsing import (DataModelLevel, PrebuiltDataModelDirectory, build_xml_namespaces, get_data_model_directory,
                                       parse_namespace)
from jinja2 import Template
from mobly import asserts


class TestSpecParsingNamespace(MatterBaseTest):
    def setup_class(self):
        # Test data setup
        self.namespace_id = 0x0001
        self.namespace_name = "Test Namespace"
        self.tags = {
            0x0000: "Tag1",
            0x0001: "Tag2",
            0x0002: "Tag3"
        }

        # Template for generating test XML
        self.template = Template("""<?xml version="1.0"?>
        <namespace xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                  xsi:schemaLocation="types types.xsd namespace namespace.xsd"
                  id="{{ namespace_id }}"
                  name="{{ namespace_name }}">
            <tags>
                {% for id, name in tags.items() %}
                <tag id="{{ "0x%04X" % id }}" name="{{ name }}"/>
                {% endfor %}
            </tags>
        </namespace>""")

    def test_namespace_parsing(self):
        """Test basic namespace parsing with valid data"""
        xml = self.template.render(
            namespace_id=f"0x{self.namespace_id:04X}",
            namespace_name=self.namespace_name,
            tags=self.tags
        )
        et = ElementTree.fromstring(xml)
        namespace, problems = parse_namespace(et)

        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing namespace")
        asserts.assert_equal(namespace.id, self.namespace_id, "Incorrect namespace ID")
        asserts.assert_equal(namespace.name, self.namespace_name, "Incorrect namespace name")
        asserts.assert_equal(len(namespace.tags), len(self.tags), "Incorrect number of tags")

        for tag_id, tag_name in self.tags.items():
            asserts.assert_true(tag_id in namespace.tags, f"Tag ID 0x{tag_id:04X} not found")
            asserts.assert_equal(namespace.tags[tag_id].name, tag_name, f"Incorrect name for tag 0x{tag_id:04X}")

    def test_bad_namespace_id(self):
        """Test parsing with invalid namespace ID"""
        xml = self.template.render(
            namespace_id="",
            namespace_name=self.namespace_name,
            tags=self.tags
        )
        et = ElementTree.fromstring(xml)
        namespace, problems = parse_namespace(et)
        asserts.assert_equal(len(problems), 1, "Namespace with blank ID did not generate a problem notice")

    def test_missing_namespace_name(self):
        """Test parsing with missing namespace name"""
        xml = self.template.render(
            namespace_id=f"0x{self.namespace_id:04X}",
            namespace_name="",
            tags=self.tags
        )
        et = ElementTree.fromstring(xml)
        namespace, problems = parse_namespace(et)
        asserts.assert_equal(len(problems), 1, "Namespace with no name did not generate a problem notice")

    def test_no_tags(self):
        """Test parsing with no tags"""
        xml = self.template.render(
            namespace_id=f"0x{self.namespace_id:04X}",
            namespace_name=self.namespace_name,
            tags={}
        )
        et = ElementTree.fromstring(xml)
        namespace, problems = parse_namespace(et)
        asserts.assert_equal(len(problems), 0, "Unexpected problems parsing empty namespace")
        asserts.assert_equal(len(namespace.tags), 0, "Empty namespace should have no tags")

    def test_spec_files(self):
        """Test parsing actual spec files from different versions"""
        one_three, _ = build_xml_namespaces(PrebuiltDataModelDirectory.k1_3)
        one_four, one_four_problems = build_xml_namespaces(PrebuiltDataModelDirectory.k1_4)
        one_four_one, one_four_one_problems = build_xml_namespaces(PrebuiltDataModelDirectory.k1_4_1)
        one_four_two, one_four_two_problems = build_xml_namespaces(PrebuiltDataModelDirectory.k1_4_2)

        asserts.assert_equal(len(one_four_problems), 0, "Problems found when parsing 1.4 spec")
        asserts.assert_equal(len(one_four_one_problems), 0, "Problems found when parsing 1.4.1 spec")
        asserts.assert_equal(len(one_four_two_problems), 0, "Problems found when parsing 1.4.2 spec")

        # Check version relationships
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_three.keys())),
                               0, "1.4.2 dir contains less namespaces than 1.3")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_four.keys())),
                               0, "1.4.2 dir contains less namespaces than 1.4")
        asserts.assert_greater(len(set(one_four_two.keys()) - set(one_four_one.keys())),
                               0, "1.4.2 dir contains less namespaces than 1.4.1")
        asserts.assert_greater(len(set(one_four.keys()) - set(one_three.keys())),
                               0, "1.4 dir contains less namespaces than 1.3")

        # Complete namespace version checks for 1.3, 1.4, 1.4.1, 1.4.2, known differences and relationships:
        # 1.3: has Common Position and Common Closure
        # 1.4/1.4.1: removed Common Position, kept Common Closure, added Common Area/Landmark/Relative Position
        # 1.4.2: removed Common Closure, added back Common Position, kept new ones from 1.4/1.4.1

        # Check changes from 1.3 to 1.4
        removed_1_3_to_1_4 = set(one_three.keys()) - set(one_four.keys())
        removed_names_1_3_to_1_4 = {one_three[id].name for id in removed_1_3_to_1_4}
        expected_removed_1_3_to_1_4 = {"Common Position"}
        asserts.assert_equal(removed_names_1_3_to_1_4, expected_removed_1_3_to_1_4,
                             f"Expected only 'Common Position' to be removed from 1.3 to 1.4, but got: {removed_names_1_3_to_1_4}")

        added_1_3_to_1_4 = set(one_four.keys()) - set(one_three.keys())
        added_names_1_3_to_1_4 = {one_four[id].name for id in added_1_3_to_1_4}
        expected_added_1_3_to_1_4 = {"Common Area", "Common Landmark", "Common Relative Position"}
        asserts.assert_equal(
            added_names_1_3_to_1_4,
            expected_added_1_3_to_1_4,
            f"Expected 'Common Area', 'Common Landmark', 'Common Relative Position' to be added from 1.3 to 1.4, but got: {added_names_1_3_to_1_4}")

        # Check 1.4 to 1.4.1 (should be identical)
        diff_1_4_to_1_4_1 = set(one_four.keys()) ^ set(one_four_one.keys())
        asserts.assert_equal(len(diff_1_4_to_1_4_1), 0, "1.4 and 1.4.1 should have identical namespaces")

        # Comprehensive checks: Compare 1.4.2 (latest) against all previous versions
        # 1.4.2 vs 1.3 comparison
        removed_1_4_2_vs_1_3 = set(one_three.keys()) - set(one_four_two.keys())
        removed_names_1_4_2_vs_1_3 = {one_three[id].name for id in removed_1_4_2_vs_1_3}
        expected_removed_1_4_2_vs_1_3 = {"Common Closure"}
        asserts.assert_equal(
            removed_names_1_4_2_vs_1_3,
            expected_removed_1_4_2_vs_1_3,
            f"Expected only 'Common Closure' to be removed from 1.3 to 1.4.2, but got: {removed_names_1_4_2_vs_1_3}")

        added_1_4_2_vs_1_3 = set(one_four_two.keys()) - set(one_three.keys())
        added_names_1_4_2_vs_1_3 = {one_four_two[id].name for id in added_1_4_2_vs_1_3}
        expected_added_1_4_2_vs_1_3 = {"Common Area", "Common Landmark", "Common Relative Position"}
        asserts.assert_equal(
            added_names_1_4_2_vs_1_3,
            expected_added_1_4_2_vs_1_3,
            f"Expected 'Common Area', 'Common Landmark', 'Common Relative Position' to be added from 1.3 to 1.4.2, but got: {added_names_1_4_2_vs_1_3}")

        # 1.4.2 vs 1.4 comparison
        removed_1_4_2_vs_1_4 = set(one_four.keys()) - set(one_four_two.keys())
        removed_names_1_4_2_vs_1_4 = {one_four[id].name for id in removed_1_4_2_vs_1_4}
        expected_removed_1_4_2_vs_1_4 = {"Common Closure"}
        asserts.assert_equal(
            removed_names_1_4_2_vs_1_4,
            expected_removed_1_4_2_vs_1_4,
            f"Expected only 'Common Closure' to be removed from 1.4 to 1.4.2, but got: {removed_names_1_4_2_vs_1_4}")

        added_1_4_2_vs_1_4 = set(one_four_two.keys()) - set(one_four.keys())
        added_names_1_4_2_vs_1_4 = {one_four_two[id].name for id in added_1_4_2_vs_1_4}
        expected_added_1_4_2_vs_1_4 = {"Common Position"}
        asserts.assert_equal(added_names_1_4_2_vs_1_4, expected_added_1_4_2_vs_1_4,
                             f"Expected only 'Common Position' to be added from 1.4 to 1.4.2, but got: {added_names_1_4_2_vs_1_4}")

        # Check changes from 1.4.1 to 1.4.2
        removed_1_4_1_to_1_4_2 = set(one_four_one.keys()) - set(one_four_two.keys())
        removed_names_1_4_1_to_1_4_2 = {one_four_one[id].name for id in removed_1_4_1_to_1_4_2}
        expected_removed_1_4_1_to_1_4_2 = {"Common Closure"}
        asserts.assert_equal(
            removed_names_1_4_1_to_1_4_2,
            expected_removed_1_4_1_to_1_4_2,
            f"Expected only 'Common Closure' to be removed from 1.4.1 to 1.4.2, but got: {removed_names_1_4_1_to_1_4_2}")

        added_1_4_2_vs_1_4_1 = set(one_four_two.keys()) - set(one_four_one.keys())
        added_names_1_4_2_vs_1_4_1 = {one_four_two[id].name for id in added_1_4_2_vs_1_4_1}
        expected_added_1_4_2_vs_1_4_1 = {"Common Position"}
        asserts.assert_equal(
            added_names_1_4_2_vs_1_4_1,
            expected_added_1_4_2_vs_1_4_1,
            f"Expected only 'Common Position' to be added from 1.4.1 to 1.4.2, but got: {added_names_1_4_2_vs_1_4_1}")

    def validate_namespace_xml(self, xml_file: Traversable) -> list[ProblemNotice]:
        """Validate namespace XML file"""
        problems: list[ProblemNotice] = []
        try:
            with xml_file.open('r', encoding="utf8") as f:
                root = ElementTree.parse(f).getroot()

                # Check for namespace ID and validate format
                namespace_id = root.get('id')
                if not namespace_id:
                    problems.append(ProblemNotice(
                        test_name="Validate Namespace XML",
                        location=NamespacePathLocation(),
                        severity=ProblemSeverity.WARNING,
                        problem=f"Missing namespace ID in {xml_file.name}"
                    ))
                else:
                    # Validate 16-bit hex format (0xNNNN)
                    try:
                        # Remove '0x' prefix if present and try to parse
                        id_value = int(namespace_id.replace('0x', ''), 16)
                        if id_value < 0 or id_value > 0xFFFF:
                            problems.append(ProblemNotice(
                                test_name="Validate Namespace XML",
                                location=NamespacePathLocation(),
                                severity=ProblemSeverity.WARNING,
                                problem=f"Namespace ID {namespace_id} is not a valid 16-bit value in {xml_file.name}"
                            ))
                        # Check format is exactly 0xNNNN where N is hex digit
                        if not namespace_id.lower().startswith('0x') or len(namespace_id) != 6:
                            problems.append(ProblemNotice(
                                test_name="Validate Namespace XML",
                                location=NamespacePathLocation(),
                                severity=ProblemSeverity.WARNING,
                                problem=f"Namespace ID {namespace_id} does not follow required format '0xNNNN' in {xml_file.name}"
                            ))
                    except ValueError:
                        problems.append(ProblemNotice(
                            test_name="Validate Namespace XML",
                            location=NamespacePathLocation(),
                            severity=ProblemSeverity.WARNING,
                            problem=f"Invalid hex format for namespace ID {namespace_id} in {xml_file.name}"
                        ))

            # Check for namespace name
            namespace_name = root.get('name', '').strip()
            if not namespace_name:
                problems.append(ProblemNotice(
                    test_name="Validate Namespace XML",
                    location=NamespacePathLocation(),
                    severity=ProblemSeverity.WARNING,
                    problem=f"Missing or empty namespace name in {xml_file.name}"
                ))

            # Check tags structure
            tags_elem = root.find('tags')
            if tags_elem is not None:
                for tag in tags_elem.findall('tag'):
                    # Check tag ID and validate format
                    tag_id = tag.get('id')
                    if not tag_id:
                        problems.append(ProblemNotice(
                            test_name="Validate Namespace XML",
                            location=NamespacePathLocation(),
                            severity=ProblemSeverity.WARNING,
                            problem=f"Missing tag ID in {xml_file.name}"
                        ))
                    else:
                        try:
                            # Remove '0x' prefix if present and try to parse
                            id_value = int(tag_id.replace('0x', ''), 0)
                            if id_value < 0 or id_value > 0xFFFF:
                                problems.append(ProblemNotice(
                                    test_name="Validate Namespace XML",
                                    location=NamespacePathLocation(),
                                    severity=ProblemSeverity.WARNING,
                                    problem=f"Tag ID {tag_id} is not a valid 16-bit value in {xml_file.name}"
                                ))
                            # Check format is exactly 0xNNNN where N is hex digit
                            if not tag_id.lower().startswith('0x') or len(tag_id) != 6:
                                problems.append(ProblemNotice(
                                    test_name="Validate Namespace XML",
                                    location=NamespacePathLocation(),
                                    severity=ProblemSeverity.WARNING,
                                    problem=f"Tag ID {tag_id} does not follow required format '0xNNNN' in {xml_file.name}"
                                ))
                        except ValueError:
                            problems.append(ProblemNotice(
                                test_name="Validate Namespace XML",
                                location=NamespacePathLocation(),
                                severity=ProblemSeverity.WARNING,
                                problem=f"Invalid hex format for tag ID {tag_id} in {xml_file.name}"
                            ))

                    # Check tag name
                    tag_name = tag.get('name', '').strip()
                    if not tag_name:
                        problems.append(ProblemNotice(
                            test_name="Validate Namespace XML",
                            location=NamespacePathLocation(),
                            severity=ProblemSeverity.WARNING,
                            problem=f"Missing or empty tag name in {xml_file.name}"
                        ))

        except Exception as e:
            problems.append(ProblemNotice(
                test_name="Validate Namespace XML",
                location=NamespacePathLocation(),
                severity=ProblemSeverity.WARNING,
                problem=f"Failed to parse {xml_file.name}: {str(e)}"
            ))

        return problems

    def test_all_namespace_files(self):
        """Test all namespace XML files in the data model namespaces directories"""
        data_model_versions = {
            "1.4": PrebuiltDataModelDirectory.k1_4,
            "1.4.1": PrebuiltDataModelDirectory.k1_4_1,
            "1.4.2": PrebuiltDataModelDirectory.k1_4_2,
        }

        for version, dm_path in data_model_versions.items():
            try:
                # First get the namespaces
                namespaces, problems = build_xml_namespaces(dm_path)
                if problems:
                    for problem in problems:
                        print(f"  - {problem}")

                # Get the directory for validation
                top = get_data_model_directory(dm_path, DataModelLevel.kNamespace)

                # Handle both zip files and directories
                if isinstance(top, zipfile.Path):
                    files = [f for f in top.iterdir() if str(f).endswith('.xml')]
                else:
                    files = [f for f in top.iterdir() if f.name.endswith('.xml')]

                # Validate each XML file
                for file in files:
                    validation_problems = self.validate_namespace_xml(file)
                    if validation_problems:
                        for problem in validation_problems:
                            asserts.assert_false(
                                problem.severity == ProblemSeverity.ERROR,
                                f"Error in {file.name}: {problem}"
                            )

                    # Parse and verify the namespace
                    with file.open('r', encoding="utf8") as xml:
                        root = ElementTree.parse(xml).getroot()
                        namespace, parse_problems = parse_namespace(root)

                        # Basic attribute verification
                        asserts.assert_true(hasattr(namespace, 'id'),
                                            f"Namespace in {file.name} missing ID")
                        asserts.assert_true(hasattr(namespace, 'name'),
                                            f"Namespace in {file.name} missing name")
                        asserts.assert_true(hasattr(namespace, 'tags'),
                                            f"Namespace in {file.name} missing tags dictionary")

                        # Verify each tag
                        for tag_id, tag in namespace.tags.items():
                            asserts.assert_true(hasattr(tag, 'id'),
                                                f"Tag in {file.name} missing ID")
                            asserts.assert_true(hasattr(tag, 'name'),
                                                f"Tag in {file.name} missing name")

            except Exception as e:
                print(f"Error processing {version}: {str(e)}")
                raise


if __name__ == "__main__":
    default_matter_test_main()
