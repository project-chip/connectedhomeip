# Copyright (c) 2026 Project CHIP Authors
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

"""Tests for the chip-cert tool.

Certificates are generated, converted and validated using chip-cert itself,
so these tests need nothing beyond the standard library.

Set CHIP_CERT to the binary under test:

    ninja -C out/default chip-cert
    CHIP_CERT=out/default/chip-cert python3 src/tools/chip-cert/tests/test_chip_cert.py
"""

import os
import subprocess
import tempfile
import unittest
from pathlib import Path

CHIP_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))
OPERATIONAL_CERTS = CHIP_ROOT / "credentials/test/operational-certificates"

# The compact-pdc-identity test vector from src/credentials/tests/CHIPCert_test_vectors.cpp
# (sTestCert_PDCID01_ChipCompact). All fields other than the public key and signature are
# implied by the specification, so a conforming encoder must reproduce these exact bytes.
PDCID01_COMPACT = bytes.fromhex(
    "153009410439ccd4ac7e5968bdf1b080111d92536494fc51624c70aa6d7308daedf3a15e3869"
    "177b1bf3d09047ebf06be8dd17be23f2fb3d6390c6cf82802f62d05362c008300b40384c1bd0"
    "8fe4caa0460791660d82145dfbfe97d314d15e000d75c073af5d7c7ecc1a01852126184ad9"
    "ebec809b4c78fff381fe324baf881cc8249e169259bd5a18"
)

# Field values mandated for a PDC Identity by the specification, as print-cert renders them.
PDC_DN = "[[ CommonName = * ]]"
PDC_NOT_BEFORE = "0x00000001  ( 2000/01/01 00:00:01 )"
PDC_NOT_AFTER = "0x00000000  ( 9999/12/31 23:59:59 )"

# Reported by chip-cert whenever it takes the compact-pdc-identity encoding path.
COMPACT_FORMAT_NOTICE = "using compact TLV format"


class ChipCertTest(unittest.TestCase):
    """Base class providing access to the chip-cert binary and a scratch directory."""

    @classmethod
    def setUpClass(cls):
        from_env = os.environ.get("CHIP_CERT")
        if not from_env:
            raise AssertionError("CHIP_CERT is not set; point it at the chip-cert binary to test")
        cls.chip_cert = Path(from_env)
        if not cls.chip_cert.is_file():
            raise AssertionError(f"chip-cert binary not found at {cls.chip_cert}")

    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.tmp_path = Path(directory.name)

    def run_chip_cert(self, *args, expect_success=True):
        """Run chip-cert, assert whether it succeeded, and return the CompletedProcess."""
        command = [str(self.chip_cert), *map(str, args)]
        result = subprocess.run(command, capture_output=True, text=True)
        if expect_success:
            self.assertEqual(result.returncode, 0, f"{' '.join(command)} failed:\n{result.stderr}")
        else:
            self.assertNotEqual(result.returncode, 0, f"{' '.join(command)} unexpectedly succeeded")
        return result

    def print_cert(self, source):
        """Return the fields of a certificate as a dict, via the print-cert command.

        Values spanning multiple lines (public key, signature) are not included, since
        their continuation lines carry no field name.
        """
        output = self.run_chip_cert("print-cert", source).stdout
        fields = {}
        for line in output.splitlines():
            name, separator, value = line.partition(":")
            if separator and value.strip():
                fields[name.strip()] = value.strip()
        return fields

    def convert(self, source, name, *options):
        """Convert a certificate with convert-cert, returning (output path, result)."""
        out = self.tmp_path / name
        result = self.run_chip_cert("convert-cert", source, out, *options)
        return out, result

    def generate_identity(self, out_format, name="identity"):
        """Generate a PDC Identity with gen-cert, returning the output path."""
        out = self.tmp_path / name
        self.run_chip_cert("gen-cert", "--type", "p", "--out", out,
                           "--out-key", self.tmp_path / f"{name}-key.pem",
                           "--out-format", out_format)
        return out

    def generate_ca_cert_with_pdc_subject(self):
        """Generate a self-signed CA certificate whose subject is CN=*.

        This is a valid certificate that is not a PDC Identity. chip-cert recognises
        identities by their subject DN alone, so this exercises that heuristic's failure
        mode. gen-cert normally refuses a subject DN that disagrees with the requested
        certificate type, so error injection is needed to produce it.
        """
        out = self.tmp_path / "ca-with-pdc-subject.pem"
        self.run_chip_cert("gen-cert", "--type", "r", "--subject-cn-u", "*",
                           "--out", out, "--out-key", self.tmp_path / "ca-key.pem",
                           "--out-format", "x509-pem", "--lifetime", "3650",
                           "--ignore-error", "--error-type", "no-error")
        return out


