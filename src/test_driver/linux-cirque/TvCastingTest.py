#!/usr/bin/env python3
"""
Copyright (c) 2026 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

# User Directed Commissioning between chip-tv-casting-app (commissionee) and
# chip-tv-app (commissioner), followed by a ContentLauncher LaunchURL from the
# casting app. Each step is driven by the log line that shows the previous
# step finished; shell commands are fed to the apps through a FIFO on stdin.

import logging
import shlex
import sys

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import CHIP_REPO_STR, CHIP_TV_APP_ESC, CHIP_TV_CASTING_APP_ESC

logger = logging.getLogger('TvCastingTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = "http://localhost:5000"
TEST_SETUP_PIN = 20202021
APP_STDIN_FIFO = "/tmp/app_stdin"

DEVICE_CONFIG = {
    'device0': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 10},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 10},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
}


class TestTvCasting(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_tv_casting_test()

    def start_app_with_stdin_fifo(self, device_id, app_esc):
        # The FIFO is opened read-write so the app never sees EOF on stdin
        # between commands.
        fifo_esc = shlex.quote(APP_STDIN_FIFO)
        setup_cmd = f"rm -rf /tmp/chip_* {fifo_esc} && mkfifo {fifo_esc}"
        self.run_checked_device_cmd(device_id, f"sh -c {shlex.quote(setup_cmd)}", "FIFO setup")
        app_cmd = f"exec {app_esc} <>{fifo_esc}"
        self.run_checked_device_cmd(device_id, f"CHIPCirqueDaemon.py -- run sh -c {shlex.quote(app_cmd)}", "App launch")

    def send_shell_command(self, device_id, command):
        fifo_esc = shlex.quote(APP_STDIN_FIFO)
        write_cmd = f"echo {shlex.quote(command)} >{fifo_esc}"
        self.run_checked_device_cmd(
            device_id,
            f"timeout 5 sh -c {shlex.quote(write_cmd)}",
            f"FIFO write ({command})",
        )

    def wait_for_log(self, device_id, pattern, timeout, note):
        self.assertTrue(self.wait_for_device_output(device_id, pattern, timeout),
                        f"{note}: '{pattern}' not found in {self.get_device_pretty_id(device_id)} log within {timeout}s")

    def run_tv_casting_test(self):
        tv_app_id = self.non_ap_devices[0]['id']
        casting_app_id = self.non_ap_devices[1]['id']

        self.start_app_with_stdin_fifo(tv_app_id, CHIP_TV_APP_ESC)
        self.wait_for_log(tv_app_id, "Server Listening", 30, "tv-app did not start")

        self.start_app_with_stdin_fifo(casting_app_id, CHIP_TV_CASTING_APP_ESC)
        self.wait_for_log(casting_app_id, "Server Listening", 30, "tv-casting-app did not start")

        # chip-tv-casting-app automatically starts DNS-SD (_matterd._udp)
        # commissioner discovery on boot when invoked with no arguments.
        self.wait_for_log(casting_app_id, "Discovered Commissioner #0", 60, "tv-app was not discovered")

        # User Directed Commissioning request; the tv-app asks the user to approve.
        self.send_shell_command(casting_app_id, "cast request 0")
        self.wait_for_log(tv_app_id, "Via Shell Enter: controller ux ok|cancel", 60, "UDC request not received")

        self.send_shell_command(tv_app_id, "controller ux ok")
        self.wait_for_log(tv_app_id, "Via Shell Enter: controller ux ok [passcode]", 30, "tv-app did not ask for the passcode")

        self.send_shell_command(tv_app_id, f"controller ux ok {TEST_SETUP_PIN}")
        self.wait_for_log(casting_app_id, "Commissioning completed successfully", 120, "Commissioning did not complete")

        # After commissioning the casting app connects and sends ContentLauncher LaunchURL.
        self.wait_for_log(tv_app_id, "ContentLauncherManager::HandleLaunchUrl", 60, "LaunchURL not handled by tv-app")

        self.assertTrue(self.sequenceMatch(self.get_device_log(tv_app_id).decode('utf-8', errors='replace'), [
            "Via Shell Enter: controller ux ok|cancel",
            "Via Shell Enter: controller ux ok [passcode]",
            "ContentLauncherManager::HandleLaunchUrl",
        ]), "tv-app log is missing the expected UDC -> commissioning -> LaunchURL sequence")


if __name__ == "__main__":
    sys.exit(TestTvCasting(DEVICE_CONFIG).run_test())
