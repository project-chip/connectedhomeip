#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2025 Project CHIP Authors
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
#    This script is intended for QA use only and does not automatically
#    verify the results of the commissioning process. Please use with caution.
#
# [TC-DA-1.9] Device Attestation Revocation [DUT-Commissioner]
#
#  This test is about commissioning a device configured with a revoked DAC and/or PAI
#  and checking if the commissioner warns about the device attestation error.
#  The test case will be successful if the commissioner warns about the device
#  attestation error and the device is not commissioned.

import os
import re
import signal
import subprocess
from typing import Optional
import click

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
DEFAULT_APP = os.path.abspath(os.path.join(CHIP_ROOT, 'out',
                                           'linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test', 'chip-all-clusters-app'))
DEFAULT_CHIP_TOOL = os.path.abspath(os.path.join(CHIP_ROOT, 'out', 'debug', 'standalone', 'chip-tool'))

test_cases = [
    {
        'name': 'tc_dac_revoked',
        'dac_provider': 'revoked-dac-01.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 302
    },
    {
        'name': 'tc_pai_revoked',
        'dac_provider': 'revoked-pai.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 202
    },
    {
        'name': 'tc_dac_and_pai_revoked',
        'dac_provider': 'revoked-dac-and-pai.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 208
    },
    {
        'name': 'tc_dac_revoked_using_delegated_crl_signer',
        'dac_provider': 'indirect-revoked-dac-01.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 302
    },
    {
        'name': 'tc_pai_revoked_using_delegated_crl_signer',
        'dac_provider': 'indirect-revoked-pai-03.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 202
    },
    {
        'name': 'tc_dac_and_pai_revoked_using_delegated_crl_signer',
        'dac_provider': 'indirect-revoked-dac-01-pai-03.json',
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': False,
        'expects_att_err': 208
    },
    {
        'name': 'tc_dac_and_pai_valid',
        'dac_provider': None,
        'revocation_set_path': 'revocation-set.json',
        'expects_commissioning_success': True,
        'expects_att_err': None
    }
]

# Check if the attestation error is present in the commissioner log
# Returns True if the attestation error is present and matches the expected error
# Returns False otherwise


def get_attestation_error_from_log(log_file: str, expected_att_err: Optional[int]) -> bool:
    att_err_matches = True
    if expected_att_err:
        with open(log_file, 'r') as log_file:
            log_content = log_file.read()
            # Look for attestation error pattern
            pattern = r"Failed in verifying 'Attestation Information' command received from the device: err (\d+)."
            match = re.search(pattern, log_content)
            if match:
                actual_att_err = int(match.group(1))
                if actual_att_err != expected_att_err:
                    print(f"WARNING: Expected attestation error {expected_att_err} but got {actual_att_err}")
                    att_err_matches = False
            else:
                print("WARNING: No attestation error found in the commissioner log")
                att_err_matches = False
    return att_err_matches


@click.command()
@click.option('--app', type=click.Path(exists=True), default=DEFAULT_APP,
              help='Path to local application to use')
@click.option('--chip-tool', type=click.Path(exists=True), default=DEFAULT_CHIP_TOOL,
              help='Path to local chip tool if automated testing is requested')
@click.option('--out-dir', type=click.Path(), default='test_results')
@click.option('--manual', is_flag=True, show_default=True, default=False,
              help="Run the tests manually with a different DUT(commissioner) and enter the results by hand")
def main(app: str, chip_tool: str, out_dir: str, manual: bool):
    dac_provider_base_path = os.path.abspath(os.path.join(
        CHIP_ROOT, 'credentials/test/revoked-attestation-certificates/dac-provider-test-vectors'))
    revocation_set_base_path = os.path.abspath(os.path.join(
        CHIP_ROOT, 'credentials/test/revoked-attestation-certificates/revocation-sets'))
    subprocess.call(f'mkdir -p {out_dir}', shell=True)
    results = {}

    for test_case in test_cases:
        if test_case['dac_provider'] is None:
            dac_provider_path = None
        else:
            dac_provider_path = str(os.path.join(dac_provider_base_path,
                                    test_case['dac_provider'])) if test_case['dac_provider'] else None

        # remove any existing KVS files (if any), equivalent to Factory Resetting the TH(all-clusters-app)
        subprocess.call("rm -f /tmp/tmpkvs*", shell=True)

        app_args = '--trace_decode 1 --KVS /tmp/tmpkvs'
        app_args += f' --dac_provider {dac_provider_path}' if dac_provider_path else ''
        app_cmd = app + ' ' + app_args

        app_log_file_name = f'{out_dir}/{str(test_case["name"])}_app.log'
        commission_log_file_name = f'{out_dir}/{str(test_case["name"])}_commissioner.log'

        with open(app_log_file_name, 'w') as app_log_file:
            print(f'Starting all clusters app with certs from {test_case["dac_provider"]}')
            app_process = subprocess.Popen(app_cmd.split(), stdout=app_log_file, stderr=app_log_file)
            if manual:
                input_done = False
                while not input_done:
                    print('Please commission the DUT, with:')
                    print('  QR Code: "MT:-24J0AFN00KA0648G00"')
                    print(f'  Revocation Set: {revocation_set_base_path}/{test_case["revocation_set_path"]}\n')

                    resp = input(f'Input "Y" if DUT(commissioner) successfully commissions the TH without any warnings:\n'
                                 f'Input "N" if commissioner warns about commissioning the non-genuine device, Or '
                                 f'Commissioning fails with device attestation error "{test_case["expects_att_err"]}"\n')

                    if resp == 'y' or resp == "Y":
                        commissioning_ok = True
                        input_done = True
                    elif resp == 'n' or resp == 'N':
                        commissioning_ok = False
                        input_done = True
                    else:
                        print('Unknown key entered, please try again')

                    # We can not get the attestation error in the manual mode, setting it to True
                    att_err_matches = True
            else:
                # Commission the dut from another process and auto-record the result
                chip_tool_cmd = chip_tool + ' pairing code 0x12344321 MT:-24J042C00KA0648G00 '
                chip_tool_cmd += f'--dac-revocation-set-path {revocation_set_base_path}/{test_case["revocation_set_path"]}' if test_case['revocation_set_path'] else ''
                print(f'Starting commissioner with revocation set: {test_case["revocation_set_path"]}')

                with open(commission_log_file_name, 'w') as commissioner_log_file:
                    commissioning_result = subprocess.run(chip_tool_cmd.split(
                    ), stdout=commissioner_log_file, stderr=commissioner_log_file)
                    commissioning_ok = commissioning_result.returncode == 0
                    print(f'Commissioning successful? {commissioning_ok}')

                # if we expect an attestation error, check if it is present in the commissioner log
                att_err_matches = get_attestation_error_from_log(commission_log_file_name, test_case['expects_att_err'])

            app_process.send_signal(signal.SIGINT.value)
            app_process.wait()

        results[test_case['name']] = (commissioning_ok == test_case['expects_commissioning_success']) and att_err_matches
        print(f'Test case {test_case["name"]} passed? {results[test_case["name"]]}\n')

    summary_file_name = f'{out_dir}/summary_{test_case["name"]}.log'

    test_passed = all(results.values())
    print(f"Test passed? {test_passed}\n")
    with open(summary_file_name, 'w') as summary_file:
        summary_file.write(f"Test passed? {test_passed}\n\n")
        for k, v in results.items():
            passed = "PASSED" if v is True else "FAILED"
            summary_file.write(f'{k}:{passed}')


if __name__ == '__main__':
    main()
