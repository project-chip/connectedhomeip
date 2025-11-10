# src/python_testing/matter_testing_infrastructure/matter/typings/matter/testing/apps.py

from dataclasses import dataclass
from sys import stderr, stdout
from typing import Any, BinaryIO, List, Optional, Union

from matter.testing.tasks import Subprocess


@dataclass
class OtaImagePath:
    path: str
    @property
    def ota_args(self) -> List[str]: ...


@dataclass
class ImageListPath:
    path: str
    @property
    def ota_args(self) -> List[str]: ...


class AppServerSubprocess(Subprocess):
    PREFIX: bytes
    log_file = ""
    err_log_file = ""
    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, port: int = 5540, app_pipe: Optional[str] = None, app_pipe_out: Optional[str] = None, extra_args: List[str] = ...) -> None: ...


class IcdAppServerSubprocess(AppServerSubprocess):
    def __init__(self, *args: Any, **kwargs: Any) -> None: ...
    def pause(self, check_state: bool = True) -> None: ...
    def resume(self, check_state: bool = True) -> None: ...
    def terminate(self) -> None: ...


class JFControllerSubprocess(Subprocess):
    PREFIX: bytes
    def __init__(self, app: str, rpc_server_port: int, storage_dir: str,
                 vendor_id: int, extra_args: List[str] = ...) -> None: ...


class OTAProviderSubprocess(AppServerSubprocess):
    DEFAULT_ADMIN_NODE_ID: int
    PREFIX: bytes

    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, ota_source: Union[OtaImagePath, ImageListPath],
                 port: int = 5541, extra_args: list[str] = [], kvs_path: Optional[str] = None,
                 log_file: Union[str, BinaryIO] = stdout.buffer, err_log_file: Union[str, BinaryIO] = stderr.buffer, app_pipe: Optional[str] = None, app_pipe_out: Optional[str] = None): ...

    def kill(self) -> None: ...

    def get_pid(self) -> int: ...
