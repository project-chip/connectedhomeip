"""Tests for zap_file_parser.py

Usage:
python -m unittest
"""

import json
import os
import shutil
import tempfile
import unittest
import uuid
from unittest import mock

import zap_file_parser

try:
    import yaml
except ImportError:
    print("Missing yaml library. Install with:\npip install pyyaml")
    exit(1)


_HERE = os.path.abspath(os.path.dirname(__file__))
_TEST_FILE = os.path.join(_HERE, "test_files", "sample_zap_file.zap")
_TEST_METADATA = os.path.join(_HERE, "test_files", "sample_zap_file_meta.yaml")


class TestZapFileParser(unittest.TestCase):
    """Testcases for zap_file_parser.py."""

    def test_generate_hash(self):
        """Tests generate_hash function."""
        with mock.patch.object(uuid, "uuid4", return_value="Xir1gEfjij"):
            hash_string = zap_file_parser.generate_hash()
            self.assertEqual(hash_string, "Xir1gEfjij",
                             "Hash is incorrectly generated.")

    def test_generate_metadata(self):
        """Tests generate_metadata."""
        generated_metadata = zap_file_parser.generate_metadata(_TEST_FILE)
        with open(_TEST_METADATA) as f:
            expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
        self.assertEqual(
            generated_metadata, expected_metadata, "Metadata not generated correctly.")

    def test_generate_metadata_file(self):
        """Tests generate_metadata_file."""
        with tempfile.TemporaryDirectory(dir=os.path.dirname(_HERE)) as tmpdir:
            zap_file = os.path.join(tmpdir, "test_file.zap")
            meta_file = os.path.join(tmpdir, "test_file_meta.yaml")
            shutil.copy(_TEST_FILE, zap_file)
            zap_file_parser.generate_metadata_file(zap_file)
            with open(meta_file) as f:
                generated_metadata = yaml.load(
                    f.read(), Loader=yaml.FullLoader)
            with open(_TEST_METADATA) as f:
                expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
            self.assertEqual(
                generated_metadata, expected_metadata, "Metadata file not generated correctly.")

    def test_generate_name(self):
        """Tests generate_name."""
        with mock.patch.object(uuid, "uuid4", return_value="Xir1gEfjij"):
            name = zap_file_parser.generate_name(_TEST_FILE)
            self.assertEqual(
                name, "rootnode_dimmablelight_Xir1gEfjij", "Name incorrectly generated.")

    def test_merge_zap_files(self):
        """Tests merge_zap_files appends non-root endpoints and returns endpoint ID mappings."""
        with tempfile.TemporaryDirectory(dir=os.path.dirname(_HERE)) as tmpdir:
            zap_1 = os.path.join(tmpdir, "device1.zap")
            zap_2 = os.path.join(tmpdir, "device2.zap")
            shutil.copy(_TEST_FILE, zap_1)
            shutil.copy(_TEST_FILE, zap_2)
            merged_zap = os.path.join(tmpdir, "merged.zap")
            mappings = zap_file_parser.merge_zap_files([zap_1, zap_2], merged_zap)
            self.assertEqual(mappings, {zap_1: {0: 0, 1: 1}, zap_2: {0: 0, 1: 2}})
            with open(merged_zap) as f:
                merged_data = json.load(f)
            endpoint_ids = [ep["endpointId"] for ep in merged_data["endpoints"]]
            self.assertEqual(endpoint_ids, [0, 1, 2])
            self.assertEqual(len(merged_data["endpointTypes"]), 3)


if __name__ == "__main__":
    unittest.main()
