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
import shlex
import subprocess

import psutil

from . import log

logger = log.get_logger(__file__)


class Bash:

    def __init__(self, command: str, sync: bool = False,
                 capture_output: bool = False,
                 cwd: str = None) -> None:
        """
        Run a bash command as a sub process
        :param command: Command to run
        :param sync: If True, wait for command to terminate upon start_command()
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
        if self.proc is None:
            if self.sync:
                self.proc = subprocess.run(self.args, capture_output=self.capture_output, cwd=self.cwd)
            else:
                self.proc = subprocess.Popen(self.args, cwd=self.cwd, stdin=subprocess.PIPE)
        else:
            self.logger.warning(f'"{self.command}" start requested more than once for same Bash instance!')

    def term_with_sudo(self, proc: multiprocessing.Process) -> None:
        self.logger.debug(f"SIGTERM {proc.pid} with sudo")
        Bash(f"sudo kill {proc.pid}", sync=True).start_command()

    def kill_with_sudo(self, proc: multiprocessing.Process) -> None:
        self.logger.debug(f"SIGKILL {proc.pid} with sudo")
        Bash(f"sudo kill -9 {proc.pid}", sync=True).start_command()

    def term(self, proc: multiprocessing.Process) -> None:
        if "sudo" in self.command:
            self.term_with_sudo(proc)
        else:
            proc.terminate()

    def kill(self, proc: multiprocessing.Process) -> None:
        if "sudo" in self.command:
            self.kill_with_sudo(proc)
        else:
            proc.kill()

    def stop_single_proc(self, proc: multiprocessing.Process) -> None:
        self.logger.debug(f"Killing process {proc.pid}")
        try:
            self.logger.debug("Sending SIGTERM")
            self.term(proc)
            proc.wait(3)
        except psutil.TimeoutExpired:
            self.logger.error("SIGTERM timeout expired")
            try:
                self.logger.debug("Sending SIGKILL")
                self.kill(proc)
                proc.wait(3)
            except psutil.TimeoutExpired:
                self.logger.critical(f"SIGKILL timeout expired, could not kill pid  {proc.pid}")

    def stop_command(self) -> None:
        if self.command_is_running():
            psutil_proc = psutil.Process(self.proc.pid)
            suffix = f"{psutil_proc.pid} for command {self.command}"
            self.logger.debug(f"Stopping children of {suffix}")
            for child_proc in psutil_proc.children(recursive=True):
                self.stop_single_proc(child_proc)
            self.logger.debug(f"Killing root proc {suffix}")
            self.stop_single_proc(psutil_proc)
        else:
            self.logger.warning(f'{self.command} stop requested while not running')

    def finished_success(self) -> bool:
        if not self.sync:
            return not self.command_is_running() and self.proc.returncode == 0
        else:
            return self.proc is not None and self.proc.returncode == 0
