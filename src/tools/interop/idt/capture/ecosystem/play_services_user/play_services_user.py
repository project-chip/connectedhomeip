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

from capture.base import EcosystemCapture, UnsupportedCapturePlatformException
from capture.platform.android.android import Android
from capture.platform.android.streams.logcat import LogcatStreamer
from utils.artifact import create_standard_log_name
from utils.log import get_logger, print_and_write

logger = get_logger(__file__)


class PlayServicesUser(EcosystemCapture):
    """
    Implementation of capture and analysis for Play Services 3P
    """

    def __init__(self, platform: Android, artifact_dir: str) -> None:
        self.logger = logger
        self.artifact_dir = artifact_dir
        self.analysis_file = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'commissioning_boundaries', 'txt'))

        if not isinstance(platform, Android):
            raise UnsupportedCapturePlatformException(
                'only platform=android is supported for '
                'ecosystem=PlayServicesUser')
        self.platform = platform
        self.logcat_fd = None
        self.output = ""
        self.logcat_stream: LogcatStreamer = self.platform.streams["LogcatStreamer"]

    async def start_capture(self) -> None:
        pass

    async def stop_capture(self) -> None:
        self.show_analysis()

    def proc_line(self, line) -> None:
        if "CommissioningServiceBin: Binding to service" in line:
            s = f"3P commissioner initiated Play Services commissioning\n{line}"
            logger.info(s)
            self.output += f"{s}\n"
        elif "CommissioningServiceBin: Sending commissioning request to bound service" in line:
            s = f"Play Services commissioning complete; passing back to 3P\n{line}"
            logger.info(s)
            self.output += f"{s}\n"
        elif "CommissioningServiceBin: Received commissioning complete from bound service" in line:
            s = f"3P commissioning complete!\n{line}"
            logger.info(s)
            self.output += f"{s}\n"

    async def analyze_capture(self) -> None:
        """"Show the start and end times of commissioning boundaries"""
        try:
            self.logcat_fd = open(self.logcat_stream.logcat_artifact, "r")
            while True:
                for line in self.logcat_fd.readlines():
                    self.proc_line(line)
                # Releasing async event loop for other analysis / monitor tasks
                await asyncio.sleep(0.5)
        except asyncio.CancelledError:
            self.logger.info("Closing logcat stream")
            if self.logcat_fd is not None:
                self.logcat_fd.close()

    def show_analysis(self) -> None:
        with open(self.analysis_file, "w") as analysis_file:
            print_and_write(self.output, analysis_file)

    async def probe_capture(self) -> None:
        pass
