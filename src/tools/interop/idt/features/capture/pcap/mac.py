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
import shutil
from typing import Optional

from features.capture.pcap.base import PacketCaptureBase
from utils.shell import Bash


class MacPacketCaptureRunner(PacketCaptureBase):

    def __init__(self,
                 artifact_dir: str,
                 interface: str,
                 monitor_mode: bool,
                 channel: int,
                 band: str,
                 width: str) -> None:
        super().__init__(artifact_dir, interface, monitor_mode, channel, band, width)
        if self.monitor_mode:
            self.pcap_command = f"sudo airport sniff {self.band}g{self.channel}"
            if width:
                self.pcap_command += "/"+width
            self.logger.info(f"Mac pcap command {self.pcap_command}")
        else:
            self.pcap_command: Optional[str] = f"tcpdump -i {self.interface} -n -w {self.output_path}"
        self.pcap_proc = Bash(self.pcap_command)

    def start_pcap(self) -> None:
        if self.monitor_mode:
            self.logger.warning("Disassociating from Wi-Fi, authorize if prompted!")
            Bash("sudo airport -z", sync=True).start_command()  # Do this to force a sudo auth
            # We can remove this line and remove sudo from self.pcap_command above;
            # simply call super() to add sudo if needed.
            # However, airport does run without sudo and its unclear if that
            # has any impact, so being explicit
            self.logger.info("Starting 802.11 pcap using airport")
        super().start_pcap()

    def find_latest_airport_pcap(self) -> str:
        captures = [f for f in os.listdir("/tmp") if f.startswith("airportSniff")]
        if captures:
            path = os.path.join("/tmp", captures[0])
            oldest_mtime = os.path.getmtime(path)
            oldest_path = path
            for capture in captures:
                path = os.path.join("/tmp", capture)
                new_mtime = os.path.getmtime(path)
                if new_mtime > oldest_mtime:
                    oldest_mtime = new_mtime
                    oldest_path = path
            return oldest_path
        else:
            self.logger.error("Could not locate latest airport pcap!")
            return ""

    def stop_pcap(self) -> None:
        super().stop_pcap()
        if self.monitor_mode:
            airport_location = self.find_latest_airport_pcap()
            if not airport_location:
                self.logger.error("Could not locate latest airport pcap")
            else:
                self.logger.info(f"Found latest airport pcap at {airport_location}")
                shutil.copy(airport_location, self.output_path)
                self.logger.info(f"Copied airport pcap to artifacts {self.output_path}")
