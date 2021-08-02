#!/usr/bin/env python3

# Copyright (c) 2020 Project CHIP Authors
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

import sys
import subprocess
import logging
import click
from dataclasses import dataclass
from enum import Enum
from multiprocessing.connection import Listener, Client

log = logging.getLogger()
log.setLevel(logging.DEBUG)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
log.addHandler(sh)


class CommandStatus(Enum):
    """Enum class for passing status code of execute CirqueDaemon command, not CHIP error codes."""
    SUCCESS = 0
    FAILURE = 1
    UNKNOWN_COMMAND = 2
    INVALID_ARGUMENT = 3


@dataclass
class CommandResponse:
    """Class for holding status of running CirqueDaemon commands."""
    status_code: CommandStatus
    error_message: str = ""


class ShellCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        if not self._args:
            return CommandResponse(CommandStatus.INVALID_ARGUMENT, "Cannot spwan background process")
        log.info("Will run command: {}".format(self._args))
        try:
            # As the command will be execued in background, we won't return the exit code of the program.
            subprocess.Popen(
                self._args, stdout=sys.stdout, stderr=sys.stderr)
            return CommandResponse(CommandStatus.SUCCESS)
        except Exception as ex:
            return CommandResponse(CommandStatus.FAILURE, "Failed to run command: {}".format(ex))


class InvalidCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        return CommandResponse(CommandStatus.FAILURE, "invalid command")


SERVER_ADDRESS = "/tmp/cirque-helper.socket"


def CommandFactory(args):
    commands = {
        "run": ShellCommand
    }
    if len(args) == 0:
        return InvalidCommand()
    return commands.get(args[0], InvalidCommand)(args[1:])


def ServerMain(args):
    extraOptions = {
        "otbr-agent": ShellCommand(["otbr-agent", "-I", "wpan0", "spinel+hdlc+uart:///dev/ttyUSB0"])
    }

    for extraOption in args:
        cmd = extraOptions.get(extraOption, InvalidCommand())
        cmd()

    with Listener(SERVER_ADDRESS) as listener:
        log.info("Server running on {}".format(SERVER_ADDRESS))
        while True:
            with listener.accept() as conn:
                log.info("Received connection")
                cmd = CommandFactory(conn.recv())
                conn.send(cmd())


def ClientMain(args):
    if len(args) == 0:
        sys.exit(1)
    with Client(SERVER_ADDRESS) as conn:
        conn.send(args)
        res = conn.recv()
        print(res)
        if res.status_code != CommandStatus.SUCCESS:
            sys.exit(1)


@click.command()
@click.option('--server', is_flag=True)
@click.argument('command', nargs=-1)
def main(server, command):
    if server:
        ServerMain(command)
    else:
        ClientMain(command)


if __name__ == '__main__':
    main()
