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
import subprocess
import sys
from enum import Enum

import click
import requests
from click_option_group import RequiredMutuallyExclusiveOptionGroup, optgroup
from cryptography import x509
from cryptography.hazmat.primitives.asymmetric import ec


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


class DCLDClient:
    '''
    A client for interacting with DCLD using either the REST API or command line interface (CLI).

    '''

    def __init__(self, use_rest:bool, dcld_exe:str, production:bool, rest_node_url:str):
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

    def build_dcld_command_line(self, cmdlist:list[str])->list[str]:
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

    def get_dcld_cmd_output_json(self, cmdlist:list[str])->dict:
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

    def get_revocation_points(self)->list[dict]:
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

    def get_paa_cert_for_crl_issuer(self, crl_signer_issuer_name_b64, crl_signer_authority_key_id)->str:
        '''
        Get PAA certificate for CRL issuer

        Parameters
        ----------
        crl_signer_issuer_name_b64: str
            The issuer name of the CRL signer.
        crl_signer_authority_key_id: str
            The authority key ID of the CRL signer.

        Returns
        -------
        str
            PAA certificate in PEM format
        '''
        if self.use_rest:
            response = requests.get(
                f"{self.rest_node_url}/dcl/pki/certificates/{crl_signer_issuer_name_b64}/{crl_signer_authority_key_id}").json()
        else:
            response = self.get_dcld_cmd_output_json(
                ['query', 'pki', 'x509-cert', '-u', crl_signer_issuer_name_b64, '-k', crl_signer_authority_key_id])

        return response["approvedCertificates"]["certs"][0]["pemCert"]

    def get_revocations_points_by_skid(self, issuer_subject_key_id)->list[dict]:
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


