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

import unittest

from data_model_errata import AccessControlEntryPrivilegeEnum, apply_errata, load_authoritative_errata
from spec_parsing import XmlAttribute, XmlCluster, XmlCommand

from matter.tlv import uint


class TestDataModelErrata(unittest.TestCase):

    def _create_dummy_cluster(self, name: str) -> XmlCluster:
        return XmlCluster(
            name=name,
            revision=1,
            derived=None,
            feature_map={},
            attribute_map={},
            command_map={},
            features={},
            attributes={},
            accepted_commands={},
            generated_commands={},
            unknown_commands=[],
            events={},
            structs={},
            enums={},
            bitmaps={},
            pics="",
            is_provisional=False,
            revision_desc={},
        )

    def _create_dummy_attribute(self, name: str) -> XmlAttribute:
        return XmlAttribute(
            name=name,
            datatype="uint8",
            conformance=None,  # type: ignore[arg-type]
            read_access=AccessControlEntryPrivilegeEnum.kOperate,
            write_access=AccessControlEntryPrivilegeEnum.kOperate,
            write_optional=False,
        )

    def _create_dummy_command(self, id_val: int, name: str) -> XmlCommand:
        return XmlCommand(
            id=id_val,
            name=name,
            conformance=None,  # type: ignore[arg-type]
            privilege=AccessControlEntryPrivilegeEnum.kAdminister,
        )

    def test_attribute_access_override(self):
        cluster = self._create_dummy_cluster("AmbientSensing")
        attr = self._create_dummy_attribute("LightLevel")

        attr_id = uint(1)
        cluster.attribute_map["LightLevel"] = attr_id
        cluster.attributes[attr_id] = attr

        clusters = {uint(10): cluster}

        errata = {
            "AmbientSensing": {
                "LightLevel": {
                    "read_access": "RV",
                    "write_access": "none",
                }
            }
        }

        problems = apply_errata(clusters, errata)

        self.assertEqual(len(problems), 0, f"Unexpected problems: {problems}")
        self.assertEqual(attr.read_access, AccessControlEntryPrivilegeEnum.kView)
        self.assertEqual(attr.write_access, AccessControlEntryPrivilegeEnum.kUnknownEnumValue)

    def test_command_privilege_override(self):
        cluster = self._create_dummy_cluster("OnOff")
        cmd = self._create_dummy_command(1, "On")

        cmd_id = uint(1)
        cluster.command_map["On"] = cmd_id
        cluster.accepted_commands[cmd_id] = cmd

        clusters = {uint(11): cluster}

        errata = {
            "OnOff": {
                "On": {
                    "invoke_access": "manage",
                }
            }
        }

        problems = apply_errata(clusters, errata)

        self.assertEqual(len(problems), 0, f"Unexpected problems: {problems}")
        self.assertEqual(cmd.privilege, AccessControlEntryPrivilegeEnum.kManage)

    def test_error_unknown_cluster(self):
        clusters: dict[uint, XmlCluster] = {}
        errata = {
            "UnknownClusterName": {
                "SomeAttr": {
                    "read_access": "RV"
                }
            }
        }

        problems = apply_errata(clusters, errata)
        self.assertEqual(len(problems), 1)
        self.assertIn("Errata referenced unknown Cluster", problems[0].problem)

    def test_error_unknown_element(self):
        cluster = self._create_dummy_cluster("BasicInformation")
        clusters = {uint(1): cluster}

        errata = {
            "BasicInformation": {
                "NonExistentAttribute": {
                    "read_access": "RV"
                }
            }
        }

        problems = apply_errata(clusters, errata)
        self.assertEqual(len(problems), 1)
        self.assertIn("Errata referenced unknown Element", problems[0].problem)

    def test_revision_compatibility_pass(self):
        cluster = self._create_dummy_cluster("AmbientSensing")
        clusters = {uint(1): cluster}
        errata = {
            "compatible_specification_revisions": ["1.6"],
            "AmbientSensing": {}
        }
        problems = apply_errata(clusters, errata, active_spec_revision="1.6.0")
        self.assertEqual(len(problems), 0)

    def test_revision_compatibility_fail(self):
        cluster = self._create_dummy_cluster("AmbientSensing")
        clusters = {uint(1): cluster}
        errata = {
            "compatible_specification_revisions": ["1.6"],
            "AmbientSensing": {}
        }
        problems = apply_errata(clusters, errata, active_spec_revision="1.5")
        self.assertEqual(len(problems), 1)
        self.assertIn("CRITICAL: Errata overlay is not compatible", problems[0].problem)

    def test_revision_compatibility_missing_active_rev(self):
        cluster = self._create_dummy_cluster("AmbientSensing")
        clusters = {uint(1): cluster}
        errata = {
            "compatible_specification_revisions": ["1.6"],
            "AmbientSensing": {}
        }
        problems = apply_errata(clusters, errata, active_spec_revision=None)
        self.assertEqual(len(problems), 1)
        self.assertIn("CRITICAL: Errata overlay mandates a compatible specification check", problems[0].problem)

    def test_load_authoritative_errata_missing_file(self):
        errata = load_authoritative_errata("non_existent_errata_file_99.yaml")
        self.assertEqual(errata, {})

    def test_reject_raw_xml_cluster_name(self):
        cluster = self._create_dummy_cluster("AmbientContextSensing")
        clusters = {uint(1): cluster}
        errata = {
            "Ambient Context Sensing": {}
        }
        problems = apply_errata(clusters, errata)
        self.assertEqual(len(problems), 1)
        self.assertIn("CRITICAL: Cluster name 'Ambient Context Sensing' in errata violates", problems[0].problem)

    def test_reject_raw_xml_element_name(self):
        cluster = self._create_dummy_cluster("AmbientContextSensing")
        clusters = {uint(1): cluster}
        errata = {
            "AmbientContextSensing": {
                "Simultaneous Detection Limit": {
                    "read_access": "RV"
                }
            }
        }
        problems = apply_errata(clusters, errata)
        self.assertEqual(len(problems), 1)
        self.assertIn("CRITICAL: Element name 'Simultaneous Detection Limit' in 'AmbientContextSensing' violates",
                      problems[0].problem)


if __name__ == "__main__":
    unittest.main()
