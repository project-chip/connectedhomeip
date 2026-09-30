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
from pics_generator_support import map_cluster_name_to_pics_xml, normalize_pics_item_number  # noqa: E402

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


if __name__ == "__main__":
    unittest.main()
