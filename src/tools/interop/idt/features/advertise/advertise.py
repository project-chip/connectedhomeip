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
import sys
from abc import ABC, abstractmethod

from utils.host import current_platform
from utils.log import get_logger
from zeroconf import IPVersion
from zeroconf.asyncio import AsyncServiceInfo, AsyncZeroconf


class FakeMatterAd(ABC):

    def __init__(self,
                 vid: int,
                 pid: int,
                 discriminator: int,
                 device_name: str,
                 device_type: int,
                 port: int,
                 commissioning_open: int,
                 mac_address: str,
                 allow_gua: bool,
                 logger=None):
        self.vid = vid
        self.pid = pid
        self.discriminator = discriminator
        self.device_name = device_name
        self.device_type = device_type
        self.port = port
        self.commissioning_open = commissioning_open
        self.mac_address = mac_address
        self.allow_gua = allow_gua
        self.logger = get_logger(__file__) if not logger else logger
        self.logger.info(f"New fake matter ad instantiated\n{self}")

    def __repr__(self):
        return f"VID {hex(self.vid)}\n" \
               f"PID {hex(self.pid)}\n" \
               f"Discriminator {hex(self.discriminator)}\n" \
               f"Device name {self.device_name}\n" \
               f"Device type {hex(self.device_type)}\n" \
               f"Port {self.port}\n" \
               f"Commissioning open {self.commissioning_open}\n" \
               f"Mac addr {self.mac_address}\n"

    @abstractmethod
    async def advertise(self):
        """
        Advertise until the user presses enter
        """
        raise NotImplementedError


class FakeMatterAdDnssd(FakeMatterAd):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.service = "_matterc._udp.local."
        self.instance = f"{self.mac_address}.{self.service}"
        self.properties = {
            "VP": f"{str(self.vid)}+{str(self.pid)}",
            "DN": self.device_name,
            "DT": self.device_type,
            "CM": int(self.commissioning_open),
            "D": self.discriminator,
        }
        self.aiozc = None
        self.ips = current_platform.ips()
        self.logger.info(f"Fake matter DNS-SD ads using the following IPs {self.ips}")

    async def advertise(self):
        addrs = self.ips.v4 + self.ips.v6_link_local + self.ips.v6_unique_local
        if self.allow_gua:
            addrs += self.ips.v6_global
        service_info = AsyncServiceInfo(
            self.service,
            self.instance,
            addresses=addrs,
            port=self.port,
            properties=self.properties,
        )
        # TODO: Make an option for v4 or v6 or v4 and v6
        self.aiozc = AsyncZeroconf(ip_version=IPVersion.V4Only)
        await self.aiozc.async_register_service(service_info)
        await asyncio.get_event_loop().run_in_executor(
            None, sys.stdin.readline)
        await self.aiozc.async_unregister_service(service_info)
        self.logger.info("Ended advertisement!")


class FakeMatterAdBle(FakeMatterAd):

    def __init__(self, *args, **kwargs):
        super().__init__()

    async def advertise(self):
        raise NotImplementedError
