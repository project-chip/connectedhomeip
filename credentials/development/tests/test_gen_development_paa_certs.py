#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Test PAA regeneration safeguards without generating or changing real credentials."""

import os
import shutil
import stat
import subprocess
import tempfile
import unittest
from pathlib import Path


class TestGenerateDevelopmentLegacyPaaCerts(unittest.TestCase):
    """Exercise the shell script using isolated output paths and fake generation tools."""

    pqc = False

    def setUp(self) -> None:
        """Copy the generator and install fake tools in a temporary checkout."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        source = Path(__file__).resolve().parents[1] / "gen-development-paa-cert.sh"
        self.script = Path(shutil.copy(source, self.root))
        variants = ("-ML-DSA-44", "-ML-DSA-65") if self.pqc else ("",)
        cert_dir = "paa-root-certs" if self.pqc else "attestation"
        self.outputs = [
            self.root / directory / f"Chip-Development-PAA{variant}-{kind}.{encoding}"
            for variant in variants
            for directory, kind in (("attestation", "Key"), (cert_dir, "Cert"))
            for encoding in ("pem", "der")
        ]
        for output in self.outputs:
            output.parent.mkdir(exist_ok=True)
        binary_dir = self.root / "bin"
        binary_dir.mkdir()
        self.env = dict(os.environ, PATH=f"{binary_dir}:{os.environ['PATH']}", TEST_ROOT=str(self.root), FAIL_AT="0")
        executables = {
            "pkg-config": '''#!/usr/bin/env bash
echo "$*" >> "$TEST_ROOT/pkg-config-calls"
exit 1
''',
            "openssl": '''#!/usr/bin/env bash
echo "$*" >> "$TEST_ROOT/openssl-calls"
exit 1
''',
            "chip-cert": '''#!/usr/bin/env bash
set -eu
echo "$1" >> "$TEST_ROOT/calls"
echo "$*" >> "$TEST_ROOT/arguments"
if [[ $1 == gen-att-cert ]]; then
    shift
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --out-key|--out) echo "new $(basename "$2")" > "$2" ;;
        esac
        shift 2
    done
else
    echo "new $(basename "$3")" > "$3"
fi
# Fail after writing output to exercise cleanup of partial generation.
if [[ $(wc -l < "$TEST_ROOT/calls") -eq $FAIL_AT ]]; then
    exit 42
fi
''',
        }
        for name, content in executables.items():
            executable = binary_dir / name
            executable.write_text(content)
            executable.chmod(0o755)

    def _run(self, *args: str) -> subprocess.CompletedProcess:
        mode = ["--pqc"] if self.pqc else []
        return subprocess.run(["bash", str(self.script), *mode, *args], env=self.env, capture_output=True, text=True, check=False)

    def _seed_outputs(self) -> None:
        for output in self.outputs:
            output.write_text(f"old {output.name}\n")

    def test_refuses_each_existing_output_by_default(self) -> None:
        """Any existing output must prevent generation, including the last DER file."""
        for output in self.outputs:
            with self.subTest(output=output.name):
                output.write_text("original")
                result = self._run("chip-cert")
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Refusing to overwrite", result.stderr)
                self.assertEqual(output.read_text(), "original")
                self.assertFalse((self.root / "calls").exists())
                output.unlink()

    def test_refuses_dangling_symlink(self) -> None:
        """A dangling link is still an existing output and must be preserved."""
        self.outputs[0].symlink_to(self.root / "missing")
        self.assertNotEqual(self._run("chip-cert").returncode, 0)
        self.assertTrue(self.outputs[0].is_symlink())
        self.assertFalse((self.root / "calls").exists())

    def test_success(self) -> None:
        """Both modes publish selected files with correct permissions without external OpenSSL checks."""
        other_names = ("Chip-Development-PAA",) if self.pqc else (
            "Chip-Development-PAA-ML-DSA-44", "Chip-Development-PAA-ML-DSA-65")
        other_cert_dir = "attestation" if self.pqc else "paa-root-certs"
        other_outputs = [
            self.root / directory / f"{name}-{kind}.{encoding}"
            for name in other_names
            for directory, kind in (("attestation", "Key"), (other_cert_dir, "Cert"))
            for encoding in ("pem", "der")
        ]
        for output in other_outputs:
            output.parent.mkdir(exist_ok=True)
            output.write_text("unselected")
        for overwrite in (False, True):
            with self.subTest(overwrite=overwrite):
                args = ("--overwrite", "chip-cert") if overwrite else ("chip-cert",)
                if overwrite:
                    self._seed_outputs()
                result = self._run(*args)
                self.assertEqual(result.returncode, 0, result.stderr)
                if overwrite:
                    for warning in ("WARNING", "Back up", "will not validate", "generate new PAIs and DACs"):
                        self.assertIn(warning, result.stderr)
                for output in self.outputs:
                    self.assertEqual(output.read_text(), f"new {output.name}\n")
                    self.assertEqual(stat.S_IMODE(output.stat().st_mode), 0o600 if "Key" in output.name else 0o644)
                for output in other_outputs:
                    self.assertEqual(output.read_text(), "unselected")
                self.assertEqual(list(self.root.glob(".paa.*")), [])
                self.assertFalse((self.root / "pkg-config-calls").exists())
                self.assertFalse((self.root / "openssl-calls").exists())
                arguments = (self.root / "arguments").read_text()
                for invocation in arguments.splitlines():
                    if invocation.startswith("gen-att-cert "):
                        self.assertIn("--valid-from 2021-06-28 14:23:43", invocation)
                if self.pqc:
                    for variant in (44, 65):
                        self.assertIn(f"--key-type ml-dsa-{variant}", arguments)
                    self.assertIn("--subject-vid FFF1", arguments)
                else:
                    self.assertNotIn("--key-type", arguments)
                    self.assertNotIn("--subject-vid", arguments)

    def test_generation_failure_preserves_all_targets(self) -> None:
        """Failure at any generation or conversion step must leave all targets untouched."""
        for overwrite in (False, True):
            if overwrite:
                self._seed_outputs()
            for fail_at in range(1, 7 if self.pqc else 4):
                with self.subTest(overwrite=overwrite, fail_at=fail_at):
                    (self.root / "calls").unlink(missing_ok=True)
                    self.env["FAIL_AT"] = str(fail_at)
                    args = ("--overwrite", "chip-cert") if overwrite else ("chip-cert",)
                    result = self._run(*args)
                    self.assertEqual(result.returncode, 42, result.stderr)
                    for output in self.outputs:
                        if overwrite:
                            self.assertEqual(output.read_text(), f"old {output.name}\n")
                        else:
                            self.assertFalse(output.exists())
                    self.assertEqual(list(self.root.glob(".paa.*")), [])

    def test_invalid_arguments(self) -> None:
        """Missing arguments and unknown flags must fail without invoking generation."""
        for args in ((), ("--overwrite",), ("--pqc",), ("--unknown",), ("--unknown", "chip-cert")):
            with self.subTest(args=args):
                result = self._run(*args)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Usage:", result.stderr)
                self.assertFalse((self.root / "calls").exists())


class TestGenerateDevelopmentPqcPaaCerts(TestGenerateDevelopmentLegacyPaaCerts):
    """Run the safeguards for both PQC roots using chip-cert's runtime checks."""

    pqc = True

    def test_flags_in_reverse_order(self) -> None:
        """Overwrite and PQC flags may be supplied in either order."""
        self._seed_outputs()
        result = subprocess.run(["bash", str(self.script), "--overwrite", "--pqc", "chip-cert"],
                                env=self.env, capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        for output in self.outputs:
            self.assertEqual(output.read_text(), f"new {output.name}\n")


if __name__ == "__main__":
    unittest.main()