class PDCIdentityTest(ChipCertTest):
    """Tests covering Wi-Fi PDC (Network Client) Identity certificates."""

    def assert_is_pdc_identity(self, source):
        """Check the field values the specification mandates for a PDC Identity."""
        fields = self.print_cert(source)
        self.assertEqual(fields["Subject"], PDC_DN)
        # The identity is self-signed, so the issuer must match the subject. Getting this
        # wrong is not caught by the compact encoding, which always writes the canonical
        # issuer, so it only shows up as a signature that fails to verify.
        self.assertEqual(fields["Issuer"], PDC_DN)
        self.assertEqual(fields["Not Before"], PDC_NOT_BEFORE)
        self.assertEqual(fields["Not After"], PDC_NOT_AFTER)
        self.assertEqual(fields["Is CA"], "false")
        # validate-cert checks the remaining requirements, including the self-signature.
        self.run_chip_cert("validate-cert", source)

    def test_gen_cert_uses_compact_format(self):
        """gen-cert writes PDC Identities in the compact-pdc-identity format."""
        out = self.generate_identity("chip")
        self.assertEqual(len(out.read_bytes()), len(PDCID01_COMPACT))

    def test_gen_cert_produces_valid_identity(self):
        """A generated PDC Identity must be well-formed and correctly self-signed.

        Regression test: the self-signed issuer was previously taken from the not-yet-populated
        subject of the certificate under construction, yielding an empty issuer and a signature
        computed over a TBSCertificate that no verifier would accept.
        """
        self.assert_is_pdc_identity(self.generate_identity("chip"))

    def test_gen_cert_x509_output_is_valid_identity(self):
        """The same must hold for X.509 output, which has no compact encoding to mask a bad issuer."""
        self.assert_is_pdc_identity(self.generate_identity("x509-pem"))

    def test_convert_spec_vector_round_trip(self):
        """compact -> X.509 -> compact must reproduce the spec test vector byte for byte."""
        compact = self.tmp_path / "vector.chip"
        compact.write_bytes(PDCID01_COMPACT)
        self.assert_is_pdc_identity(compact)

        pem_file, _ = self.convert(compact, "vector.pem", "--x509-pem")
        self.assert_is_pdc_identity(pem_file)

        round_tripped, _ = self.convert(pem_file, "vector-rt.chip", "--chip")
        self.assertEqual(round_tripped.read_bytes(), PDCID01_COMPACT)

    def test_convert_non_identity_is_unaffected(self):
        """Certificates that are not PDC Identities must not be re-encoded as compact."""
        out, result = self.convert(OPERATIONAL_CERTS / "Chip-Test-Root01-Cert.pem",
                                   "root.chip", "--chip")

        self.assertGreater(len(out.read_bytes()), len(PDCID01_COMPACT))
        self.assertNotIn(COMPACT_FORMAT_NOTICE, result.stderr)

    def test_convert_ca_cert_with_pdc_subject_is_rewritten(self):
        """A CA certificate that merely happens to use CN=* is lossily rewritten as an identity.

        Identities are recognised by their subject DN alone, so such a certificate takes the
        compact path, which retains only the public key and signature. Basic constraints and
        validity are replaced with the specification's values. chip-cert reports when it does
        this, since the conversion is not reversible.
        """
        source = self.generate_ca_cert_with_pdc_subject()
        self.assertEqual(self.print_cert(source)["Is CA"], "true")

        compact, result = self.convert(source, "ca.chip", "--chip")

        self.assertIn(COMPACT_FORMAT_NOTICE, result.stderr)
        self.assertEqual(len(compact.read_bytes()), len(PDCID01_COMPACT))

        # Confirm what the compact encoding discarded.
        fields = self.print_cert(compact)
        self.assertEqual(fields["Is CA"], "false")
        self.assertEqual(fields["Not Before"], PDC_NOT_BEFORE)

    def test_validate_cert_rejects_tampered_identity(self):
        """A PDC Identity whose signature has been altered must be rejected."""
        tampered = bytearray(PDCID01_COMPACT)
        tampered[-5] ^= 0xFF
        path = self.tmp_path / "tampered.chip"
        path.write_bytes(bytes(tampered))

        self.run_chip_cert("validate-cert", path, expect_success=False)

    def test_validate_cert_rejects_ca_cert_with_pdc_subject(self):
        """A CN=* CA certificate is recognised as an identity but fails the remaining checks.

        This is the safety net for the subject-DN heuristic: such a certificate takes the
        PDC validation path, where the full set of specification requirements rejects it.
        """
        source = self.generate_ca_cert_with_pdc_subject()

        self.run_chip_cert("validate-cert", source, expect_success=False)


