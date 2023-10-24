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

import multiprocessing
import time

from capture.utils.artifact import create_standard_log_name, get_observer_proc
from typing import TYPE_CHECKING
from ..base import AndroidStream

import log

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
        self.observer_proc = get_observer_proc(self.logcat_artifact)
        self.runner_proc = multiprocessing.Process(target=self.restart_logcat_as_needed)
        self.was_ever_running = False

    def restart_logcat_as_needed(self) -> None:
        while True:
            if not self.logcat_proc.command_is_running():
                self.logcat_proc = self.platform.get_adb_background_command(self.logcat_command)
                self.logcat_proc.start_command()
                if self.was_ever_running:
                    self.logger.critical("Had to start logcat again!!!")
                else:
                    self.was_ever_running = True
            time.sleep(10)

    async def start(self):
        if not self.runner_proc.is_alive():
            self.runner_proc.start()
        if not self.observer_proc.is_alive():
            self.observer_proc.start()

    async def stop(self):
        self.runner_proc.kill()
        self.observer_proc.kill()
        self.runner_proc.join()
        self.observer_proc.join()
