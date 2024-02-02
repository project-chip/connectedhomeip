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
from datetime import datetime

import zeroconf

from features.probe import ProbeTarget
from utils.net import get_addr_type
from utils.artifact import create_standard_log_name, log
from utils.log import add_border, border_print
from zeroconf import ServiceBrowser, ServiceInfo, ServiceListener, Zeroconf
from .parsers import ServiceLibrary, MatterTxtRecordParser

logger = log.get_logger(__file__)


class MatterDnssdListener(ServiceListener):

    def __init__(self,
                 artifact_dir: str,
                 vendor_id: str | None,
                 product_id: str | None,
                 v4=True,
                 v6=True) -> None:
        super().__init__()
        self.artifact_dir = artifact_dir
        self.logger = logger
        self.discovered_matter_devices: [str, ServiceInfo] = {}
        self.vendor_id = vendor_id
        self.product_id = product_id
        self.service_library = ServiceLibrary()
        self.v4 = v4
        self.v6 = v6

    def write_log(self, line: str, log_name: str) -> None:
        with open(self.create_device_log_name(log_name), "a+") as log_file:
            time_stamp = add_border(datetime.now().strftime("%Y-%m-%d\t%H %M\t%S.%f") + "\n")
            log_file.write(time_stamp + line)

    def create_device_log_name(self, device_name) -> str:
        return os.path.join(
            self.artifact_dir,
            create_standard_log_name(f"{device_name}_dnssd", "txt"))

    @staticmethod
    def log_addr(info: ServiceInfo) -> str:
        addrs = set()
        ret = add_border("This device has the following IP addresses\n")
        for addr in info.parsed_scoped_addresses():
            if addr not in addrs:  # For some reason, there are duplicates in the list sometimes, so we dedup here
                addrs.add(addr)
                ret += f"{get_addr_type(addr)}: {addr}\n"
        return ret

    def handle_service_info(
            self,
            zc: Zeroconf,
            type_: str,
            name: str,
            delta_type: str) -> None:
        service_type_info = self.service_library.get_service_type_info(type_)
        to_log = f"{name}\n" \
                 f"SERVICE {delta_type}\n" \
                 f"BROADCAST ADDR IPv{zc.ipv}\n" \
                 f"{service_type_info.type}\n" \
                 f"{service_type_info.description}\n"
        info = zc.get_service_info(type_, name)
        if info is not None:
            to_log += f"A/SRV TTL: {str(info.host_ttl)}\n" \
                      f"PTR/TXT TTL: {str(info.other_ttl)}\n"
            txt_parser = MatterTxtRecordParser()
            to_log += txt_parser.parse_txt_records(info)
            to_log += self.log_addr(info)
            if self.vendor_id:
                if txt_parser.vid != self.vendor_id:
                    return
                if self.product_id and txt_parser.pid != self.product_id:
                    return
            self.discovered_matter_devices[name + zc.ipv] = info
            self.logger.info(to_log)
            self.write_log(to_log, name + zc.ipv)
        else:
            self.logger.warning(f"No info found for {to_log}")

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "ADDED")

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "UPDATED")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        if name + zc.ipv in self.discovered_matter_devices:
            del self.discovered_matter_devices[name + zc.ipv]
        else:
            return
        service_type_info = self.service_library.get_service_type_info(type_)
        to_log = f"{name}\n" \
                 f"SERVICE REMOVED\n" \
                 f"{service_type_info.type}\n" \
                 f"{service_type_info.description}\n"
        self.logger.warning(to_log)
        self.write_log(to_log, name)

    def browse_interactive(self) -> None:
        self.logger.warning("\nBrowsing Matter DNS-SD\n"
                            "DCL Lookup: https://webui.dcl.csa-iot.org/\n"
                            "See spec section 4.3 for details of Matter TXT records.\n")
        border_print("Press enter to stop!", important=True)
        if self.v4:
            zc4 = Zeroconf(ip_version=zeroconf.IPVersion.V4Only)
            zc4.ipv = "4"
            ServiceBrowser(zc4, list(self.service_library.known_service_types()), self)
        if self.v6:
            zc6 = Zeroconf(ip_version=zeroconf.IPVersion.V6Only)
            zc6.ipv = "6"
            ServiceBrowser(zc6, list(self.service_library.known_service_types()), self)
        try:
            input("")
        finally:
            if self.v4:
                zc4.close()
            if self.v6:
                zc6.close()

    async def browse_once(self, browse_time_seconds: int) -> [ProbeTarget]:
        if self.v4:
            zc4 = Zeroconf(ip_version=zeroconf.IPVersion.V4Only)
            zc4.ipv = "4"
            ServiceBrowser(zc4, list(self.service_library.known_service_types()), self)
        if self.v6:
            zc6 = Zeroconf(ip_version=zeroconf.IPVersion.V6Only)
            zc6.ipv = "6"
            ServiceBrowser(zc6, list(self.service_library.known_service_types()), self)
        await asyncio.sleep(browse_time_seconds)
        if self.v4:
            zc4.close()
        if self.v6:
            zc6.close()
        ret = []
        for name in self.discovered_matter_devices:
            info: ServiceInfo = self.discovered_matter_devices[name]
            for addr in info.parsed_scoped_addresses():
                ret.append(ProbeTarget(name, addr, info.port))
        return ret
