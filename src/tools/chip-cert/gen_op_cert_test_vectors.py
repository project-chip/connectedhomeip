#!/usr/bin/env python

import argparse
import os
import os.path
import subprocess
import sys
from enum import Enum

from dacs import copyrightNotice, make_array_header, make_c_array


class CertType(Enum):
    NOC = 1
    ICAC = 2
    RCAC = 3


class CertFormat(Enum):
    DER = 1
    CHIP = 2


#
# Generate operational certificates (NOC, ICAC, RCAC) test vectors in DER format.
# All these certificates are expected to fail when converted to CHIP TLV form
# using ConvertX509CertToChipCert() function.
#
# Note that not all malformed DER certificates are expected to fail when converted
# to CHIP TLV format, however, they are expected to fail later at the validation process.
#
DER_CERT_ERROR_TEST_CASES = [
    {
        "description": "Certificate size exceeds its muximum supported limit",
        "test_name": 'Oversized',
        "error_flag": 'cert-oversized',
    },
    {
        "description": 'Invalid certificate version field set to v2(1)',
        "test_name": 'Cert-Version-V2',
        "error_flag": 'cert-version',
    },
    {
        "description": 'Invalid certificate signature algorithm ECDSA_WITH_SHA1',
        "test_name": 'Sig-Algo-ECDSA-With-SHA1',
        "error_flag": 'sig-algo',
    },
    {
        "description": "Certificate doesn't include Validity not-before field",
        "test_name": 'Validity-Not-Before-Missing',
        "error_flag": 'validity-not-before-missing',
    },
    {
        "description": "Certificate doesn't include Validity not-after field",
        "test_name": 'Validity-Not-After-Missing',
        "error_flag": 'validity-not-after-missing',
    },
    {
        "description": "Certificate Validity fields are invalid (values are switched)",
        "test_name": 'Validity-Wrong',
        "error_flag": 'validity-wrong',
    },
    {
        "description": "Subject's NodeId value is invalid",
        "test_name": 'Subject-NodeId-Invalid',
        "error_flag": 'subject-node-id-invalid',
    },
    {
        "description": "Subject's FabricId value is invalid",
        "test_name": 'Subject-FabricId-Invalid',
        "error_flag": 'subject-fabric-id-invalid',
    },
    {
        "description": "Subject's CAT value is invalid",
        "test_name": 'Subject-CAT-Invalid',
        "error_flag": 'subject-cat-invalid',
    },
    {
        "description": "Invalid certificate public key curve secp256k1",
        "test_name": 'Sig-Curve-Secp256k1',
        "error_flag": 'sig-curve',
    },
    {
        "description": "Certificate Basic Constraint extension critical field is missing",
        "test_name": 'Ext-Basic-Critical-Missing',
        "error_flag": 'ext-basic-critical-missing',
    },
    {
        "description": "Certificate Basic Constraint extension critical field is set as 'non-critical'",
        "test_name": 'Ext-Basic-Critical-Wrong',
        "error_flag": 'ext-basic-critical-wrong',
    },
    {
        "description": "Certificate Basic Constraint extension CA field is missing",
        "test_name": 'Ext-Basic-CA-Missing',
        "error_flag": 'ext-basic-ca-missing',
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field presence is wrong (present for NOC)",
        "test_name": 'Ext-Basic-PathLen-Presence-Wrong',
        "error_flag": 'ext-basic-pathlen-presence-wrong',
    },
    {
        "description": "Certificate Key Usage extension critical field is missing",
        "test_name": 'Ext-KeyUsage-Critical-Missing',
        "error_flag": 'ext-key-usage-critical-missing',
    },
    {
        "description": "Certificate Key Usage extension critical field is set as 'non-critical'",
        "test_name": 'Ext-KeyUsage-Critical-Wrong',
        "error_flag": 'ext-key-usage-critical-wrong',
    },
    {
        "description": "Authority Key ID (AKID) extension length is 19 bytes instead of required 20",
        "test_name": 'Ext-AKID-Length-Invalid',
        "error_flag": 'ext-akid-len-invalid',
    },
    {
        "description": "Subject Key ID (SKID) extension length is 19 bytes instead of required 20",
        "test_name": 'Ext-SKID-Length-Invalid',
        "error_flag": 'ext-skid-len-invalid',
    },
]

