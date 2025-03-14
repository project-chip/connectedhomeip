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
# Note: py cryptography supports indirect CRLs trom version 44.0.0 and above.
#       You may need to update your cryptography version.
#
# Generates a basic RevocationSet from TestNet
# Usage:
#     python ./credentials/generate-revocation-set.py --help

import base64
import dataclasses
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
from click_option_group import AllOptionGroup, RequiredMutuallyExclusiveOptionGroup, optgroup
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.extensions import ExtensionNotFound
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


class CertVerificationResult(Enum):
    SUCCESS = 1
    SKID_NOT_FOUND = 2
    AKID_NOT_FOUND = 3
    SIGNATURE_VERIFICATION_FAILED = 4
    ISSUER_MISMATCH = 5
    AKID_MISMATCH = 6


@dataclasses.dataclass
class RevocationPoint:
    vid: int
    label: str
    issuerSubjectKeyID: str
    pid: int
    isPAA: bool
    crlSignerCertificate: str
    dataURL: str
    dataFileSize: str
    dataDigest: str
    dataDigestType: int
    revocationType: int
    schemaVersion: int
    crlSignerDelegator: str


@dataclasses.dataclass
class RevocationSet:
    type: str
    issuer_subject_key_id: str
    issuer_name: str
    revoked_serial_numbers: [str]
    crl_signer_cert: str
    crl_signer_delegator: str = None

    def asDict(self):
        return dataclasses.asdict(self)


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


def get_akid(cert: x509.Certificate) -> str:
    return cert.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier.hex().upper()


def get_skid(cert: x509.Certificate) -> str:
    return cert.extensions.get_extension_for_oid(x509.OID_SUBJECT_KEY_IDENTIFIER).value.key_identifier.hex().upper()


def verify_cert(cert: x509.Certificate, root: x509.Certificate) -> CertVerificationResult:
    '''
    Verifies if the cert is signed by root.
    '''
    try:
        cert_akid = get_akid(cert)
    except ExtensionNotFound:
        return CertVerificationResult.AKID_NOT_FOUND
    try:
        root_skid = get_skid(root)
    except ExtensionNotFound:
        return CertVerificationResult.SKID_NOT_FOUND

    if cert_akid != root_skid:
        return CertVerificationResult.AKID_MISMATCH

    if cert.issuer != root.subject:
        return CertVerificationResult.ISSUER_MISMATCH

    # public_key().verify() do not return anything if signature is valid,
    # will raise an exception if signature is invalid
    try:
        root.public_key().verify(cert.signature, cert.tbs_certificate_bytes, ec.ECDSA(cert.signature_hash_algorithm))
    except Exception:
        return CertVerificationResult.SIGNATURE_VERIFICATION_FAILED

    return CertVerificationResult.SUCCESS


def is_self_signed_certificate(cert: x509.Certificate) -> bool:
    result = verify_cert(cert, cert)
    if result == CertVerificationResult.SUCCESS:
        return True
    else:
        logging.debug(
            f"Certificate with subject: {cert.subject.rfc4514_string()} is not a valid self-signed certificate. Result: {result.name}")
    return False


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
        result_signer = verify_cert(crl_signer, crl_signer_delegator)
        if not result_signer == CertVerificationResult.SUCCESS:
            logging.debug(
                f"Cannot verify certificate subject: {crl_signer.subject.rfc4514_string()} issued by certificate subject: {crl_signer_delegator.subject.rfc4514_string()}. Result: {result_signer.name}")
            return False

        result_delegator = verify_cert(crl_signer_delegator, paa)
        if not result_delegator == CertVerificationResult.SUCCESS:
            logging.debug(
                f"Cannot verify certificate subject: {crl_signer_delegator.subject.rfc4514_string()} issued by certificate subject: {paa.subject.rfc4514_string()}. Result: {result_delegator.name}")
            return False
        return True
    else:
        result = verify_cert(crl_signer, paa)
        if not result == CertVerificationResult.SUCCESS:
            logging.debug(
                f"Cannot verify certificate subject: {crl_signer.subject.rfc4514_string()} issued by certificate subject: {paa.subject.rfc4514_string()}. Result: {result.name}")
            return False
        return True


