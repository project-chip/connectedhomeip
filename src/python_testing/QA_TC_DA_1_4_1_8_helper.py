#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2022 Project CHIP Authors
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

import click
import json
import os
import signal
import subprocess

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
DEFAULT_APP = os.path.abspath(os.path.join(CHIP_ROOT, 'out',
                                           'linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test', 'chip-all-clusters-app'))
DEFAULT_CHIP_TOOL = os.path.abspath(os.path.join(CHIP_ROOT, 'out', 'debug', 'standalone', 'chip-tool'))


def get_pid(dirname: str) -> int:
    if 'fallback_encoding' in dirname:
        return 177
    return 32768

# 1.4 is all the failure cases, 1.8 is all the success cases


@click.command()
@click.option('--app', type=click.Path(exists=True), default=DEFAULT_APP,
              help='Path to local application to use')
@click.option('--chip-tool', type=click.Path(exists=True), default=DEFAULT_CHIP_TOOL,
              help='Path to local chip tool if automated testing is requested')
@click.option('--out-dir', type=click.Path(), default='test_results')
@click.option('--test-case',
              type=click.Choice(['1.4', '1.8'], case_sensitive=False))
@click.option('--manual', is_flag=True, show_default=True, default=False, help="Run the tests manually from a different TH and enter the results by hand")
def main(app: str, chip_tool: str, out_dir: str, test_case: str, manual: bool):
    success_tc = test_case == '1.8'
    cert_path = os.path.abspath(os.path.join(CHIP_ROOT, 'credentials/development/commissioner_dut'))

    # struct_cd_version_number_wrong - excluded because this is a DCL test not covered by cert
    # struct_cd_cert_id_mismatch - excluded because this is a DCL test not covered by cert
    exclude_cases = ['struct_cd_version_number_wrong', 'struct_cd_cert_id_mismatch']

    subprocess.call(f'mkdir -p {out_dir}', shell=True)
    results = {}
    for p in os.listdir(cert_path):
        if str(p) in exclude_cases:
            continue

        path = str(os.path.join(cert_path, p, 'test_case_vector.json'))
        with open(path, 'r') as f:
            j = json.loads(f.read())
            success_expected = j['is_success_case'].lower() == 'true'

        if (success_expected != success_tc):
            continue

        pid = get_pid(p)
        # Start the chip tool running, as for manual verification
        subprocess.call("rm -f tmpkvs", shell=True)
        app_args = f'--trace_decode 1 --dac_provider {path} --product-id {pid}  --KVS tmpkvs'
        app_log_file_name = f'{out_dir}/{str(p)}_app.log'
        commission_log_file_name = f'{out_dir}/{str(p)}_commissioner.log'
        app_cmd = app + ' ' + app_args
        with open(app_log_file_name, 'w') as app_log_file:
            print(f'Starting all clusters app with certs from {p}')
            app_process = subprocess.Popen(app_cmd.split(), stdout=app_log_file, stderr=app_log_file)
            if manual:
                input_done = False
                while not input_done:
                    resp = input('Please commissiong the DUT. Please input Y if the commissioning is successful, N if it is unsuccessful')
                    if resp == 'y' or resp == "Y":
                        commissioning_ok = True
                        input_done = True
                    elif resp == 'n' or resp == 'N':
                        commissioning_ok = False
                        input_done = True
                    else:
                        print('Unknown key entered, please try again')
            else:
                # Commission the dut from another process and auto-record the result
                chip_tool_cmd = chip_tool + ' pairing onnetwork-long 0x12344321 20202021 3840'
                with open(commission_log_file_name, 'w') as commissioner_log_file:
                    commissioning_result = subprocess.run(chip_tool_cmd.split(
                    ), stdout=commissioner_log_file, stderr=commissioner_log_file)
                    print(commissioning_result)
                    commissioning_ok = commissioning_result.returncode == 0
                    print(f'Commissioning successful? {commissioning_ok}')
            app_process.send_signal(signal.SIGINT.value)
            app_process.wait()
        results[p] = commissioning_ok == success_tc

    summary_file_name = f'{out_dir}/summary_{test_case}.log'

    print(results)
    with open(summary_file_name, 'w') as summary_file:
        for k, v in results.items():
            passed = "PASSED" if v is True else "FAILED"
            summary_file.write(f'{k}:{passed}\n')


if __name__ == '__main__':
    main()
