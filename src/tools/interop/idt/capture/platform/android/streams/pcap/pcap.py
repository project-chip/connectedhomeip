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

from capture.utils.artifact import create_standard_log_name, safe_mkdir
from capture.utils.shell import Bash

from ... import config
from ..base import AndroidStream

if TYPE_CHECKING:
    from capture.platform.android import Android

import log

logger = log.get_logger(__file__)


class AndroidPcap(AndroidStream):

    def __init__(self, platform: "Android"):
        self.logger = logger
        self.platform = platform
        self.pcap_artifact = create_standard_log_name("android_tcpdump", "cap", parent=platform.artifact_dir)
        self.pcap_phone_out_path = f"/sdcard/Movies/{os.path.basename(self.pcap_artifact)}"
        self.pcap_phone_bin_location = "tcpdump" if platform.capabilities.c_has_tcpdump else "/sdcard/Movies/tcpdump"
        self.pcap_command = f"shell {self.pcap_phone_bin_location} -w {self.pcap_phone_out_path}"
        self.pcap_proc = platform.get_adb_background_command(self.pcap_command)
        self.pcap_pull = False
        self.pcap_pull_command = f"pull {self.pcap_phone_out_path} {self.pcap_artifact}"
        self.build_dir = os.path.join(os.path.dirname(__file__), "../BUILD")

    async def pull_packet_capture(self) -> None:
        if self.pcap_pull:
            self.logger.info("Attempting to pull android pcap")
            await asyncio.sleep(3)
            self.platform.run_adb_command(self.pcap_pull_command)
            self.pcap_pull = False

    async def start(self):
        # TODO: Build on macOS, no wget, check root
        if self.platform.capabilities.c_has_tcpdump:
            self.logger.info("tcpdump already available; using!")
            self.pcap_proc.start_command()
            self.pcap_pull = True
            return
        if not config.enable_build_push_tcpdump:
            self.logger.critical("Android TCP Dump build and push disabled in configs!")
            return
        if not os.path.exists(os.path.join(self.build_dir, "tcpdump")):
            safe_mkdir(self.build_dir)
            self.logger.warning("tcpdump bin not found, attempting to build, please wait a few moments!")
            build_script = os.path.join(os.path.dirname(__file__), "build_tcpdump_64.sh")
            Bash(f"{build_script} 2>&1 >> BUILD_LOG.txt", sync=True, cwd=self.build_dir).start_command()
        else:
            self.logger.warning("Reusing existing tcpdump build")
        if not self.platform.run_adb_command("shell ls /sdcard/Movies/tcpdump").finished_success():
            self.logger.warning("Pushing tcpdump to device")
            self.platform.run_adb_command(f'push {os.path.join(self.build_dir, "tcpdump")} /sdcard/Movies/')
            self.platform.run_adb_command("chmod +x /sdcard/Movies/tcpdump")
        else:
            self.logger.info("tcpdump already in the expected location, not pushing!")
        self.logger.info("Starting Android pcap command")
        self.pcap_proc.start_command()
        self.pcap_pull = True

    async def run_observer(self) -> None:
        # TODO: Implement
        pass

    async def stop(self):
        self.logger.info("Stopping android pcap proc")
        self.pcap_proc.stop_command()
        await self.pull_packet_capture()
