from __future__ import annotations

import argparse
import ssl
import sys
import warnings
from typing import List, Optional

from .config import Config
from .run import run

sentinel = object()


def _load_config(config_path: Optional[str]) -> Config:
    if config_path is None:
        return Config()
    elif config_path.startswith("python:"):
        return Config.from_object(config_path[len("python:") :])
    elif config_path.startswith("file:"):
        return Config.from_pyfile(config_path[len("file:") :])
    else:
        return Config.from_toml(config_path)


def main(sys_args: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "application", help="The application to dispatch to as path.to.module:instance.path"
    )
    parser.add_argument("--access-log", help="Deprecated, see access-logfile", default=sentinel)
    parser.add_argument(
        "--access-logfile",
        help="The target location for the access log, use `-` for stdout",
        default=sentinel,
    )
    parser.add_argument(
        "--access-logformat",
        help="The log format for the access log, see help docs",
        default=sentinel,
    )
    parser.add_argument(
        "--backlog", help="The maximum number of pending connections", type=int, default=sentinel
    )
    parser.add_argument(
        "-b",
        "--bind",
        dest="binds",
        help=""" The TCP host/address to bind to. Should be either host:port, host,
        unix:path or fd://num, e.g. 127.0.0.1:5000, 127.0.0.1,
        unix:/tmp/socket or fd://33 respectively.  """,
        default=[],
        action="append",
    )
    parser.add_argument("--ca-certs", help="Path to the SSL CA certificate file", default=sentinel)
    parser.add_argument("--certfile", help="Path to the SSL certificate file", default=sentinel)
    parser.add_argument("--cert-reqs", help="See verify mode argument", type=int, default=sentinel)
    parser.add_argument("--ciphers", help="Ciphers to use for the SSL setup", default=sentinel)
    parser.add_argument(
        "-c",
        "--config",
        help="Location of a TOML config file, or when prefixed with `file:` a Python file, or when prefixed with `python:` a Python module.",  # noqa: E501
        default=None,
    )
    parser.add_argument(
        "--debug",
        help="Enable debug mode, i.e. extra logging and checks",
        action="store_true",
        default=sentinel,
    )
    parser.add_argument("--error-log", help="Deprecated, see error-logfile", default=sentinel)
    parser.add_argument(
        "--error-logfile",
        "--log-file",
        dest="error_logfile",
        help="The target location for the error log, use `-` for stderr",
        default=sentinel,
    )
    parser.add_argument(
        "--graceful-timeout",
        help="""Time to wait after SIGTERM or Ctrl-C for any remaining requests (tasks)
        to complete.""",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "--read-timeout",
        help="""Seconds to wait before timing out reads on TCP sockets""",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "--max-requests",
        help="""Maximum number of requests a worker will process before restarting""",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "--max-requests-jitter",
        help="This jitter causes the max-requests per worker to be "
        "randomized by randint(0, max_requests_jitter)",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "-g", "--group", help="Group to own any unix sockets.", default=sentinel, type=int
    )
    parser.add_argument(
        "-k",
        "--worker-class",
        dest="worker_class",
        help="The type of worker to use. "
        "Options include asyncio, uvloop (pip install hypercorn[uvloop]), "
        "and trio (pip install hypercorn[trio]).",
        default=sentinel,
    )
    parser.add_argument(
        "--keep-alive",
        help="Seconds to keep inactive connections alive for",
        default=sentinel,
        type=int,
    )
    parser.add_argument("--keyfile", help="Path to the SSL key file", default=sentinel)
    parser.add_argument(
        "--keyfile-password", help="Password to decrypt the SSL key file", default=sentinel
    )
    parser.add_argument(
        "--insecure-bind",
        dest="insecure_binds",
        help="""The TCP host/address to bind to. SSL options will not apply to these binds.
        See *bind* for formatting options. Care must be taken! See HTTP -> HTTPS redirection docs.
        """,
        default=[],
        action="append",
    )
    parser.add_argument(
        "--log-config",
        help=""""A Python logging configuration file. This can be prefixed with
        'json:' or 'toml:' to load the configuration from a file in
        that format. Default is the logging ini format.""",
        default=sentinel,
    )
    parser.add_argument(
        "--log-level", help="The (error) log level, defaults to info", default=sentinel
    )
    parser.add_argument(
        "-p", "--pid", help="Location to write the PID (Program ID) to.", default=sentinel
    )
    parser.add_argument(
        "--quic-bind",
        dest="quic_binds",
        help="""The UDP/QUIC host/address to bind to. See *bind* for formatting
        options.
        """,
        default=[],
        action="append",
    )
    parser.add_argument(
        "--reload",
        help="Enable automatic reloads on code changes",
        action="store_true",
        default=sentinel,
    )
    parser.add_argument(
        "--root-path", help="The setting for the ASGI root_path variable", default=sentinel
    )
    parser.add_argument(
        "--server-name",
        dest="server_names",
        help="""The hostnames that can be served, requests to different hosts
        will be responded to with 404s.
        """,
        default=[],
        action="append",
    )
    parser.add_argument(
        "--statsd-host", help="The host:port of the statsd server", default=sentinel
    )
    parser.add_argument("--statsd-prefix", help="Prefix for all statsd messages", default="")
    parser.add_argument(
        "-m",
        "--umask",
        help="The permissions bit mask to use on any unix sockets.",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "-u", "--user", help="User to own any unix sockets.", default=sentinel, type=int
    )

    def _convert_verify_mode(value: str) -> ssl.VerifyMode:
        try:
            return ssl.VerifyMode[value]
        except KeyError:
            raise argparse.ArgumentTypeError(f"'{value}' is not a valid verify mode")

    parser.add_argument(
        "--verify-mode",
        help="SSL verify mode for peer's certificate, see ssl.VerifyMode enum for possible values.",
        type=_convert_verify_mode,
        default=sentinel,
    )
    parser.add_argument(
        "--websocket-ping-interval",
        help="""If set this is the time in seconds between pings sent to the client.
        This can be used to keep the websocket connection alive.""",
        default=sentinel,
        type=int,
    )
    parser.add_argument(
        "-w",
        "--workers",
        dest="workers",
        help="The number of workers to spawn and use",
        default=sentinel,
        type=int,
    )
    args = parser.parse_args(sys_args or sys.argv[1:])
    config = _load_config(args.config)
    config.application_path = args.application

    if args.log_level is not sentinel:
        config.loglevel = args.log_level
    if args.access_logformat is not sentinel:
        config.access_log_format = args.access_logformat
    if args.access_log is not sentinel:
        warnings.warn(
            "The --access-log argument is deprecated, use `--access-logfile` instead",
            DeprecationWarning,
        )
        config.accesslog = args.access_log
    if args.access_logfile is not sentinel:
        config.accesslog = args.access_logfile
    if args.backlog is not sentinel:
        config.backlog = args.backlog
    if args.ca_certs is not sentinel:
        config.ca_certs = args.ca_certs
    if args.certfile is not sentinel:
        config.certfile = args.certfile
    if args.cert_reqs is not sentinel:
        config.cert_reqs = args.cert_reqs
    if args.ciphers is not sentinel:
        config.ciphers = args.ciphers
    if args.debug is not sentinel:
        config.debug = args.debug
    if args.error_log is not sentinel:
        warnings.warn(
            "The --error-log argument is deprecated, use `--error-logfile` instead",
            DeprecationWarning,
        )
        config.errorlog = args.error_log
    if args.error_logfile is not sentinel:
        config.errorlog = args.error_logfile
    if args.graceful_timeout is not sentinel:
        config.graceful_timeout = args.graceful_timeout
    if args.read_timeout is not sentinel:
        config.read_timeout = args.read_timeout
    if args.group is not sentinel:
        config.group = args.group
    if args.keep_alive is not sentinel:
        config.keep_alive_timeout = args.keep_alive
    if args.keyfile is not sentinel:
        config.keyfile = args.keyfile
    if args.keyfile_password is not sentinel:
        config.keyfile_password = args.keyfile_password
    if args.log_config is not sentinel:
        config.logconfig = args.log_config
    if args.max_requests is not sentinel:
        config.max_requests = args.max_requests
    if args.max_requests_jitter is not sentinel:
        config.max_requests_jitter = args.max_requests
    if args.pid is not sentinel:
        config.pid_path = args.pid
    if args.root_path is not sentinel:
        config.root_path = args.root_path
    if args.reload is not sentinel:
        config.use_reloader = args.reload
    if args.statsd_host is not sentinel:
        config.statsd_host = args.statsd_host
    if args.statsd_prefix is not sentinel:
        config.statsd_prefix = args.statsd_prefix
    if args.umask is not sentinel:
        config.umask = args.umask
    if args.user is not sentinel:
        config.user = args.user
    if args.worker_class is not sentinel:
        config.worker_class = args.worker_class
    if args.verify_mode is not sentinel:
        config.verify_mode = args.verify_mode
    if args.websocket_ping_interval is not sentinel:
        config.websocket_ping_interval = args.websocket_ping_interval
    if args.workers is not sentinel:
        config.workers = args.workers

    if len(args.binds) > 0:
        config.bind = args.binds
    if len(args.insecure_binds) > 0:
        config.insecure_bind = args.insecure_binds
    if len(args.quic_binds) > 0:
        config.quic_bind = args.quic_binds
    if len(args.server_names) > 0:
        config.server_names = args.server_names

    return run(config)


if __name__ == "__main__":
    sys.exit(main())