def validate_vid_pid(revocation_point: RevocationPoint, crl_signer_certificate: x509.Certificate, crl_signer_delegator_certificate: x509.Certificate) -> bool:
    crl_signer_vid, crl_signer_pid = parse_vid_pid_from_distinguished_name(crl_signer_certificate.subject)
    logging.debug(f"vid: {revocation_point.vid})")
    logging.debug(f"crl_signer_vid: {crl_signer_vid})")
    if revocation_point.isPAA:
        if crl_signer_vid is not None:
            if revocation_point.vid != crl_signer_vid:
                logging.warning("VID in CRL Signer Certificate does not match with VID in revocation point, continue...")
                return False
    else:
        vid_to_match = crl_signer_vid
        pid_to_match = crl_signer_pid

        # if the CRL Signer is delegated then match the VID and PID of the CRL Signer Delegator
        if crl_signer_delegator_certificate:
            vid_to_match, pid_to_match = parse_vid_pid_from_distinguished_name(crl_signer_delegator_certificate.subject)
        logging.debug(f"vid_to_match: {vid_to_match})")
        logging.debug(f"pid_to_match: {pid_to_match})")
        if vid_to_match is None or revocation_point.vid != vid_to_match:
            logging.warning("VID in CRL Signer Certificate does not match with VID in revocation point, continue...")
            return False

        if pid_to_match is not None:
            if revocation_point.pid != pid_to_match:
                logging.warning("PID in CRL Signer Certificate does not match with PID in revocation point, continue...")
                return False

    return True


def generate_revocation_set_from_crl(crl_file: x509.CertificateRevocationList,
                                     crl_signer_certificate: x509.Certificate,
                                     certificate_authority_name: x509.Name,
                                     certificate_akid_hex: str,
                                     crl_signer_delegator_cert: x509.Certificate) -> RevocationSet:
    """Generate a revocation set from a CRL file.

    Args:
        crl_file: The CRL object containing revoked certificates
        crl_signer_certificate: The certificate object used to sign the CRL
        certificate_authority_name: x509.Name of the issuer
        certificate_akid_hex: Hex encoded Authority Key Identifier
        crl_signer_delegator_cert: crl signer delegator certificate object

    Returns:
        RevocationSet containing the revocation set data with fields:
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
            revoked_cert_issuer = cert_issuer_entry_ext.value.get_values_for_type(x509.DirectoryName)[0]
            if revoked_cert_issuer is not None:
                # check if this really are the same thing
                if revoked_cert_issuer != x509.DirectoryName(certificate_authority_name).value:
                    logging.warning("CRL entry issuer is not CRL File Issuer, continue...")
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

    entry = RevocationSet(
        type='revocation_set',
        issuer_subject_key_id=certificate_akid_hex,
        issuer_name=get_b64_name(certificate_authority_name),
        revoked_serial_numbers=serialnumber_list,
        crl_signer_cert=base64.b64encode(crl_signer_certificate.public_bytes(serialization.Encoding.DER)).decode('utf-8'),
    )

    if crl_signer_delegator_cert:
        entry.crl_signer_delegator = base64.b64encode(
            crl_signer_delegator_cert.public_bytes(serialization.Encoding.DER)).decode('utf-8')

    return entry


# This is implemented as per point (9) in 6.2.4.1. Conceptual algorithm for revocation set construction
def get_certificate_authority_details(crl_signer_certificate: x509.Certificate,
                                      crl_signer_delegator_cert: x509.Certificate,
                                      paa_certificate_object: x509.Certificate,
                                      is_paa: bool) -> tuple[x509.Name, str]:
    """Get certificate authority name and AKID based on certificate hierarchy.

    Args:
        crl_signer_certificate: The CRL signer certificate
        crl_signer_delegator_cert: Optional delegator certificate
        paa_certificate_object: Optional PAA certificate
        is_paa: Whether this is a PAA certificate

    Returns:
        tuple[str, str]: (certificate_authority_name, certificate_akid_hex)
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

    certificate_authority_name = cert_for_details.subject
    try:
        certificate_akid = get_skid(cert_for_details)
        logging.debug(f"Certificate Authority Name: {certificate_authority_name}")
        logging.debug(f"Certificate AKID: {certificate_akid}")

        return certificate_authority_name, certificate_akid
    except ExtensionNotFound:
        logging.warning("Certificate SKID not found in authoarity certificate.")


