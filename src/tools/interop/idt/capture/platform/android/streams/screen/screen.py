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
import multiprocessing
import os

from capture.utils.artifact import create_standard_log_name
from typing import TYPE_CHECKING
from ..base import AndroidStream

if TYPE_CHECKING:
    from capture.platform.android import Android

import log

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
        return "true" in screen_cmd_output.get_captured_output()

    async def prepare_screen_recording(self) -> None:
        # try:
        # async with asyncio.timeout_at(asyncio.get_running_loop().time() + 20.0):
        screen_on = self.check_screen()
        while not screen_on:
            await asyncio.sleep(4)
            screen_on = self.check_screen()
            if not screen_on:
                self.logger.error("Please turn the screen on so screen recording can start or check connection!")

    # except TimeoutError:
    # self.logger.error("Screen recording timeout")
    # return

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

    def run_recorder(self) -> None:
        while True:
            asyncio.run(self.prepare_screen_recording())
            self.update_commands()
            self.logger.info(f"New screen recording file started {self.screen_phone_out_path} {self.screen_artifact}")
            self.platform.run_adb_command(self.screen_command)

    async def start(self):
        await self.prepare_screen_recording()
        if self.check_screen() and not self.screen_pull:
            self.screen_pull = True
            self.screen_proc = multiprocessing.Process(target=self.run_recorder)
            self.screen_proc.start()

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
        if self.screen_proc is not None:
            self.screen_proc.kill()
            self.screen_proc.join()
        await self.pull_screen_recording()
