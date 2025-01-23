#!/usr/bin/env python3

#
# Copyright (c) 2023-2024 Project CHIP Authors
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

# Generates a basic RevocationSet from TestNet
# Usage:
#     python ./credentials/generate-revocation-set.py --help

import base64
import json
import logging
import os
import subprocess
import sys
import unittest
from enum import Enum
from typing import Optional

import click
import requests
from click_option_group import RequiredMutuallyExclusiveOptionGroup, optgroup
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


class RevocationType(Enum):
    CRL = 1


OID_VENDOR_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.1")
OID_PRODUCT_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.2")

PRODUCTION_NODE_URL = "https://on.dcl.csa-iot.org:26657"
PRODUCTION_NODE_URL_REST = "https://on.dcl.csa-iot.org"
TEST_NODE_URL_REST = "https://on.test-net.dcl.csa-iot.org"


def extract_single_integer_attribute(subject, oid):
    attribute_list = subject.get_attributes_for_oid(oid)

    if len(attribute_list) == 1:
        return int(attribute_list[0].value, 16)

    return None


def extract_fallback_tag_from_common_name(cn, marker):
    val_len = 4
    start_idx = cn.find(marker)

    if start_idx != -1:
        val_start_idx = start_idx + len(marker)
        val = cn[val_start_idx:val_start_idx + val_len]
        return int(val, 16) if len(val) == 4 else None

    return None


def parse_vid_pid_from_distinguished_name(distinguished_name):
    # VID/PID encoded using Matter specific RDNs
    vid = extract_single_integer_attribute(distinguished_name, OID_VENDOR_ID)
    pid = extract_single_integer_attribute(distinguished_name, OID_PRODUCT_ID)

    # Fallback method to get the VID/PID, encoded in CN as "Mvid:FFFF Mpid:1234"
    if vid is None and pid is None:
        cn = distinguished_name.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value
        vid = extract_fallback_tag_from_common_name(cn, 'Mvid:')
        pid = extract_fallback_tag_from_common_name(cn, 'Mpid:')

    return vid, pid


def get_akid(cert: x509.Certificate) -> Optional[bytes]:
    try:
        return cert.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier
    except Exception:
        logging.warning("AKID not found in certificate")
        return None


def get_skid(cert: x509.Certificate) -> Optional[bytes]:
    try:
        return cert.extensions.get_extension_for_oid(x509.OID_SUBJECT_KEY_IDENTIFIER).value.key_identifier
    except Exception:
        logging.warning("SKID not found in certificate")
        return None


def get_subject_b64(cert: x509.Certificate) -> str:
    return base64.b64encode(cert.subject.public_bytes()).decode('utf-8')


def get_issuer_b64(cert: x509.Certificate) -> str:
    return base64.b64encode(cert.issuer.public_bytes()).decode('utf-8')


def verify_cert(cert: x509.Certificate, root: x509.Certificate) -> bool:
    '''
    Verifies if the cert is signed by root.
    '''

    cert_akid = get_akid(cert)
    root_skid = get_skid(root)
    if cert_akid is None or root_skid is None or cert_akid != root_skid:
        return False

    if cert.issuer != root.subject:
        return False

    # public_key().verify() do not return anything if signature is valid,
    # will raise an exception if signature is invalid
    try:
        root.public_key().verify(cert.signature, cert.tbs_certificate_bytes, ec.ECDSA(cert.signature_hash_algorithm))
    except Exception:
        logging.warning(f"Signature verification failed for cert subject: {get_subject_b64(cert)}, issuer: {get_issuer_b64(cert)}")
        return False

    return True


def is_self_signed_certificate(cert: x509.Certificate) -> bool:
    return verify_cert(cert, cert)


# delegator is optional so can be None, but crl_signer and paa has to be present
def validate_cert_chain(crl_signer: x509.Certificate, crl_signer_delegator: x509.Certificate, paa: x509.Certificate):
    '''
    There could be four scenarios:
    1. CRL Signer is PAA itself, hence its self-signed certificate
    2. CRL Signer is PAI certificate, and we can validate (crl_signer -> paa) chain
    3. CRL Signer delegator is PAA, and we can validate (crl_signer -> crl_signer_delegator(paa) -> paa) chain
    4. CRL Signer delegator is PAI, and we can validate (crl_signer -> crl_signer_delegator -> paa) chain
    '''

    if crl_signer_delegator:
        return verify_cert(crl_signer, crl_signer_delegator) and verify_cert(crl_signer_delegator, paa)
    else:
        return verify_cert(crl_signer, paa)


