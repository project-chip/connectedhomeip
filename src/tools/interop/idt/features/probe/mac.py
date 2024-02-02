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

from utils.host import current_platform
from utils.log import get_logger

from . import ProbeTarget, config
from . import probe as p

logger = get_logger(__file__)


class ProberMacHost(p.GenericMatterProber):

    def __init__(self, artifact_dir: str, dnssd_artifact_dir: str) -> None:
        super().__init__(artifact_dir, dnssd_artifact_dir)
        self.logger = logger
        self.ll_int = current_platform.get_link_local_interface()

    def probe_v4(self, target: ProbeTarget) -> None:
        self.logger.info("Ping IPv4")
        self.run_command(f"ping -c {config.PING_COUNT} {target.ip}")

    def probe_v6(self, target: ProbeTarget) -> None:
        self.logger.info("Ping IPv6")
        self.run_command(f"ping6 -c {config.PING_COUNT} {target.ip}")

    def probe_v6_ll(self, target: ProbeTarget) -> None:
        self.logger.info("Ping IPv6 Link Local")
        self.run_command(f"ping6 -c {config.PING_COUNT} -I {self.ll_int} {target.ip}")

    def get_general_details(self) -> None:
        self.logger.info("Host interfaces")
        self.run_command("ifconfig")
        self.logger.info("v4 routes from host")
        self.run_command("netstat -r -f inet -n")
        self.logger.info("v6 routes from host")
        self.run_command("netstat -r -f inet6 -n")
