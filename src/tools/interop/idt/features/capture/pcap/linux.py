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
import os.path
from typing import Optional

from features.capture.pcap.base import PacketCaptureBase
from utils.host import current_platform
from utils.shell import Bash


class LinuxPacketCaptureRunner(PacketCaptureBase):

    def __init__(self,
                 artifact_dir: str,
                 interface: str,
                 monitor_mode: bool,
                 channel: int,
                 band: str,
                 width: str) -> None:
        super().__init__(artifact_dir, interface, monitor_mode, channel, band, width)
        if self.monitor_mode:
            self.interface = current_platform.get_link_local_interface()
            setup_script_path = os.path.join(os.path.dirname(__file__), "setup_linux_interface.sh")
            setup_proc = Bash(f"{setup_script_path} {self.interface} {self.channel}", sync=True)
            setup_proc.start_command()
            if not setup_proc.finished_success():
                self.logger.critical("Monitor mode setup failed!")
            else:
                self.interface = self.interface+"mon"
        self.pcap_command: Optional[str] = f"tcpdump -i {self.interface} -n -w {self.output_path}"
        self.pcap_proc = Bash(self.pcap_command)

    def start_pcap(self) -> None:
        super().start_pcap()

    def stop_pcap(self) -> None:
        super().stop_pcap()
        if self.monitor_mode:
            teardown_script_path = os.path.join(os.path.dirname(__file__), "teardown_linux_interface.sh")
            teardown_proc = Bash(f"{teardown_script_path} {self.interface}", sync=True)
            teardown_proc.start_command()
