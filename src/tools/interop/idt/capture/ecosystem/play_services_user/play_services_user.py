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

from capture.base import EcosystemCapture, UnsupportedCapturePlatformException
from capture.file_utils import create_standard_log_name, print_and_write
from capture.platform.android.android import Android


class PlayServicesUser(EcosystemCapture):
    """
    Implementation of capture and analysis for Play Services 3P
    """

    def __init__(self, platform: Android, artifact_dir: str) -> None:

        self.artifact_dir = artifact_dir
        self.analysis_file = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'commissioning_boundaries', 'txt'))

        if not isinstance(platform, Android):
            raise UnsupportedCapturePlatformException(
                'only platform=android is supported for '
                'ecosystem=PlayServicesUser')
        self.platform = platform

    async def start_capture(self) -> None:
        await self.platform.start_streaming()

    async def stop_capture(self) -> None:
        await self.platform.stop_streaming()

    async def analyze_capture(self) -> None:
        """"Show the start and end times of commissioning boundaries"""
        analysis_file = open(self.analysis_file, mode='w+')
        with open(self.platform.logcat_output_path, mode='r') as logcat_file:
            for line in logcat_file:
                if "CommissioningServiceBin: Binding to service" in line:
                    print_and_write(
                        f"3P commissioner initiated Play Services commissioning\n{line}",
                        analysis_file)
                elif "CommissioningServiceBin: Sending commissioning request to bound service" in line:
                    print_and_write(
                        f"Play Services commissioning complete; passing back to 3P\n{line}",
                        analysis_file)
                elif "CommissioningServiceBin: Received commissioning complete from bound service" in line:
                    print_and_write(
                        f"3P commissioning complete!\n{line}",
                        analysis_file)
        analysis_file.close()