@click.command()
@click.help_option('-h', '--help')
@optgroup.group('Input data sources', cls=RequiredMutuallyExclusiveOptionGroup)
@optgroup.option('--use-main-net-dcld', type=str, default='', metavar='PATH', help="Location of `dcld` binary, to use `dcld` for mirroring MainNet.")
@optgroup.option('--use-test-net-dcld', type=str, default='', metavar='PATH', help="Location of `dcld` binary, to use `dcld` for mirroring TestNet.")
@optgroup.option('--use-main-net-http', is_flag=True, type=str, help="Use RESTful API with HTTPS against public MainNet observer.")
@optgroup.option('--use-test-net-http', is_flag=True, type=str, help="Use RESTful API with HTTPS against public TestNet observer.")
@optgroup.group('Optional arguments')
@optgroup.option('--output', default='sample_revocation_set_list.json', type=str, metavar='FILEPATH', help="Output filename (default: sample_revocation_set_list.json)")
def main(use_main_net_dcld, use_test_net_dcld, use_main_net_http, use_test_net_http, output):
    """Tool to construct revocation set from DCL"""

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
            print("Revocation Type is not CRL, continue...")
            continue

        # 2. Parse the certificate
        crl_signer_certificate = x509.load_pem_x509_certificate(bytes(revocation_point["crlSignerCertificate"], 'utf-8'))

        vid = revocation_point["vid"]
        pid = revocation_point["pid"]
        is_paa = revocation_point["isPAA"]

        # 3. && 4. Validate VID/PID
        # TODO: Need to support alternate representation of VID/PID (see spec "6.2.2.2. Encoding of Vendor ID and Product ID in subject and issuer fields")
        crl_vid = extract_single_integer_attribute(crl_signer_certificate.subject, OID_VENDOR_ID)
        crl_pid = extract_single_integer_attribute(crl_signer_certificate.subject, OID_PRODUCT_ID)

        if is_paa:
            if crl_vid is not None:
                if vid != crl_vid:
                    # TODO: Need to log all situations where a continue is called
                    print("VID is not CRL VID, continue...")
                    continue
        else:
            if crl_vid is None or vid != crl_vid:
                print("VID is not CRL VID, continue...")
                continue
            if crl_pid is not None:
                if pid != crl_pid:
                    print("PID is not CRL PID, continue...")
                    continue

        # 5. Validate the certification path containing CRLSignerCertificate.
        crl_signer_issuer_name = base64.b64encode(crl_signer_certificate.issuer.public_bytes()).decode('utf-8')

        crl_signer_authority_key_id = crl_signer_certificate.extensions.get_extension_for_oid(
            x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier

        # Convert CRL Signer AKID to colon separated hex
        crl_signer_authority_key_id = crl_signer_authority_key_id.hex().upper()
        crl_signer_authority_key_id = ':'.join([crl_signer_authority_key_id[i:i+2]
                                               for i in range(0, len(crl_signer_authority_key_id), 2)])

        paa_certificate = dcld_client.get_paa_cert_for_crl_issuer(crl_signer_issuer_name, crl_signer_authority_key_id)

        if paa_certificate is None:
            print("PAA Certificate not found, continue...")
            continue

        paa_certificate_object = x509.load_pem_x509_certificate(bytes(paa_certificate, 'utf-8'))

        # TODO: use verify_directly_issued_by() method when we upgrade cryptography to v40.0.0
        # Verify issuer matches with subject
        if crl_signer_certificate.issuer != paa_certificate_object.subject:
            print("CRL Signer Certificate issuer does not match with PAA Certificate subject, continue...")
            continue

        # Check crl signers AKID matches with SKID of paa_certificate_object's AKID
        paa_skid = paa_certificate_object.extensions.get_extension_for_oid(x509.OID_SUBJECT_KEY_IDENTIFIER).value.key_identifier
        crl_akid = crl_signer_certificate.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier
        if paa_skid != crl_akid:
            print("CRL Signer's AKID does not match with PAA Certificate SKID, continue...")
            continue

        # verify if PAA singed the crl's public key
        try:
            paa_certificate_object.public_key().verify(crl_signer_certificate.signature,
                                                       crl_signer_certificate.tbs_certificate_bytes,
                                                       ec.ECDSA(crl_signer_certificate.signature_hash_algorithm))
        except Exception as e:
            print("CRL Signer Certificate is not signed by PAA Certificate, continue...")
            print("Error: ", e)
            continue

        # 6. Obtain the CRL
        try:
            r = requests.get(revocation_point["dataURL"], timeout=5)
        except requests.exceptions.Timeout:
            print("Timeout fetching CRL from ", revocation_point["dataURL"])
            continue

        crl_file = x509.load_der_x509_crl(r.content)

        # 7. Perform CRL File Validation
        crl_authority_key_id = crl_file.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier
        crl_signer_subject_key_id = crl_signer_certificate.extensions.get_extension_for_oid(
            x509.OID_SUBJECT_KEY_IDENTIFIER).value.key_identifier
        if crl_authority_key_id != crl_signer_subject_key_id:
            print("CRL Authority Key ID is not CRL Signer Subject Key ID, continue...")
            continue

        issuer_subject_key_id = ''.join('{:02X}'.format(x) for x in crl_authority_key_id)

        # b.
        same_issuer_points = dcld_client.get_revocations_points_by_skid(issuer_subject_key_id)
        count_with_matching_vid_issuer_skid = sum(item.get('vid') == vid for item in same_issuer_points)

        if count_with_matching_vid_issuer_skid > 1:
            try:
                issuing_distribution_point = crl_file.extensions.get_extension_for_oid(
                    x509.OID_ISSUING_DISTRIBUTION_POINT).value
            except Exception:
                print("CRL Issuing Distribution Point not found, continue...")
                continue

            uri_list = issuing_distribution_point.full_name
            if len(uri_list) == 1 and isinstance(uri_list[0], x509.UniformResourceIdentifier):
                if uri_list[0].value != revocation_point["dataURL"]:
                    print("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                    continue
            else:
                print("CRL Issuing Distribution Point URI is not CRL URL, continue...")
                continue

        # 9. Assign CRL File Issuer
        certificate_authority_name = base64.b64encode(crl_file.issuer.public_bytes()).decode('utf-8')
        print(f"CRL File Issuer: {certificate_authority_name}")

        serialnumber_list = []
        # 10. Iterate through the Revoked Certificates List
        for revoked_cert in crl_file:
            # a.
            try:
                revoked_cert_issuer = revoked_cert.extensions.get_extension_for_oid(
                    x509.CRLEntryExtensionOID.CERTIFICATE_ISSUER).value.get_values_for_type(x509.DirectoryName).value

                if revoked_cert_issuer is not None:
                    if revoked_cert_issuer != certificate_authority_name:
                        print("CRL Issuer is not CRL File Issuer, continue...")
                        continue
            except Exception:
                pass

            # b.
            try:
                revoked_cert_authority_key_id = revoked_cert.extensions.get_extension_for_oid(
                    x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier

                if revoked_cert_authority_key_id is None or revoked_cert_authority_key_id != crl_signer_subject_key_id:
                    print("CRL Authority Key ID is not CRL Signer Subject Key ID, continue...")
                    continue
            except Exception:
                print("CRL Authority Key ID not found, continue...")
                continue

            # c. and d.
            serialnumber_list.append(bytes(str('{:02X}'.format(revoked_cert.serial_number)), 'utf-8').decode('utf-8'))

        issuer_name = base64.b64encode(crl_file.issuer.public_bytes()).decode('utf-8')

        revocation_set.append({"type": "revocation_set",
                               "issuer_subject_key_id": issuer_subject_key_id,
                               "issuer_name": issuer_name,
                               "revoked_serial_numbers": serialnumber_list})

    with open(output, 'w+') as outfile:
        json.dump(revocation_set, outfile, indent=4)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        main.main(['--help'])
    else:
        main()
