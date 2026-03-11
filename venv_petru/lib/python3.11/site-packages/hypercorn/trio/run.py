from __future__ import annotations

import sys
from functools import partial
from multiprocessing.synchronize import Event as EventType
from random import randint
from typing import Awaitable, Callable, Optional

import trio

from .lifespan import Lifespan
from .statsd import StatsdLogger
from .tcp_server import TCPServer
from .udp_server import UDPServer
from .worker_context import WorkerContext
from ..config import Config, Sockets
from ..typing import AppWrapper, ConnectionState, LifespanState
from ..utils import (
    check_multiprocess_shutdown_event,
    load_application,
    raise_shutdown,
    repr_socket_addr,
    ShutdownError,
)

if sys.version_info < (3, 11):
    from exceptiongroup import BaseExceptionGroup


async def worker_serve(
    app: AppWrapper,
    config: Config,
    *,
    sockets: Optional[Sockets] = None,
    shutdown_trigger: Optional[Callable[..., Awaitable[None]]] = None,
    task_status: trio._core._run._TaskStatus = trio.TASK_STATUS_IGNORED,
) -> None:
    config.set_statsd_logger_class(StatsdLogger)

    lifespan_state: LifespanState = {}
    lifespan = Lifespan(app, config, lifespan_state)
    max_requests = None
    if config.max_requests is not None:
        max_requests = config.max_requests + randint(0, config.max_requests_jitter)
    context = WorkerContext(max_requests)

    async with trio.open_nursery() as lifespan_nursery:
        await lifespan_nursery.start(lifespan.handle_lifespan)
        await lifespan.wait_for_startup()

        async with trio.open_nursery() as server_nursery:
            if sockets is None:
                sockets = config.create_sockets()
                for sock in sockets.secure_sockets:
                    sock.listen(config.backlog)
                for sock in sockets.insecure_sockets:
                    sock.listen(config.backlog)

            ssl_context = config.create_ssl_context()
            listeners = []
            binds = []
            for sock in sockets.secure_sockets:
                listeners.append(
                    trio.SSLListener(
                        trio.SocketListener(trio.socket.from_stdlib_socket(sock)),
                        ssl_context,
                        https_compatible=True,
                    )
                )
                bind = repr_socket_addr(sock.family, sock.getsockname())
                binds.append(f"https://{bind}")
                await config.log.info(f"Running on https://{bind} (CTRL + C to quit)")

            for sock in sockets.insecure_sockets:
                listeners.append(trio.SocketListener(trio.socket.from_stdlib_socket(sock)))
                bind = repr_socket_addr(sock.family, sock.getsockname())
                binds.append(f"http://{bind}")
                await config.log.info(f"Running on http://{bind} (CTRL + C to quit)")

            for sock in sockets.quic_sockets:
                await server_nursery.start(
                    UDPServer(
                        app, config, context, ConnectionState(lifespan_state.copy()), sock
                    ).run
                )
                bind = repr_socket_addr(sock.family, sock.getsockname())
                await config.log.info(f"Running on https://{bind} (QUIC) (CTRL + C to quit)")

            task_status.started(binds)
            try:
                async with trio.open_nursery(strict_exception_groups=True) as nursery:
                    if shutdown_trigger is not None:
                        nursery.start_soon(raise_shutdown, shutdown_trigger)
                    nursery.start_soon(raise_shutdown, context.terminate.wait)

                    nursery.start_soon(
                        partial(
                            trio.serve_listeners,
                            partial(
                                TCPServer,
                                app,
                                config,
                                context,
                                ConnectionState(lifespan_state.copy()),
                            ),
                            listeners,
                            handler_nursery=server_nursery,
                        ),
                    )

                    await trio.sleep_forever()
            except BaseExceptionGroup as error:
                _, other_errors = error.split((ShutdownError, KeyboardInterrupt))
                if other_errors is not None:
                    raise other_errors
            finally:
                await context.terminated.set()
                server_nursery.cancel_scope.deadline = trio.current_time() + config.graceful_timeout

        await lifespan.wait_for_shutdown()
        lifespan_nursery.cancel_scope.cancel()


def trio_worker(
    config: Config, sockets: Optional[Sockets] = None, shutdown_event: Optional[EventType] = None
) -> None:
    if sockets is not None:
        for sock in sockets.secure_sockets:
            sock.listen(config.backlog)
        for sock in sockets.insecure_sockets:
            sock.listen(config.backlog)
    app = load_application(config.application_path, config.wsgi_max_body_size)

    shutdown_trigger = None
    if shutdown_event is not None:
        shutdown_trigger = partial(check_multiprocess_shutdown_event, shutdown_event, trio.sleep)

    trio.run(partial(worker_serve, app, config, sockets=sockets, shutdown_trigger=shutdown_trigger))
