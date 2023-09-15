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
from enum import auto, Flag

try:
    from matter_idl.matter_idl_parser import CreateParser
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.matter_idl_parser import CreateParser

from matter_idl.matter_idl_types import Idl
from matter_idl.backwards_compatibility import IsBackwardsCompatible


class Compatibility(Flag):
    FORWARD_FAIL  = auto() # old -> new is wrong
    BACKWARD_FAIL = auto() # new -> old is wrong


class TestCompatibilityChecks(unittest.TestCase):

    def _AssumeCompatiblity(self, old: str, new: str, old_idl: Idl, new_idl: Idl, expect_compatible: bool):
        if expect_compatible != IsBackwardsCompatible(old_idl, new_idl):
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

    def ValidateUpdate(self, old: str , new: str, flags: Compatibility):
        old_idl = CreateParser(skip_meta=True).parse(old)
        new_idl = CreateParser(skip_meta=True).parse(new)

        # Validate compatibility and that no changes are always compatible

        self._AssumeCompatiblity(old, new, old_idl, new_idl, Compatibility.FORWARD_FAIL not in flags)
        self._AssumeCompatiblity(new, old, new_idl, old_idl, Compatibility.BACKWARD_FAIL not in flags)
        self._AssumeCompatiblity(new, new, new_idl, new_idl, True)
        self._AssumeCompatiblity(old, old, old_idl, old_idl, True)


    def test_top_level_enums(self):
        # deleting a top level enum
        self.ValidateUpdate(
            "enum A: ENUM8{} enum B: ENUM8{}",
            "enum A: ENUM8{}",
            Compatibility.FORWARD_FAIL)

        # Changing type of an enum
        self.ValidateUpdate(
            "enum A: ENUM8{}",
            "enum A: ENUM16{}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

        # Adding values is ok, removing values is not
        self.ValidateUpdate(
            "enum A: ENUM8 {A = 1; B = 2;}",
            "enum A: ENUM8 {A = 1; }",
            Compatibility.FORWARD_FAIL)


if __name__ == '__main__':
    unittest.main()
