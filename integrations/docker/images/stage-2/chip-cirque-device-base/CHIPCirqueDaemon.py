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


def _handle_stop(signum, frame):
    """Ignore further stop signals and exit so ServerMain cleans up once."""
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    raise SystemExit(0)


def _scan_proc_children() -> tuple[list[tuple[int, int]], list[int]]:
    """Return live (pid, pgid) pairs and zombie pids with ppid == getpid()."""
    my_pid = os.getpid()
    live: list[tuple[int, int]] = []
    zombies: list[int] = []
    proc_dir = Path("/proc")
    if not proc_dir.is_dir():
        return live, zombies

    for entry in proc_dir.iterdir():
        if not entry.name.isdigit():
            continue
        pid = int(entry.name)
        if pid == my_pid:
            continue
        try:
            stat = (entry / "stat").read_text(
                encoding="utf-8", errors="replace"
            )
        except OSError:
            continue
        rparen = stat.rfind(")")
        if rparen == -1:
            continue
        after = stat[rparen + 1:].split()
        if len(after) < 3:
            continue
        state = after[0]
        try:
            ppid = int(after[1])
            pgrp = int(after[2])
        except ValueError:
            continue
        if ppid != my_pid:
            continue
        if state == "Z":
            zombies.append(pid)
        else:
            live.append((pid, pgrp))
    return live, zombies


def _signal_target(
    pid: int,
    pgid: int | None,
    sig: int,
    signaled_pgids: set[int] | None = None,
) -> bool:
    """Signal process group (when distinct from ours) or fall back to pid."""
    my_pgrp = os.getpgrp()
    if pgid is not None and pgid > 0 and pgid != my_pgrp:
        try:
            os.killpg(pgid, sig)
            if signaled_pgids is not None:
                signaled_pgids.add(pgid)
            return True
        except ProcessLookupError:
            # Group leader or process group may already be gone; try direct pid.
            pass
        except OSError as ex:
            log.debug("Error sending signal %s to pgid %s: %s", sig, pgid, ex)

    try:
        os.kill(pid, sig)
        return True
    except ProcessLookupError:
        log.debug("Child %s already exited", pid)
        return False
    except OSError as ex:
        log.debug("Error sending signal %s to child %s: %s", sig, pid, ex)
        return False


def _reap_untracked_zombies(zombies: list[int], tracked_pids: set[int]) -> None:
    """Reap zombie children not currently tracked by a live Popen handle."""
    for zpid in zombies:
        if zpid in tracked_pids:
            continue
        try:
            os.waitpid(zpid, os.WNOHANG)
        except ChildProcessError:
            log.debug("Zombie child %s already reaped", zpid)


def _collect_live_children(
    procs: list[subprocess.Popen],
) -> tuple[list[subprocess.Popen], list[tuple[int, int]]]:
    """Return live Popen handles and adopted (pid, pgid) pairs."""
    terminating = [p for p in procs if p.poll() is None]
    tracked_pids = {p.pid for p in terminating}
    live_proc, zombies = _scan_proc_children()
    _reap_untracked_zombies(zombies, tracked_pids)
    adopted = [
        (pid, pgid) for pid, pgid in live_proc if pid not in tracked_pids
    ]
    return terminating, adopted


def _wait_adopted_exit(pid: int, timeout: float = 1.0) -> None:
    """Wait briefly for a killed adopted child to exit and reap it."""
    kill_deadline = time.monotonic() + timeout
    while time.monotonic() < kill_deadline:
        try:
            waited_pid, _ = os.waitpid(pid, os.WNOHANG)
        except ChildProcessError:
            return
        if waited_pid == pid:
            return
        time.sleep(0.05)


_CLEANUP_GRACE_SECONDS = 5.0
_MAX_FOLLOWUP_WAVES = 2


