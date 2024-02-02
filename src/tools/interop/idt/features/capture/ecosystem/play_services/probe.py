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

from utils.shell import Bash, log

from . import config

_LOGGER = log.get_logger(__file__)


class PlayServicesProber:

    def __init__(self, platform, artifact_dir):
        # TODO: Handle all resolved addresses
        self.platform = platform
        self.artifact_dir = artifact_dir
        self.logger = _LOGGER
        self.probe_artifact = os.path.join(self.artifact_dir, "net_probes.txt")
        self.command_suffix = f" 2>&1  | tee -a {self.probe_artifact}"
        self.target = ""
        self.tracert_limit = config.PROBER_TRACEROUTE_LIMIT

    def run_command(self, command):
        Bash(f"{command} {self.command_suffix}", sync=True).start_command()

    async def _probe_tracert_icmp(self) -> None:
        self.logger.info(f"icmp traceroute to {self.target}")
        self.run_command(f"traceroute -m {self.tracert_limit} {self.target}")

    async def _probe_tracert_udp(self) -> None:
        # TODO: Per-host-platform impl
        self.logger.info(f"udp traceroute to {self.target}")
        self.run_command(f"traceroute -m {self.tracert_limit} -U -p 443 {self.target}")

    async def _probe_tracert_tcp(self) -> None:
        # TODO: Per-host-platform impl
        self.logger.info(f"tcp traceroute to {self.target}")
        self.run_command(f"traceroute -m {self.tracert_limit} -T -p 443 {self.target}")

    async def _probe_ping(self) -> None:
        self.logger.info(f"ping {self.target}")
        self.run_command(f"ping -c 4 {self.target}")

    async def _probe_dns(self) -> None:
        self.logger.info(f"dig {self.target}")
        self.run_command(f"dig {self.target}")

    async def _probe_from_phone_ping(self) -> None:
        self.logger.info(f"ping {self.target} from phone")
        self.platform.run_adb_command(f"shell ping -c 4 {self.target} {self.command_suffix}")

    async def probe_services(self) -> None:
        if not os.path.exists(config.PROBER_URLS_FILE_NAME):
            self.logger.info("No probe targets configured")
            return
        with open(config.PROBER_URLS_FILE_NAME) as urls_file:
            for line in urls_file:
                self.target = line
                self.logger.info(f"Probing {self.target}")
                for probe_func in [s for s in dir(self) if s.startswith('_probe')]:
                    await getattr(self, probe_func)()
