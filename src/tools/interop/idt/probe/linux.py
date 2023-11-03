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

import probe.probe as p
from utils.host_platform import get_ll_interface
from . import config
from utils.log import get_logger

logger = get_logger(__file__)


class ProberLinuxHost(p.GenericMatterProber):

    def __init__(self, artifact_dir: str, dnssd_artifact_dir: str) -> None:
        # TODO: Parity with macOS
        super().__init__(artifact_dir, dnssd_artifact_dir)
        self.logger = logger

    def probe_v4(self, ipv4: str, port: str) -> None:
        self.run_command(f"ping -c {config.ping_count} {ipv4}")

    def probe_v6(self, ipv6: str, port: str) -> None:
        self.run_command(f"ping -c {config.ping_count} -6 {ipv6}")

    def probe_v6_ll(self, ipv6_ll: str, port: str) -> None:
        interface = get_ll_interface()
        self.run_command(f"ping -c {config.ping_count} -6 {ipv6_ll}%{interface}")

    def discover_targets_by_neighbor(self) -> None:
        pass

    def check_routes(self) -> None:
        self.run_command("route -6 -n")