CHIP_TLV_CERT_ERROR_TEST_CASES = [
    {
        "description": "Certificate size exceeds its muximum supported limit",
        "test_name": 'Oversized',
        "error_flag": 'cert-oversized',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": 'Invalid certificate version field set to v2(1)',
        "test_name": 'Cert-Version-V2',
        "error_flag": 'cert-version',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Serial Number field",
        "test_name": 'Serial-Number-Missing',
        "error_flag": 'serial-number-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": 'Invalid certificate signature algorithm ECDSA_WITH_SHA1',
        "test_name": 'Sig-Algo-ECDSA-With-SHA1',
        "error_flag": 'sig-algo',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Issuer field",
        "test_name": 'Issuer-Missing',
        "error_flag": 'issuer-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Validity not-before field",
        "test_name": 'Validity-Not-Before-Missing',
        "error_flag": 'validity-not-before-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Validity not-after field",
        "test_name": 'Validity-Not-After-Missing',
        "error_flag": 'validity-not-after-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Validity fields are invalid (values are switched)",
        "test_name": 'Validity-Wrong',
        "error_flag": 'validity-wrong',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Subject field",
        "test_name": 'Subject-Missing',
        "error_flag": 'subject-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject doesn't have MatterId attribute",
        "test_name": 'Subject-MatterId-Missing',
        "error_flag": 'subject-matter-id-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": True,
    },
    {
        "description": "Subject's NodeId value is invalid",
        "test_name": 'Subject-NodeId-Invalid',
        "error_flag": 'subject-node-id-invalid',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject includes two MatterId attributes",
        "test_name": 'Subject-MatterId-Twice',
        "error_flag": 'subject-matter-id-twice',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": True,
    },
    {
        "description": "Subject doesn't have FabricId field",
        "test_name": 'Subject-FabricId-Missing',
        "error_flag": 'subject-fabric-id-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": True,
    },
    {
        "description": "Subject's FabricId value is invalid",
        "test_name": 'Subject-FabricId-Invalid',
        "error_flag": 'subject-fabric-id-invalid',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject includes two FabricId attributes",
        "test_name": 'Subject-FabricId-Twice',
        "error_flag": 'subject-fabric-id-twice',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": True,
    },
    {
        "description": "Subject's FabricId doesn't match Issuer's FabricId value",
        "test_name": 'Subject-FabricId-Mismatch',
        "error_flag": 'subject-fabric-id-mismatch',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject's CAT value is invalid",
        "test_name": 'Subject-CAT-Invalid',
        "error_flag": 'subject-cat-invalid',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject includes two CAT attributes with the same CAT Values but different Versions",
        "test_name": 'Subject-CAT-Twice',
        "error_flag": 'subject-cat-twice',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": True,
    },
    {
        "description": "Invalid certificate public key curve secp256k1",
        "test_name": 'Sig-Curve-Secp256k1',
        "error_flag": 'sig-curve',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Error injected into one of the bytes of the public key",
        "test_name": 'PublicKey-Wrong',
        "error_flag": 'publickey',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Basic Constraint extension",
        "test_name": 'Ext-Basic-Missing',
        "error_flag": 'ext-basic-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension CA field is missing",
        "test_name": 'Ext-Basic-CA-Missing',
        "error_flag": 'ext-basic-ca-missing',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension CA field is wrong (TRUE for NOC and FALSE for ICAC/RCAC)",
        "test_name": 'Ext-Basic-CA-Wrong',
        "error_flag": 'ext-basic-ca-wrong',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field presence is wrong (present for NOC)",
        "test_name": 'Ext-Basic-PathLen-Presence-Wrong',
        "error_flag": 'ext-basic-pathlen-presence-wrong',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 0",
        "test_name": 'Ext-Basic-PathLen0',
        "error_flag": 'ext-basic-pathlen0',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 1",
        "test_name": 'Ext-Basic-PathLen1',
        "error_flag": 'ext-basic-pathlen1',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 2",
        "test_name": 'Ext-Basic-PathLen2',
        "error_flag": 'ext-basic-pathlen2',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Key Usage extension",
        "test_name": 'Ext-KeyUsage-Missing',
        "error_flag": 'ext-key-usage-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Key Usage extension digitalSignature field is wrong "
        "(not present for NOC and present for ICAC/RCAC)",
        "test_name": 'Ext-KeyUsage-DigSig-Wrong',
        "error_flag": 'ext-key-usage-dig-sig',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Key Usage extension keyCertSign field is wrong (present for NOC and not present for ICAC/RCAC)",
        "test_name": 'Ext-KeyUsage-KeyCertSign-Wrong',
        "error_flag": 'ext-key-usage-key-cert-sign',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate Key Usage extension cRLSign field is wrong (present for NOC and not present for ICAC/RCAC)",
        "test_name": 'Ext-KeyUsage-CRLSign-Wrong',
        "error_flag": 'ext-key-usage-crl-sign',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Authority Key ID (AKID) extension",
        "test_name": 'Ext-AKID-Missing',
        "error_flag": 'ext-akid-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Authority Key ID (AKID) extension length is 19 bytes instead of required 20",
        "test_name": 'Ext-AKID-Length-Invalid',
        "error_flag": 'ext-akid-len-invalid',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate doesn't include Subject Key ID (SKID) extension",
        "test_name": 'Ext-SKID-Missing',
        "error_flag": 'ext-skid-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Subject Key ID (SKID) extension length is 19 bytes instead of required 20",
        "test_name": 'Ext-SKID-Length-Invalid',
        "error_flag": 'ext-skid-len-invalid',
        "is_chip_to_x509_expected_to_fail": True,
        "is_chip_cert_load_expected_to_fail": True,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Certificate includes optional Extended Key Usage extension",
        "test_name": 'Ext-ExtendedKeyUsage-Missing',
        "error_flag": 'ext-extended-key-usage-missing',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": False,
        "is_get_cert_type_expected_to_fail": False,
    },
    {
        "description": "Error injected into one of the bytes of the signature",
        "test_name": 'Signature-Wrong',
        "error_flag": 'signature',
        "is_chip_to_x509_expected_to_fail": False,
        "is_chip_cert_load_expected_to_fail": False,
        "is_validate_chip_rcac_expected_to_fail": True,
        "is_get_cert_type_expected_to_fail": False,
    },
]


