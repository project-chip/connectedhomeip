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
import time

from utils.artifact import create_standard_log_name, log
from utils.shell import Bash

logger = log.get_logger(__file__)


class PacketCaptureRunner:

    def __init__(self, artifact_dir: str, interface: str) -> None:
        self.logger = logger
        self.artifact_dir = artifact_dir
        self.output_path = str(
            os.path.join(
                self.artifact_dir,
                create_standard_log_name(
                    "pcap",
                    "pcap")))
        self.start_delay_seconds = 2
        self.interface = interface
        self.pcap_command = f"tcpdump -i {self.interface} -n -w {self.output_path}"
        self.pcap_proc = Bash(self.pcap_command)

    def start_pcap(self) -> None:
        self.pcap_proc.start_command()
        self.logger.info("Pausing to check if pcap started...")
        time.sleep(self.start_delay_seconds)
        if not self.pcap_proc.command_is_running():
            self.logger.error(
                "Pcap did not start, you might need root; please authorize if prompted.")
            Bash("sudo echo \"\"", sync=True).start_command()
            self.logger.warning("Retrying pcap with sudo...")
            self.pcap_command = f"sudo {self.pcap_command}"
            self.pcap_proc = Bash(self.pcap_command)
            self.pcap_proc.start_command()
            time.sleep(self.start_delay_seconds)
        if not self.pcap_proc.command_is_running():
            self.logger.error("Failed to start pcap!")
        else:
            self.logger.info(f"Pcap output path {self.output_path}")

    def stop_pcap(self) -> None:
        self.logger.info("Stopping pcap proc")
        self.pcap_proc.stop_command()
