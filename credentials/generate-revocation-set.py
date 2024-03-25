#!/usr/bin/python

#
# Copyright (c) 2023 Project CHIP Authors
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


class RevocationType(Enum):
    CRL = 1


OID_VENDOR_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.1")
OID_PRODUCT_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.2")

PRODUCTION_NODE_URL = "https://on.dcl.csa-iot.org:26657"
PRODUCTION_NODE_URL_REST = "https://on.dcl.csa-iot.org"
TEST_NODE_URL_REST = "https://on.test-net.dcl.csa-iot.org"


def use_dcld(dcld, production, cmdlist):
    return [dcld] + cmdlist + (['--node', PRODUCTION_NODE_URL] if production else [])


def extract_single_integer_attribute(subject, oid):
    attribute_list = subject.get_attributes_for_oid(oid)

    if len(attribute_list) == 1:
        if attribute_list[0].value.isdigit():
            return int(attribute_list[0].value)

    return None


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
    """DCL PAA mirroring tools"""

    production = False
    dcld = use_test_net_dcld

    if len(use_main_net_dcld) > 0:
        dcld = use_main_net_dcld
        production = True

    use_rest = use_main_net_http or use_test_net_http
    if use_main_net_http:
        production = True

    rest_node_url = PRODUCTION_NODE_URL_REST if production else TEST_NODE_URL_REST

    # TODO: Extract this to a helper function
    if use_rest:
        revocation_point_list = requests.get(f"{rest_node_url}/dcl/pki/revocation-points").json()["PkiRevocationDistributionPoint"]
    else:
        cmdlist = ['config', 'output', 'json']
        subprocess.Popen([dcld] + cmdlist)

        cmdlist = ['query', 'pki', 'all-revocation-points']

        cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        revocation_point_list = json.loads(cmdpipe.stdout.read())["PkiRevocationDistributionPoint"]

    revocation_set = []

    for revocation_point in revocation_point_list:
        # 1. Validate Revocation Type
        if revocation_point["revocationType"] != RevocationType.CRL:
            continue

        # 2. Parse the certificate
        crl_signer_certificate = x509.load_pem_x509_certificate(revocation_point["crlSignerCertificate"])

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
                    continue
        else:
            if crl_vid is None or vid != crl_vid:
                continue
            if crl_pid is not None:
                if pid != crl_pid:
                    continue

        # 5. Validate the certification path containing CRLSignerCertificate.
        crl_signer_issuer_name = base64.b64encode(crl_signer_certificate.issuer.public_bytes()).decode('utf-8')

        crl_signer_authority_key_id = crl_signer_certificate.extensions.get_extension_for_oid(
            x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier

        paa_certificate = None

        # TODO: Extract this to a helper function
        if use_rest:
            response = requests.get(
                f"{rest_node_url}/dcl/pki/certificates/{crl_signer_issuer_name}/{crl_signer_authority_key_id}").json()["approvedCertificates"]["certs"][0]
            paa_certificate = response["pemCert"]
        else:
            cmdlist = ['query', 'pki', 'x509-cert', '-u', crl_signer_issuer_name, '-k', crl_signer_authority_key_id]
            cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            paa_certificate = json.loads(cmdpipe.stdout.read())["approvedCertificates"]["certs"][0]["pemCert"]

        if paa_certificate is None:
            continue

        paa_certificate_object = x509.load_pem_x509_certificate(paa_certificate)

        try:
            crl_signer_certificate.verify_directly_issued_by(paa_certificate_object)
        except Exception:
            continue

        # 6. Obtain the CRL
        r = requests.get(revocation_point["dataURL"])
        crl_file = x509.load_der_x509_crl(r.content)

        # 7. Perform CRL File Validation
        crl_authority_key_id = crl_file.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier
        crl_signer_subject_key_id = crl_signer_certificate.extensions.get_extension_for_oid(
            x509.OID_SUBJECT_KEY_IDENTIFIER).value.key_identifier
        if crl_authority_key_id != crl_signer_subject_key_id:
            continue

        issuer_subject_key_id = ''.join('{:02X}'.format(x) for x in crl_authority_key_id)

        same_issuer_points = None

        # TODO: Extract this to a helper function
        if use_rest:
            response = requests.get(
                f"{rest_node_url}/dcl/pki/revocation-points/{issuer_subject_key_id}").json()["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]
            same_issuer_points = response["points"]
        else:
            cmdlist = ['query', 'pki', 'revocation-points', '--issuer-subject-key-id', issuer_subject_key_id]
            cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            same_issuer_points = json.loads(cmdpipe.stdout.read())[
                "pkiRevocationDistributionPointsByIssuerSubjectKeyID"]["points"]

        matching_entries = False
        for same_issuer_point in same_issuer_points:
            if same_issuer_point["vid"] == vid:
                matching_entries = True
                break

        if matching_entries:
            try:
                issuing_distribution_point = crl_file.extensions.get_extension_for_oid(
                    x509.OID_ISSUING_DISTRIBUTION_POINT).value
            except Exception:
                continue

            uri_list = issuing_distribution_point.full_name
            if len(uri_list) == 1 and isinstance(uri_list[0], x509.UniformResourceIdentifier):
                if uri_list[0].value != revocation_point["dataURL"]:
                    continue
            else:
                continue

        # 9. Assign CRL File Issuer
        certificate_authority_name = base64.b64encode(crl_file.issuer.public_bytes()).decode('utf-8')

        serialnumber_list = []
        # 10. Iterate through the Revoked Certificates List
        for revoked_cert in crl_file:
            try:
                revoked_cert_issuer = revoked_cert.extensions.get_extension_for_oid(
                    x509.CRLEntryExtensionOID.CERTIFICATE_ISSUER).value.get_values_for_type(x509.DirectoryName).value

                if revoked_cert_issuer is not None:
                    if revoked_cert_issuer != certificate_authority_name:
                        continue
            except Exception:
                pass

            # b.
            try:
                revoked_cert_authority_key_id = revoked_cert.extensions.get_extension_for_oid(
                    x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier

                if revoked_cert_authority_key_id is None or revoked_cert_authority_key_id != crl_signer_subject_key_id:
                    continue
            except Exception:
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
