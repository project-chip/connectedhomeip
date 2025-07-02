# src/python_testing/matter_testing_infrastructure/chip/typings/chip/testing/tasks.py

import threading
from typing import Any, BinaryIO, Callable, Optional, Pattern, Union


def forward_f(f_in: BinaryIO, f_out: BinaryIO,
              cb: Optional[Callable[[bytes, bool], bytes]] = ...,
              is_stderr: bool = ...) -> None: ...


class Subprocess(threading.Thread):
    program: str
    args: tuple[str, ...]
    output_cb: Optional[Callable[[bytes, bool], bytes]]
    f_stdout: BinaryIO
    f_stderr: BinaryIO
    output_match: Optional[Pattern[bytes]]
    returncode: Optional[int]
    p: Any
    event: threading.Event
    event_started: threading.Event
    expected_output: Optional[Union[str, Pattern[bytes]]]

    def __init__(self, program: str, *args: str,
                 output_cb: Optional[Callable[[bytes, bool], bytes]] = ...,
                 f_stdout: BinaryIO = ...,
                 f_stderr: BinaryIO = ...) -> None: ...

    def _set_output_match(self, pattern: Union[str, Pattern[bytes]]) -> None: ...

    def _check_output(self, line: bytes, is_stderr: bool) -> bytes: ...

    def run(self) -> None: ...

    def start(self,
              expected_output: Optional[Union[str, Pattern[bytes]]] = ...,
              timeout: Optional[float] = ...) -> None: ...

    def send(self, message: str, end: str = ...,
             expected_output: Optional[Union[str, Pattern[bytes]]] = ...,
             timeout: Optional[float] = ...) -> None: ...

    def terminate(self) -> None: ...

    def wait(self, timeout: Optional[float] = ...) -> Optional[int]: ...
