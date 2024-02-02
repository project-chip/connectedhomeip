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

import asyncio
import os
import shutil
from typing import TYPE_CHECKING

from utils.artifact import create_standard_log_name, log, safe_mkdir
from utils.host import current_platform
from utils.shell import Bash

from ... import config
from ..base import AndroidStream

if TYPE_CHECKING:
    from features.capture.platform.android import Android

logger = log.get_logger(__file__)


class AndroidPcap(AndroidStream):

    def __init__(self, platform: "Android"):
        self.pcap_artifact = None
        self.pcap_phone_out_path = None
        self.pcap_command = None
        self.pcap_proc = None
        self.logger = logger
        self.platform = platform
        self.target_output_dir = "/sdcard/Download"
        self.pcap_phone_bin_location = "tcpdump" if platform.capabilities.c_has_tcpdump \
            else f"{self.target_output_dir}/tcpdump"
        self.pcap_pull = False
        self.file_counter = 0
        self.build_dir = os.path.join(os.path.dirname(__file__), "BUILD")
        self.pull_commands: [str] = []
        self.manifest_file = os.path.join(platform.artifact_dir, "packet_capture_manifest.txt")

    def start_pcap(self) -> None:
        self.pcap_artifact = create_standard_log_name("android_tcpdump" + str(self.file_counter),
                                                      "pcap",
                                                      parent=self.platform.artifact_dir)
        self.pcap_phone_out_path = f"{self.target_output_dir}/{os.path.basename(self.pcap_artifact)}"
        pcap_pull_command = f"pull {self.pcap_phone_out_path} {self.pcap_artifact}"
        self.pull_commands.append(pcap_pull_command)
        with open(self.manifest_file, "a+") as manifest:
            manifest.write(pcap_pull_command + "\n")
        self.pcap_command = f"shell {self.pcap_phone_bin_location} -w {self.pcap_phone_out_path}"
        self.pcap_proc = self.platform.get_adb_background_command(self.pcap_command)
        self.pcap_proc.start_command()
        self.file_counter += 1
        self.pcap_pull = True
        self.logger.info(f"New pcap started {self.pcap_phone_out_path} {self.pcap_artifact}")

    async def start(self) -> None:
        # TODO: Move build to setup function?
        if not self.platform.capabilities.c_has_root:
            self.logger.warning("Phone is not rooted, cannot take pcap!")
            return
        if self.platform.capabilities.c_has_tcpdump:
            self.logger.info("tcpdump already available; using!")
            self.start_pcap()
            return
        if not config.ENABLE_BUILD_PUSH_TCPDUMP:
            self.logger.critical("Android TCP Dump build and push disabled in configs!")
            return
        if not os.path.exists(os.path.join(self.build_dir, "tcpdump")):
            self.logger.warning("tcpdump bin not found, attempting to build, please wait a few moments!")
            safe_mkdir(self.build_dir)
            build_script = "mac_build_tcpdump_64.sh" if current_platform.is_mac() else "linux_build_tcpdump_64.sh"
            build_script = os.path.join(os.path.dirname(__file__), build_script)
            build_command = Bash(f"{build_script} 2>&1 >> BUILD_LOG.txt", sync=True, cwd=self.build_dir)
            build_command.start_command()
            if not build_command.finished_success():
                self.logger.error("Build failed, cleaning build dir!")
                shutil.rmtree(self.build_dir)
                return
        else:
            self.logger.warning("Reusing existing tcpdump build")
        if not self.platform.run_adb_command(f"shell ls {self.target_output_dir}/tcpdump").finished_success():
            self.logger.warning("Pushing tcpdump to device")
            push_command = self.platform.run_adb_command(
                f"push {os.path.join(self.build_dir, 'tcpdump')} f{self.target_output_dir}")
            chmod_command = self.platform.run_adb_command(f"chmod +x {self.target_output_dir}/tcpdump")
            if not push_command.finished_success() or not chmod_command.finished_success():
                self.logger.error("Failed to push tcp dump!")
                return
        else:
            self.logger.info("tcpdump already in the expected location, not pushing!")
        self.logger.info("Starting Android pcap command")
        self.start_pcap()

    async def run_observer(self) -> None:
        while True:
            if not self.pcap_pull:
                await asyncio.sleep(120)
            else:
                if not self.pcap_proc.command_is_running():
                    self.logger.warning(
                        f"Pcap proc needs restart (unexpected!) {self.platform.device_id}")
                    await self.start()
                await asyncio.sleep(8)

    async def pull_packet_capture(self) -> None:
        if self.pcap_pull:
            self.logger.info("Attempting to pull android pcap")
            await asyncio.sleep(3)
            with open(self.manifest_file) as manifest:
                for line in manifest:
                    self.platform.run_adb_command(line)
            self.pcap_pull = False

    async def stop(self) -> None:
        self.logger.info("Stopping android pcap proc")
        self.pcap_proc.stop_command()
        await self.pull_packet_capture()