def get_b64_name(name: x509.name.Name) -> str:
    '''
    Get base64 encoded name
    '''
    return base64.b64encode(name.public_bytes()).decode('utf-8')


def fetch_crl_from_url(url: str, timeout: int) -> x509.CertificateRevocationList:
    logging.debug(f"Fetching CRL from {url}")

    try:
        r = requests.get(url, timeout=timeout)
        logging.debug(f"Fetched CRL: {r.content}")
        return x509.load_der_x509_crl(r.content)
    except Exception as e:
        logging.error('Failed to fetch a valid CRL', e)


class DclClientInterface:
    '''
    An interface for interacting with DCLD.
    '''

    def send_get_request(self, url: str) -> dict:
        '''
        Send a GET request for a json object.
        '''
        try:
            response = requests.get(url).json()
            return response
        except Exception as e:
            logging.error(f"Failed to fetch {url}: {e}")
            return None

    def get_revocation_points(self) -> list[RevocationPoint]:
        '''
        Get revocation points from DCL

        Returns
        -------
        list[RevocationPoint]
            List of revocation points
        '''
        raise NotImplementedError

    def get_revocation_points_by_skid(self, issuer_subject_key_id) -> list[RevocationPoint]:
        '''
        Get revocation points by subject key ID

        Parameters
        ----------
        issuer_subject_key_id: str
            Subject key ID

        Returns
        -------
        list[RevocationPoint]
            List of revocation points
        '''
        raise NotImplementedError

    def get_approved_certificate(self, subject_name: x509.name.Name, skid_hex: str) -> tuple[bool, x509.Certificate]:
        '''
        Get certificate from DCL.
        '''
        raise NotImplementedError

    def get_only_approved_certificate(self, response: dict, skid_hex: str) -> tuple[bool, Optional[x509.Certificate]]:
        '''
        Get only approved certificate from DCL resposne.
        '''
        if response is None or not response.get("approvedCertificates", {}).get("certs", []):
            raise requests.exception.NotFound(f"No certificate found for {skid_hex}")
        if len(response["approvedCertificates"]["certs"]) > 1:
            raise ValueError(f"Multiple certificates found for {skid_hex}")
        issuer_certificate = x509.load_pem_x509_certificate(bytes(response["approvedCertificates"]["certs"][0]["pemCert"], "utf-8"))
        return response["approvedCertificates"]["certs"][0]["isRoot"], issuer_certificate

    def get_paa_cert(self, initial_cert: x509.Certificate) -> Optional[x509.Certificate]:
        '''
        Get the PAA certificate for the CRL Signer Certificate.
        '''
        issuer_name = initial_cert.issuer
        try:
            akid = get_akid(initial_cert)
        except ExtensionNotFound:
            logging.warning('Certificate AKID not found.')
            return
        paa_certificate = None
        while not paa_certificate:
            try:
                is_root, issuer_certificate = self.get_approved_certificate(issuer_name, akid)
                if is_root:
                    paa_certificate = issuer_certificate
                    break

            except Exception as e:
                logging.error('Failed to get PAA certificate', e)
                return
            logging.debug(f"issuer_name: {issuer_certificate.subject.rfc4514_string()}")
            issuer_name = issuer_certificate.issuer
            try:
                akid = get_akid(issuer_certificate)
            except ExtensionNotFound:
                logging.warning('Issuer Certificate AKID not found.')
            logging.debug(f"akid: {akid}")
        if paa_certificate is None:
            logging.warning("PAA Certificate not found, continue...")
        return paa_certificate

    def get_crl_file(self,
                     revocation_point: RevocationPoint,
                     crl_signer_certificate: x509.Certificate) -> x509.CertificateRevocationList:
        """Obtain the CRL."""
        try:
            r = requests.get(revocation_point.dataURL, timeout=5)
            logging.debug(f"Fetched CRL: {r.content}")
            return x509.load_der_x509_crl(r.content)
        except Exception:
            logging.warning(f"Failed to fetch a valid CRL for': {crl_signer_certificate.subject.rfc4514_string()}")

    def get_formatted_hex_skid(self, skid_hex: str) -> str:
        return ':'.join([skid_hex[i:i+2] for i in range(0, len(skid_hex), 2)])


