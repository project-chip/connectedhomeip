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

import os
import unittest
from unittest.mock import patch

from build_libdatachannel import main
from click.testing import CliRunner


class TestOpenSSLSelection(unittest.TestCase):
    """Check that CMake uses the SDK selection without retaining stale paths."""

    def configure(self, *args: str) -> list[str]:
        """Capture the CMake invocation without compiling libdatachannel."""
        with patch("build_libdatachannel.subprocess.run") as run:
            result = CliRunner().invoke(main, ["--build-dir", "/tmp/libdatachannel-test", *args])
            self.assertEqual(result.exit_code, 0, result.output)
            self.assertEqual(run.call_count, 2)
            return run.call_args_list[0].args[0]

    def test_private_static_openssl(self) -> None:
        """Pass the private root intact and select its static archives."""
        command = self.configure("--openssl-root", "/opt/private openssl", "--openssl-static")
        self.assertIn("-DOPENSSL_ROOT_DIR=/opt/private openssl", command)
        self.assertIn("-DOPENSSL_USE_STATIC_LIBS=TRUE", command)

    def test_private_shared_openssl(self) -> None:
        """Preserve the explicit private root for shared linkage."""
        command = self.configure("--openssl-root", "/opt/private-openssl")
        self.assertIn("-DOPENSSL_ROOT_DIR=/opt/private-openssl", command)
        self.assertIn("-DOPENSSL_USE_STATIC_LIBS=FALSE", command)

    def test_switching_selection_discards_cached_paths(self) -> None:
        """Both directions of a mode change must clear CMake's cached paths."""
        for args in [(), ("--openssl-root", "/opt/private-openssl", "--openssl-static")]:
            with self.subTest(args=args):
                command = self.configure(*args)
                for name in ["INCLUDE_DIR", "CRYPTO_LIBRARY", "SSL_LIBRARY", "ROOT_DIR"]:
                    self.assertIn(f"-UOPENSSL_{name}", command)
                if args:
                    self.assertLess(command.index("-UOPENSSL_ROOT_DIR"),
                                    command.index("-DOPENSSL_ROOT_DIR=/opt/private-openssl"))

    def test_cross_build_does_not_inherit_private_host_openssl(self) -> None:
        """GN decides whether SDK environment defaults apply to the target."""
        with patch.dict(os.environ, CHIP_OPENSSL_ROOT="/opt/host-openssl", CHIP_OPENSSL_STATIC="true"):
            command = self.configure("--cross-compile-cpu-type", "arm64",
                                     "--target-cc", "aarch64-linux-gnu-gcc", "--target-cxx", "aarch64-linux-gnu-g++")
        self.assertFalse(any(arg.startswith("-DOPENSSL_ROOT_DIR=") for arg in command))
        self.assertIn("-DOPENSSL_USE_STATIC_LIBS=FALSE", command)
        self.assertIn("-DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc", command)


if __name__ == "__main__":
    unittest.main()
