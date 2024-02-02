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

import os

from features.capture.thread.base import ThreadCapture
from utils import log
from utils.artifact import create_standard_log_name
from utils.shell import Bash
import time

_LOGGER = log.get_logger(__file__)


class MacThreadCaptureSniffer(ThreadCapture):

    def __init__(self, artifact_dir: str) -> None:
        raise NotImplementedError

    def start(self) -> None:
        raise NotImplementedError

    def stop(self) -> None:
        pass


class LinuxThreadCaptureSniffer(ThreadCapture):
    """
    PySpinel had to be patched like this
    /thread/BUILD/sniff/pyspinel]
    └─$ git diff
    diff --git a/sniffer.py b/sniffer.py
    index 8492431..c56a4b2 100755
    --- a/sniffer.py
    +++ b/sniffer.py
    @@ -325,6 +325,7 @@ def main():

                     # Some old version NCP doesn't contain timestamp information in metadata
                     else:
    +                    metadata = ""
                         timestamp = datetime.utcnow() - epoch
                         timestamp_sec = timestamp.days * 24 * 60 * 60 + timestamp.seconds
                         timestamp_usec = timestamp.microseconds
    """

    def __init__(self, artifact_dir: str, logger=_LOGGER) -> None:
        self.artifact_dir = artifact_dir
        self.artifact = create_standard_log_name("THREAD_SNIFF", "cap", parent=self.artifact_dir)
        self.logger = logger
        self.script_dir = os.path.join(os.path.dirname(__file__),
                                       "BUILD",
                                       "sniff",
                                       "pyspinel")
        self.uart = "/dev/ttyACM0"
        self.channel = 15
        self.run_sniffer_command: Bash | None = None

    def start(self) -> None:
        # TODO: Need to verify board is connected here
        # TODO: Allow port as arg
        self.logger.info("Starting thread sniff")
        # TODO: CRITICAL: Make sudo auth automatic in Bash!
        Bash("sudo echo \"\"", sync=True).start_command()
        self.run_sniffer_command = Bash(
            f"sudo ./sniffer.py -c {self.channel} -n 1 --no-reset -u {self.uart} -b 115200 > {self.artifact}",
            cwd=self.script_dir)
        self.run_sniffer_command.start_command()
        self.logger.info("Waiting to check if sniffer started")
        time.sleep(5)
        if not self.run_sniffer_command.command_is_running():
            self.logger.critical("Sniffer is down!")
        self.logger.info("Done starting thread sniff")

    def stop(self) -> None:
        self.run_sniffer_command.stop_command()