def _signal_adopted_children(
    adopted: list[tuple[int, int]],
    signaled_pids: set[int],
    signaled_pgids: set[int],
    already_hit_pgids: set[int] | None = None,
) -> None:
    """Forward SIGTERM to newly discovered adopted children."""
    newly_signaled_pgids: set[int] = (
        set(already_hit_pgids) if already_hit_pgids is not None else set()
    )
    for pid, pgid in adopted:
        if pid in signaled_pids:
            continue
        if pgid not in signaled_pgids:
            _signal_target(pid, pgid, signal.SIGTERM, signaled_pgids)
            if pgid in signaled_pgids:
                newly_signaled_pgids.add(pgid)
        elif pgid not in newly_signaled_pgids:
            # Group was already signaled in an earlier pass; target only this
            # newly adopted descendant without re-signaling its group.
            _signal_target(pid, None, signal.SIGTERM)
        signaled_pids.add(pid)


def _run_cleanup_wave(
    terminating: list[subprocess.Popen],
    adopted: list[tuple[int, int]],
    signaled_pids: set[int],
    signaled_pgids: set[int],
    killed_pids: set[int],
) -> None:
    """Wait up to grace window for children to exit, then SIGKILL stragglers."""
    deadline = time.monotonic() + _CLEANUP_GRACE_SECONDS
    while (terminating or adopted) and time.monotonic() < deadline:
        terminating, live_adopted = _collect_live_children(terminating)
        adopted = [
            (pid, pgid)
            for pid, pgid in live_adopted
            if pid not in killed_pids
        ]
        _signal_adopted_children(adopted, signaled_pids, signaled_pgids)
        if terminating or adopted:
            time.sleep(0.1)

    terminating, live_adopted = _collect_live_children(terminating)
    adopted = [
        (pid, pgid) for pid, pgid in live_adopted if pid not in killed_pids
    ]

    # Fallback: SIGKILL any remaining signaled children that didn't terminate
    # in time; unsignaled adoptees get a follow-up SIGTERM wave.
    for proc in terminating:
        log.warning(
            "Child PID %s did not terminate in time; killing", proc.pid
        )
        _signal_target(proc.pid, proc.pid, signal.SIGKILL)
        killed_pids.add(proc.pid)
        try:
            proc.wait(timeout=1.0)
        except subprocess.TimeoutExpired:
            log.debug("Timed out waiting for killed child %s", proc.pid)

    for pid, pgid in adopted:
        if pid not in signaled_pids:
            continue
        log.warning(
            "Adopted child PID %s did not terminate in time; killing", pid
        )
        _signal_target(pid, pgid, signal.SIGKILL)
        killed_pids.add(pid)
        _wait_adopted_exit(pid)


def _cleanup_children() -> None:
    """Forward SIGTERM to all child processes/groups and reap them."""
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    # Disable async SIGCHLD handler so poll() and explicit reaping never race.
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)

    terminating, adopted = _collect_live_children(_SPAWNED_CHILDREN)
    if not terminating and not adopted:
        _reap_children()
        return

    log.info(
        "CHIPCirqueDaemon terminating; forwarding SIGTERM to %d tracked "
        "and %d adopted children...",
        len(terminating),
        len(adopted),
    )

    signaled_pids: set[int] = {proc.pid for proc in terminating}
    signaled_pgids: set[int] = set()
    killed_pids: set[int] = set()
    still_terminating = []
    for proc in terminating:
        if _signal_target(proc.pid, proc.pid, signal.SIGTERM, signaled_pgids):
            still_terminating.append(proc)
        else:
            proc.poll()
    terminating = still_terminating

    _signal_adopted_children(
        adopted,
        signaled_pids,
        signaled_pgids,
        already_hit_pgids=signaled_pgids,
    )

    for wave in range(_MAX_FOLLOWUP_WAVES + 1):
        _run_cleanup_wave(
            terminating,
            adopted,
            signaled_pids,
            signaled_pgids,
            killed_pids,
        )

        terminating = []
        _, next_adopted = _collect_live_children([])
        adopted = [
            (pid, pgid)
            for pid, pgid in next_adopted
            if pid not in signaled_pids and pid not in killed_pids
        ]
        if not adopted or wave == _MAX_FOLLOWUP_WAVES:
            break

        log.info(
            "Found %d newly reparented adopted child(ren) after cleanup "
            "wave %d; forwarding SIGTERM...",
            len(adopted),
            wave + 1,
        )
        _signal_adopted_children(adopted, signaled_pids, signaled_pgids)

    _reap_children()


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
