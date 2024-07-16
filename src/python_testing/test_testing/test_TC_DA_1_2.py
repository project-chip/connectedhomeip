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

import json
import os
import subprocess
import sys

CHIP_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '../../..'))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT, 'scripts/tests')


def run_single_test(dac_provider: str, product_id: int, factory_reset: bool = False) -> int:

    reset = ""
    if factory_reset:
        reset = ' --factoryreset'

    app = os.path.join(
        CHIP_ROOT, 'out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app')

    # Certs in the commissioner_dut directory use 0x8000 as the PID
    app_args = '--discriminator 1234 --KVS kvs1 --product-id ' + \
        str(product_id) + ' --vendor-id 65521 --dac_provider ' + dac_provider

    ci_pics_values = os.path.abspath(os.path.join(
        CHIP_ROOT, 'src/app/tests/suites/certification/ci-pics-values'))
    script_args = '--storage-path admin_storage.json --discriminator 1234 --passcode 20202021 --dut-node-id 1 --PICS ' + \
        str(ci_pics_values)

    # for any test with a dac_provider, we don't want to recommission because there's a chance the
    # dac could be wrong and the commissioning would fail. Rely on the original commissioning. This is also faster.
    if factory_reset:
        script_args = script_args + ' --commissioning-method on-network'

    script = os.path.abspath(os.path.join(
        CHIP_ROOT, 'src/python_testing/TC_DA_1_2.py'))

    # run_python_test uses click so call as a command
    run_python_test = os.path.abspath(os.path.join(
        RUNNER_SCRIPT_DIR, 'run_python_test.py'))
    cmd = str(run_python_test) + reset + ' --app ' + str(app) + ' --app-args "' + \
        app_args + '" --script ' + \
        str(script) + ' --script-args "' + script_args + '"'

    return subprocess.call(cmd, shell=True)


def main():
    cert_path = os.path.abspath(os.path.join(
        CHIP_ROOT, 'credentials/development/commissioner_dut'))

    # Commission first using a known good set, then run the rest of the tests without recommissioning
    path = str(os.path.join(
        cert_path, "struct_cd_authorized_paa_list_count1_valid/test_case_vector.json"))
    run_single_test(path, 32768, factory_reset=True)

    test_cases = ['struct_cd', 'fallback_encoding']

    # struct_cd_version_number_wrong - excluded because this is a DCL test not covered by cert
    # struct_cd_cert_id_mismatch - excluded because this is a DCL test not covered by cert
    exclude_cases = ['struct_cd_version_number_wrong',
                     'struct_cd_cert_id_mismatch']

    passes = []
    for p in os.listdir(cert_path):
        matches = list(filter(lambda t: t in str(p), test_cases))
        if len(matches) != 1:
            continue

        if str(p) in exclude_cases:
            continue

        path = str(os.path.join(cert_path, p, 'test_case_vector.json'))
        with open(path, 'r') as f:
            j = json.loads(f.read())
            success_expected = j['is_success_case'].lower() == 'true'
            pid = j['basic_info_pid']

        ret = run_single_test(path, pid)
        passes.append((str(p), ret, success_expected))

    retval = 0
    for p in passes:
        success = p[1] == 0
        if p[2] != success:
            print('INCORRECT: ' + p[0])
            retval = 1

    sys.exit(retval)


if __name__ == '__main__':
    main()
