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

from utils import log
from utils.host.base import HostPlatform
from utils.shell import Bash

logger = log.get_logger(__file__)


class MacPlatform(HostPlatform):

    def __init__(self):
        HostPlatform.__init__(self)

    def get_interfaces_available_for_pcap(self) -> [str]:
        """
        Returns a list of available network interfaces
        """
        return ["any"]

    def get_link_local_interface(self) -> str:
        """
        Returns an interface appropriate for link local connections
        """
        return "en0"

    def current_wifi_channel_width(self, display=False) -> tuple[str, str | None]:
        """
        Returns the currently used Wi-Fi channel and width if available
        """
        get_config = Bash("airport -I", sync=True, capture_output=True)
        get_config.start_command()
        config_output = get_config.get_captured_output()
        for line in config_output.split("\n"):
            search_term = "channel:"
            if search_term in line:
                try:
                    value = line[line.index(search_term) + len(search_term) + 1:]
                    if "," in value:
                        channel = str(int(value[:value.index(",")]))
                        width = str(int(value[value.index(",")+1:]))
                        if display:
                            logger.info(f"Current Wi-Fi channel is {channel} width {width}")
                        return channel, width
                    else:
                        channel = str(int(value))
                        if display:
                            logger.info(f"Current Wi-Fi channel is {channel} width not detected")
                        return channel, None
                except Exception:
                    logger.critical(f"Error parsing channel/width from {line}")
        logger.warning("Could not find default Wi-Fi channel, using 6 with no width")
        return "6", None