def cert_type_as_str(cert_type: CertType) -> str:
    type_strs = {CertType.NOC: 'NOC',
                 CertType.ICAC: 'ICAC',
                 CertType.RCAC: 'RCAC'}
    return type_strs[cert_type]


def headers(filepath: str) -> tuple[str, str]:
    """Returns strings with the headers for the output C-formated (.h and .cpp) files"""
    filename = filepath.split("/")[-1]
    namespaces = 'namespace chip {\nnamespace TestCerts {\n\n'
    h_top = copyrightNotice + '\n#pragma once\n\n#include <lib/support/Span.h>\n\n' + namespaces
    c_top = copyrightNotice + '\n#include "' + filename + '.h"\n\n' + namespaces
    return [h_top, c_top]


def footer() -> str:
    """Returns a string with the footer for the output C-formated (.h and .cpp) files"""
    return '\n} // namespace TestCerts\n} // namespace chip\n'


def full_cert_arrays(cert_file: str, array_name: str) -> tuple[str, str]:
    """Returns the certificate byte arrays and declarations"""
    with open(cert_file, "rb") as infile:
        b = infile.read()
        cert = make_c_array(b, array_name, 21)
        header = make_array_header(b, array_name)
    return [header, cert]


def start_test_cases_array(array_name: str, array_size: int) -> tuple[str, str]:
    """Returns strings with the headers for the test cases array"""
    h = 'extern const ByteSpan %s[%d];\n\n' % (array_name, array_size)
    c = 'const ByteSpan %s[%d] = {\n' % (array_name, array_size)
    return [h, c]


