#!/usr/bin/env python3
"""
Copyright (c) 2021 Project CHIP Authors
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

import json
import logging
import os
import subprocess
import sys
import time

from helper.CHIPTestBase import CHIPVirtualHome

logger = logging.getLogger("AndroidTest")
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = "http://localhost:5000"
CHIP_REPO = os.path.join(os.path.abspath(
    os.path.dirname(__file__)), "..", "..", "..")


class TestAndroidController(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_controller_test()

    def run_controller_test(self):
        self.android_emulator_device_id = [device["id"] for device in self.non_ap_devices
                                   if device["base_image"] == "android-emulator"][0]
        end_device = [device for device in self.non_ap_devices
                      if device["base_image"] == "connectedhomeip/chip-cirque-device-base"][0]
        end_device_id = end_device["id"]

        self.execute_device_cmd(end_device_id, "CHIPCirqueDaemon.py -- run {} --thread".format(
            os.path.join(CHIP_REPO, "out/debug/standalone/chip-all-clusters-app")))

        self.execute_device_cmd(self.android_emulator_device_id, "adb start-server")

        # Can take a while for emulator to come online, check every 30s and timeout after 5min.
        timeout_seconds = 300
        interval_seconds = 30
        adb_return_code = "-1"
        for i in range(timeout_seconds // interval_seconds):
            time.sleep(interval_seconds)
            adb_result = self.execute_device_cmd(self.android_emulator_device_id, "adb shell")
            adb_return_code = adb_result["return_code"]
            if adb_return_code == "0":
                self.run_gradle_test(end_device["description"]["ipv4_addr"])
                break

        self.assertTrue(adb_return_code == "0")

    def run_gradle_test(self, ip_address):
        chiptool_path = CHIP_REPO + "/src/android/CHIPTool"
        gradle_test_command = [chiptool_path + "/gradlew connectedAndroidTest",
            "-p", chiptool_path,
            "-Pandroid.testInstrumentationRunnerArguments.class={}".format(
                "com.google.chip.chiptool.OnOffUiTest"),
            "-Pandroid.testInstrumentationRunnerArguments.endDeviceIpAddress=\"{}\"".format(
                ip_address),
        ]
        gradle_test_command_str = ' '.join(gradle_test_command)

        test_result = self.execute_device_cmd(self.android_emulator_device_id, gradle_test_command_str)
        self.assertEqual(test_result["return_code"], "0")

    def save_device_logs(self):
        super().save_device_logs()

        if self.android_emulator_device_id == None:
            return

        log_dir = os.environ.get("DEVICE_LOG_DIR", None)
        gradle_log_dir = os.path.join(log_dir, "gradle")
        logcat_file_in_container = "/tmp/logcat.log"
        logcat_file = os.path.join(log_dir, "logcat.log")

        # Copy gradle logs from the docker container into the Cirque logs dir.
        copy_timeout_seconds = 15
        copy_report_command = ["docker", "cp", self.android_emulator_device_id + ":" + CHIP_REPO + "/src/android/CHIPTool/app/build/reports"
            , gradle_log_dir]
        copy_report_ret = subprocess.call(copy_report_command, timeout=copy_timeout_seconds)
        if copy_report_ret != 0:
            print("Failed to copy Gradle test reports: " + str(copy_ret))

        # Write logcat output to the Cirque logs dir.
        logcat_timeout_seconds = 15
        logcat_command = "docker exec -it {} adb logcat -d > {}".format(self.android_emulator_device_id, logcat_file)
        logcat_ret = subprocess.call(logcat_command, timeout=logcat_timeout_seconds, shell=True)
        if logcat_ret != 0:
            print("Failed to write logcat output: " + str(logcat_ret))

def _parse_mount_dir(config):
    for v in config.values():
        if "Mount" not in v.get("capability", {}):
            continue
        _mount_pairs = v.get("mount_pairs", [])
        for mount in _mount_pairs:
            mount[0] = mount[0].format(chip_repo=CHIP_REPO)
            mount[1] = mount[1].format(chip_repo=CHIP_REPO)
        v["mount_pairs"] = _mount_pairs
    return config


DEVICE_CONFIG = {}
with open(os.path.join(CHIP_REPO, "src/test_driver/linux-cirque/topologies/one_node_one_android.json"), "r") as f:
    config_operations = [_parse_mount_dir]
    DEVICE_CONFIG = json.load(f)
    for op in config_operations:
        DEVICE_CONFIG = op(DEVICE_CONFIG)

if __name__ == "__main__":
    sys.exit(TestAndroidController(DEVICE_CONFIG).run_test())
