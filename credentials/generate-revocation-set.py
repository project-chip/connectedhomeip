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

import copy
import os
import re
import subprocess
import sys
import json
import base64

import click
import requests
from click_option_group import RequiredMutuallyExclusiveOptionGroup, optgroup

PRODUCTION_NODE_URL = "https://on.dcl.csa-iot.org:26657"
PRODUCTION_NODE_URL_REST = "https://on.dcl.csa-iot.org"
TEST_NODE_URL_REST = "https://on.test-net.dcl.csa-iot.org"


def use_dcld(dcld, production, cmdlist):
    return [dcld] + cmdlist + (['--node', PRODUCTION_NODE_URL] if production else [])


def is_base64(sb):
        try:
                if isinstance(sb, str):
                        # If there's any unicode here, an exception will be thrown and the function will return false
                        sb_bytes = bytes(sb, 'ascii')
                elif isinstance(sb, bytes):
                        sb_bytes = sb
                else:
                        raise ValueError("Argument must be string or bytes")
                return base64.b64encode(base64.b64decode(sb_bytes)) == sb_bytes
        except Exception:
                return False


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
        revoked_cert_list = requests.get(f"{rest_node_url}/dcl/pki/revoked-certificates").json()["revokedCertificates"]
    else:
        cmdlist = ['config', 'output', 'json']
        subprocess.Popen([dcld] + cmdlist)

        cmdlist = ['query', 'pki', 'all-revoked-x509-certs']

        cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        revoked_cert_list = json.loads(cmdpipe.stdout.read())["revokedCertificates"]

    revocation_set = []

    for revoked_cert in revoked_cert_list:
        serialnumber_list = []

        for certificate_info in revoked_cert["certs"]:
            serialnumber_list.append(certificate_info["serialNumber"])

        subject_key_id = revoked_cert['subjectKeyId'].split(':')
        for subject_key_id_idx in range(len(subject_key_id)):
            if len(subject_key_id[subject_key_id_idx]) == 1:
                subject_key_id[subject_key_id_idx] = '0' + subject_key_id[subject_key_id_idx]

        subject_key_id = ''.join(subject_key_id)

        if is_base64(revoked_cert['subject']):
            issuer_name = revoked_cert['subject']
        else:
            issuer_name = base64.b64encode(bytes(revoked_cert['subject'], 'utf-8')).decode('utf-8')

        revocation_set.append({"type": "revocation_set",
                               "issuer_subject_key_id": subject_key_id,
                               "issuer_name": issuer_name,
                               "revoked_serial_numbers": serialnumber_list})

    with open(output, 'w+') as outfile:
        json.dump(revocation_set, outfile, indent=4)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        main.main(['--help'])
    else:
        main()
