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

import click
import requests
from click_option_group import RequiredMutuallyExclusiveOptionGroup, optgroup
from cryptography import x509

OID_VENDOR_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.1")
OID_PRODUCT_ID = x509.ObjectIdentifier("1.3.6.1.4.1.37244.2.2")

PRODUCTION_NODE_URL = "https://on.dcl.csa-iot.org:26657"
PRODUCTION_NODE_URL_REST = "https://on.dcl.csa-iot.org"
TEST_NODE_URL_REST = "https://on.test-net.dcl.csa-iot.org"


def use_dcld(dcld, production, cmdlist):
    return [dcld] + cmdlist + (['--node', PRODUCTION_NODE_URL] if production else [])


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
        if revocation_point["revocationType"] != 1:

            # 2. Parse the certificate
            crl_signer_certificate = x509.load_pem_x509_certificate(revocation_point["crlSignerCertificate"])

            vid = revocation_point["vid"]
            pid = revocation_point["pid"]
            is_paa = revocation_point["isPAA"]

            # 3. && 4. Validate VID/PID
            crl_vid_list = crl_signer_certificate.subject.get_attributes_for_oid(OID_VENDOR_ID)
            crl_pid_list = crl_signer_certificate.subject.get_attributes_for_oid(OID_PRODUCT_ID)

            if is_paa == True:
                if len(crl_vid_list) == 1:
                    if vid != int(crl_vid_list[0].value):
                        continue
            else:
                if vid != int(crl_vid_list[0].value):
                    continue
                if len(crl_pid_list) == 1:
                    if pid != int(crl_pid_list[0].value):
                        continue

            # TODO: 5.

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

            if use_rest:
                response = requests.get(
                    f"{rest_node_url}/dcl/pki/revocation-points/{issuer_subject_key_id}").json()["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]
                same_issuer_points = response["points"]
            else:
                cmdlist = ['query', 'pki', 'revocation-points', '--issuer-subject-key-id', issuer_subject_key_id]
                cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                same_issuer_points = json.loads(cmdpipe.stdout.read())["pkiRevocationDistributionPointsByIssuerSubjectKeyID"]["points"]

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

            # TODO: 8.

            # 9. Assign CRL File Issuer
            certificate_authority_name = base64.b64encode(crl_file.issuer.public_bytes()).decode('utf-8')

            serialnumber_list = []
            # 10. Iterate through the Revoked Certificates List
            for revoked_cert in crl_file:
                # TODO: How to find certificate issuer extension in revoked cert?

                serialnumber_list.append(bytes(str('{:02X}'.format(r.serial_number)), 'utf-8').decode('utf-8'))

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