class NodeDclClient(DclClientInterface):
    '''
    A client for interacting with DCLD using command line interface (CLI).
    '''

    def __init__(self, dcld_exe: str, use_test_net: bool):
        '''
        Initialize the client.

        dcld_exe: str
            Path to `dcld` executable.
         use_test_net: bool
            Indicates if the client should use TestNet or MainNet URL with dcld executable.
        '''

        self.dcld_exe = dcld_exe
        self.use_test_net = use_test_net

    def build_dcld_command_line(self, cmdlist: list[str]) -> list[str]:
        '''
        Build command line for `dcld` executable.

        Parameters
        ----------
        cmdlist: list[str]
            List of command line arguments to append to some predefined arguments.

        Returns
        -------
        list[str]
            The complete command list including the DCLD executable and node option if in production.
        '''

        return [self.dcld_exe] + cmdlist + ([] if self.use_test_net else ['--node', PRODUCTION_NODE_URL])

    def get_dcld_cmd_output_json(self, cmdlist: list[str]) -> dict:
        '''
        Executes a DCLD CLI command and returns the JSON output.

        Parameters
        ----------
        cmdlist: list[str]
            List of command line arguments to append to some predefined arguments.

        Returns
        -------
        dict
            The JSON output from the command.
        '''

        # Set the output as JSON
        subprocess.Popen([self.dcld_exe, 'config', 'output', 'json'])

        cmdpipe = subprocess.Popen(self.build_dcld_command_line(cmdlist),
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return json.loads(cmdpipe.stdout.read())

    def get_revocation_points(self) -> list[RevocationPoint]:
        '''
        Get revocation points from DCL.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points.
        '''

        response = self.get_dcld_cmd_output_json(['query', 'pki', 'all-revocation-points'])
        return [RevocationPoint(**r) for r in response["PkiRevocationDistributionPoint"]]

    def get_revocation_points_by_skid(self, issuer_subject_key_id) -> list[RevocationPoint]:
        '''
        Get revocation points by subject key ID.

        Parameters
        ----------
        issuer_subject_key_id: str
            Subject key ID.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points.
        '''

        response = self.get_dcld_cmd_output_json(['query', 'pki', 'revocation-points',
                                                  '--issuer-subject-key-id', issuer_subject_key_id])
        logging.debug(f"Response revocation points: {response}")
        return [RevocationPoint(**r) for r in response["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]["points"]]

    def get_approved_certificate(self, subject_name: x509.name.Name, skid_hex: str) -> tuple[bool, x509.Certificate]:
        '''
        Get certificate from DCL.

        Parameters
        ----------
        subject_name: x509.name.Name
            Subject Name object.
        skid_hex: str
            Subject Key ID in hex format.

        Returns
        -------
        tuple[bool, x509.Certificate]
            Tuple of is_paa and the certificate from the DCL.
        '''
        subject_name_b64 = get_b64_name(subject_name)
        query_cmd_list = ['query', 'pki', 'x509-cert', '-u', subject_name_b64, '-k', skid_hex]
        logging.debug(
            f"Fetching issuer from dcl query{' '.join(query_cmd_list)}")
        response = self.get_dcld_cmd_output_json(query_cmd_list)
        return self.get_only_approved_certificate(response, skid_hex)


class RestDclClient(DclClientInterface):
    '''
    A client for interacting with DCLD using the REST API.
    '''

    def __init__(self, use_test_net: bool):
        '''
        Initialize the client.

        use_test_net: bool
            Indicates if the client should use TestNet or MainNet REST API URL.
        '''
        self.rest_node_url = TEST_NODE_URL_REST if use_test_net else PRODUCTION_NODE_URL_REST

    def get_revocation_points(self) -> list[RevocationPoint]:
        '''
        Get revocation points from DCL.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points.
        '''

        response = self.send_get_request(f"{self.rest_node_url}/dcl/pki/revocation-points")
        return [RevocationPoint(**r) for r in response["PkiRevocationDistributionPoint"]]

    def get_revocation_points_by_skid(self, issuer_subject_key_id) -> list[RevocationPoint]:
        '''
        Get revocation points by subject key ID.

        Parameters
        ----------
        issuer_subject_key_id: str
            Subject key ID.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points.
        '''

        response = self.send_get_request(f"{self.rest_node_url}/dcl/pki/revocation-points/{issuer_subject_key_id}")
        return [RevocationPoint(**r) for r in response["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]["points"]]

    def get_approved_certificate(self, subject_name: x509.name.Name, skid_hex: str) -> tuple[bool, x509.Certificate]:
        '''
        Get certificate from DCL.

        Parameters
        ----------
        subject_name: x509.name.Name
            Subject Name object.
        skid_hex: str
            Subject Key ID in hex format.

        Returns
        -------
        tuple[bool, x509.Certificate]
            Tuple of is_paa and the certificate from the DCL.
        '''
        logging.debug(
            f"Fetching issuer from:{self.rest_node_url}/dcl/pki/certificates/{get_b64_name(subject_name)}/{self.get_formatted_hex_skid(skid_hex)}")
        response = self.send_get_request(
            f"{self.rest_node_url}/dcl/pki/certificates/{get_b64_name(subject_name)}/{self.get_formatted_hex_skid(skid_hex)}")
        logging.debug(f"Response certificate: {response}")
        return self.get_only_approved_certificate(response, skid_hex)


class LocalFilesDclClient(DclClientInterface):
    '''
    A client for interacting with local DLCD response data.
    '''

    def __init__(self, crls: [], dcl_certificates: [], revocation_points_response_file: str):
        '''
        Initialize the client.

        Parameters
        ----------
        crls: list
            List of CRL files.
        dcl_certificates: list
            List of certificate files.
        revocation_points_response_file: str
            Path to the get-revocation-points response json file.
        '''

        logging.debug(f"Loading certificates from {dcl_certificates}")
        logging.debug(f"Loading crls from {crls}")
        logging.debug(f"Loading revocation points response from {revocation_points_response_file}")
        self.crls = self.get_crls(crls)
        self.revocation_points = [RevocationPoint(**r)
                                  for r in json.load(revocation_points_response_file)["PkiRevocationDistributionPoint"]]
        self.authoritative_certs = self.get_authoritative_certificates(dcl_certificates)

    def get_lookup_key(self, certificate: x509.Certificate) -> str:
        '''
        Get key used in this class to lookup certificates.

        Parameters
        ----------
        certificate: x509.Certificate
            Certificate object.

        Returns
        -------
        str:
            lookup key derived from the certificate.
        '''
        base64_name = get_b64_name(certificate.subject)
        try:
            skid = get_skid(certificate)
            return self.format_lookup_key(base64_name, skid)
        except ExtensionNotFound:
            logging.warning("CertificateSKID not found, continue...")

    def format_lookup_key(self, base64_name: str, skid_hex: str) -> str:
        '''
        Get formatted key used in this class to lookup certificates.

        Parameters
        ----------
        base64_name: str
            Base64 encoded subject name.
        skid_hex: str
            Subject Key ID in hex format.

        Returns
        -------
        str:
            Key used in this class to lookup certificates.
        '''
        delimiter = '/'
        skid_hex_formatted = self.get_formatted_hex_skid(skid_hex)
        return delimiter.join([base64_name, skid_hex_formatted])

    def get_crls(self, unread_crls: []) -> list[x509.CertificateRevocationList]:
        '''
        Get CRLs from list of files.

        Parameters
        ----------
        unread_crls: list
            List of CRL files.

        Returns
        -------
        list[x509.CertificateRevocationList]
            List of CRLs.
        '''
        crls = []
        for file in unread_crls:
            crl_content = file.read()
            crl_file = x509.load_der_x509_crl(crl_content)
            crls.append(crl_file)
        return crls

    def get_authoritative_certificates(self, dcl_certificates: []) -> dict[str, x509.Certificate]:
        '''
        Get certificates from revocation points response file and list of provided dcl certificates.

        Parameters
        ----------
        dcl_certificates: list
            List of certificate files.

        Returns
        -------
        dict[str, x509.CertificateRevocationList]
            Dictionary of certificates, keyed by lookup key.
        '''
        certificates = {}
        logging.debug(f"Loading certificates from {dcl_certificates}")
        if dcl_certificates:
            for file in dcl_certificates:
                logging.debug(f"Loading certificate from {file}")
                # with open(file, "r") as f:
                certificate = x509.load_pem_x509_certificate(file.read())
                certificates[self.get_lookup_key(certificate)] = certificate

        logging.debug("Loading certificates from revocation_points_response file.")
        for point in self.revocation_points:
            if point.crlSignerDelegator:
                certificate = x509.load_pem_x509_certificate(bytes(point.crlSignerDelegator, 'utf-8'))
                certificates[self.get_lookup_key(certificate)] = certificate
            elif point.crlSignerCertificate:
                certificate = x509.load_pem_x509_certificate(bytes(point.crlSignerCertificate, 'utf-8'))
                certificates[self.get_lookup_key(certificate)] = certificate
        return certificates

    def get_revocation_points(self) -> list[RevocationPoint]:
        '''
        Get revocation points from DCL.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points.
        '''
        return self.revocation_points

    def get_revocation_points_by_skid(self, issuer_subject_key_id) -> list[RevocationPoint]:
        '''
        Get revocation points by subject key ID

        Parameters
        ----------
        issuer_subject_key_id: str
            Subject key ID.

        Returns
        -------
        list[RevocationPoint]
            List of revocation points with the same issuer subject key ID.
        '''
        same_issuer_points = []
        for point in self.revocation_points:
            if point.issuerSubjectKeyID == issuer_subject_key_id:
                same_issuer_points.append(point)
        return same_issuer_points

    def get_approved_certificate(self, subject_name: x509.name.Name, skid_hex: str) -> tuple[bool, x509.Certificate]:
        '''
        Get certificate from DCL

        Parameters
        ----------
        subject_name: x509.name.Name
            Subject name object.

        skid_hex: str
            Subject Key ID in hex format.

        Returns
        -------
        tuple[bool, x509.Certificate]
            Tuple of is_paa and the certificate from the DCL.
        '''
        lookup_key = self.format_lookup_key(get_b64_name(subject_name), skid_hex)
        if lookup_key in self.authoritative_certs:
            return is_self_signed_certificate(self.authoritative_certs[lookup_key]), self.authoritative_certs[lookup_key]
        return False, None

    def get_crl_file(self,
                     unused_revocation_point: RevocationPoint,
                     crl_signer_certificate: x509.Certificate) -> x509.CertificateRevocationList:
        '''
        Obtain the CRL.

        Parameters
        ----------
        unused_revocation_point: RevocationPoint
            Revocation point. Not used.

        crl_signer_certificate: x509.Certificate
            Crl signer certificate.

        Returns
        -------
        x509.CertificateRevocationList
            CRL signed by the CRL signer certificate.
        '''
        for crl in self.crls:
            if crl.issuer.public_bytes() == crl_signer_certificate.subject.public_bytes():
                logging.debug(f"Found CRL for issuer: {crl.issuer.rfc4514_string()}")
                return crl
        return None


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
@optgroup.option('--use-local-data', is_flag=True, type=bool, help="Fake response directory: see \" DATA_DIR/",)
@optgroup.group('Required arguments if use-local-data is used', cls=AllOptionGroup)
@optgroup.option('--certificates', type=click.File('rb'), multiple=True, help='Paths to PEM formated certificates (i.e. PAA) in DCL but missing from the revocation-points-response file.')
@optgroup.option('--crls', type=click.File('rb'), multiple=True, help='Paths to the crl der files')
@optgroup.option('--revocation-points-response', type=click.File('rb'), help='Path to the get-revocation-points response json file.')
@optgroup.group('Optional output arguments')
@optgroup.option('--output', default='sample_revocation_set_list.json', type=str, metavar='FILEPATH',
                 help="Output filename (default: sample_revocation_set_list.json)")
@optgroup.option('--log-level', default='INFO', show_default=True, type=click.Choice(__LOG_LEVELS__.keys(),
                                                                                     case_sensitive=False), callback=lambda c, p, v: __LOG_LEVELS__[v],
                 help='Determines the verbosity of script output')
def from_dcl(use_main_net_dcld: str, use_test_net_dcld: str, use_main_net_http: bool, use_test_net_http: bool, use_local_data: bool, revocation_points_response: str, crls: [], certificates: [], output: str, log_level: str):
    """Generate revocation set from DCL using generation algorithm from Matter Spec section 6.2.4.1."""
    logging.basicConfig(
        level=log_level,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

    if use_local_data:
        dcld_client = LocalFilesDclClient(crls, certificates, revocation_points_response)
    elif use_main_net_http or use_test_net_http:
        dcld_client = RestDclClient(True if use_test_net_http else False)
    else:
        dcld_client = NodeDclClient(use_main_net_dcld or use_test_net_dcld, True if use_test_net_dcld else False)

    revocation_point_list = dcld_client.get_revocation_points()
    revocation_set = []
    for revocation_point in revocation_point_list:
        # 1. Validate Revocation Type
        if revocation_point.revocationType != RevocationType.CRL.value:
            logging.warning("Revocation Type is not CRL, continue...")
            continue

        # 2. Parse the certificate
        try:
            crl_signer_certificate = x509.load_pem_x509_certificate(bytes(revocation_point.crlSignerCertificate, 'utf-8'))
        except Exception:
            logging.warning("CRL Signer Certificate is not valid, continue...")
            continue

        # Parse the crl signer delegator
        crl_signer_delegator_cert = None
        if revocation_point.crlSignerDelegator:
            crl_signer_delegator_cert_pem = revocation_point.crlSignerDelegator
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
        paa_certificate_object = dcld_client.get_paa_cert(crl_signer_certificate)
        if paa_certificate_object is None:
            logging.warning("PAA Certificate not found, continue...")
            continue

        if validate_cert_chain(crl_signer_certificate, crl_signer_delegator_cert, paa_certificate_object) is False:
            logging.warning("Failed to validate CRL Signer Certificate chain, continue...")
            continue

        # 6. Obtain the CRL
        crl_file = dcld_client.get_crl_file(revocation_point, crl_signer_certificate)
        if crl_file is None:
            logging.warning("CRL file not found for revocation point, continue...")
            continue

        # 7. Perform CRL File Validation
        # a.
        try:
            crl_signer_skid = get_skid(crl_signer_certificate)
        except ExtensionNotFound:
            logging.warning("CRL Signer SKID not found, continue...")
            continue
        try:
            crl_akid = get_akid(crl_file)
        except ExtensionNotFound:
            logging.warning("CRL AKID is not found, continue...")
            continue
        if crl_akid != crl_signer_skid:
            logging.warning("CRL AKID is not CRL Signer SKID, continue...")
            continue

        # b.
        same_issuer_points = dcld_client.get_revocation_points_by_skid(crl_akid)
        count_with_matching_vid_issuer_skid = sum(item.vid == revocation_point.vid for item in same_issuer_points)

        if count_with_matching_vid_issuer_skid > 1:
            try:
                issuing_distribution_point = crl_file.extensions.get_extension_for_oid(
                    x509.oid.ExtensionOID.ISSUING_DISTRIBUTION_POINT).value
            except Exception:
                logging.warning("CRL Issuing Distribution Point not found, continue...")
                continue

            uri_list = issuing_distribution_point.full_name
            if len(uri_list) == 1 and isinstance(uri_list[0], x509.UniformResourceIdentifier):
                if uri_list[0].value != revocation_point.dataURL:
                    logging.warning("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                    continue
            else:
                logging.warning("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                continue

        # TODO: 8. Validate CRL as per Section 6.3 of RFC 5280

        # 9. Decide on certificate authority name and AKID
        certificate_authority_name, certificate_akid_hex = get_certificate_authority_details(
            crl_signer_certificate, crl_signer_delegator_cert, paa_certificate_object, revocation_point.isPAA)

        # validate issuer skid matchces with the one in revocation points
        logging.debug(f"revocation_point.issuerSubjectKeyID: {revocation_point.issuerSubjectKeyID}")

        if revocation_point.issuerSubjectKeyID != certificate_akid_hex:
            logging.warning("CRL Issuer Subject Key ID is not CRL Signer Subject Key ID, continue...")
            continue

        # 10. Iterate through the Revoked Certificates List
        entry = generate_revocation_set_from_crl(crl_file, crl_signer_certificate,
                                                 certificate_authority_name, certificate_akid_hex, crl_signer_delegator_cert)
        logging.debug(f"Entry to append: {entry}")
        revocation_set.append(entry)

    with open(output, 'w+') as outfile:
        json.dump([revocation.asDict() for revocation in revocation_set], outfile, indent=4)


class TestRevocationSetGeneration(unittest.TestCase):
    """Test class for revocation set generation"""

    def setUp(self):
        # Get the directory containing this file
        self.test_base_dir = os.path.dirname(os.path.abspath(__file__))

    def get_test_file_path(self, filename):
        return os.path.join(self.test_base_dir, 'test', filename)

    def compare_revocation_sets(self, generated_set, expected_file):
        with open(os.path.join(self.test_base_dir, expected_file), 'r') as f:
            expected_set = [RevocationSet(**r) for r in json.load(f)]

        # Compare the contents
        self.assertEqual(len([generated_set]), len(expected_set))
        expected = expected_set[0]

        # Compare required fields
        self.assertEqual(generated_set.type, expected.type)
        self.assertEqual(generated_set.issuer_subject_key_id, expected.issuer_subject_key_id)
        self.assertEqual(generated_set.issuer_name, expected.issuer_name)
        self.assertEqual(set(generated_set.revoked_serial_numbers), set(expected.revoked_serial_numbers))
        self.assertEqual(generated_set.crl_signer_cert, expected.crl_signer_cert)

        # Compare optional fields if present in either set
        if generated_set.crl_signer_delegator or expected.crl_signer_delegator:
            self.assertEqual(generated_set.crl_signer_delegator, expected.crl_signer_delegator,
                             f'CRL signer delegator certificates do not match, expected: {expected.crl_signer_delegator}, actual: {generated_set.crl_signer_delegator}')

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
