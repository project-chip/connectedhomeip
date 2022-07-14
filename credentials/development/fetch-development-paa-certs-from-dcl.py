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
# The script expects the path to the dcld tool binary as an input argument.
#
# Usage example when the script is run from the CHIP SDK root directory:
#     python ./credentials/development/fetch-development-paa-certs-from-dcl.py /path/to/dcld
#
# The result will be stored in:
#     credentials/development/paa-root-certs
#

import os
import sys
import subprocess
import copy
import re
from cryptography.hazmat.primitives import serialization
from cryptography import x509


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
                result[key.strip(b' -')] = value.strip()
                parse_paa_root_certs.counter += 1
                if parse_paa_root_certs.counter % 2 == 0:
                    paa_list.append(copy.deepcopy(result))


def write_paa_root_cert(cmdpipe, subject):
    pem_read = False
    subject_as_text_read = False

    filename = 'paa-root-certs/dcld_mirror_' + \
        re.sub('[^a-zA-Z0-9_-]', '', re.sub('[=, ]', '_', subject))
    with open(filename + '.pem', 'wb+') as outfile:
        while True:
            line = cmdpipe.stdout.readline()
            if not line:
                break
            else:
                if b'pemCert: |' in line:
                    while True:
                        line = cmdpipe.stdout.readline()
                        outfile.write(line.strip(b' \t'))
                        if b'-----END CERTIFICATE-----' in line:
                            pem_read = True
                            break
                if b'subjectAsText:' in line:
                    new_subject = line.split(b': ')[1].strip().decode("utf-8")
                    new_filename = 'paa-root-certs/dcld_mirror_' + \
                        re.sub('[=,\\\\ ]', '_', new_subject)
                    subject_as_text_read = True
                    break

    # if successfully obtained all mandatory fields from the root certificate
    if pem_read == True and subject_as_text_read == True:
        os.rename(filename + '.pem', new_filename + '.pem')
        # convert pem file to der
        with open(new_filename + '.pem', 'rb') as infile:
            pem_certificate = x509.load_pem_x509_certificate(infile.read())
        with open(new_filename + '.der', 'wb+') as outfile:
            der_certificate = pem_certificate.public_bytes(
                serialization.Encoding.DER)
            outfile.write(der_certificate)


def main():
    if len(sys.argv) == 2:
        dcld = sys.argv[1]
    else:
        sys.exit(
            "Error: Please specify exactly one input argument; the path to the dcld tool binary")

    previous_dir = os.getcwd()
    abspath = os.path.dirname(sys.argv[0])
    os.chdir(abspath)

    os.makedirs('paa-root-certs', exist_ok=True)

    cmdpipe = subprocess.Popen([dcld, 'query', 'pki', 'all-x509-root-certs'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    paa_list = []
    parse_paa_root_certs.counter = 0
    parse_paa_root_certs(cmdpipe, paa_list)

    for paa in paa_list:
        cmdpipe = subprocess.Popen(
            [dcld, 'query', 'pki', 'x509-cert', '-u',
                paa[b'subject'].decode("utf-8"), '-k', paa[b'subjectKeyId'].decode("utf-8")],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        write_paa_root_cert(cmdpipe, paa[b'subject'].decode("utf-8"))

    os.chdir(previous_dir)


if __name__ == "__main__":
    main()
