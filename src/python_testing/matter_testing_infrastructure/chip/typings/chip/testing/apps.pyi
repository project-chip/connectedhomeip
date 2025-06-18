# src/python_testing/matter_testing_infrastructure/chip/typings/chip/testing/apps.py

from typing import Any

from chip.testing.tasks import Subprocess


class AppServerSubprocess(Subprocess):
    PREFIX: bytes

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, port: int = ...) -> None: ...

    def __del__(self) -> None: ...

    kvs_fd: int


class IcdAppServerSubprocess(AppServerSubprocess):
    paused: bool

    def __init__(self, *args: Any, **kwargs: Any) -> None: ...

    def pause(self, check_state: bool = ...) -> None: ...

    def resume(self, check_state: bool = ...) -> None: ...

    def terminate(self) -> None: ...
