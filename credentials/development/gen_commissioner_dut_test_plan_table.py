#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import argparse
import json
import os
from dataclasses import dataclass


@dataclass
class TestInfo:
    desc: str
    dir: str
    pid: int


CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT, 'scripts/tests')


def write_test_plan_output(filename: str, cases: TestInfo, test_num: str):
    with open(filename, "w") as output:
        output.write(f'[[ref:da-{test_num}-certs]]\n')
        output.write(f'Certificates for TC-DA-{test_num}\n')
        output.write('|===\n')
        output.write('|#| Cert Description| Example certs\n')
        for i, f in enumerate(cases):
            output.write(f'|{i+1}|{f.desc} | {f.dir} (pid={f.pid})\n')
        output.write('|===\n')


def write_validation_steps(filename: str, cases: TestInfo):
    with open(filename, "w") as output:
        for f in cases:
            cmd = f'./chip-all-clusters-app --trace_decode 1 --dac_provider $CHIP_ROOT/credentials/development/commissioner_dut/{f.dir}/test_case_vector.json --product-id {f.pid}'
            output.write(f'{f.desc.replace(",","")}, {f.dir}, {f.pid}, {cmd}\n')


def main():

    argparser = argparse.ArgumentParser()

    argparser.add_argument("--failure_table", default="failure_table")
    argparser.add_argument("--success_table", default="success_table")
    argparser.add_argument("--success_vs", default="success_vs.csv")
    argparser.add_argument("--failure_vs", default="failure_vs.csv")
    args = argparser.parse_args()

    cred_path = 'credentials/development/commissioner_dut'
    cert_path = os.path.abspath(os.path.join(CHIP_ROOT, cred_path))

    # The following test vectors are success conditions for an SDK commissioner for the following reasons:
    # struct_cd_device_type_id_mismatch - requires DCL access, which the SDK does not have and is not required
    # struct_cd_security_info_wrong - while devices are required to set this to 0, commissioners are required to ignore it
    #                                 (see 6.3.1)
    #                                 hence this is marked as a failure for devices, but should be a success case for
    #                                 commissioners
    # struct_cd_security_level_wrong - as with security info, commissioners are required to ignore this value (see 6.3.1)
    # struct_cd_version_number_wrong - this value is not meant to be interpreted by commissioners, so errors here should be
    #                                  ignored (6.3.1)
    # struct_cd_cert_id_mismatch - requires DCL access, which the SDK does not have and is not required.
    skip_cases = ['struct_cd_device_type_id_mismatch', 'struct_cd_security_info_wrong',
                  'struct_cd_security_level_wrong', 'struct_cd_version_number_wrong', 'struct_cd_cert_id_mismatch']

    success_cases = []
    failure_cases = []
    for p in sorted(os.listdir(cert_path)):
        if p in skip_cases:
            continue
        path = str(os.path.join(cert_path, p, 'test_case_vector.json'))
        with open(path, 'r') as f:
            j = json.loads(f.read())
            success_expected = j['is_success_case'].lower() == 'true'
            pid = 177 if 'fallback_encoding' in p else 32768
            desc = TestInfo(desc=j['description'], dir=p, pid=pid)
            if success_expected:
                success_cases.append(desc)
            else:
                failure_cases.append(desc)

    write_test_plan_output(args.failure_table, failure_cases, '1.4')
    write_test_plan_output(args.success_table, success_cases, '1.8')

    write_validation_steps(args.failure_vs, failure_cases)
    write_validation_steps(args.success_vs, success_cases)


if __name__ == '__main__':
    main()