class Names:
    def __init__(self, cert_type: CertType, cert_form: CertFormat, test_case_out_dir: str, test_name: str):
        files_path_prefix = test_case_out_dir + '/Chip-Test-' + cert_type_as_str(cert_type) + '-' + test_name
        cert_array_name_prefix = 'sChipTest_' + cert_type_as_str(cert_type) + '_' + test_name.replace("-", "_") + '_Cert'

        if cert_form == CertFormat.DER:
            self.cert_file_name = files_path_prefix + '-Cert.der'
            self.key_file_name = files_path_prefix + '-Key.der'
            self.cert_array_name = cert_array_name_prefix + '_DER'
        else:
            self.cert_file_name = files_path_prefix + '-Cert.chip'
            self.key_file_name = files_path_prefix + '-Key.chip'
            self.cert_array_name = cert_array_name_prefix + '_CHIP'


class OpCertBuilder:
    def __init__(self, cert_type: CertType, cert_form: CertFormat, signer_cert: str, signer_key: str, error_type: str,
                 test_name: str, test_case_out_dir: str, chip_cert: str):
        self.cert_type = cert_type
        self.cert_form = cert_form
        self.error_type = error_type
        self.chipcert = chip_cert
        self.signer_cert = signer_cert
        self.signer_key = signer_key
        self.own = Names(cert_type, cert_form, test_case_out_dir, test_name)

    def make_certs_and_keys(self) -> None:
        """Creates the PEM and DER certs and keyfiles"""
        error_type_flag = ' -I -E ' + self.error_type
        validity_flags = ' -V "2020-10-15 14:23:43" -l 7305 '

        if self.cert_type == CertType.NOC:
            type_flag = ' -t n '
            suject_id_flags = ' -i DEDEDEDE00010001 -f FAB000000000001D '
            signer_key_and_cert = ' -K ' + self.signer_key + ' -C ' + self.signer_cert
        elif self.cert_type == CertType.ICAC:
            type_flag = ' -t c '
            suject_id_flags = ' -i CACACACA00000003 -f FAB000000000001D '
            signer_key_and_cert = ' -K ' + self.signer_key + ' -C ' + self.signer_cert
        else:
            type_flag = ' -t r '
            suject_id_flags = ' -i CACACACA00000001 '
            if self.error_type == 'subject-fabric-id-invalid' or self.error_type == 'subject-fabric-id-twice':
                suject_id_flags = ' -f FAB000000000001D '
            signer_key_and_cert = ' '

        if self.cert_form == CertFormat.DER:
            format_flag = ' -F x509-der '
        else:
            format_flag = ' -F chip '

        cmd = 'echo ' + self.own.cert_file_name
        subprocess.run(cmd, shell=True)

        # Generate privatkey/certificate in DER or CHIP TLV format
        cmd = self.chipcert + ' gen-cert ' + type_flag + error_type_flag + suject_id_flags + signer_key_and_cert \
            + validity_flags + format_flag + ' -o ' + self.own.cert_file_name + ' -O ' + self.own.key_file_name
        subprocess.run(cmd, shell=True)

    def full_arrays(self) -> tuple[str, str]:
        """Returns DER and CHIP TLV certificate byte arrays and declarations"""
        return full_cert_arrays(self.own.cert_file_name, self.own.cert_array_name)

    def add_cert_to_error_cases(self) -> str:
        """Returns a string with the new entry to the test cases array"""
        return '    ByteSpan(' + self.own.cert_array_name + '),\n'


