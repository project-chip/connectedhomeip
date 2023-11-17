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
import json
import os
from typing import IO, Dict

from capture.base import EcosystemCapture, UnsupportedCapturePlatformException
from capture.platform.android import Android
from capture.platform.android.streams.logcat import LogcatStreamer
from utils.artifact import create_standard_log_name, log

from . import config
from .command_map import dumpsys, getprop
from .play_services_analysis import PlayServicesAnalysis
from .prober import PlayServicesProber

logger = log.get_logger(__file__)


class PlayServices(EcosystemCapture):
    """
    Implementation of capture and analysis for Play Services
    """

    def __init__(self, platform: Android, artifact_dir: str) -> None:
        self.artifact_dir = artifact_dir

        if not isinstance(platform, Android):
            raise UnsupportedCapturePlatformException(
                'only platform=android is supported for ecosystem=play_services')
        self.platform = platform

        self.standard_info_file_path = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'phone_info', 'json'))
        self.standard_info_data: Dict[str, str] = {}

        self.analysis = PlayServicesAnalysis(self.platform, self.artifact_dir)

        self.service_ids = ['336',  # Home
                            '305',  # Thread
                            '168',  # mDNS
                            ]
        self.logcat_stream: LogcatStreamer = self.platform.streams["LogcatStreamer"]
        self.logcat_file: IO = None

    def _write_standard_info_file(self) -> None:
        for k, v in self.standard_info_data.items():
            logger.info(f"{k}: {v}")
        standard_info_data_json = json.dumps(self.standard_info_data, indent=2)
        with open(self.standard_info_file_path, mode='w+') as standard_info_file:
            standard_info_file.write(standard_info_data_json)

    def _parse_get_prop(self) -> None:
        get_prop = self.platform.run_adb_command(
            "shell getprop",
            capture_output=True).get_captured_output()
        for output in get_prop.split("\n"):
            for prop in getprop:
                if prop in output:
                    self.standard_info_data[prop] = output[output.rindex("["):]

    def _parse_dumpsys(self) -> None:
        for attr_name, command in dumpsys.items():
            command = f"shell dumpsys {command}"
            command_output = self.platform.run_adb_command(
                command,
                capture_output=True).get_captured_output()
            self.standard_info_data[attr_name] = command_output

    def _get_standard_info(self) -> None:
        self._parse_get_prop()
        self._parse_dumpsys()
        self._write_standard_info_file()

    async def start_capture(self) -> None:
        for service_id in self.service_ids:
            verbose_command = f"shell setprop log.tag.gms_svc_id:{service_id} VERBOSE"
            self.platform.run_adb_command(verbose_command)
        self._get_standard_info()

    async def analyze_capture(self):
        try:
            self.logcat_file = open(self.logcat_stream.logcat_artifact, "r")
            while True:
                self.analysis.do_analysis(self.logcat_file.readlines())
                # Releasing async event loop for other analysis / monitor topics
                await asyncio.sleep(0.5)
        except asyncio.CancelledError:
            logger.info("Closing logcat stream")
            if self.logcat_file:
                self.logcat_file.close()

    async def stop_capture(self) -> None:
        self.analysis.show_analysis()

    async def probe_capture(self) -> None:
        if config.enable_foyer_probers:
            await PlayServicesProber(self.platform, self.artifact_dir).probe_services()
        else:
            logger.critical("Foyer probers disabled in config!")
