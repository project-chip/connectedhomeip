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
from typing import TYPE_CHECKING

from utils.artifact import create_standard_log_name, log, safe_mkdir
from utils.host_platform import is_mac
from utils.shell import Bash

from ... import config
from ..base import AndroidStream

if TYPE_CHECKING:
    from capture.platform.android import Android

logger = log.get_logger(__file__)


class AndroidPcap(AndroidStream):

    def __init__(self, platform: "Android"):
        self.logger = logger
        self.platform = platform
        self.target_dir = "/sdcard/Download"
        self.pcap_artifact = create_standard_log_name("android_tcpdump", "pcap", parent=self.platform.artifact_dir)
        self.pcap_phone_out_path = f"{self.target_dir}/{os.path.basename(self.pcap_artifact)}"
        self.pcap_phone_bin_location = "tcpdump" if platform.capabilities.c_has_tcpdump \
            else f"{self.target_dir}/tcpdump"
        self.pcap_command = f"shell {self.pcap_phone_bin_location} -w {self.pcap_phone_out_path}"
        self.pcap_proc = platform.get_adb_background_command(self.pcap_command)
        self.pcap_pull = False
        self.pcap_pull_command = f"pull {self.pcap_phone_out_path} {self.pcap_artifact}"
        self.build_dir = os.path.join(os.path.dirname(__file__), "BUILD")

    async def pull_packet_capture(self) -> None:
        if self.pcap_pull:
            self.logger.info("Attempting to pull android pcap")
            await asyncio.sleep(3)
            self.platform.run_adb_command(self.pcap_pull_command)
            self.pcap_pull = False

    async def start(self):
        if not self.platform.capabilities.c_has_root:
            self.logger.warning("Phone is not rooted, cannot take pcap!")
            return
        if self.platform.capabilities.c_has_tcpdump:
            self.logger.info("tcpdump already available; using!")
            self.pcap_proc.start_command()
            self.pcap_pull = True
            return
        if not config.enable_build_push_tcpdump:
            self.logger.critical("Android TCP Dump build and push disabled in configs!")
            return
        if not os.path.exists(os.path.join(self.build_dir, "tcpdump")):
            self.logger.warning("tcpdump bin not found, attempting to build, please wait a few moments!")
            safe_mkdir(self.build_dir)
            if is_mac():
                build_script = os.path.join(os.path.dirname(__file__), "mac_build_tcpdump_64.sh")
                Bash(f"{build_script} 2>&1 >> BUILD_LOG.txt", sync=True, cwd=self.build_dir).start_command()
            else:
                build_script = os.path.join(os.path.dirname(__file__), "linux_build_tcpdump_64.sh")
                Bash(f"{build_script} 2>&1 >> BUILD_LOG.txt", sync=True, cwd=self.build_dir).start_command()
        else:
            self.logger.warning("Reusing existing tcpdump build")
        if not self.platform.run_adb_command(f"shell ls {self.target_dir}/tcpdump").finished_success():
            self.logger.warning("Pushing tcpdump to device")
            self.platform.run_adb_command(f"push {os.path.join(self.build_dir, 'tcpdump')} f{self.target_dir}")
            self.platform.run_adb_command(f"chmod +x {self.target_dir}/tcpdump")
        else:
            self.logger.info("tcpdump already in the expected location, not pushing!")
        self.logger.info("Starting Android pcap command")
        self.pcap_proc.start_command()
        self.pcap_pull = True

    async def run_observer(self) -> None:
        while True:
            # TODO: Implement, need to restart w/ new out file (no append) and keep pull manifest, much like `screen`
            await asyncio.sleep(120)

    async def stop(self):
        self.logger.info("Stopping android pcap proc")
        self.pcap_proc.stop_command()
        await self.pull_packet_capture()
