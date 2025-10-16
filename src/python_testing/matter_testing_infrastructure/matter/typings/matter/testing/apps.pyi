# src/python_testing/matter_testing_infrastructure/matter/typings/matter/testing/apps.py

from dataclasses import dataclass
from typing import Any, List, Optional, Union

from matter.ChipDeviceCtrl import ChipDeviceController
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
    def __init__(self, app: str, storage_dir: str, discriminator: int,
                 passcode: int, port: int = 5540, extra_args: List[str] = ...) -> None: ...


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
                 port: int = 5541, extra_args: List[str] = ...,
                 kvs_path: Optional[str] = ..., persist_kvs: bool = ...) -> None: ...

    def create_acl_entry(self, dev_ctrl: ChipDeviceController, provider_node_id: int,
                         requestor_node_id: Optional[int] = None) -> Any: ...
