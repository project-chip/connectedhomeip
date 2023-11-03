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
from typing import TYPE_CHECKING

from utils.artifact import create_standard_log_name, log

from ..base import AndroidStream

logger = log.get_logger(__file__)

if TYPE_CHECKING:
    from capture.platform.android import Android


class LogcatStreamer(AndroidStream):

    def __init__(self, platform: "Android"):
        self.logger = logger
        self.platform = platform
        self.logcat_artifact = create_standard_log_name("logcat", "txt", parent=platform.artifact_dir)
        self.logcat_command = f"logcat -T 1 >> {self.logcat_artifact}"
        self.logcat_proc = platform.get_adb_background_command(self.logcat_command)
        self.was_ever_running = False

    async def run_observer(self) -> None:
        last_size = 0
        if not os.path.exists(self.logcat_artifact):
            self.logger.warning("Logcat artifact does not exist yes, this might be normal at the start of execution")
            asyncio.sleep(15)
        while True:
            try:
                new_size = os.path.getsize(self.logcat_artifact)
                if not (new_size > last_size):
                    self.logger.warning(f"Logcat file not growing for {self.platform.device_id}, check connection!")
                last_size = new_size
            except OSError:
                self.logger.error(f"Logcat file does not exist for {self.platfrom.device_id}, check connection!")
            if not self.logcat_proc.command_is_running():
                self.logger.error("Logcat proc is not running, trying to restart!")
                self.logcat_proc = self.platform.get_adb_background_command(self.logcat_command)
                self.logcat_proc.start_command()
            await asyncio.sleep(4)

    async def start(self):
        self.logcat_proc.start_command()

    async def stop(self):
        self.logcat_proc.stop_command()
