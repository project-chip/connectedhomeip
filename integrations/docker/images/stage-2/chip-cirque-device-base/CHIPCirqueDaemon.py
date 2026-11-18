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

import logging
import os
import signal
import subprocess
import sys
import time
from dataclasses import dataclass
from enum import Enum
from multiprocessing.connection import Client, Listener
from pathlib import Path

import click

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


# Global registry of spawned child processes for graceful termination
_SPAWNED_CHILDREN: list[subprocess.Popen] = []


def _cleanup_children(signum=None, frame=None):
    """Gracefully forwards SIGTERM to all child processes and waits for exit."""
    log.info("CHIPCirqueDaemon terminating; forwarding SIGTERM to %d children...", len(_SPAWNED_CHILDREN))
    alive = []
    for proc in _SPAWNED_CHILDREN:
        if proc.poll() is None:
            alive.append(proc)
            try:
                proc.terminate()
            except Exception as ex:
                log.debug("Error sending SIGTERM to child %s: %s", proc.pid, ex)

    # Wait up to 5 seconds for children to exit cleanly
    deadline = time.time() + 5.0
    while alive and time.time() < deadline:
        alive = [p for p in alive if p.poll() is None]
        if alive:
            time.sleep(0.1)

    # Fallback: SIGKILL any remaining children that didn't terminate
    for proc in alive:
        try:
            log.warning("Child PID %s did not terminate in time; killing", proc.pid)
            proc.kill()
        except Exception:
            pass

    if signum is not None:
        sys.exit(0)


def _reap_children(signum=None, frame=None):
    """Reaps exited children.

    entrypoint.sh execs this daemon, so it runs as PID 1 and also inherits
    orphaned grandchildren. Reap them so they do not linger as zombies.
    Popen.poll() reports 0 for a child reaped here.
    """
    while True:
        try:
            pid, _ = os.waitpid(-1, os.WNOHANG)
        except ChildProcessError:
            return
        if pid == 0:
            return


class ShellCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        if not self._args:
            return CommandResponse(CommandStatus.INVALID_ARGUMENT, "Cannot spwan background process")
        log.info("Will run command: %s", self._args)
        try:
            # As the command will be execued in background, we track the child so we can forward SIGTERM.
            proc = subprocess.Popen(
                self._args, stdout=sys.stdout, stderr=sys.stderr)
            _SPAWNED_CHILDREN[:] = [p for p in _SPAWNED_CHILDREN if p.poll() is None]
            _SPAWNED_CHILDREN.append(proc)
            return CommandResponse(CommandStatus.SUCCESS)
        except Exception as ex:
            return CommandResponse(CommandStatus.FAILURE, f"Failed to run command: {ex}")


class InvalidCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        return CommandResponse(CommandStatus.FAILURE, "invalid command")


SERVER_ADDRESS = "/tmp/cirque-helper.socket"
CLIENT_WAIT_TIMEOUT_SECONDS = 5


def CommandFactory(args):
    commands = {
        "run": ShellCommand
    }
    if len(args) == 0:
        return InvalidCommand()
    return commands.get(args[0], InvalidCommand)(args[1:])


def ServerMain(args):
    signal.signal(signal.SIGTERM, _cleanup_children)
    signal.signal(signal.SIGINT, _cleanup_children)
    signal.signal(signal.SIGCHLD, _reap_children)

    extraOptions = {
        "otbr-agent": ShellCommand(["otbr-agent", "-I", "wpan0", "-B", "eth0", "spinel+hdlc+uart:///dev/ttyUSB0"])
    }

    try:
        with Listener(SERVER_ADDRESS) as listener:
            log.info("Server running on %s", SERVER_ADDRESS)
            for extraOption in args:
                cmd = extraOptions.get(extraOption, InvalidCommand())
                cmd()

            while True:
                with listener.accept() as conn:
                    log.info("Received connection")
                    cmd = CommandFactory(conn.recv())
                    conn.send(cmd())
    finally:
        _cleanup_children()


def ClientMain(args):
    if len(args) == 0:
        sys.exit(1)
    # The server may start very slowly, wait for a few seconds to see if the server will start.
    for _ in range(CLIENT_WAIT_TIMEOUT_SECONDS):
        socks = Path(SERVER_ADDRESS)
        if socks.exists():
            break
        time.sleep(1)
    # If the address does not exist, Client constructor will throw an exception, so no need to add a flag.
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
