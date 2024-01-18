#!/usr/bin/env -S python3 -B

# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
import signal
import subprocess
import sys
import time

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


class TestDriver:
    def __init__(self):
        self.app_path = os.path.abspath(os.path.join(DEFAULT_CHIP_ROOT, 'out',
                                                     'linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test', 'chip-all-clusters-app'))
        self.run_python_test_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'run_python_test.py'))

        self.script_path = os.path.abspath(os.path.join(
            DEFAULT_CHIP_ROOT, 'src', 'python_testing', 'TestTimeSyncTrustedTimeSource.py'))
        if not os.path.exists(self.app_path):
            msg = 'chip-all-clusters-app not found'
            logging.error(msg)
            raise FileNotFoundError(msg)
        if not os.path.exists(self.run_python_test_path):
            msg = 'run_python_test.py script not found'
            logging.error(msg)
            raise FileNotFoundError(msg)
        if not os.path.exists(self.script_path):
            msg = 'TestTimeSyncTrustedTimeSource.py script not found'
            logging.error(msg)
            raise FileNotFoundError(msg)

    def get_base_run_python_cmd(self, run_python_test_path, app_path, app_args, script_path, script_args):
        return f'{str(run_python_test_path)} --app {str(app_path)} --app-args "{app_args}" --script {str(script_path)} --script-args "{script_args}"'

    def run_test_section(self, app_args: str, script_args: str, factory_reset_all: bool = False, factory_reset_app: bool = False) -> int:
        # quotes are required here
        cmd = self.get_base_run_python_cmd(self.run_python_test_path, self.app_path, app_args,
                                           self.script_path, script_args)
        if factory_reset_all:
            cmd = cmd + ' --factoryreset'
        if factory_reset_app:
            cmd = cmd + ' --factoryreset-app-only'

        logging.info(f'Running cmd {cmd}')

        process = subprocess.Popen(cmd, stdout=sys.stdout, stderr=sys.stderr, shell=True, bufsize=1)

        return process.wait()


def kill_process(app2_process):
    logging.warning("Stopping app with SIGINT")
    app2_process.send_signal(signal.SIGINT.value)
    app2_process.wait()


def main():
    # in the first round, we're just going to commission the device
    base_app_args = '--discriminator 1234 --KVS kvs1'
    app_args = base_app_args
    base_script_args = '--storage-path admin_storage.json --discriminator 1234 --passcode 20202021'
    script_args = base_script_args + ' --commissioning-method on-network --commission-only'

    driver = TestDriver()
    ret = driver.run_test_section(app_args, script_args, factory_reset_all=True)
    if ret != 0:
        return ret

    # For this test, we need to have a time source set up already for the simulated no-internal-time source to query.
    # This means it needs to be commissioned onto the same fabric, and the ACLs need to be set up to allow
    # access to the time source cluster.
    # This simulates the second device, so its using a different KVS and nodeid, which will allow both apps to run simultaneously
    app2_args = '--discriminator 1235 --KVS kvs2 --secured-device-port 5580'
    script_args = '--storage-path admin_storage.json --discriminator 1235 --passcode 20202021 --commissioning-method on-network --dut-node-id 2 --tests test_SetupTimeSourceACL'

    ret = driver.run_test_section(app2_args, script_args, factory_reset_app=True)
    if ret != 0:
        return ret

    # Now we've got something commissioned, we're going to test what happens when it resets, but we're simulating no time.
    # In this case, the commissioner hasn't set the time after the reboot, so there should be no time returned (checked in test)
    app_args = base_app_args + ' --simulate-no-internal-time'
    script_args = base_script_args + ' --tests test_SimulateNoInternalTime'

    ret = driver.run_test_section(app_args, script_args)
    if ret != 0:
        return ret

    # Make sure we come up with internal time correctly if we don't set that flag
    app_args = base_app_args
    script_args = base_script_args + ' --tests test_HaveInternalTime'

    ret = driver.run_test_section(app_args, script_args)
    if ret != 0:
        return ret

    # Bring up app2 again, it needs to run for the duration of the next test so app1 has a place to query time
    # App1 will come up, it is simulating having no internal time (confirmed in previous test), but we have
    # set up app2 as the trusted time source, so it should query out to app2 for the time.
    app2_cmd = str(driver.app_path) + ' ' + app2_args
    app2_process = subprocess.Popen(app2_cmd.split(), stdout=sys.stdout, stderr=sys.stderr, bufsize=0)

    # Give app2 a second to come up and start advertising
    time.sleep(1)

    # This first test ensures that we read from the trusted time source right after it is set.
    app_args = base_app_args + ' --simulate-no-internal-time --trace_decode 1'
    script_args = base_script_args + ' --tests test_SetAndReadFromTrustedTimeSource --int-arg trusted_time_source:2'
    ret = driver.run_test_section(app_args, script_args)
    if ret != 0:
        kill_process(app2_process)
        return ret

    # This next test ensures the trusted time source is saved during a reboot
    script_args = base_script_args + ' --tests test_ReadFromTrustedTimeSource'
    ret = driver.run_test_section(app_args, script_args)

    kill_process(app2_process)
    sys.exit(ret)


if __name__ == '__main__':
    main()
