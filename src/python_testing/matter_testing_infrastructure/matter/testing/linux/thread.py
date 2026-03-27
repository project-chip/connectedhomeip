#
#    Copyright (c) 2026 Project CHIP Authors
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

import logging
import os
import re
import subprocess
import threading
from typing import Optional, Pattern

from matter.testing.tasks import SubprocessKind

from .namespace import IsolatedNetworkNamespace

log = logging.getLogger(__name__)


class ThreadBorderRouter:

    # The Thread radio simulation node id, choose other if there is a conflict.
    NODE_ID = 9

    def __init__(self, dataset: str, ns: IsolatedNetworkNamespace):
        self._event = threading.Event()
        self._pattern: Optional[Pattern[str]] = None
        self._event.set()
        self._netns_app = ns.netns_for_subprocess_kind(SubprocessKind.APP)
        self._link_name_app = ns.app_link.name

        radio_url = f'spinel+hdlc+forkpty:///usr/bin/env?forkpty-arg=ot-rcp&forkpty-arg={self.NODE_ID}'
        cmd = self._netns_app.wrap_cmd(['otbr-agent', '-d7', '-v', f'-B{self._link_name_app}', radio_url])

        self._otbr = subprocess.Popen(cmd,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT,
                                      text=True,
                                      encoding='UTF-8')

        threading.Thread(target=self._otbr_read_stdout, daemon=True).start()

        self.expect(r'Co-processor version:', timeout=20)
        self.join_network(dataset)

    def join_network(self, dataset):
        status = os.system(
            f'ot-ctl dataset init tlvs {dataset} &&'
            'ot-ctl dataset commit active &&'
            'ot-ctl ifconfig up &&'
            'ot-ctl routerselectionjitter 1 &&'
            'ot-ctl thread start &&'
            'ot-ctl state leader &&'
            'while ! ot-ctl state | grep -q leader; do sleep 1; done &&'
            'ot-ctl netdata show &&'
            'ot-ctl srp server enable &&'
            'while ! ot-ctl br state | grep -q running; do sleep 1; done &&'
            'echo TBR ready'
        )
        if status != 0:
            raise RuntimeError("Failed to control Thread Border Router")

        self.expect(r'Sent RA on infra netif', timeout=15)

    def expect(self, pattern: str, timeout=10):
        self._pattern = re.compile(pattern)
        self._event.clear()
        if not self._event.wait(timeout):
            raise TimeoutError(f'Failed to expect: {pattern}')

    def _otbr_read_stdout(self):
        assert self._otbr.stdout is not None
        while (line := self._otbr.stdout.readline()):
            log.info(line)
            if self._event.is_set():
                continue
            if not self._pattern:
                continue
            if self._pattern.search(line):
                self._event.set()

    def get_border_agent_port(self) -> int:
        cmd = self._netns_app.wrap_cmd('ot-ctl ba port')
        output = subprocess.check_output(cmd, text=True)
        # ot-ctl output includes the port number followed by "Done"
        # Using regex to find the first number in the output
        match = re.search(r'(\d+)', output)
        if not match:
            raise RuntimeError(f"Failed to parse border agent port from: {output}")
        return int(match.group(1))

    def get_border_agent_host(self) -> str:
        return '10.10.10.1'

    def terminate(self):
        if self._otbr:
            self._otbr.terminate()
            self._otbr.wait()
