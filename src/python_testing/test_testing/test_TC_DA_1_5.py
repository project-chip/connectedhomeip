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

import os
import subprocess

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT, 'scripts/tests')


def run_single_test(flag: str, factory_reset: bool = False) -> int:

    reset = ""
    if factory_reset:
        reset = ' --factoryreset'

    app = os.path.join(CHIP_ROOT, 'out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app')

    # Certs in the commissioner_dut directory use 0x8000 as the PID
    app_args = '--discriminator 1234 --KVS kvs1 ' + flag

    ci_pics_values = os.path.abspath(os.path.join(CHIP_ROOT, 'src/app/tests/suites/certification/ci-pics-values'))
    script_args = '--storage-path admin_storage.json --discriminator 1234 --passcode 20202021 --dut-node-id 1 --PICS ' + \
        str(ci_pics_values)

    # for any test with a dac_provider, we don't want to recommission because there's a chance the
    # dac could be wrong and the commissioning would fail. Rely on the original commissioning. This is also faster.
    if factory_reset:
        script_args = script_args + ' --commissioning-method on-network'

    script = os.path.abspath(os.path.join(CHIP_ROOT, 'src/python_testing/TC_DA_1_5.py'))

    # run_python_test uses click so call as a command
    run_python_test = os.path.abspath(os.path.join(RUNNER_SCRIPT_DIR, 'run_python_test.py'))
    cmd = str(run_python_test) + reset + ' --app ' + str(app) + ' --app-args "' + \
        app_args + '" --script ' + str(script) + ' --script-args "' + script_args + '"'

    return subprocess.call(cmd, shell=True)


def main():
    # Commission first using a known good set, then run the rest of the tests without recommissioning
    passes = []
    # test flag, test result, success expected
    passes.append(("", run_single_test("", factory_reset=True), True))

    failure_flags = ['--cert_error_csr_incorrect_type',
                     '--cert_error_csr_existing_keypair',
                     '--cert_error_csr_nonce_incorrect_type',
                     '--cert_error_csr_nonce_too_long',
                     '--cert_error_csr_nonce_invalid',
                     '--cert_error_nocsrelements_too_long',
                     '--cert_error_attestation_signature_incorrect_type',
                     '--cert_error_attestation_signature_invalid']

    for f in failure_flags:
        passes.append((f, run_single_test(f), False))

    retval = 0
    for p in passes:
        success = p[1] == 0
        if p[2] != success:
            print('INCORRECT: ' + p[0])
            retval = 1

    return retval


if __name__ == '__main__':
    main()
