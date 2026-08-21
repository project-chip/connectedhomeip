#!/usr/bin/env python3
#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Scriptable serial-console control for a Matter ED (Raspberry Pi) over UART.

This is the out-of-band control channel used by ``run-cert-test.sh`` and the
TC_COMPRO EDFixture when the ED runs in ``standalone-serial`` mode: eth0 is
physically disconnected, so the device is driven entirely over its serial
login console (``serial-getty@ttyS0`` at 115200, with autologin for ``ubuntu``).

Commands are framed with per-call BEGIN/END markers so that kernel console
spew (``console=ttyS0``) and shell echo can be reliably stripped, and the
command's exit status recovered.

CLI:
    serial_console.py run "hostname"                 # print stdout, exit=rc
    serial_console.py --port /dev/ttyUSB0 run "id"
    serial_console.py wait-shell                     # block until shell ready
"""

from __future__ import annotations

import argparse
import re
import sys
import time

import serial  # pyserial

DEFAULT_PORT = "/dev/ttyUSB0"
DEFAULT_BAUD = 115200
DEFAULT_USER = "ubuntu"
# Enter key. The ED's getty tty is in canonical mode with ICRNL, so CR is
# translated to newline just like a real terminal.
EOL = b"\r"


class SerialConsole:
    """A framed command channel over a serial login console."""

    def __init__(
        self,
        port: str = DEFAULT_PORT,
        baud: int = DEFAULT_BAUD,
        user: str = DEFAULT_USER,
        password: str | None = None,
    ):
        self._port = port
        self._baud = baud
        self._user = user
        self._password = password
        self._ser: serial.Serial | None = None
        self._nonce = 0

    def __enter__(self) -> SerialConsole:
        self.open()
        return self

    def __exit__(self, *exc) -> None:
        self.close()

    def open(self) -> None:
        # exclusive=True so we fail loudly if an interactive `screen` still
        # holds the port, rather than corrupting a shared session.
        self._ser = serial.Serial(self._port, self._baud, timeout=0.2, exclusive=True)

    def close(self) -> None:
        if self._ser is not None:
            self._ser.close()
            self._ser = None

    # -- low level ---------------------------------------------------------
    def _write(self, data: bytes) -> None:
        assert self._ser is not None
        self._ser.write(data)
        self._ser.flush()

    def _read_until(self, needle: str, timeout: float) -> str:
        assert self._ser is not None
        deadline = time.monotonic() + timeout
        buf = bytearray()
        target = needle.encode()
        while time.monotonic() < deadline:
            chunk = self._ser.read(256)
            if chunk:
                buf += chunk
                if target in buf:
                    return buf.decode(errors="replace")
        return buf.decode(errors="replace")

    # -- session -----------------------------------------------------------
    def ensure_shell(self, timeout: float = 60.0) -> None:
        """Get the console to an interactive shell (handles autologin/login)."""
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            self._write(EOL)
            data = self._read_until("no-such-marker", 1.5)
            if "login:" in data:
                self._write(self._user.encode() + EOL)
                data = self._read_until("assword", 3.0)
                if "assword" in data and self._password is not None:
                    self._write(self._password.encode() + EOL)
                    self._read_until("no-such-marker", 3.0)
            # Probe: if a trivial framed command round-trips, the shell is up.
            try:
                rc, _ = self.run("true", timeout=5.0)
                if rc == 0:
                    return
            except TimeoutError:
                pass
        raise TimeoutError(f"no shell on {self._port} within {timeout:.0f}s")

    def run(self, cmd: str, timeout: float = 30.0) -> tuple[int, str]:
        """Run ``cmd`` on the ED, returning (exit_code, stdout)."""
        assert self._ser is not None
        self._nonce += 1
        n = self._nonce
        begin = f"__B{n}__"
        end = f"__E{n}__"
        # Emit the markers via a shell variable so the *echoed* command line
        # contains only the literal "__B%s__"/"__E%s__" format strings, never
        # the expanded marker. Only the executed printf output carries the real
        # __B<n>__/__E<n>__, so read/parse cannot lock onto the shell's echo of
        # a long, line-wrapped command (e.g. the ED app-start invocation).
        framed = (
            f'__n={n}; printf "\\n__B%s__\\n" "$__n"; '
            f'{cmd}; '
            f'printf "\\n__E%s__ %s\\n" "$__n" "$?"'
        )
        self._ser.reset_input_buffer()
        self._write(framed.encode() + EOL)
        raw = self._read_until(end, timeout)
        if end not in raw:
            raise TimeoutError(f"command timed out after {timeout:.0f}s: {cmd!r}")
        return self._parse(raw, begin, end)

    @staticmethod
    def _parse(raw: str, begin: str, end: str) -> tuple[int, str]:
        text = raw.replace("\r", "")
        # The expanded markers appear only in the executed printf output. Anchor
        # on the last begin marker for safety and read the rc off the end line.
        region = text.rsplit(begin, 1)[1] if begin in text else text
        rc = -1
        m = re.search(rf"{re.escape(end)}\s+(\d+)", region)  # "__En__ <rc>"
        if m:
            rc = int(m.group(1))
        body = region.rsplit(end, 1)[0] if end in region else region
        body = re.sub(r"\x1b\[[0-9;?]*[a-zA-Z]", "", body)  # strip ANSI/paste
        lines = [ln for ln in body.splitlines() if ln.strip()]
        return rc, "\n".join(lines).strip()


def _main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", default=DEFAULT_PORT)
    ap.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    ap.add_argument("--user", default=DEFAULT_USER)
    ap.add_argument("--password", default=None)
    ap.add_argument("--timeout", type=float, default=30.0)
    sub = ap.add_subparsers(dest="action", required=True)
    p_run = sub.add_parser("run", help="run a command and print its output")
    p_run.add_argument("cmd")
    sub.add_parser("wait-shell", help="block until an interactive shell is ready")
    args = ap.parse_args(argv)

    with SerialConsole(args.port, args.baud, args.user, args.password) as con:
        con.ensure_shell(timeout=max(args.timeout, 60.0))
        if args.action == "wait-shell":
            print("shell ready")
            return 0
        rc, out = con.run(args.cmd, timeout=args.timeout)
        if out:
            print(out)
        return rc


if __name__ == "__main__":
    sys.exit(_main(sys.argv[1:]))
