#
#    Copyright (c) 2024 Project CHIP Authors
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

from utils.host.base import HostPlatform
from utils.log import get_logger
from utils.shell import Bash

logger = get_logger(__file__)


class LinuxPlatform(HostPlatform):

    def __init__(self):
        HostPlatform.__init__(self)

    def get_interfaces_available_for_pcap(self) -> str:
        """
        Returns a list of available network interfaces
        """
        net_interface_path = "/sys/class/net/"
        available_net_interfaces = os.listdir(net_interface_path) \
            if os.path.exists(net_interface_path) \
            else []
        available_net_interfaces.append("any")
        return available_net_interfaces

    def get_link_local_interface(self) -> str:
        """
        Returns an interface appropriate for link local connections
        """
        net_interface_path = "/sys/class/net/"
        available_net_interfaces = os.listdir(net_interface_path) \
            if os.path.exists(net_interface_path) \
            else []
        for interface in available_net_interfaces:
            if "wl" in interface:
                return interface

    def current_wifi_channel_width(self, display=False) -> tuple[int, int | None]:
        """
        Returns the currently used Wi-Fi channel and width if available
        """
        interface = self.get_link_local_interface()
        channel_cmd = Bash(f"iw {interface} info", sync=True, capture_output=True)
        channel_cmd.start_command()
        try:
            lines = channel_cmd.get_captured_output().split("\n")
            for i in range(0, len(lines)):
                lines[i] = lines[i].strip()
            for line in lines:
                if line.startswith("channel"):
                    channel = int(line.split(" ")[1])
                    if display:
                        logger.info(f"Current Wi-Fi channel is {channel}")
                    return channel, None
        except Exception:
            logger.critical(f"Error parsing channel for interface {interface}")
        logger.warning("Could not find default Wi-Fi channel, using 6")
        return 6, None
