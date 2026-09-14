#!/usr/bin/env python3

# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


@unittest.skipUnless(shutil.which("pkg-config"), "pkg-config is required")
class TestPkgConfigPath(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        root = Path(self.directory.name)
        self.system = root / "system"
        self.private = root / "private"
        for directory, library in ((self.system, "system_crypto"), (self.private, "sdk_crypto")):
            directory.mkdir()
            (directory / "openssl.pc").write_text(
                f"Name: OpenSSL fixture\nDescription: Query isolation fixture\n"
                f"Version: 3.5.0\nLibs: -l{library}\nLibs.private: -ldl\n")
        self.env = dict(os.environ, PKG_CONFIG_PATH=str(self.system), PKG_CONFIG_LIBDIR=str(self.system),
                        CHIP_OPENSSL_ROOT=str(self.private), CHIP_OPENSSL_STATIC="true")

    def query(self, *args):
        output = subprocess.check_output(
            [sys.executable, str(Path(__file__).with_name("pkg-config.py")), *args, "openssl"],
            env=self.env, text=True)
        return json.loads(output)

    def test_sdk_environment_does_not_change_unrelated_queries(self):
        self.assertEqual(self.query()[3], ["system_crypto"])
        flags = subprocess.check_output(["pkg-config", "--libs", "openssl"], env=self.env, text=True)
        self.assertEqual(flags.strip(), "-lsystem_crypto")

    def test_private_path_is_scoped_to_one_query(self):
        self.assertEqual(self.query("--pkg-config-path", str(self.private))[3], ["sdk_crypto"])
        self.assertEqual(self.query()[3], ["system_crypto"])

    def test_private_static_query_includes_private_dependencies(self):
        self.assertEqual(self.query("--pkg-config-path", str(self.private), "--static")[3], ["sdk_crypto", "dl"])


if __name__ == "__main__":
    unittest.main()
