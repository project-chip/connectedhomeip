# src/python_testing/matter_testing_infrastructure/matter/typings/matter/testing/tasks.py

import threading
from collections.abc import Callable
from re import Pattern
from typing import Any, BinaryIO

def forward_f(f_in: BinaryIO, f_out: BinaryIO,
              cb: Callable[[bytes, bool], bytes] | None = ...,
              is_stderr: bool = ...) -> None: ...


class Subprocess(threading.Thread):
    program: str
    args: tuple[str, ...]
    output_cb: Callable[[bytes, bool], bytes] | None
    f_stdout: BinaryIO
    f_stderr: BinaryIO
    output_match: Pattern[bytes] | None
    returncode: int | None
    p: Any
    event: threading.Event
    event_started: threading.Event
    expected_output: str | Pattern[bytes] | None

    def __init__(self, program: str, *args: str,
                 output_cb: Callable[[bytes, bool], bytes] | None = ...,
                 f_stdout: BinaryIO = ...,
                 f_stderr: BinaryIO = ...) -> None: ...

    def _set_output_match(self, pattern: str | Pattern[bytes]) -> None: ...

    def _check_output(self, line: bytes, is_stderr: bool) -> bytes: ...

    def run(self) -> None: ...

    def start(self,
              expected_output: str | Pattern[bytes] | None = ...,
              timeout: float | None = ...) -> None: ...

    def send(self, message: str, end: str = ...,
             expected_output: str | Pattern[bytes] | None = ...,
             timeout: float | None = ...) -> None: ...

    def terminate(self) -> None: ...

    def wait(self, timeout: float | None = ...) -> int | None: ...