def validate_vid_pid(revocation_point: dict, crl_signer_certificate: x509.Certificate, crl_signer_delegator_certificate: x509.Certificate) -> bool:
    crl_signer_vid, crl_signer_pid = parse_vid_pid_from_distinguished_name(crl_signer_certificate.subject)

    if revocation_point["isPAA"]:
        if crl_signer_vid is not None:
            if revocation_point["vid"] != crl_signer_vid:
                logging.warning("VID in CRL Signer Certificate does not match with VID in revocation point, continue...")
                return False
    else:
        vid_to_match = crl_signer_vid
        pid_to_match = crl_signer_pid

        # if the CRL Signer is delegated then match the VID and PID of the CRL Signer Delegator
        if crl_signer_delegator_certificate:
            vid_to_match, pid_to_match = parse_vid_pid_from_distinguished_name(crl_signer_delegator_certificate.subject)

        if vid_to_match is None or revocation_point["vid"] != vid_to_match:
            logging.warning("VID in CRL Signer Certificate does not match with VID in revocation point, continue...")
            return False

        if pid_to_match is not None:
            if revocation_point["pid"] != pid_to_match:
                logging.warning("PID in CRL Signer Certificate does not match with PID in revocation point, continue...")
                return False

    return True


def fetch_crl_from_url(url: str, timeout: int) -> x509.CertificateRevocationList:
    logging.debug(f"Fetching CRL from {url}")

    try:
        r = requests.get(url, timeout=timeout)
        return x509.load_der_x509_crl(r.content)
    except Exception:
        logging.error('Failed to fetch a valid CRL')


def generate_revocation_set_from_crl(crl_file: x509.CertificateRevocationList,
                                     crl_signer_certificate: x509.Certificate,
                                     certificate_authority_name_b64: str,
                                     certificate_akid_hex: str,
                                     crl_signer_delegator_cert: x509.Certificate) -> dict:
    """Generate a revocation set from a CRL file.

    Args:
        crl_file: The CRL object containing revoked certificates
        crl_signer_certificate: The certificate object used to sign the CRL
        certificate_authority_name_b64: Base64 encoded issuer name
        certificate_akid_hex: Hex encoded Authority Key Identifier
        crl_signer_delegator_cert: crl signer delegator certificate object

    Returns:
        dict: A dictionary containing the revocation set data with fields:
            - type: "revocation_set"
            - issuer_subject_key_id: Authority Key Identifier (hex)
            - issuer_name: Issuer name (base64)
            - revoked_serial_numbers: List of revoked serial numbers
            - crl_signer_cert: CRL signer certificate (base64 DER)
            - crl_signer_delegator: Optional delegator certificate (base64 DER)
    """
    serialnumber_list = []

    for revoked_cert in crl_file:
        try:
            cert_issuer_entry_ext = revoked_cert.extensions.get_extension_for_oid(x509.CRLEntryExtensionOID.CERTIFICATE_ISSUER)
            revoked_cert_issuer = cert_issuer_entry_ext.value.get_values_for_type(x509.DirectoryName)[0].public_bytes()
            revoked_cert_issuer_b64 = base64.b64encode(revoked_cert_issuer).decode('utf-8')

            if revoked_cert_issuer_b64 is not None:
                # check if this really are the same thing
                if revoked_cert_issuer_b64 != certificate_authority_name_b64:
                    logging.warning("CRL Issuer is not CRL File Issuer, continue...")
                    continue
        except Exception:
            pass

        # Ensure the serial number is always a 2-byte aligned hex string.
        # TestDACRevocationDelegateImpl encodes the serial number as an even-length hex string
        # using BytesToHex in src/lib/support/BytesToHex.cpp.
        # As the primary consumer of this data, we should use the same here.
        serialnumber = '{:02X}'.format(revoked_cert.serial_number)
        serialnumber = serialnumber if len(serialnumber) % 2 == 0 else '0' + serialnumber
        serialnumber_list.append(serialnumber)

    entry = {
        "type": "revocation_set",
        "issuer_subject_key_id": certificate_akid_hex,
        "issuer_name": certificate_authority_name_b64,
        "revoked_serial_numbers": serialnumber_list,
        "crl_signer_cert": base64.b64encode(crl_signer_certificate.public_bytes(serialization.Encoding.DER)).decode('utf-8'),
    }

    if crl_signer_delegator_cert:
        entry["crl_signer_delegator"] = base64.b64encode(
            crl_signer_delegator_cert.public_bytes(serialization.Encoding.DER)).decode('utf-8')

    return entry


