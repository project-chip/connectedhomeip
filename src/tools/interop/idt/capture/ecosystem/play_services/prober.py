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

import log
from capture.utils.shell import Bash

logger = log.get_logger(__file__)


class PlayServicesProber:

    def __init__(self, platform):
        self.platform = platform
        self.artifact_dir = self.platform.artifact_dir
        self.logger = logger
        self.probe_artifact = os.path.join(self.artifact_dir, "net_probes.txt")
        self.command_suffix = f" 2>&1  | tee -a {self.probe_artifact}"
        self.target = "googlehomefoyer-pa.googleapis.com"

    async def _probe_tracert_icmp_foyer(self) -> None:
        self.logger.info(f"icmp traceroute to {self.target}")
        Bash(f"traceroute {self.target} {self.command_suffix}", sync=True).start_command()

    async def _probe_tracert_udp_foyer(self) -> None:
        self.logger.info(f"udp traceroute to {self.target}")
        Bash(f"traceroute -U -p 443 {self.target} {self.command_suffix}", sync=True).start_command()

    async def _probe_tracert_tcp_foyer(self) -> None:
        self.logger.info(f"tcp traceroute to {self.target}")
        Bash(f"traceroute -T -p 443 {self.target} {self.command_suffix}", sync=True).start_command()

    async def _probe_ping_foyer(self) -> None:
        self.logger.info(f"ping {self.target}")
        Bash(f"ping -c 4 {self.target} {self.command_suffix}", sync=True).start_command()

    async def _probe_dns_foyer(self) -> None:
        self.logger.info(f"dig {self.target}")
        Bash(f"dig {self.target} {self.command_suffix}", sync=True).start_command()

    async def _probe_from_phone_ping_foyer(self) -> None:
        self.logger.info(f"ping {self.target} from phone")
        self.platform.run_adb_command(f"shell ping -c 4 {self.target} {self.command_suffix}")

    async def probe_services(self) -> None:
        self.logger.info(f"Probing {self.target}")
        for probe_func in [s for s in dir(self) if s.startswith('_probe')]:
            await getattr(self, probe_func)()
