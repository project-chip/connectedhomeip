from __future__ import annotations

import inspect
import os
import socket
import sys
from enum import Enum
from importlib import import_module
from multiprocessing.synchronize import Event as EventType
from pathlib import Path
from typing import (
    Any,
    Awaitable,
    Callable,
    cast,
    Dict,
    Iterable,
    List,
    Literal,
    Optional,
    Tuple,
    TYPE_CHECKING,
)

from .app_wrappers import ASGIWrapper, WSGIWrapper
from .config import Config
from .typing import AppWrapper, ASGIFramework, Framework, WSGIFramework

if TYPE_CHECKING:
    from .protocol.events import Request


class ShutdownError(Exception):
    pass


class NoAppError(Exception):
    pass


class LifespanTimeoutError(Exception):
    def __init__(self, stage: str) -> None:
        super().__init__(
            f"Timeout whilst awaiting {stage}. Your application may not support the ASGI Lifespan "
            f"protocol correctly, alternatively the {stage}_timeout configuration is incorrect."
        )


class LifespanFailureError(Exception):
    def __init__(self, stage: str, message: str) -> None:
        super().__init__(f"Lifespan failure in {stage}. '{message}'")


class UnexpectedMessageError(Exception):
    def __init__(self, state: Enum, message_type: str) -> None:
        super().__init__(f"Unexpected message type, {message_type} given the state {state}")


class FrameTooLargeError(Exception):
    pass


def suppress_body(method: str, status_code: int) -> bool:
    return method == "HEAD" or 100 <= status_code < 200 or status_code in {204, 304}


def build_and_validate_headers(headers: Iterable[Tuple[bytes, bytes]]) -> List[Tuple[bytes, bytes]]:
    # Validates that the header name and value are bytes
    validated_headers: List[Tuple[bytes, bytes]] = []
    for name, value in headers:
        if name[0] == b":"[0]:
            raise ValueError("Pseudo headers are not valid")
        validated_headers.append((bytes(name).strip(), bytes(value).strip()))
    return validated_headers


def filter_pseudo_headers(headers: List[Tuple[bytes, bytes]]) -> List[Tuple[bytes, bytes]]:
    filtered_headers: List[Tuple[bytes, bytes]] = [(b"host", b"")]  # Placeholder
    authority = None
    host = b""
    for name, value in headers:
        if name == b":authority":  # h2 & h3 libraries validate this is present
            authority = value
        elif name == b"host":
            host = value
        elif name[0] != b":"[0]:
            filtered_headers.append((name, value))
    filtered_headers[0] = (b"host", authority if authority is not None else host)
    return filtered_headers


def load_application(path: str, wsgi_max_body_size: int) -> AppWrapper:
    mode: Optional[Literal["asgi", "wsgi"]] = None
    if ":" not in path:
        module_name, app_name = path, "app"
    elif path.count(":") == 2:
        mode, module_name, app_name = path.split(":", 2)  # type: ignore
        if mode not in {"asgi", "wsgi"}:
            raise ValueError("Invalid mode, must be 'asgi', or 'wsgi'")
    else:
        module_name, app_name = path.split(":", 1)

    module_path = Path(module_name).resolve()
    sys.path.insert(0, str(module_path.parent))
    if module_path.is_file():
        import_name = module_path.with_suffix("").name
    else:
        import_name = module_path.name
    try:
        module = import_module(import_name)
    except ModuleNotFoundError as error:
        if error.name == import_name:
            raise NoAppError(f"Cannot load application from '{path}', module not found.")
        else:
            raise
    try:
        app = eval(app_name, vars(module))
    except NameError:
        raise NoAppError(f"Cannot load application from '{path}', application not found.")
    else:
        return wrap_app(app, wsgi_max_body_size, mode)


def wrap_app(
    app: Framework, wsgi_max_body_size: int, mode: Optional[Literal["asgi", "wsgi"]]
) -> AppWrapper:
    if mode is None:
        mode = "asgi" if is_asgi(app) else "wsgi"
    if mode == "asgi":
        return ASGIWrapper(cast(ASGIFramework, app))
    else:
        return WSGIWrapper(cast(WSGIFramework, app), wsgi_max_body_size)


def files_to_watch() -> Dict[Path, float]:
    last_updates: Dict[Path, float] = {}
    for module in list(sys.modules.values()):
        filename = getattr(module, "__file__", None)
        if filename is None:
            continue
        path = Path(filename)
        try:
            last_updates[Path(filename)] = path.stat().st_mtime
        except (FileNotFoundError, NotADirectoryError):
            pass
    return last_updates


def check_for_updates(files: Dict[Path, float]) -> bool:
    for path, last_mtime in files.items():
        try:
            mtime = path.stat().st_mtime
        except FileNotFoundError:
            return True
        else:
            if mtime > last_mtime:
                return True
            else:
                files[path] = mtime
    return False


async def raise_shutdown(shutdown_event: Callable[..., Awaitable]) -> None:
    await shutdown_event()
    raise ShutdownError()


async def check_multiprocess_shutdown_event(
    shutdown_event: EventType, sleep: Callable[[float], Awaitable[Any]]
) -> None:
    while True:
        if shutdown_event.is_set():
            return
        await sleep(0.1)


def write_pid_file(pid_path: str) -> None:
    with open(pid_path, "w") as file_:
        file_.write(f"{os.getpid()}")


def parse_socket_addr(family: int, address: tuple) -> Optional[Tuple[str, int]]:
    if family == socket.AF_INET:
        return address
    elif family == socket.AF_INET6:
        return (address[0], address[1])
    else:
        return None


def repr_socket_addr(family: int, address: tuple) -> str:
    if family == socket.AF_INET:
        return f"{address[0]}:{address[1]}"
    elif family == socket.AF_INET6:
        return f"[{address[0]}]:{address[1]}"
    elif family == socket.AF_UNIX:
        return f"unix:{address}"
    else:
        return f"{address}"


def valid_server_name(config: Config, request: "Request") -> bool:
    if len(config.server_names) == 0:
        return True

    host = ""
    for name, value in request.headers:
        if name.lower() == b"host":
            host = value.decode()
            break
    return host in config.server_names


def is_asgi(app: Any) -> bool:
    if inspect.iscoroutinefunction(app):
        return True
    elif hasattr(app, "__call__"):
        return inspect.iscoroutinefunction(app.__call__)
    return False
