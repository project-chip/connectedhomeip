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
import math
import os
import signal
import subprocess
import sys
import time
from contextlib import suppress
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


_DEFAULT_CLEANUP_GRACE_SECONDS = 1.5
_KILL_REAP_TIMEOUT_SECONDS = 0.4
_CLEANUP_POLL_INTERVAL_SECONDS = 0.05
_CLEANUP_GRACE_ENV_VAR = "CHIP_CIRQUE_SHUTDOWN_GRACE_SECONDS"

# Global registry of spawned child processes for graceful termination
_SPAWNED_CHILDREN: list[subprocess.Popen] = []


def _handle_stop(signum, frame):
    """Ignore further stop signals and exit so ServerMain cleans up once."""
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    raise SystemExit(0)


def _get_cleanup_grace_seconds() -> float:
    """Return shutdown grace timeout from env, falling back to default."""
    raw = os.environ.get(_CLEANUP_GRACE_ENV_VAR)
    if raw is None or not raw.strip():
        return _DEFAULT_CLEANUP_GRACE_SECONDS
    try:
        value = float(raw)
    except ValueError:
        value = math.nan
    if math.isfinite(value) and value > 0:
        return value
    log.warning(
        "Invalid %s=%r (must be finite and > 0); using default %.2fs",
        _CLEANUP_GRACE_ENV_VAR,
        raw,
        _DEFAULT_CLEANUP_GRACE_SECONDS,
    )
    return _DEFAULT_CLEANUP_GRACE_SECONDS


def _drain_children_until(deadline: float) -> bool:
    """Reap exited children until ECHILD or deadline. Return True if gone."""
    while True:
        try:
            pid, _ = os.waitpid(-1, os.WNOHANG)
        except ChildProcessError:
            return True
        if pid > 0:
            continue
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return False
        time.sleep(min(_CLEANUP_POLL_INTERVAL_SECONDS, remaining))


def _broadcast_container_signal(sig: int) -> None:
    """Broadcast sig to all other processes in the container PID namespace."""
    try:
        os.kill(-1, sig)
    except (ProcessLookupError, PermissionError) as ex:
        log.debug("Broadcast signal %s returned %s", sig, ex)


def _signal_proc_group(proc: subprocess.Popen, sig: int) -> None:
    """Send sig to proc's process group, falling back to direct process."""
    if proc.pid > 0 and proc.pid != os.getpgrp():
        try:
            os.killpg(proc.pid, sig)
            return
        except ProcessLookupError:
            log.debug(
                "Process group %s not found; signaling pid directly",
                proc.pid,
            )
        except OSError as ex:
            log.debug("Failed to signal pgid %s with %s: %s", proc.pid, sig, ex)
    try:
        if sig == signal.SIGKILL:
            proc.kill()
        else:
            proc.terminate()
    except OSError as ex:
        log.debug("Failed to signal child %s with %s: %s", proc.pid, sig, ex)


def _wait_spawned_until(
    procs: list[subprocess.Popen], deadline: float
) -> list[subprocess.Popen]:
    """Poll spawned handles until exit or deadline; return stragglers."""
    live = [p for p in procs if p.poll() is None]
    while live:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return live
        time.sleep(min(_CLEANUP_POLL_INTERVAL_SECONDS, remaining))
        live = [p for p in live if p.poll() is None]
    return live


def _cleanup_children() -> None:
    """Forward SIGTERM to children and reap them within the grace budget."""
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    # Disable async SIGCHLD handler so poll() and explicit reaping never race.
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)

    grace_seconds = _get_cleanup_grace_seconds()

    if os.getpid() == 1:
        log.info(
            "CHIPCirqueDaemon terminating as PID 1; "
            "broadcasting SIGTERM (grace %.2fs)...",
            grace_seconds,
        )
        _broadcast_container_signal(signal.SIGTERM)
        if not _drain_children_until(time.monotonic() + grace_seconds):
            log.warning(
                "Container processes did not terminate within %.2fs; "
                "broadcasting SIGKILL",
                grace_seconds,
            )
            _broadcast_container_signal(signal.SIGKILL)
            reap_deadline = time.monotonic() + _KILL_REAP_TIMEOUT_SECONDS
            if not _drain_children_until(reap_deadline):
                log.warning(
                    "Timed out reaping remaining container processes "
                    "after SIGKILL"
                )
        for proc in _SPAWNED_CHILDREN:
            with suppress(OSError):
                proc.poll()
        _SPAWNED_CHILDREN.clear()
        return

    live = [p for p in _SPAWNED_CHILDREN if p.poll() is None]
    if not live:
        _reap_children()
        _SPAWNED_CHILDREN.clear()
        return

    log.info(
        "CHIPCirqueDaemon terminating; forwarding SIGTERM to %d tracked "
        "child(ren) (grace %.2fs)...",
        len(live),
        grace_seconds,
    )
    for proc in live:
        _signal_proc_group(proc, signal.SIGTERM)

    stragglers = _wait_spawned_until(live, time.monotonic() + grace_seconds)
    if stragglers:
        log.warning(
            "%d child(ren) did not terminate within %.2fs; sending SIGKILL",
            len(stragglers),
            grace_seconds,
        )
        for proc in stragglers:
            _signal_proc_group(proc, signal.SIGKILL)
        remaining = _wait_spawned_until(
            stragglers, time.monotonic() + _KILL_REAP_TIMEOUT_SECONDS
        )
        if remaining:
            log.warning(
                "Timed out waiting for %d killed child(ren): %s",
                len(remaining),
                [p.pid for p in remaining],
            )

    _reap_children()
    _SPAWNED_CHILDREN.clear()


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
            # Track background child so we can forward SIGTERM on shutdown.
            blocked = {signal.SIGTERM, signal.SIGINT, signal.SIGCHLD}
            old_mask = signal.pthread_sigmask(signal.SIG_BLOCK, blocked)
            try:
                proc = subprocess.Popen(
                    self._args,
                    stdout=sys.stdout,
                    stderr=sys.stderr,
                    start_new_session=True,
                    preexec_fn=lambda: signal.pthread_sigmask(
                        signal.SIG_SETMASK, old_mask
                    ),
                )
                _SPAWNED_CHILDREN[:] = [
                    p for p in _SPAWNED_CHILDREN if p.poll() is None
                ]
                _SPAWNED_CHILDREN.append(proc)
            finally:
                signal.pthread_sigmask(signal.SIG_SETMASK, old_mask)
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


def _serve_loop(args):
    extraOptions = {
        "otbr-agent": ShellCommand(
            [
                "otbr-agent",
                "-I",
                "wpan0",
                "-B",
                "eth0",
                "spinel+hdlc+uart:///dev/ttyUSB0",
            ]
        )
    }

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


def ServerMain(args):
    signal.signal(signal.SIGTERM, _handle_stop)
    signal.signal(signal.SIGINT, _handle_stop)
    signal.signal(signal.SIGCHLD, _reap_children)

    try:
        _serve_loop(args)
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
