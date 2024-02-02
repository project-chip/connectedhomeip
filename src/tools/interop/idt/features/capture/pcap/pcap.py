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

from features.capture.pcap.linux import LinuxPacketCaptureRunner
from features.capture.pcap.mac import MacPacketCaptureRunner
from utils.artifact import log
from utils.host import current_platform

logger = log.get_logger(__file__)


class PacketCaptureRunner:

    def __init__(self,
                 artifact_dir: str,
                 interface: str,
                 monitor_mode: bool,
                 channel: int,
                 band: str,
                 width: int) -> None:
        if current_platform.is_mac():
            self.runner = MacPacketCaptureRunner(artifact_dir, interface, monitor_mode, channel, band, width)
        else:
            self.runner = LinuxPacketCaptureRunner(artifact_dir, interface, monitor_mode, channel, band, width)

    def start_pcap(self) -> None:
        self.runner.start_pcap()

    def stop_pcap(self) -> None:
        # TODO: Display helpful starting wireshark queries to user here
        self.runner.stop_pcap()
