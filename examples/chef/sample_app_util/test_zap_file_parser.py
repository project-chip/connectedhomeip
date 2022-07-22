"""Tests for zap_file_parser.py

Usage:
python -m unittest
"""

import os
import shutil
import tempfile
import unittest

try:
    import yaml
except ImportError:
    print("Missing yaml library. Install with:\npip install pyyaml")
    exit(1)

import zap_file_parser


_HERE = os.path.abspath(os.path.dirname(__file__))
_TEST_FILE = os.path.join(_HERE, "test_files", "sample_zap_file.zap")
_TEST_METADATA = os.path.join(_HERE, "test_files", "sample_zap_file_hashmeta.yaml")


class TestZapFileParser(unittest.TestCase):
    """Testcases for zap_file_parser.py."""

    def test_generate_hash(self):
        """Tests generate_hash function."""
        hash_string = zap_file_parser.generate_hash(_TEST_FILE)
        self.assertEqual(hash_string, "Xir1gEfjij", "Hash is incorrectly generated.")

    def test_generate_metadata(self):
        """Tests generate_metadata."""
        generated_metadata = zap_file_parser.generate_hash_metadata(_TEST_FILE)
        with open(_TEST_METADATA) as f:
            expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
        self.assertEqual(
            generated_metadata, expected_metadata, "Metadata not generated correctly.")

    def test_generate_metadata_file(self):
        """Tests generate_metadata_file."""
        with tempfile.TemporaryDirectory(dir=os.path.dirname(_HERE)) as dir:
            zap_file = os.path.join(dir, "test_file.zap")
            hashmeta_file = os.path.join(dir, "test_file_hashmeta.yaml")
            shutil.copy(_TEST_FILE, zap_file)
            zap_file_parser.generate_hash_metadata_file(zap_file)
            with open(hashmeta_file) as f:
                generated_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
            with open(_TEST_METADATA) as f:
                expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
            self.assertEqual(
                generated_metadata, expected_metadata, "Metadata file not generated correctly.")

    def test_generate_name(self):
        """Tests generate_name."""
        name = zap_file_parser.generate_name(_TEST_FILE)
        self.assertEqual(name, "rootnode_dimmablelight_Xir1gEfjij", "Name incorrectly generated.")


if __name__ == "__main__":
    unittest.main()