def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-t', '--test_dir', dest='testdir',
                           default='credentials/test/operational-certificates-error-cases',
                           help='Output directory for all generated test vectors')
    argparser.add_argument('-o', '--out_dir', dest='outfile',
                           default='src/credentials/tests/CHIPCert_error_test_vectors',
                           help='Output file for all C-Style arrays')
    argparser.add_argument('-r', '--rcac', dest='rcacpath',
                           default='credentials/test/operational-certificates/Chip-Test-Root01-',
                           help='Valid RCAC to generate invalid ICACs')
    argparser.add_argument('-i', '--icac', dest='icacpath',
                           default='credentials/test/operational-certificates/Chip-Test-ICA02-',
                           help='Valid ICAC to generate invalid NOCs')
    argparser.add_argument('-c', '--chip-cert-dir', dest='chipcertdir',
                           default='out/debug/linux_x64_clang/', help='Directory where chip-cert tool is located')

    args = argparser.parse_args()

    chipcert = args.chipcertdir + 'chip-cert'
    rcac_cert = args.rcacpath + 'Cert.pem'
    rcac_key = args.rcacpath + 'Key.pem'
    icac_cert = args.icacpath + 'Cert.pem'
    icac_key = args.icacpath + 'Key.pem'

    if not os.path.exists(args.testdir):
        os.mkdir(args.testdir)

    if not os.path.exists(chipcert):
        raise Exception('Path not found: %s' % chipcert)

    if not os.path.exists(rcac_cert):
        raise Exception('Path not found: %s' % rcac_cert)

    if not os.path.exists(rcac_key):
        raise Exception('Path not found: %s' % rcac_key)

    if not os.path.exists(icac_cert):
        raise Exception('Path not found: %s' % icac_cert)

    if not os.path.exists(icac_key):
        raise Exception('Path not found: %s' % icac_key)

    with open(args.outfile + '.h', "w") as hfile:
        with open(args.outfile + '.cpp', "w") as cfile:
            h_cert_arrays_declarations = ''
            c_cert_arrays_definitions = ''

            c_x509_to_chip_error_cases = ''
            x509_to_chip_error_cases_count = 0
            for test_case in DER_CERT_ERROR_TEST_CASES:
                for cert_type in [CertType.NOC, CertType.ICAC, CertType.RCAC]:
                    # The following error cases are applicable only for NOC
                    if (test_case["error_flag"] == 'subject-node-id-invalid' or
                            test_case["error_flag"] == 'ext-basic-pathlen-presence-wrong') and cert_type != CertType.NOC:
                        break

                    if cert_type == CertType.NOC:
                        signer_cert = icac_cert
                        signer_key = icac_key
                    elif cert_type == CertType.ICAC:
                        signer_cert = rcac_cert
                        signer_key = rcac_key
                    else:
                        signer_cert = ""
                        signer_key = ""

                    # Generate Cert/Key
                    builder = OpCertBuilder(cert_type, CertFormat.DER, signer_cert, signer_key,
                                            test_case["error_flag"], test_case["test_name"], args.testdir, chipcert)
                    builder.make_certs_and_keys()

                    c_x509_to_chip_error_cases += builder.add_cert_to_error_cases()

                    [h, c] = builder.full_arrays()
                    h_cert_arrays_declarations += h
                    c_cert_arrays_definitions += c
                    x509_to_chip_error_cases_count += 1

            c_chip_to_x509_error_cases = ''
            c_chip_cert_load_error_cases = ''
            c_validate_chip_rcac_error_cases = ''
            c_get_cert_type_error_cases = ''
            chip_to_x509_error_cases_count = 0
            chip_cert_load_error_cases_count = 0
            validate_chip_rcac_error_cases_count = 0
            get_cert_type_error_cases_count = 0
            for test_case in CHIP_TLV_CERT_ERROR_TEST_CASES:
                for cert_type in [CertType.NOC, CertType.ICAC, CertType.RCAC]:
                    if not (test_case["is_chip_to_x509_expected_to_fail"] or
                            test_case["is_chip_cert_load_expected_to_fail"] or
                            test_case["is_validate_chip_rcac_expected_to_fail"] or
                            test_case["is_get_cert_type_expected_to_fail"]):
                        break

                    # The following error cases are applicable only for NOC
                    if (test_case["error_flag"] == 'subject-node-id-invalid'
                            or test_case["error_flag"] == 'subject-fabric-id-missing') and cert_type != CertType.NOC:
                        break

                    if cert_type == CertType.NOC:
                        signer_cert = icac_cert
                        signer_key = icac_key
                    elif cert_type == CertType.ICAC:
                        signer_cert = rcac_cert
                        signer_key = rcac_key
                    else:
                        signer_cert = ""
                        signer_key = ""

                    # Generate Cert/Key
                    builder = OpCertBuilder(cert_type, CertFormat.CHIP, signer_cert, signer_key,
                                            test_case["error_flag"], test_case["test_name"], args.testdir, chipcert)
                    builder.make_certs_and_keys()

                    if test_case["is_chip_to_x509_expected_to_fail"]:
                        c_chip_to_x509_error_cases += builder.add_cert_to_error_cases()
                        chip_to_x509_error_cases_count += 1
                    if test_case["is_chip_cert_load_expected_to_fail"]:
                        c_chip_cert_load_error_cases += builder.add_cert_to_error_cases()
                        chip_cert_load_error_cases_count += 1
                    if test_case["is_validate_chip_rcac_expected_to_fail"]:
                        c_validate_chip_rcac_error_cases += builder.add_cert_to_error_cases()
                        validate_chip_rcac_error_cases_count += 1
                    if test_case["is_get_cert_type_expected_to_fail"] and not (test_case["error_flag"] == 'subject-cat-twice'
                                                                               and cert_type == CertType.NOC):
                        c_get_cert_type_error_cases += builder.add_cert_to_error_cases()
                        get_cert_type_error_cases_count += 1

                    [h, c] = builder.full_arrays()
                    h_cert_arrays_declarations += h
                    c_cert_arrays_definitions += c

            [h_top, c_top] = headers(args.outfile)
            [h_x509_to_chip_error_cases, c_x509_to_chip_error_cases_head] = start_test_cases_array(
                'gTestCert_X509ToChip_ErrorCases', x509_to_chip_error_cases_count)
            [h_chip_to_x509_error_cases, c_chip_to_x509_error_cases_head] = start_test_cases_array(
                'gTestCert_ChipToX509_ErrorCases', chip_to_x509_error_cases_count)
            [h_chip_cert_load_error_cases, c_chip_cert_load_error_cases_head] = start_test_cases_array(
                'gTestCert_ChipCertLoad_ErrorCases', chip_cert_load_error_cases_count)
            [h_validate_chip_rcac_error_cases, c_validate_chip_rcac_error_cases_head] = start_test_cases_array(
                'gTestCert_ValidateChipRCAC_ErrorCases', validate_chip_rcac_error_cases_count)
            [h_get_cert_type_error_cases, c_get_cert_type_error_cases_head] = start_test_cases_array(
                'gTestCert_GetCertType_ErrorCases', get_cert_type_error_cases_count)
            foot = footer()

            hfile.write(h_top)
            hfile.write(h_x509_to_chip_error_cases)
            hfile.write(h_chip_to_x509_error_cases)
            hfile.write(h_chip_cert_load_error_cases)
            hfile.write(h_validate_chip_rcac_error_cases)
            hfile.write(h_get_cert_type_error_cases)
            hfile.write(h_cert_arrays_declarations)
            hfile.write(foot)

            cfile.write(c_top)
            cfile.write(c_x509_to_chip_error_cases_head + c_x509_to_chip_error_cases + '};\n\n')
            cfile.write(c_chip_to_x509_error_cases_head + c_chip_to_x509_error_cases + '};\n\n')
            cfile.write(c_chip_cert_load_error_cases_head + c_chip_cert_load_error_cases + '};\n\n')
            cfile.write(c_validate_chip_rcac_error_cases_head + c_validate_chip_rcac_error_cases + '};\n\n')
            cfile.write(c_get_cert_type_error_cases_head + c_get_cert_type_error_cases + '};\n\n')
            cfile.write(c_cert_arrays_definitions)
            cfile.write(foot)


if __name__ == '__main__':
    sys.exit(main())