# This is implemented as per point (9) in 6.2.4.1. Conceptual algorithm for revocation set construction
def get_certificate_authority_details(crl_signer_certificate: x509.Certificate,
                                      crl_signer_delegator_cert: x509.Certificate,
                                      paa_certificate_object: x509.Certificate,
                                      is_paa: bool) -> tuple[str, str]:
    """Get certificate authority name and AKID based on certificate hierarchy.

    Args:
        crl_signer_certificate: The CRL signer certificate
        crl_signer_delegator_cert: Optional delegator certificate
        paa_certificate_object: Optional PAA certificate
        is_paa: Whether this is a PAA certificate

    Returns:
        tuple[str, str]: (certificate_authority_name_b64, certificate_akid_hex)
    """
    if is_paa and not is_self_signed_certificate(crl_signer_certificate):
        cert_for_details = paa_certificate_object
        logging.debug("Using PAA certificate for details")
    elif crl_signer_delegator_cert:
        cert_for_details = crl_signer_delegator_cert
        logging.debug("Using CRL Signer Delegator certificate for details")
    else:
        cert_for_details = crl_signer_certificate
        logging.debug("Using CRL Signer certificate for details")

    certificate_authority_name_b64 = get_subject_b64(cert_for_details)
    certificate_akid = get_skid(cert_for_details)
    certificate_akid_hex = ''.join('{:02X}'.format(x) for x in certificate_akid)

    logging.debug(f"Certificate Authority Name: {certificate_authority_name_b64}")
    logging.debug(f"Certificate AKID: {certificate_akid_hex}")

    return certificate_authority_name_b64, certificate_akid_hex


