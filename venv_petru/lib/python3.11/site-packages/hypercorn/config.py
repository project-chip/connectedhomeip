from __future__ import annotations

import importlib
import importlib.util
import logging
import os
import socket
import stat
import sys
import types
import warnings
from dataclasses import dataclass
from ssl import (
    create_default_context,
    OP_NO_COMPRESSION,
    Purpose,
    SSLContext,
    TLSVersion,
    VerifyFlags,
    VerifyMode,
)
from time import time
from typing import Any, AnyStr, Dict, List, Mapping, Optional, Tuple, Type, Union
from wsgiref.handlers import format_date_time

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib

from .logging import Logger

BYTES = 1
OCTETS = 1
SECONDS = 1.0

FilePath = Union[AnyStr, os.PathLike]
SocketKind = Union[int, socket.SocketKind]


@dataclass
class Sockets:
    secure_sockets: List[socket.socket]
    insecure_sockets: List[socket.socket]
    quic_sockets: List[socket.socket]


class SocketTypeError(Exception):
    def __init__(self, expected: SocketKind, actual: SocketKind) -> None:
        super().__init__(
            f'Unexpected socket type, wanted "{socket.SocketKind(expected)}" got '
            f'"{socket.SocketKind(actual)}"'
        )


class Config:
    _bind = ["127.0.0.1:8000"]
    _insecure_bind: List[str] = []
    _quic_bind: List[str] = []
    _quic_addresses: List[Tuple] = []
    _log: Optional[Logger] = None
    _root_path: str = ""

    access_log_format = '%(h)s %(l)s %(l)s %(t)s "%(r)s" %(s)s %(b)s "%(f)s" "%(a)s"'
    accesslog: Union[logging.Logger, str, None] = None
    alpn_protocols = ["h2", "http/1.1"]
    alt_svc_headers: List[str] = []
    application_path: str
    backlog = 100
    ca_certs: Optional[str] = None
    certfile: Optional[str] = None
    ciphers: str = "ECDHE+AESGCM"
    debug = False
    dogstatsd_tags = ""
    errorlog: Union[logging.Logger, str, None] = "-"
    graceful_timeout: float = 3 * SECONDS
    read_timeout: Optional[int] = None
    group: Optional[int] = None
    h11_max_incomplete_size = 16 * 1024 * BYTES
    h11_pass_raw_headers = False
    h2_max_concurrent_streams = 100
    h2_max_header_list_size = 2**16
    h2_max_inbound_frame_size = 2**14 * OCTETS
    include_date_header = True
    include_server_header = True
    keep_alive_timeout = 5 * SECONDS
    keep_alive_max_requests = 1000
    keyfile: Optional[str] = None
    keyfile_password: Optional[str] = None
    logconfig: Optional[str] = None
    logconfig_dict: Optional[dict] = None
    logger_class = Logger
    loglevel: str = "INFO"
    max_app_queue_size: int = 10
    max_requests: Optional[int] = None
    max_requests_jitter: int = 0
    pid_path: Optional[str] = None
    server_names: List[str] = []
    shutdown_timeout = 60 * SECONDS
    ssl_handshake_timeout = 60 * SECONDS
    startup_timeout = 60 * SECONDS
    statsd_host: Optional[str] = None
    statsd_prefix = ""
    umask: Optional[int] = None
    use_reloader = False
    user: Optional[int] = None
    verify_flags: Optional[VerifyFlags] = None
    verify_mode: Optional[VerifyMode] = None
    websocket_max_message_size = 16 * 1024 * 1024 * BYTES
    websocket_ping_interval: Optional[float] = None
    worker_class = "asyncio"
    workers = 1
    wsgi_max_body_size = 16 * 1024 * 1024 * BYTES

    def set_cert_reqs(self, value: int) -> None:
        warnings.warn("Please use verify_mode instead", Warning)
        self.verify_mode = VerifyMode(value)

    cert_reqs = property(None, set_cert_reqs)

    @property
    def log(self) -> Logger:
        if self._log is None:
            self._log = self.logger_class(self)
        return self._log

    @property
    def bind(self) -> List[str]:
        return self._bind

    @bind.setter
    def bind(self, value: Union[List[str], str]) -> None:
        if isinstance(value, str):
            self._bind = [value]
        else:
            self._bind = value

    @property
    def insecure_bind(self) -> List[str]:
        return self._insecure_bind

    @insecure_bind.setter
    def insecure_bind(self, value: Union[List[str], str]) -> None:
        if isinstance(value, str):
            self._insecure_bind = [value]
        else:
            self._insecure_bind = value

    @property
    def quic_bind(self) -> List[str]:
        return self._quic_bind

    @quic_bind.setter
    def quic_bind(self, value: Union[List[str], str]) -> None:
        if isinstance(value, str):
            self._quic_bind = [value]
        else:
            self._quic_bind = value

    @property
    def root_path(self) -> str:
        return self._root_path

    @root_path.setter
    def root_path(self, value: str) -> None:
        self._root_path = value.rstrip("/")

    def create_ssl_context(self) -> Optional[SSLContext]:
        if not self.ssl_enabled:
            return None

        context = create_default_context(Purpose.CLIENT_AUTH)
        context.set_ciphers(self.ciphers)
        context.minimum_version = TLSVersion.TLSv1_2  # RFC 7540 Section 9.2: MUST be TLS >=1.2
        context.options = OP_NO_COMPRESSION  # RFC 7540 Section 9.2.1: MUST disable compression
        context.set_alpn_protocols(self.alpn_protocols)

        if self.certfile is not None and self.keyfile is not None:
            context.load_cert_chain(
                certfile=self.certfile,
                keyfile=self.keyfile,
                password=self.keyfile_password,
            )

        if self.ca_certs is not None:
            context.load_verify_locations(self.ca_certs)
        if self.verify_mode is not None:
            context.verify_mode = self.verify_mode
        if self.verify_flags is not None:
            context.verify_flags = self.verify_flags

        return context

    @property
    def ssl_enabled(self) -> bool:
        return self.certfile is not None and self.keyfile is not None

    def create_sockets(self) -> Sockets:
        if self.ssl_enabled:
            secure_sockets = self._create_sockets(self.bind)
            insecure_sockets = self._create_sockets(self.insecure_bind)
            quic_sockets = self._create_sockets(self.quic_bind, socket.SOCK_DGRAM)
            self._set_quic_addresses(quic_sockets)
        else:
            secure_sockets = []
            insecure_sockets = self._create_sockets(self.bind)
            quic_sockets = []
        return Sockets(secure_sockets, insecure_sockets, quic_sockets)

    def _set_quic_addresses(self, sockets: List[socket.socket]) -> None:
        self._quic_addresses = []
        for sock in sockets:
            name = sock.getsockname()
            if type(name) is not str and len(name) >= 2:
                self._quic_addresses.append(name)
            else:
                warnings.warn(
                    f'Cannot create a alt-svc header for the QUIC socket with address "{name}"',
                    Warning,
                )

    def _create_sockets(
        self, binds: List[str], type_: int = socket.SOCK_STREAM
    ) -> List[socket.socket]:
        sockets: List[socket.socket] = []
        for bind in binds:
            binding: Any = None
            if bind.startswith("unix:"):
                sock = socket.socket(socket.AF_UNIX, type_)
                binding = bind[5:]
                try:
                    if stat.S_ISSOCK(os.stat(binding).st_mode):
                        os.remove(binding)
                except FileNotFoundError:
                    pass
            elif bind.startswith("fd://"):
                sock = socket.socket(fileno=int(bind[5:]))
                actual_type = sock.getsockopt(socket.SOL_SOCKET, socket.SO_TYPE)
                if actual_type != type_:
                    raise SocketTypeError(type_, actual_type)
            else:
                bind = bind.replace("[", "").replace("]", "")
                try:
                    value = bind.rsplit(":", 1)
                    host, port = value[0], int(value[1])
                except (ValueError, IndexError):
                    host, port = bind, 8000
                sock = socket.socket(socket.AF_INET6 if ":" in host else socket.AF_INET, type_)

                if type_ == socket.SOCK_STREAM:
                    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

                if self.workers > 1:
                    try:
                        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
                    except AttributeError:
                        pass
                binding = (host, port)

            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

            if bind.startswith("unix:"):
                if self.umask is not None:
                    current_umask = os.umask(self.umask)
                sock.bind(binding)
                if self.user is not None and self.group is not None:
                    os.chown(binding, self.user, self.group)
                if self.umask is not None:
                    os.umask(current_umask)
            elif bind.startswith("fd://"):
                pass
            else:
                sock.bind(binding)

            sock.setblocking(False)
            try:
                sock.set_inheritable(True)
            except AttributeError:
                pass
            sockets.append(sock)
        return sockets

    def response_headers(self, protocol: str) -> List[Tuple[bytes, bytes]]:
        headers = []
        if self.include_date_header:
            headers.append((b"date", format_date_time(time()).encode("ascii")))
        if self.include_server_header:
            headers.append((b"server", f"hypercorn-{protocol}".encode("ascii")))

        for alt_svc_header in self.alt_svc_headers:
            headers.append((b"alt-svc", alt_svc_header.encode()))
        if len(self.alt_svc_headers) == 0 and self._quic_addresses:
            from aioquic.h3.connection import H3_ALPN

            for version in H3_ALPN:
                for addr in self._quic_addresses:
                    port = addr[1]
                    headers.append((b"alt-svc", b'%s=":%d"; ma=3600' % (version.encode(), port)))

        return headers

    def set_statsd_logger_class(self, statsd_logger: Type[Logger]) -> None:
        if self.logger_class == Logger and self.statsd_host is not None:
            self.logger_class = statsd_logger

    @classmethod
    def from_mapping(
        cls: Type["Config"], mapping: Optional[Mapping[str, Any]] = None, **kwargs: Any
    ) -> "Config":
        """Create a configuration from a mapping.

        This allows either a mapping to be directly passed or as
        keyword arguments, for example,

        .. code-block:: python

            config = {'keep_alive_timeout': 10}
            Config.from_mapping(config)
            Config.from_mapping(keep_alive_timeout=10)

        Arguments:
            mapping: Optionally a mapping object.
            kwargs: Optionally a collection of keyword arguments to
                form a mapping.
        """
        mappings: Dict[str, Any] = {}
        if mapping is not None:
            mappings.update(mapping)
        mappings.update(kwargs)
        config = cls()
        for key, value in mappings.items():
            try:
                setattr(config, key, value)
            except AttributeError:
                pass

        return config

    @classmethod
    def from_pyfile(cls: Type["Config"], filename: FilePath) -> "Config":
        """Create a configuration from a Python file.

        .. code-block:: python

            Config.from_pyfile('hypercorn_config.py')

        Arguments:
            filename: The filename which gives the path to the file.
        """
        file_path = os.fspath(filename)
        spec = importlib.util.spec_from_file_location("module.name", file_path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return cls.from_object(module)

    @classmethod
    def from_toml(cls: Type["Config"], filename: FilePath) -> "Config":
        """Load the configuration values from a TOML formatted file.

        This allows configuration to be loaded as so

        .. code-block:: python

            Config.from_toml('config.toml')

        Arguments:
            filename: The filename which gives the path to the file.
        """
        file_path = os.fspath(filename)
        with open(file_path, "rb") as file_:
            data = tomllib.load(file_)
        return cls.from_mapping(data)

    @classmethod
    def from_object(cls: Type["Config"], instance: Union[object, str]) -> "Config":
        """Create a configuration from a Python object.

        This can be used to reference modules or objects within
        modules for example,

        .. code-block:: python

            Config.from_object('module')
            Config.from_object('module.instance')
            from module import instance
            Config.from_object(instance)

        are valid.

        Arguments:
            instance: Either a str referencing a python object or the
                object itself.

        """
        if isinstance(instance, str):
            try:
                instance = importlib.import_module(instance)
            except ImportError:
                path, config = instance.rsplit(".", 1)
                module = importlib.import_module(path)
                instance = getattr(module, config)

        mapping = {
            key: getattr(instance, key)
            for key in dir(instance)
            if not isinstance(getattr(instance, key), types.ModuleType) and not key.startswith("__")
        }
        return cls.from_mapping(mapping)
