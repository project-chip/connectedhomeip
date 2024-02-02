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

from abc import ABC

from features.capture.thread.on_network import MacThreadCaptureOnNetwork, LinuxThreadCaptureOnNetwork
from features.capture.thread.sniff import MacThreadCaptureSniffer, LinuxThreadCaptureSniffer
from utils.artifact import log
from utils.host import current_platform

logger = log.get_logger(__file__)


class ThreadCaptureRunner(ABC):

    def __init__(self,
                 artifact_dir: str,
                 mode: str,
                 ) -> None:
        self.logger = logger
        self.artifact_dir = artifact_dir
        self.mode = mode
        if current_platform.is_mac():
            if mode == "sniff":
                self.runner = MacThreadCaptureSniffer(self.artifact_dir)
            elif mode == "on_network":
                self.runner = MacThreadCaptureOnNetwork(self.artifact_dir)
        else:
            if mode == "sniff":
                self.runner = LinuxThreadCaptureSniffer(self.artifact_dir)
            elif mode == "on_network":
                self.runner = LinuxThreadCaptureOnNetwork(self.artifact_dir)

    def start(self) -> None:
        self.runner.start()

    def stop(self) -> None:
        self.runner.stop()
