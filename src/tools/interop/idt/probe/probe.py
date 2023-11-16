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
import os.path
from abc import ABC, abstractmethod

from discovery import MatterDnssdListener
from discovery.dnssd import ServiceInfo
from utils.artifact import create_standard_log_name
from utils.log import get_logger
from utils.shell import Bash

from . import ProbeTarget, config
from .ip_utils import is_ipv4, is_ipv6, is_ipv6_ll

logger = get_logger(__file__)


class GenericMatterProber(ABC):

    def __init__(self, artifact_dir: str, dnssd_artifact_dir: str) -> None:
        self.artifact_dir = artifact_dir
        self.dnssd_artifact_dir = dnssd_artifact_dir
        self.logger = logger
        self.targets: [GenericMatterProber.ProbeTarget] = []
        self.output = os.path.join(self.artifact_dir,
                                   create_standard_log_name("generic_probes", "txt"))
        self.suffix = f"2>&1 | tee -a {self.output}"

    def run_command(self, cmd: str, capture_output=False) -> Bash:
        cmd = f"{cmd} {self.suffix}"
        self.logger.debug(cmd)
        bash = Bash(cmd, sync=True, capture_output=capture_output)
        bash.start_command()
        return bash

    @abstractmethod
    def probe_v4(self, target: ProbeTarget) -> None:
        raise NotImplementedError

    @abstractmethod
    def probe_v6(self, target: ProbeTarget) -> None:
        raise NotImplementedError

    @abstractmethod
    def probe_v6_ll(self, target: ProbeTarget) -> None:
        raise NotImplementedError

    @abstractmethod
    def discover_targets_by_neighbor(self) -> None:
        raise NotImplementedError

    @abstractmethod
    def get_general_details(self) -> None:
        raise NotImplementedError

    def discover_targets_by_browsing(self) -> None:
        browser = MatterDnssdListener(self.dnssd_artifact_dir)
        asyncio.run(browser.browse_once(config.dnssd_browsing_time_seconds))
        for name in browser.discovered_matter_devices:
            info: ServiceInfo = browser.discovered_matter_devices[name]
            for addr in info.parsed_scoped_addresses():
                self.targets.append(ProbeTarget(name, addr, info.port))

    def probe_single_target(self, target: ProbeTarget) -> None:
        if is_ipv4(target.ip):
            self.logger.debug(f"Probing v4 {target.ip}")
            self.probe_v4(target)
        elif is_ipv6_ll(target.ip):
            self.logger.debug(f"Probing v6 ll {target.ip}")
            self.probe_v6_ll(target)
        elif is_ipv6(target.ip):
            self.logger.debug(f"Probing v6 {target.ip}")
            self.probe_v6(target)

    def probe_targets(self) -> None:
        for target in self.targets:
            self.logger.info(f"Probing target {target}")
            self.probe_single_target(target)

    def probe(self) -> None:
        self.discover_targets_by_browsing()
        self.discover_targets_by_neighbor()
        self.probe_targets()
        self.get_general_details()
