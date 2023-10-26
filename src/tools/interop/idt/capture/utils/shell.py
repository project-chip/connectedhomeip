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

import shlex
import subprocess
import psutil

import log

logger = log.get_logger(__file__)


class Bash:

    def __init__(self, command: str, sync: bool = False,
                 capture_output: bool = False,
                 cwd: str = None) -> None:
        """
        Run a bash command as a sub process
        :param command: Command to run
        :param sync: If True, wait for command to terminate
        :param capture_output: Only applies to sync; if True, store and suppress stdout and stderr
        :param cwd: Set working directory of command
        """
        self.logger = logger
        self.command: str = command
        self.sync = sync
        self.capture_output = capture_output
        self.cwd = cwd

        self.args: list[str] = []
        self._init_args()
        self.proc: None | subprocess.CompletedProcess | subprocess.Popen = None

    def _init_args(self) -> None:
        command_escaped = self.command.replace('"', '\"')
        self.args = shlex.split(f'/bin/bash -c "{command_escaped}"')

    def command_is_running(self) -> bool:
        return self.proc is not None and self.proc.poll() is None

    def get_captured_output(self) -> str:
        return "" if not self.capture_output or not self.sync \
            else self.proc.stdout.decode().strip()

    def start_command(self) -> None:
        if self.sync:
            self.proc = subprocess.run(self.args, capture_output=self.capture_output, cwd=self.cwd)
            return
        if not self.command_is_running():
            self.proc = subprocess.Popen(self.args, cwd=self.cwd)
        else:
            self.logger.warning(f'{self.command} start requested while running')

    def stop_command(self, soft: bool = False) -> None:
        if self.command_is_running():
            if soft:
                self.proc.terminate()
                if self.proc.stdout:
                    self.proc.stdout.close()
                if self.proc.stderr:
                    self.proc.stderr.close()
                self.proc.wait()
            else:
                psutil_proc = psutil.Process(self.proc.pid)
                for child_proc in psutil_proc.children(recursive=True):
                    try:
                        child_proc.terminate()
                        child_proc.wait(3)
                    except subprocess.TimeoutExpired:
                        try:
                            child_proc.kill()
                            child_proc.wait(3)
                        except subprocess.TimeoutExpired:
                            self.logger.error(f"Could not kill pid {child_proc.pid}")
                try:
                    psutil_proc.terminate()
                    psutil_proc.wait(3)
                except subprocess.TimeoutExpired:
                    try:
                        psutil_proc.kill()
                        psutil_proc.wait(3)
                    except subprocess.TimeoutExpired:
                        self.logger.error(f"Could not kill pid {psutil_proc.pid}")

        else:
            self.logger.warning(f'{self.command} stop requested while not running')
        self.proc = None

    def finished_success(self) -> bool:
        if not self.sync:
            return not self.command_is_running() and self.proc.returncode == 0
        else:
            return self.proc is not None and self.proc.returncode == 0
