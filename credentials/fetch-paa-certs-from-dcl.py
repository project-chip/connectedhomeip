#!/usr/bin/python

#
# Copyright (c) 2022 Project CHIP Authors
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

# Script that was used to fetch CHIP Development Product Attestation Authority (PAA)
# certificates from DCL.
# For usage please run:.
#     python ./credentials/fetch-paa-certs-from-dcl.py --help

import copy
import os
import re
import subprocess
import sys

import click
import requests
from click_option_group import RequiredMutuallyExclusiveOptionGroup, optgroup
from cryptography import x509
from cryptography.hazmat.primitives import serialization

PRODUCTION_NODE_URL = "https://on.dcl.csa-iot.org:26657"
PRODUCTION_NODE_URL_REST = "https://on.dcl.csa-iot.org"
TEST_NODE_URL_REST = "https://on.test-net.dcl.csa-iot.org"


def parse_paa_root_certs(cmdpipe, paa_list):
    """
    example output of a query to all x509 root certs in DCL:

    certs:
    - subject: MCExHzAdBgNVBAMMFk1hdHRlciBEZXZlbG9wbWVudCBQQUE=
    subjectKeyId: FA:92:CF:09:5E:FA:42:E1:14:30:65:16:32:FE:FE:1B:2C:77:A7:C8
    - subject: MDAxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTEUMBIGCisGAQQBgqJ8AgEMBDEyNUQ=
    subjectKeyId: E2:90:8D:36:9C:3C:A3:C1:13:BB:09:E2:4D:C1:CC:C5:A6:66:91:D4
    - subject: MEsxCzAJBgNVBAYTAlVTMQ8wDQYDVQQKDAZHb29nbGUxFTATBgNVBAMMDE1hdHRlciBQQUEgMTEUMBIGCisGAQQBgqJ8AgEMBDYwMDY=
    subjectKeyId: B0:00:56:81:B8:88:62:89:62:80:E1:21:18:A1:A8:BE:09:DE:93:21
    - subject: MFUxCzAJBgNVBAYTAlVTMRcwFQYDVQQKEw5EaWdpQ2VydCwgSW5jLjEtMCsGA1UEAxMkRGlnaUNlcnQgVEVTVCBSb290IENBIGZvciBNQVRURVIgUEtJ
    subjectKeyId: C0:E0:64:15:00:EC:67:E2:7C:AF:7C:6E:2D:49:94:C7:73:DE:B7:BA
    - subject: MDAxLjAsBgNVBAMMJU5vbiBQcm9kdWN0aW9uIE9OTFkgLSBYRk4gUEFBIENsYXNzIDM=
    subjectKeyId: F8:99:A9:D5:AD:71:71:E4:C3:81:7F:14:10:7F:78:F0:D9:F7:62:E9
    - subject: MEIxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTEQMA4GA1UECgwHU2Ftc3VuZzEUMBIGCisGAQQBgqJ8AgEMBDEwRTE=
    subjectKeyId: CF:9E:0A:16:78:8B:40:30:EC:DD:AB:34:B9:C2:EC:7B:E5:34:55:C0

    Brief:
    This method will search for the first line that contains ': ' char sequence.
    From there, it assumes every 2 lines contain subject and subject key id info of
    a valid PAA root certificate.
    The paa_list parameter will contain a list of all valid PAA Root certificates
    from DCL.
    """

    result = {}

    while True:
        line = cmdpipe.stdout.readline()
        if not line:
            break
        else:
            if b': ' in line:
                key, value = line.split(b': ')
                result[key.strip(b' -').decode("utf-8")] = value.strip().decode("utf-8")
                parse_paa_root_certs.counter += 1
                if parse_paa_root_certs.counter % 2 == 0:
                    paa_list.append(copy.deepcopy(result))


def write_paa_root_cert(certificate, subject):
    filename = 'dcld_mirror_' + \
        re.sub('[^a-zA-Z0-9_-]', '', re.sub('[=, ]', '_', subject))
    with open(filename + '.pem', 'w+') as outfile:
        outfile.write(certificate)
    # convert pem file to der
    try:
        with open(filename + '.pem', 'rb') as infile:
            pem_certificate = x509.load_pem_x509_certificate(infile.read())
        with open(filename + '.der', 'wb+') as outfile:
            der_certificate = pem_certificate.public_bytes(
                serialization.Encoding.DER)
            outfile.write(der_certificate)
    except (IOError, ValueError) as e:
        print(f"ERROR: Failed to convert {filename + '.pem'}: {str(e)}. Skipping...")


def parse_paa_root_cert_from_dcld(cmdpipe):
    subject = None
    certificate = ""

    while True:
        line = cmdpipe.stdout.readline()
        if not line:
            break
        else:
            if b'pemCert: |' in line:
                while True:
                    line = cmdpipe.stdout.readline()
                    certificate += line.strip(b' \t').decode("utf-8")
                    if b'-----END CERTIFICATE-----' in line:
                        break
            if b'subjectAsText:' in line:
                subject = line.split(b': ')[1].strip().decode("utf-8")
                break

    return (certificate, subject)


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
@optgroup.option('--paa-trust-store-path', default='paa-root-certs', type=str, metavar='PATH', help="PAA trust store path (default: paa-root-certs)")
def main(use_main_net_dcld, use_test_net_dcld, use_main_net_http, use_test_net_http, paa_trust_store_path):
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

    os.makedirs(paa_trust_store_path, exist_ok=True)
    os.chdir(paa_trust_store_path)

    if use_rest:
        paa_list = requests.get(f"{rest_node_url}/dcl/pki/root-certificates").json()["approvedRootCertificates"]["certs"]
    else:
        cmdlist = ['query', 'pki', 'all-x509-root-certs']

        cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        paa_list = []
        parse_paa_root_certs.counter = 0
        parse_paa_root_certs(cmdpipe, paa_list)

    for paa in paa_list:
        if use_rest:
            response = requests.get(
                f"{rest_node_url}/dcl/pki/certificates/{paa['subject']}/{paa['subjectKeyId']}").json()["approvedCertificates"]["certs"][0]
            certificate = response["pemCert"]
            subject = response["subjectAsText"]
        else:
            cmdlist = ['query', 'pki', 'x509-cert', '-u', paa['subject'], '-k', paa['subjectKeyId']]

            cmdpipe = subprocess.Popen(use_dcld(dcld, production, cmdlist), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            (certificate, subject) = parse_paa_root_cert_from_dcld(cmdpipe)

        certificate = certificate.rstrip('\n')

        print(f"Downloaded certificate with subject: {subject}")
        write_paa_root_cert(certificate, subject)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        main.main(['--help'])
    else:
        main()