class DCLDClient:
    '''
    A client for interacting with DCLD using either the REST API or command line interface (CLI).

    '''

    def __init__(self, use_rest: bool, dcld_exe: str, production: bool, rest_node_url: str):
        '''
        Initialize the client

        use_rest: bool
            Use RESTful API with HTTPS against `rest_node_url`
        dcld_exe: str
            Path to `dcld` executable
        production: bool
            Use MainNet DCL URL with dcld executable
        rest_node_url: str
            RESTful API URL
        '''

        self.use_rest = use_rest
        self.dcld_exe = dcld_exe
        self.production = production
        self.rest_node_url = rest_node_url

    def build_dcld_command_line(self, cmdlist: list[str]) -> list[str]:
        '''
        Build command line for `dcld` executable.

        Parameters
        ----------
        cmdlist: list[str]
            List of command line arguments to append to some predefined arguments

        Returns
        -------
        list[str]
            The complete command list including the DCLD executable and node option if in production
        '''

        return [self.dcld_exe] + cmdlist + (['--node', PRODUCTION_NODE_URL] if self.production else [])

    def get_dcld_cmd_output_json(self, cmdlist: list[str]) -> dict:
        '''
        Executes a DCLD CLI command and returns the JSON output.

        Parameters
        ----------
        cmdlist: list[str]
            List of command line arguments to append to some predefined arguments

        Returns
        -------
        dict
            The JSON output from the command
        '''

        # Set the output as JSON
        subprocess.Popen([self.dcld_exe, 'config', 'output', 'json'])

        cmdpipe = subprocess.Popen(self.build_dcld_command_line(cmdlist),
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return json.loads(cmdpipe.stdout.read())

    def get_revocation_points(self) -> list[dict]:
        '''
        Get revocation points from DCL

        Returns
        -------
        list[dict]
            List of revocation points
        '''

        if self.use_rest:
            response = requests.get(f"{self.rest_node_url}/dcl/pki/revocation-points").json()
        else:
            response = self.get_dcld_cmd_output_json(['query', 'pki', 'all-revocation-points'])

        return response["PkiRevocationDistributionPoint"]

    def get_issuer_cert(self, cert: x509.Certificate) -> Optional[x509.Certificate]:
        '''
        Get the issuer certificate for

        Parameters
        ----------
        cert: x509.Certificate
            Certificate

        Returns
        -------
        str
            Issuer certificate in PEM format
        '''
        issuer_name_b64 = get_issuer_b64(cert)
        akid = get_akid(cert)
        if akid is None:
            return

        # Convert CRL Signer AKID to colon separated hex
        akid_hex = akid.hex().upper()
        akid_hex = ':'.join([akid_hex[i:i+2] for i in range(0, len(akid_hex), 2)])

        logging.debug(
            f"Fetching issuer from:{self.rest_node_url}/dcl/pki/certificates/{issuer_name_b64}/{akid_hex}")

        if self.use_rest:
            response = requests.get(
                f"{self.rest_node_url}/dcl/pki/certificates/{issuer_name_b64}/{akid_hex}").json()
        else:
            response = self.get_dcld_cmd_output_json(
                ['query', 'pki', 'x509-cert', '-u', issuer_name_b64, '-k', akid_hex])

        issuer_certificate = response["approvedCertificates"]["certs"][0]["pemCert"]

        logging.debug(f"issuer: {issuer_certificate}")

        try:
            issuer_certificate_object = x509.load_pem_x509_certificate(bytes(issuer_certificate, 'utf-8'))
        except Exception:
            logging.error('Failed to parse PAA certificate')
            return

        return issuer_certificate_object

    def get_revocations_points_by_skid(self, issuer_subject_key_id: str) -> list[dict]:
        '''
        Get revocation points by subject key ID

        Parameters
        ----------
        issuer_subject_key_id: str
            Subject key ID

        Returns
        -------
        list[dict]
            List of revocation points
        '''

        if self.use_rest:
            response = requests.get(f"{self.rest_node_url}/dcl/pki/revocation-points/{issuer_subject_key_id}").json()
        else:
            response = self.get_dcld_cmd_output_json(['query', 'pki', 'revocation-points',
                                                      '--issuer-subject-key-id', issuer_subject_key_id])

        return response["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]["points"]


@click.group()
def cli():
    pass


@cli.command('from-dcl')
@click.help_option('-h', '--help')
@optgroup.group('Input data sources', cls=RequiredMutuallyExclusiveOptionGroup)
@optgroup.option('--use-main-net-dcld', type=str, default='', metavar='PATH', help="Location of `dcld` binary, to use `dcld` for mirroring MainNet.")
@optgroup.option('--use-test-net-dcld', type=str, default='', metavar='PATH', help="Location of `dcld` binary, to use `dcld` for mirroring TestNet.")
@optgroup.option('--use-main-net-http', is_flag=True, type=str, help="Use RESTful API with HTTPS against public MainNet observer.")
@optgroup.option('--use-test-net-http', is_flag=True, type=str, help="Use RESTful API with HTTPS against public TestNet observer.")
@optgroup.group('Optional arguments')
@optgroup.option('--output', default='sample_revocation_set_list.json', type=str, metavar='FILEPATH',
                 help="Output filename (default: sample_revocation_set_list.json)")
@optgroup.option('--log-level', default='INFO', show_default=True, type=click.Choice(__LOG_LEVELS__.keys(),
                                                                                     case_sensitive=False), callback=lambda c, p, v: __LOG_LEVELS__[v],
                 help='Determines the verbosity of script output')
def from_dcl(use_main_net_dcld, use_test_net_dcld, use_main_net_http, use_test_net_http, output, log_level):
    """Generate revocation set from DCL"""
    logging.basicConfig(
        level=log_level,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

    production = False
    dcld = use_test_net_dcld

    if len(use_main_net_dcld) > 0:
        dcld = use_main_net_dcld
        production = True

    use_rest = use_main_net_http or use_test_net_http
    if use_main_net_http:
        production = True

    rest_node_url = PRODUCTION_NODE_URL_REST if production else TEST_NODE_URL_REST

    dcld_client = DCLDClient(use_rest, dcld, production, rest_node_url)

    revocation_point_list = dcld_client.get_revocation_points()

    revocation_set = []

    for revocation_point in revocation_point_list:
        # 1. Validate Revocation Type
        if revocation_point["revocationType"] != RevocationType.CRL.value:
            logging.warning("Revocation Type is not CRL, continue...")
            continue

        # 2. Parse the certificate
        try:
            crl_signer_certificate = x509.load_pem_x509_certificate(bytes(revocation_point["crlSignerCertificate"], 'utf-8'))
        except Exception:
            logging.warning("CRL Signer Certificate is not valid, continue...")
            continue

        # Parse the crl signer delegator
        crl_signer_delegator_cert = None
        if "crlSignerDelegator" in revocation_point:
            crl_signer_delegator_cert_pem = revocation_point["crlSignerDelegator"]
            logging.debug(f"CRLSignerDelegator: {crl_signer_delegator_cert_pem}")
            try:
                crl_signer_delegator_cert = x509.load_pem_x509_certificate(bytes(crl_signer_delegator_cert_pem, 'utf-8'))
            except Exception:
                logging.warning("CRL Signer Delegator Certificate not found...")

        # 3. and 4. Validate VID/PID
        if not validate_vid_pid(revocation_point, crl_signer_certificate, crl_signer_delegator_cert):
            logging.warning("Failed to validate VID/PID, continue...")
            continue

        # 5. Validate the certification path containing CRLSignerCertificate.
        paa_certificate_object = dcld_client.get_issuer_cert(crl_signer_certificate)
        if paa_certificate_object is None:
            logging.warning("PAA Certificate not found, continue...")
            continue

        if validate_cert_chain(crl_signer_certificate, crl_signer_delegator_cert, paa_certificate_object) is False:
            logging.warning("Failed to validate CRL Signer Certificate chain, continue...")
            continue

        # 6. Obtain the CRL
        crl_file = fetch_crl_from_url(revocation_point["dataURL"], 5)  # timeout in seconds
        if crl_file is None:
            continue

        # 7. Perform CRL File Validation
        # a.
        crl_signer_skid = get_skid(crl_signer_certificate)
        crl_akid = get_akid(crl_file)
        if crl_akid != crl_signer_skid:
            logging.warning("CRL AKID is not CRL Signer SKID, continue...")
            continue

        crl_akid_hex = ''.join('{:02X}'.format(x) for x in crl_akid)

        # b.
        same_issuer_points = dcld_client.get_revocations_points_by_skid(crl_akid_hex)
        count_with_matching_vid_issuer_skid = sum(item.get('vid') == revocation_point["vid"] for item in same_issuer_points)

        if count_with_matching_vid_issuer_skid > 1:
            try:
                issuing_distribution_point = crl_file.extensions.get_extension_for_oid(
                    x509.OID_ISSUING_DISTRIBUTION_POINT).value
            except Exception:
                logging.warning("CRL Issuing Distribution Point not found, continue...")
                continue

            uri_list = issuing_distribution_point.full_name
            if len(uri_list) == 1 and isinstance(uri_list[0], x509.UniformResourceIdentifier):
                if uri_list[0].value != revocation_point["dataURL"]:
                    logging.warning("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                    continue
            else:
                logging.warning("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                continue

        # TODO: 8. Validate CRL as per Section 6.3 of RFC 5280

        # 9. decide on certificate authority name and AKID
        certificate_authority_name_b64, certificate_akid_hex = get_certificate_authority_details(
            crl_signer_certificate, crl_signer_delegator_cert, paa_certificate_object, revocation_point["isPAA"])

        # validate issuer skid matchces with the one in revocation points
        logging.debug(f"revocation_point['issuerSubjectKeyID']: {revocation_point['issuerSubjectKeyID']}")

        if revocation_point["issuerSubjectKeyID"] != certificate_akid_hex:
            logging.warning("CRL Issuer Subject Key ID is not CRL Signer Subject Key ID, continue...")
            continue

        # 10. Iterate through the Revoked Certificates List
        entry = generate_revocation_set_from_crl(crl_file, crl_signer_certificate,
                                                 certificate_authority_name_b64, certificate_akid_hex, crl_signer_delegator_cert)
        logging.debug(f"Entry to append: {entry}")
        revocation_set.append(entry)

    with open(output, 'w+') as outfile:
        json.dump(revocation_set, outfile, indent=4)


@cli.command('from-crl')
@click.option('--crl', required=True, type=click.File('rb'), help='Path to the CRL file')
@click.option('--crl-signer', required=True, type=click.File('rb'), help='Path to the signer certificate')
@click.option('--delegator', type=click.File('rb'), help='Path to the delegator certificate (optional)')
@click.option('--paa', type=click.File('rb'), help='Path to the PAA certificate (optional)')
@click.option('--output', default='revocation_set.json', type=click.File('w'), help='Output filename (default: revocation_set.json)')
@click.option('--is-paa', default=False, is_flag=True, help='Indicates if the CRL issuer is the PAA')
def from_crl(crl, crl_signer, delegator, paa, output, is_paa):
    """Generate revocation set from a single CRL file"""
    crl = x509.load_pem_x509_crl(crl.read())
    crl_signer = x509.load_pem_x509_certificate(crl_signer.read())
    delegator = x509.load_pem_x509_certificate(delegator.read()) if delegator else None
    paa = x509.load_pem_x509_certificate(paa.read()) if paa else None

    ca_name_b64, ca_akid_hex = get_certificate_authority_details(crl_signer, delegator, paa, is_paa)
    revocation_set = generate_revocation_set_from_crl(crl, crl_signer, ca_name_b64, ca_akid_hex, delegator)
    output.write(json.dumps([revocation_set], indent=4))


class TestRevocationSetGeneration(unittest.TestCase):
    """Test class for revocation set generation"""

    def setUp(self):
        # Get the directory containing this file
        self.test_base_dir = os.path.dirname(os.path.abspath(__file__))

    def get_test_file_path(self, filename):
        return os.path.join(self.test_base_dir, 'test', filename)

    def compare_revocation_sets(self, generated_set, expected_file):
        with open(os.path.join(self.test_base_dir, expected_file), 'r') as f:
            expected_set = json.load(f)

        # Compare the contents
        self.assertEqual(len([generated_set]), len(expected_set))
        expected = expected_set[0]

        # Compare required fields
        self.assertEqual(generated_set['type'], expected['type'])
        self.assertEqual(generated_set['issuer_subject_key_id'], expected['issuer_subject_key_id'])
        self.assertEqual(generated_set['issuer_name'], expected['issuer_name'])
        self.assertEqual(set(generated_set['revoked_serial_numbers']), set(expected['revoked_serial_numbers']))
        self.assertEqual(generated_set['crl_signer_cert'], expected['crl_signer_cert'])

        # Compare optional fields if present in either set
        if 'crl_signer_delegator' in generated_set and 'crl_signer_delegator' in expected:
            self.assertEqual(generated_set['crl_signer_delegator'], expected['crl_signer_delegator'],
                             "CRL signer delegator certificates do not match")
        elif 'crl_signer_delegator' in generated_set or 'crl_signer_delegator' in expected:
            self.fail("CRL signer delegator certificate is missing in one of the sets")

    def test_paa_revocation_set(self):
        """Test generation of PAA revocation set"""
        with open(self.get_test_file_path('revoked-attestation-certificates/Chip-Test-PAA-FFF1-CRL.pem'), 'rb') as f:
            crl = x509.load_pem_x509_crl(f.read())
        with open(self.get_test_file_path('revoked-attestation-certificates/Chip-Test-PAA-FFF1-Cert.pem'), 'rb') as f:
            crl_signer = x509.load_pem_x509_certificate(f.read())

        ca_name_b64, ca_akid_hex = get_certificate_authority_details(
            crl_signer, None, None, True)
        revocation_set = generate_revocation_set_from_crl(
            crl, crl_signer, ca_name_b64, ca_akid_hex, None)

        self.compare_revocation_sets(
            revocation_set,
            'test/revoked-attestation-certificates/revocation-sets/revocation-set-for-paa.json'
        )

    def test_pai_revocation_set(self):
        """Test generation of PAI revocation set"""
        with open(self.get_test_file_path('revoked-attestation-certificates/Matter-Development-PAI-FFF1-noPID-CRL.pem'), 'rb') as f:
            crl = x509.load_pem_x509_crl(f.read())
        with open(self.get_test_file_path('revoked-attestation-certificates/Matter-Development-PAI-FFF1-noPID-Cert.pem'), 'rb') as f:
            crl_signer = x509.load_pem_x509_certificate(f.read())
        with open(self.get_test_file_path('revoked-attestation-certificates/Chip-Test-PAA-FFF1-Cert.pem'), 'rb') as f:
            paa = x509.load_pem_x509_certificate(f.read())

        ca_name_b64, ca_akid_hex = get_certificate_authority_details(
            crl_signer, None, paa, False)
        revocation_set = generate_revocation_set_from_crl(
            crl, crl_signer, ca_name_b64, ca_akid_hex, None)

        self.compare_revocation_sets(
            revocation_set,
            'test/revoked-attestation-certificates/revocation-sets/revocation-set-for-pai.json'
        )


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == 'test':
        # Remove the 'test' argument and run tests
        sys.argv.pop(1)
        unittest.main()
    elif len(sys.argv) == 1:
        cli.main(['--help'])
    else:
        cli()
