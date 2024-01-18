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

import logging
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

from matter_idl.backwards_compatibility import CompatibilityChecker, is_backwards_compatible
from matter_idl.matter_idl_types import Idl


class Compatibility(Flag):
    ALL_OK = auto()  # placeholder allowing anything
    FORWARD_FAIL = auto()  # old -> new is wrong
    BACKWARD_FAIL = auto()  # new -> old is wrong


class DisableLogger():
    def __enter__(self):
        logging.disable(logging.CRITICAL)

    def __exit__(self, exit_type, exit_value, exit_traceback):
        logging.disable(logging.NOTSET)


class TestCompatibilityChecks(unittest.TestCase):

    def _AssumeCompatiblity(self, old: str, new: str, old_idl: Idl, new_idl: Idl, expect_compatible: bool):
        with DisableLogger():
            if expect_compatible == is_backwards_compatible(old_idl, new_idl):
                return

            # re-run to figure out reasons:
            checker = CompatibilityChecker(old_idl, new_idl)
            checker.check()

        if expect_compatible:
            reason = "EXPECTED COMPATIBLE, but failed"
            fail_reasons = "\nREASONS:" + "\n  - ".join([""] + checker.errors)
        else:
            reason = "EXPECTED NOT COMPATIBLE, but succeeded"
            fail_reasons = ""

        self.fail(f"""Failed compatibility test
{reason}:
/-------------------- OLD ----------------/
{old}
/-------------------- NEW ----------------/
{new}
/-----------------------------------------/{fail_reasons}""")

    def ValidateUpdate(self, name: str, old: str, new: str, flags: Compatibility):
        old_idl = CreateParser(skip_meta=True).parse(old)
        new_idl = CreateParser(skip_meta=True).parse(new)
        with self.subTest(validate=name):
            # Validate compatibility and that no changes are always compatible
            with self.subTest(direction="Forward"):
                self._AssumeCompatiblity(
                    old, new, old_idl, new_idl, Compatibility.FORWARD_FAIL not in flags)

            with self.subTest(direction="Backward"):
                self._AssumeCompatiblity(
                    new, old, new_idl, old_idl, Compatibility.BACKWARD_FAIL not in flags)

            with self.subTest(direction="NEW-to-NEW"):
                self._AssumeCompatiblity(new, new, new_idl, new_idl, True)

            with self.subTest(direction="OLD-to-OLD"):
                self._AssumeCompatiblity(old, old, old_idl, old_idl, True)

    def test_basic_clusters_enum(self):
        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 {}",
            "server cluster A = 0x10 { enum X : ENUM8 {} }",
            Compatibility.BACKWARD_FAIL)

    def test_clusters_enum_add_remove(self):
        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 { enum X : ENUM8 { A = 1; B = 2; }}",
            "server cluster A = 16 { enum X : ENUM8 { A = 1; }}",
            Compatibility.FORWARD_FAIL)

    def test_provisional_cluster(self):
        self.ValidateUpdate(
            "Provisional cluster changes are ok.",
            "provisional server cluster A = 16 { enum X : ENUM8 { A = 1; B = 2; } info event A = 1 { int8u x = 1;} }",
            "provisional server cluster A = 16 { enum X : ENUM8 { A = 1; B = 3; } info event A = 2 { int16u x = 1;} }",
            Compatibility.ALL_OK)

    def test_clusters_enum_code(self):
        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 { enum X : ENUM8 { A = 1; B = 2; }}",
            "server cluster A = 16 { enum X : ENUM8 { A = 1; B = 3; }}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_clusters_enum_delete(self):
        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 { enum X : ENUM8 {}}",
            "server cluster A = 16 { }",
            Compatibility.FORWARD_FAIL)

    def test_clusters_enum_change(self):
        self.ValidateUpdate(
            "Adding an enum is ok. Also validates code formatting",
            "server cluster A = 16 { enum X : ENUM16 {}}",
            "server cluster A = 16 { enum X : ENUM8 {}}",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_bitmaps_delete(self):
        self.ValidateUpdate(
            "Deleting a bitmap is not ok",
            "client Cluster X = 1 { bitmap A: BITMAP8{} bitmap B: BITMAP8{} }",
            "client Cluster X = 1 { bitmap A: BITMAP8{} }",
            Compatibility.FORWARD_FAIL)

    def test_bitmaps_type(self):
        self.ValidateUpdate(
            "Changing a bitmap type is never ok",
            " client cluster X = 1 { bitmap A: BITMAP8{} }",
            " client cluster X = 1 { bitmap A: BITMAP16{} }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_bitmaps_values(self):
        self.ValidateUpdate(
            "Adding bitmap values is ok, removing values is not",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x02; } }",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; } }",
            Compatibility.FORWARD_FAIL)

    def test_bitmaps_code(self):
        self.ValidateUpdate(
            "Switching bitmap codes is never ok",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x02; } }",
            " client cluster X = 1 { bitmap A: BITMAP8 { kA = 0x01; kB = 0x04; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_events_delete(self):
        self.ValidateUpdate(
            "Deleting an event is not ok",
            "client Cluster X = 1 { info event A = 1 {} info event B = 2 {} }",
            "client Cluster X = 1 { info event B = 2 {} }",
            Compatibility.FORWARD_FAIL)

    def test_events_code(self):
        self.ValidateUpdate(
            "Changing event code is never ok",
            "client Cluster X = 1 { info event A = 1 {} }",
            "client Cluster X = 1 { info event A = 2 {} }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_events_struct_content_change(self):
        self.ValidateUpdate(
            "Changing event struct is never ok",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; } }",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; int8u y = 2; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_events_struct_renames(self):
        self.ValidateUpdate(
            "Renaming struct content is ok",
            "client Cluster X = 1 { info event A = 1 { int8u a = 1; int8u b = 2; } }",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; int8u y = 2; } }",
            Compatibility.ALL_OK)

    def test_events_struct_reorders_renames(self):
        self.ValidateUpdate(
            "Renaming struct content is ok",
            "client Cluster X = 1 { info event A = 1 { int8u a = 1; int16u b = 2; } }",
            "client Cluster X = 1 { info event A = 1 { int16u x = 2; int8u a = 1; } }",
            Compatibility.ALL_OK)

    def test_events_struct_type_change(self):
        self.ValidateUpdate(
            "Changing event struct is never ok",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; } }",
            "client Cluster X = 1 { info event A = 1 { int16u x = 1; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_events_struct_id_change(self):
        self.ValidateUpdate(
            "Changing event struct is never ok",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; } }",
            "client Cluster X = 1 { info event A = 1 { int8u x = 2; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_events_severity(self):
        self.ValidateUpdate(
            "Changing event severity is ok",
            "client Cluster X = 1 { info event A = 1 { int8u x = 1; } }",
            "client Cluster X = 1 { critical event A = 1 { int8u x = 1; } }",
            Compatibility.ALL_OK)

    def test_commands_remove(self):
        self.ValidateUpdate(
            "Removing commands is not ok",
            "client Cluster X = 1 { command A() : DefaulSuccess = 0; command B() : DefaultSuccess = 1; }",
            "client Cluster X = 1 { command A() : DefaulSuccess = 0; }",
            Compatibility.FORWARD_FAIL)

    def test_commands_id(self):
        self.ValidateUpdate(
            "Changing command IDs is never ok",
            "client Cluster X = 1 { command A() : DefaulSuccess = 1; }",
            "client Cluster X = 1 { command A() : DefaulSuccess = 2; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_commands_input(self):
        self.ValidateUpdate(
            "Changing command Inputs is never ok",
            "client Cluster X = 1 { command A() : DefaulSuccess = 1; }",
            "client Cluster X = 1 { command A(ARequest) : DefaulSuccess = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_commands_output(self):
        self.ValidateUpdate(
            "Changing command Outputs is never ok",
            "client Cluster X = 1 { command A() : DefaultSuccess = 1; }",
            "client Cluster X = 1 { timed command A() : DefaultSuccess = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_commands_quality_fabric(self):
        self.ValidateUpdate(
            "Changing command quality is not ok",
            "client Cluster X = 1 { command A() : DefaultSuccess = 1; }",
            "client Cluster X = 1 { fabric command A() : DefaultSuccess = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_commands_quality_timed(self):
        self.ValidateUpdate(
            "Changing command qualities is never ok",
            "client Cluster X = 1 { fabric command A() : DefaultSuccess = 1; }",
            "client Cluster X = 1 { fabric timed command A() : DefaultSuccess = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_commands_change_acl(self):
        self.ValidateUpdate(
            "Switching access is ok",
            "client Cluster X = 1 { command A() : DefaultSuccess = 1; }",
            "client Cluster X = 1 { command access(invoke: administer) A() : DefaultSuccess = 1; }",
            Compatibility.ALL_OK)

    def test_struct_removal(self):
        self.ValidateUpdate(
            "Structure removal is not ok, but adding is ok",
            "client Cluster X = 1 { struct Foo {} struct Bar {} }",
            "client Cluster X = 1 { struct Foo {} }",
            Compatibility.FORWARD_FAIL)

    def test_struct_content_type_change(self):
        self.ValidateUpdate(
            "Changing structure data types is never ok",
            "client Cluster X = 1 { struct Foo { int32u x = 1; } }",
            "client Cluster X = 1 { struct Foo { int64u x = 1; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_struct_content_rename_reorder(self):
        self.ValidateUpdate(
            "Structure content renames and reorder is ok.",
            "client Cluster X = 1 { struct Foo { int32u x = 1; int8u y = 2; } }",
            "client Cluster X = 1 { struct Foo { int8u a = 2; int32u y = 1; } }",
            Compatibility.ALL_OK)

    def test_struct_content_add_remove(self):
        self.ValidateUpdate(
            "Structure content change is not ok.",
            "client Cluster X = 1 { struct Foo { int32u x = 1; } }",
            "client Cluster X = 1 { struct Foo { int32u x = 1; int8u y = 2; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_struct_tag_change_request(self):
        self.ValidateUpdate(
            "Structure type (request/regular) change is not ok",
            "client Cluster X = 1 { struct Foo { int32u x = 1; } }",
            "client Cluster X = 1 { request struct Foo { int32u x = 1; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_struct_tag_change_response(self):
        self.ValidateUpdate(
            "Structure type (request/response) change is not ok",
            "client Cluster X = 1 { request struct Foo { int32u x = 1; } }",
            "client Cluster X = 1 { response struct Foo = 1 { int32u x = 1; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_struct_code(self):
        self.ValidateUpdate(
            "Changing struct code is not ok",
            "client Cluster X = 1 { response struct Foo = 1 { int32u x = 1; } }",
            "client Cluster X = 1 { response struct Foo = 2 { int32u x = 1; } }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_add_remove(self):
        self.ValidateUpdate(
            "Attribute removal is not ok.",
            "client Cluster X = 1 { readonly attribute int8u a = 1; readonly attribute int8u b = 2; }",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            Compatibility.FORWARD_FAIL)

    def test_attribute_change_type(self):
        self.ValidateUpdate(
            "Attribute type cannot be changed.",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            "client Cluster X = 1 { readonly attribute int32u a = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_change_code(self):
        self.ValidateUpdate(
            "Attribute codes cannot be changed.",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            "client Cluster X = 1 { readonly attribute int8u a = 2; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_change_optionality(self):
        self.ValidateUpdate(
            "Attribute optionality must stay the same",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            "client Cluster X = 1 { readonly attribute optional int8u a = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_change_list(self):
        self.ValidateUpdate(
            "Attribute list type must stay the same",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            "client Cluster X = 1 { readonly attribute int8u a[] = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_change_rw(self):
        self.ValidateUpdate(
            "Attribute read/write status must stay the same",
            "client Cluster X = 1 { readonly attribute int8u a = 1; }",
            "client Cluster X = 1 { attribute int8u a = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)

    def test_attribute_type(self):
        self.ValidateUpdate(
            "Attribute data type must stay the same",
            "client Cluster X = 1 { attribute int8u a = 1; }",
            "client Cluster X = 1 { attribute char_string a = 1; }",
            Compatibility.FORWARD_FAIL | Compatibility.BACKWARD_FAIL)


if __name__ == '__main__':
    unittest.main()
