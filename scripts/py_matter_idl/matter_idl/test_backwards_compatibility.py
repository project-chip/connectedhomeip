#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
from enum import Flag, auto

try:
    from matter_idl.matter_idl_parser import CreateParser
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.matter_idl_parser import CreateParser

from matter_idl.backwards_compatibility import IsBackwardsCompatible
from matter_idl.matter_idl_types import Idl


class Compatibility(Flag):
    FORWARD_FAIL = auto()  # old -> new is wrong
    BACKWARD_FAIL = auto()  # new -> old is wrong


class TestCompatibilityChecks(unittest.TestCase):

    def _AssumeCompatiblity(self, old: str, new: str, old_idl: Idl, new_idl: Idl, expect_compatible: bool):
        if expect_compatible == IsBackwardsCompatible(old_idl, new_idl):
            return

        if expect_compatible:
            reason = "EXPECTED COMPATIBLE, but failed"
        else:
            reason = "EXPECTED NOT COMPATIBLE, but succeeded"

        self.fail(f"""Failed compatibility test
{reason}:
--------------------- OLD -----------------
{old}
--------------------- NEW -----------------
{new}
-------------------------------------------""")

    def ValidateUpdate(self, name: str, old: str, new: str, flags: Compatibility):
        old_idl = CreateParser(skip_meta=True).parse(old)
        new_idl = CreateParser(skip_meta=True).parse(new)
        with self.subTest(validate=name):
            # Validate compatibility and that no changes are always compatible
            with self.subTest(direction="Forward"):
                self._AssumeCompatiblity(old, new, old_idl, new_idl, Compatibility.FORWARD_FAIL not in flags)

            with self.subTest(direction="Backward"):
                self._AssumeCompatiblity(new, old, new_idl, old_idl, Compatibility.BACKWARD_FAIL not in flags)

            with self.subTest(direction="NEW-to-NEW"):
                self._AssumeCompatiblity(new, new, new_idl, new_idl, True)

            with self.subTest(direction="OLD-to-OLD"):
                self._AssumeCompatiblity(old, old, old_idl, old_idl, True)

    def test_top_level_enums(self):
        self.ValidateUpdate(
            "delete a top level enum",
            "enum A: ENUM8{} enum B: ENUM8{}",
            "enum A: ENUM8{}",
            Compatibility.FORWARD_FAIL)

        self.ValidateUpdate(
            "change an enum type",
            "enum A: ENUM8{}",
            "enum A: ENUM16{}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

        self.ValidateUpdate(
            "Adding enum values is ok, removing values is not",
            "enum A: ENUM8 {A = 1; B = 2;}",
            "enum A: ENUM8 {A = 1; }",
            Compatibility.FORWARD_FAIL)

        self.ValidateUpdate(
            "Switching enum codes is never ok",
            "enum A: ENUM8 {A = 1; B = 2; }",
            "enum A: ENUM8 {A = 1; B = 3; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_basic_clusters(self):
        self.ValidateUpdate(
            "Switching cluster codes is never ok",
            "client cluster A = 1 {}",
            "client cluster A = 2 {}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

        self.ValidateUpdate(
            "Removing a cluster is not ok",
            "client cluster A = 1 {} client cluster B = 2 {}",
            "client cluster A = 1 {}",
            Compatibility.FORWARD_FAIL)

        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 {}",
            "server cluster A = 0x10 { enum X : ENUM8 {} }",
            Compatibility.BACKWARD_FAIL)

        self.ValidateUpdate(
            "Detects side switch for clusters",
            "client cluster A = 1 {}",
            "server cluster A = 1 {}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_bitmaps(self):
        self.ValidateUpdate(
            "Deleting a bitmap is not ok",
            "client Cluster X = 1 { bitmap A: BITMAP8{} bitmap B: BITMAP8{} }",
            "client Cluster X = 1 { bitmap A: BITMAP8{} }",
            Compatibility.FORWARD_FAIL)

        self.ValidateUpdate(
            "Changing a bitmap type is never ok",
            " client cluster X = 1 { bitmap A: BITMAP8{} }",
            " client cluster X = 1 { bitmap A: BITMAP16{} }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

        self.ValidateUpdate(
            "Adding bitmap values is ok, removing values is not",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x02; } }",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; } }",
            Compatibility.FORWARD_FAIL)

        self.ValidateUpdate(
            "Switching bitmap codes is never ok",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x02; } }",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x04; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)


if __name__ == '__main__':
    unittest.main()
