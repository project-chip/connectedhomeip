# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import shlex
import subprocess
import sys
from typing import Dict, Optional

import constants

_ENV_FILENAME = ".shell_env"
_OUTPUT_FILENAME = ".shell_output"
_HERE = os.path.dirname(os.path.abspath(__file__))

TermColors = constants.TermColors


class StatefulShell:
    """A Shell that tracks state changes of the environment."""

    def __init__(self) -> None:
        if sys.platform == "linux" or sys.platform == "linux2":
            self.shell_app = '/bin/bash'
        elif sys.platform == "darwin":
            self.shell_app = '/bin/zsh'
        elif sys.platform == "win32":
            print('Windows is currently not supported. Use Linux or MacOS platforms')
            exit(1)

        self.env: Dict[str, str] = os.environ.copy()
        self.cwd: str = self.env["PWD"]

        # This file holds the env after running a command. This is a better approach
        # than writing to stdout because commands could redirect the stdout.
        self.envfile_path: str = os.path.join(_HERE, _ENV_FILENAME)
        self.cmd_output_path: str = os.path.join(_HERE, _OUTPUT_FILENAME)

    def print_env(self) -> None:
        """Print environment variables in commandline friendly format for export.

        The purpose of this function is to output the env variables in such a way
        that a user can copy the env variables and paste them in their terminal to
        quickly recreate the environment state.
        """
        for env_var in self.env:
            quoted_value = shlex.quote(self.env[env_var])
            if env_var:
                print(f"export {env_var}={quoted_value}")

    def run_cmd(
        self, cmd: str, *,
        raise_on_returncode=False,
        return_cmd_output=False,
    ) -> Optional[str]:
        """Runs a command and updates environment.

        Args:
          cmd: Command to execute.
            This does not support commands that run in the background e.g. `<cmd> &`
          raise_on_returncode: Whether to raise an error if the return code is nonzero.
          return_cmd_output: Whether to return the command output.
            If enabled, the text piped to screen won't be colorized due to output
            being passed through `tee`.

        Raises:
          RuntimeError: If raise_on_returncode is set and nonzero return code is given.

        Returns:
          Output of command if return_cmd_output set to True.
        """
        env_dict = {}
        # Set OLDPWD at beginning because opening the shell clears this. This handles 'cd -'.
        # env -0 prints the env variables separated by null characters for easy parsing.

        if return_cmd_output:
            # Piping won't work here because piping will affect how environment variables
            # are propagated. This solution uses tee without piping to preserve env variables.
            redirect = f" > >(tee \"{self.cmd_output_path}\") 2>&1 "  # include stderr
        else:
            redirect = ""

        command_with_state = (
            f"OLDPWD={self.env.get('OLDPWD', '')}; {cmd} {redirect}; RETCODE=$?;"
            f" env -0 > {self.envfile_path}; exit $RETCODE")
        with subprocess.Popen(
            [command_with_state],
            env=self.env, cwd=self.cwd,
            shell=True, executable=self.shell_app
        ) as proc:
            returncode = proc.wait()

        # Load env state from envfile.
        with open(self.envfile_path, encoding="latin1") as f:
            # Split on null char because we use env -0.
            env_entries = f.read().split("\0")
            for entry in env_entries:
                parts = entry.split("=")
                # Handle case where an env variable contains text with '='.
                env_dict[parts[0]] = "=".join(parts[1:])
            self.env = env_dict
            self.cwd = self.env["PWD"]

        if raise_on_returncode and returncode != 0:
            raise RuntimeError(
                "Error. Nonzero return code."
                f"\nReturncode: {returncode}"
                f"\nCmd: {cmd}")

        if return_cmd_output:
            with open(self.cmd_output_path, encoding="latin1") as f:
                output = f.read()
            return output
