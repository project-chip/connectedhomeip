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

if TYPE_CHECKING:
    from capture.platform.android import Android

logger = log.get_logger(__file__)


class ScreenRecorder(AndroidStream):

    def __init__(self, platform: "Android"):
        self.screen_artifact = None
        self.screen_phone_out_path = None
        self.screen_command = None
        self.screen_proc = None
        self.logger = logger
        self.platform = platform
        self.screen_check_command = "shell dumpsys deviceidle | grep mScreenOn"
        self.screen_pull = False
        self.file_counter = 0
        self.pull_commands: [str] = []
        self.manifest_file = os.path.join(platform.artifact_dir, "screen_manifest.txt")

    def check_screen(self) -> bool:
        screen_cmd_output = self.platform.run_adb_command(
            self.screen_check_command, capture_output=True)
        return "mScreenOn=true" == screen_cmd_output.get_captured_output().strip()

    async def prepare_screen_recording(self) -> None:
        screen_on = self.check_screen()
        while not screen_on:
            await asyncio.sleep(3)
            screen_on = self.check_screen()
            if not screen_on:
                self.logger.error("Please turn the screen on so screen recording can start or check connection!")

    def update_commands(self) -> None:
        self.screen_artifact = create_standard_log_name("screencast" + str(self.file_counter),
                                                        "mp4",
                                                        parent=self.platform.artifact_dir)
        self.screen_phone_out_path = f"/sdcard/Movies/{os.path.basename(self.screen_artifact)}"
        self.screen_command = f"shell screenrecord --bugreport {self.screen_phone_out_path}"
        screen_pull_command = f"pull {self.screen_phone_out_path} {self.screen_artifact}\n"
        self.pull_commands.append(screen_pull_command)
        with open(self.manifest_file, "a+") as manifest:
            manifest.write(screen_pull_command)
        self.file_counter += 1

    async def start(self):
        await self.prepare_screen_recording()
        if self.check_screen():
            self.screen_pull = True
            self.update_commands()
            self.screen_proc = self.platform.get_adb_background_command(self.screen_command)
            self.screen_proc.start_command()
            self.logger.info(f"New screen recording file started {self.screen_phone_out_path} {self.screen_artifact}")

    async def run_observer(self) -> None:
        while True:
            if not self.screen_proc.command_is_running():
                self.logger.warning(f"Screen recording proc needs restart (may be normal) {self.platform.device_id}")
                await self.start()
            await asyncio.sleep(4)

    async def pull_screen_recording(self) -> None:
        if self.screen_pull:
            self.logger.info("Attempting to pull screen recording")
            await asyncio.sleep(3)
            with open(self.manifest_file) as manifest:
                for line in manifest:
                    self.platform.run_adb_command(line)
            self.screen_pull = False

    async def stop(self):
        self.logger.info("Stopping screen proc")
        self.screen_proc.stop_command()
        await self.pull_screen_recording()
