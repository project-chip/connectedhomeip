from __future__ import annotations

import warnings
from typing import Awaitable, Callable, Literal, Optional

import trio

from .run import worker_serve
from ..config import Config
from ..typing import Framework
from ..utils import wrap_app


async def serve(
    app: Framework,
    config: Config,
    *,
    shutdown_trigger: Optional[Callable[..., Awaitable[None]]] = None,
    task_status: trio._core._run._TaskStatus = trio.TASK_STATUS_IGNORED,
    mode: Optional[Literal["asgi", "wsgi"]] = None,
) -> None:
    """Serve an ASGI framework app given the config.

    This allows for a programmatic way to serve an ASGI framework, it
    can be used via,

    .. code-block:: python

        trio.run(serve, app, config)

    It is assumed that the event-loop is configured before calling
    this function, therefore configuration values that relate to loop
    setup or process setup are ignored.

    Arguments:
        app: The ASGI application to serve.
        config: A Hypercorn configuration object.
        shutdown_trigger: This should return to trigger a graceful
            shutdown.
        mode: Specify if the app is WSGI or ASGI.
    """
    if config.debug:
        warnings.warn("The config `debug` has no affect when using serve", Warning)
    if config.workers != 1:
        warnings.warn("The config `workers` has no affect when using serve", Warning)

    await worker_serve(
        wrap_app(app, config.wsgi_max_body_size, mode),
        config,
        shutdown_trigger=shutdown_trigger,
        task_status=task_status,
    )
