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

import re
import sys
import uuid
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Callable

from utils import log
from utils.log import border_print
from utils.net import WIFI_CHANNELS_2G, is_ipv4, is_ipv6_global_unicast, is_ipv6_link_local, is_ipv6_unique_local
from utils.shell import Bash

import config
from config import HOST_DEPENDENCIES

logger = log.get_logger(__file__)


@dataclass
class HostIps:
    """
    Object that holds three types of unicast addresses the host may have
    """
    v4: [str]
    v6_global: [str]
    v6_unique_local: [str]
    v6_link_local: [str]


class HostPlatform(ABC):

    def __init__(self) -> None:
        self.interface_config = self.get_interface_config()

    @abstractmethod
    def get_interfaces_available_for_pcap(self) -> str:
        """
        Returns a list of available network interfaces for pcaps
        """
        raise NotImplementedError

    @abstractmethod
    def get_link_local_interface(self) -> str:
        """
        Returns an interface appropriate for link local connections
        """
        raise NotImplementedError

    def get_mac_addr(self) -> str:
        return ':'.join(re.findall('..', '%012x' % uuid.getnode())).replace(":", "").upper()

    def get_interface_config(self) -> [str]:
        """
        Returns the chunk of ifconfig relevant to the link local interface
        """
        all_ifconfig_cmd = Bash("ifconfig", sync=True, capture_output=True)
        all_ifconfig_cmd.start_command()
        all_ifconfig_output = all_ifconfig_cmd.get_captured_output().split("\n")
        start_search = self.get_link_local_interface() + ":"
        stop_search = ": flags"
        in_chunk, accum = False, []
        for line in all_ifconfig_output:
            if in_chunk and stop_search in line:
                in_chunk = False
            if start_search in line:
                in_chunk = True
            if in_chunk:
                accum.append(line)
        logger.debug("Found ifconfig chunk:")
        for line in accum:
            logger.debug(line)
        return accum

    def get_addresses_of_types(self,
                               address_type: str,
                               search_prefix: str,
                               match_function: Callable[[str], bool]) -> [str]:
        ret = []
        for line in self.interface_config:
            if line.strip().startswith(f"{search_prefix} "):
                potential_match = line.strip().split(" ")[1]
                if match_function(potential_match):
                    ret.append(potential_match)
        logger.debug(f"{address_type} addresses {ret}")
        return ret

    def get_ipv4_addresses(self) -> [str]:
        return self.get_addresses_of_types("v4",
                                           "inet",
                                           is_ipv4)

    def get_ipv6_global_addresses(self) -> [str]:
        return self.get_addresses_of_types("v6 globally unique",
                                           "inet6",
                                           is_ipv6_global_unicast)

    def get_ipv6_unique_local_addresses(self) -> [str]:
        return self.get_addresses_of_types("v6 unique local",
                                           "inet6",
                                           is_ipv6_unique_local)

    def get_ipv6_link_local_addresses(self) -> str | None:
        return self.get_addresses_of_types("v6 link local",
                                           "inet6",
                                           is_ipv6_link_local)

    def ips(self) -> HostIps:
        """
        Returns addresses this host has
        """
        return HostIps(self.get_ipv4_addresses(),
                       self.get_ipv6_global_addresses(),
                       self.get_ipv6_unique_local_addresses(),
                       self.get_ipv6_link_local_addresses())

    @abstractmethod
    def current_wifi_channel_width(self, display=False) -> tuple[int, int | None]:
        """
        Returns the currently used Wi-Fi channel and width if available
        """
        raise NotImplementedError

    def using_5g_band(self) -> bool:
        """
        Returns true if the host is using 5g band
        """
        using_5g = str(self.current_wifi_channel_width()[0]) not in WIFI_CHANNELS_2G
        if using_5g:
            logger.info(f"Using 5g Wi-Fi")
        else:
            logger.info(f"Using 2g Wi-Fi")
        return using_5g

    def verify_py_version(self) -> None:
        """
        Verify the python version used on the host
        Exit the entire program is there is a version mismatch
        """
        py_version_major = sys.version_info[0]
        py_version_minor = sys.version_info[1]
        have = f"{py_version_major}.{py_version_minor}"
        need = f"{config.PY_MAJOR_VERSION}.{config.PY_MINOR_VERSION}"
        if not (py_version_major == config.PY_MAJOR_VERSION
                and py_version_minor >= config.PY_MINOR_VERSION):
            # TODO: Autoclean venv
            logger.critical(
                f"IDT requires python >= {need} but you have {have} for the command python3")
            logger.critical("Please install / configure the correct python version, delete idt/venv, and re-run!")
            sys.exit(1)

    def command_is_available(self, command: str) -> bool:
        """
        Returns True if the command is available on the host system
        """
        cmd = Bash(f"which {command}", sync=True, capture_output=True)
        cmd.start_command()
        return cmd.finished_success()

    def check_dependencies(self, deps: [str]) -> None:
        """
        Checks if a dependency from the provided list is missing and logs it
        Exit the entire program is there is any missing dependency
        """
        if not self.command_is_available("which"):
            logger.critical("which is required to verify host dependencies, exiting as its not available!")
            sys.exit(1)
        missing_deps = []
        for dep in deps:
            logger.info(f"Verifying host dependency {dep}")
            if not self.command_is_available(dep):
                missing_deps.append(dep)
        if missing_deps:
            for missing_dep in missing_deps:
                border_print(f"Missing dependency, please install {missing_dep}!", important=True)
                print(f"Help: {deps[missing_dep]}")
            sys.exit(1)

    def verify_host_dependencies(self) -> None:
        self.check_dependencies(HOST_DEPENDENCIES["ALL"])
        if self.is_mac():
            logger.info("Verifying Mac specific dependencies")
            self.check_dependencies(HOST_DEPENDENCIES["MAC"])
        else:
            logger.info("Verifying Linux specific dependencies")
            self.check_dependencies(HOST_DEPENDENCIES["LINUX"])

    def is_mac(self) -> bool:
        """
        Returns True if the current host is a Mac
        """
        from .mac import MacPlatform  # Avoid circular import
        return isinstance(self, MacPlatform)
