#!/usr/bin/env python3
#
#    Copyright (c) 2026 Project CHIP Authors
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

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

import cbor2 as cbor

TOOLS_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def generate_partition(data: dict) -> bytes:
    """Generate partition and return content of partition binary file."""
    with tempfile.TemporaryDirectory() as out_dir:
        input_file = Path(out_dir) / "fd.json"
        output_file_prefix = Path(out_dir) / "partition"

        with open(input_file, "w", encoding="utf-8") as f:
            json.dump(data, f)

        subprocess.check_call(["python3",
                               os.path.join(TOOLS_DIR, "nrfconnect_generate_partition.py"),
                               "--offset", "0x0",
                               "--size", "0x1000",
                               "--input", input_file,
                               "--output", output_file_prefix,
                               "--raw"
                               ])

        with open(output_file_prefix.with_suffix(".bin"), "rb") as f:
            return f.read()


class TestPartitionCreatorConvertToDict(unittest.TestCase):

    def test_hex_prefix_string_converted_to_bytes(self):
        data = {"key": "hex:deadbeef"}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["key"], bytes([0xde, 0xad, 0xbe, 0xef]))

    def test_plain_string_converted_to_utf8(self):
        data = {"name": "hello"}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["name"], b"hello")

    def test_integer_not_converted(self):
        data = {"version": 42}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["version"], 42)

    def test_nested_dict_hex_prefix_string_converted_to_bytes(self):
        data = {"user": {"raw": "hex:cafe"}}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["user"]["raw"], bytes([0xca, 0xfe]))

    def test_nested_dict_plain_string_not_converted(self):
        data = {"user": {"name": "test"}}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["user"]["name"], "test")

    def test_nested_dict_integer_not_converted(self):
        data = {"user": {"value": 11}}
        result = cbor.loads(generate_partition(data))
        self.assertEqual(result["user"]["value"], 11)


if __name__ == "__main__":
    unittest.main()
