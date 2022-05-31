"""Tests for zap_file_parser.py

Usage:
python -m unittest
"""

import os
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
_TEST_METADATA = os.path.join(_HERE, "test_files", "sample_zap_file.yaml")


class TestZapFileParser(unittest.TestCase):
    """Testcases for zap_file_parser.py."""

    def test_generate_hash(self):
        """Tests generate_hash function."""
        hash_string = zap_file_parser.generate_hash(_TEST_FILE)
        self.assertEqual(hash_string, "nrwiChOZab", "Hash is incorrectly generated.")

    def test_generate_metadata(self):
        """Tests generate_metadata."""
        generated_metadata = zap_file_parser.generate_metadata(_TEST_FILE)
        with open(_TEST_METADATA) as f:
            expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
        self.assertEqual(
            generated_metadata, expected_metadata, "Metadata not generated correctly.")

    def test_generate_metadata(self):
        """Tests generate_metadata_file."""
        dir = tempfile.gettempdir()
        output_file = os.path.join(dir, "test_output.yaml")
        zap_file_parser.generate_metadata_file(_TEST_FILE, output_file_path=output_file)
        with open(output_file) as f:
            generated_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
        with open(_TEST_METADATA) as f:
            expected_metadata = yaml.load(f.read(), Loader=yaml.FullLoader)
        self.assertEqual(
            generated_metadata, expected_metadata, "Metadata file not generated correctly.")

    def test_generate_name(self):
        """Tests generate_name."""
        name = zap_file_parser.generate_name(_TEST_FILE)
        self.assertEqual(name, "rootnode_dimmablelight_nrwiChOZab", "Name incorrectly generated.")


if __name__ == "__main__":
    unittest.main()