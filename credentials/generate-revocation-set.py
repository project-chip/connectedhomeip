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

    crl_list = []

    # TODO: Use the following two lines for testing purposes. Remove them once DCL is populated with valid CRLs.
    # r = requests.get("<crl url>")
    # crl_list.append(x509.load_der_x509_crl(r.content))

    for revocation_point in revocation_point_list:
        r = requests.get(revocation_point["dataURL"])
        crl_list.append(x509.load_der_x509_crl(r.content))

    revocation_set = []

    for crl in crl_list:
        serialnumber_list = []

        for r in crl:
            serialnumber_list.append(bytes(str('{:02X}'.format(r.serial_number)), 'utf-8').decode('utf-8'))

        authority_key_id = crl.extensions.get_extension_for_oid(x509.OID_AUTHORITY_KEY_IDENTIFIER).value.key_identifier
        issuer_subject_key_id = ''.join('{:02X}'.format(x) for x in authority_key_id)

        issuer_name = base64.b64encode(crl.issuer.public_bytes()).decode('utf-8')

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
