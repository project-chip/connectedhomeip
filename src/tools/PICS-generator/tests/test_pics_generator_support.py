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

"""Tests for the PICS generator support helpers.

These need nothing beyond the standard library:

    python3 src/tools/PICS-generator/tests/test_pics_generator_support.py
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from pics_generator_support import (CaseMismatchedPicsItem, format_case_mismatch_warning, is_case_only_mismatch,  # noqa: E402
                                    map_cluster_name_to_pics_xml, normalize_pics_item_number)

# Sorted like pics_xml_file_list_loader returns it. The first entry is what an
# empty prefix would match, and several names share a prefix on purpose.
PICS_XML_FILE_LIST = sorted([
    "AV Analytics Control Cluster Test Plan.xml",
    "Access Control Cluster Test Plan.xml",
    "Access Control Enforcement Test Plan.xml",
    "Humidistat Cluster Test Plan.xml",
    "Media Cluster Test Plan.xml",
])


class TestMapClusterNameToPicsXml(unittest.TestCase):

    def test_cluster_without_template_returns_empty(self):
        for cluster_name in ("Temperature Alarm", "Water Tank Level Monitoring"):
            with self.subTest(cluster_name=cluster_name):
                self.assertEqual(map_cluster_name_to_pics_xml(cluster_name, PICS_XML_FILE_LIST), "")

    def test_cluster_name_matches_template_prefix(self):
        self.assertEqual(map_cluster_name_to_pics_xml("Humidistat", PICS_XML_FILE_LIST),
                         "Humidistat Cluster Test Plan.xml")

    def test_mapped_names(self):
        expected = {
            "Application Launcher": "Media Cluster Test Plan.xml",
            "AV Analysis": "AV Analytics Control Cluster Test Plan.xml",
            "Access Control": "Access Control Cluster Test Plan.xml",
        }
        for cluster_name, file_name in expected.items():
            with self.subTest(cluster_name=cluster_name):
                self.assertEqual(map_cluster_name_to_pics_xml(cluster_name, PICS_XML_FILE_LIST), file_name)

    def test_access_control_does_not_depend_on_sort_order(self):
        # "Access Control" is a prefix of both Access Control templates.
        reversed_list = list(reversed(PICS_XML_FILE_LIST))
        self.assertEqual(map_cluster_name_to_pics_xml("Access Control", reversed_list),
                         "Access Control Cluster Test Plan.xml")


class TestNormalizePicsItemNumber(unittest.TestCase):

    def test_hex_case_is_ignored(self):
        # Same bundle, both styles: BRBINFO uses lowercase, HSTAT uses uppercase.
        self.assertEqual(normalize_pics_item_number("HSTAT.S.A000A"),
                         normalize_pics_item_number("HSTAT.S.A000a"))
        self.assertEqual(normalize_pics_item_number("BRBINFO.S.A000a"),
                         normalize_pics_item_number("BRBINFO.S.A000A"))

    def test_generated_codes_match_uppercase_template_ids(self):
        # Built the same way DeviceMapping builds them (lowercase hex).
        generated = {
            normalize_pics_item_number(f"HSTAT.S.A{0x000A:04x}"),
            normalize_pics_item_number(f"AUDIOCONTROL.S.A{0x000B:04x}"),
            normalize_pics_item_number(f"ACS.S.A{0x000C:04x}"),
            normalize_pics_item_number(f"TSTAT.S.A{0x005A:04x}"),
        }
        for template_id in ("HSTAT.S.A000A", "AUDIOCONTROL.S.A000B", "ACS.S.A000C", "TSTAT.S.A005A"):
            with self.subTest(template_id=template_id):
                self.assertIn(normalize_pics_item_number(template_id), generated)

    def test_different_ids_still_differ(self):
        self.assertNotEqual(normalize_pics_item_number("HSTAT.S.A000A"),
                            normalize_pics_item_number("HSTAT.S.A000B"))
        self.assertNotEqual(normalize_pics_item_number("HSTAT.S"),
                            normalize_pics_item_number("HSTAT.C"))

    def test_missing_item_number_matches_nothing(self):
        self.assertEqual(normalize_pics_item_number(None), "")
        self.assertNotIn(normalize_pics_item_number(None),
                         {normalize_pics_item_number("HSTAT.S.A0000")})


class TestIsCaseOnlyMismatch(unittest.TestCase):

    def test_uppercase_template_id_is_flagged(self):
        # TC-IDM-10.4 looks up the lowercase-hex code, so these fail it even when marked.
        cases = {
            "HSTAT.S.A000A": f"HSTAT.S.A{0x000A:04x}",
            "TSTAT.S.A005D": f"TSTAT.S.A{0x005D:04x}",
            "OO.S.C0A.Rsp": f"OO.S.C{0x0A:02x}.Rsp",
            "OO.S.F0A": f"OO.S.F{10:02x}",
            "ACL.S.E0A": f"ACL.S.E{0x0A:02x}",
        }
        for item_number, pics_code in cases.items():
            with self.subTest(item_number=item_number):
                self.assertTrue(is_case_only_mismatch(item_number, pics_code))

    def test_exact_match_is_not_flagged(self):
        for code in ("HSTAT.S", "HSTAT.S.A0000", "BRBINFO.S.A000a", "HSTAT.S.C00.Rsp"):
            with self.subTest(code=code):
                self.assertFalse(is_case_only_mismatch(code, code))

    def test_different_ids_are_not_flagged(self):
        self.assertFalse(is_case_only_mismatch("HSTAT.S.A000B", "HSTAT.S.A000a"))
        # Decimal feature numbers are wrong for another reason, not case.
        self.assertFalse(is_case_only_mismatch("RVCRUNM.S.F20", "RVCRUNM.S.F14"))


class TestFormatCaseMismatchWarning(unittest.TestCase):

    def test_lists_every_item_under_its_file(self):
        items = [
            CaseMismatchedPicsItem("endpoint1/Humidistat Cluster Test Plan.xml", "HSTAT.S.A000A", "HSTAT.S.A000a"),
            CaseMismatchedPicsItem("endpoint1/Humidistat Cluster Test Plan.xml", "HSTAT.S.A000B", "HSTAT.S.A000b"),
            CaseMismatchedPicsItem("endpoint1/Thermostat Cluster Test Plan.xml", "TSTAT.S.A005B", "TSTAT.S.A005b"),
        ]
        lines = format_case_mismatch_warning(items).splitlines()
        self.assertEqual(lines[0], "3 supported PICS item(s) will fail TC-IDM-10.4.")
        self.assertEqual(lines.count("endpoint1/Humidistat Cluster Test Plan.xml"), 1)
        humidistat = lines.index("endpoint1/Humidistat Cluster Test Plan.xml")
        self.assertEqual(lines[humidistat + 1:humidistat + 3], [
            "  HSTAT.S.A000A  (TC-IDM-10.4 expects HSTAT.S.A000a)",
            "  HSTAT.S.A000B  (TC-IDM-10.4 expects HSTAT.S.A000b)",
        ])
        thermostat = lines.index("endpoint1/Thermostat Cluster Test Plan.xml")
        self.assertEqual(lines[thermostat + 1], "  TSTAT.S.A005B  (TC-IDM-10.4 expects TSTAT.S.A005b)")


if __name__ == "__main__":
    unittest.main()