class OperationalCertTest(ChipCertTest):
    """Tests covering ordinary operational certificates."""

    def generate_chain(self):
        """Generate a self-signed root and a NOC issued by it, returning both paths."""
        root = self.tmp_path / "root.pem"
        root_key = self.tmp_path / "root-key.pem"
        noc = self.tmp_path / "noc.pem"

        self.run_chip_cert("gen-cert", "--type", "r", "--subject-chip-id", "CACACACA00000001",
                           "--out", root, "--out-key", root_key,
                           "--out-format", "x509-pem", "--lifetime", "3650")
        self.run_chip_cert("gen-cert", "--type", "n", "--subject-chip-id", "DEDEDEDE00010001",
                           "--subject-fab-id", "FAB000000000001D",
                           "--ca-cert", root, "--ca-key", root_key,
                           "--out", noc, "--out-key", self.tmp_path / "noc-key.pem",
                           "--out-format", "x509-pem", "--lifetime", "3650")
        return root, noc

    def test_validate_cert_validates_chain(self):
        """A NOC must validate against the root that issued it.

        Also guards the PDC Identity handling in validate-cert, which must not disturb
        ordinary chain validation.
        """
        root, noc = self.generate_chain()

        self.run_chip_cert("validate-cert", "-t", root, noc)

    def test_validate_cert_rejects_untrusted_chain(self):
        """A NOC must not validate without the root that issued it."""
        _, noc = self.generate_chain()

        self.run_chip_cert("validate-cert", noc, expect_success=False)

    def test_convert_between_chip_formats_is_stable(self):
        """The CHIP TLV encoding must not depend on which container format carries it."""
        source = OPERATIONAL_CERTS / "Chip-Test-Root01-Cert.pem"

        raw, _ = self.convert(source, "root.chip", "--chip")
        base64_form, _ = self.convert(source, "root.chip-b64", "--chip-b64")
        hex_form, _ = self.convert(source, "root.chip-hex", "--chip-hex")

        from_base64, _ = self.convert(base64_form, "from-b64.chip", "--chip")
        from_hex, _ = self.convert(hex_form, "from-hex.chip", "--chip")

        self.assertEqual(from_base64.read_bytes(), raw.read_bytes())
        self.assertEqual(from_hex.read_bytes(), raw.read_bytes())

    def test_convert_x509_round_trip_is_stable(self):
        """Converting TLV to X.509 and back must reproduce the original TLV."""
        source = OPERATIONAL_CERTS / "Chip-Test-Root01-Cert.pem"

        raw, _ = self.convert(source, "root.chip", "--chip")
        pem_file, _ = self.convert(raw, "root.pem", "--x509-pem")
        round_tripped, _ = self.convert(pem_file, "root-rt.chip", "--chip")

        self.assertEqual(round_tripped.read_bytes(), raw.read_bytes())


if __name__ == "__main__":
    unittest.main()
