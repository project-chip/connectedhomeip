from __future__ import annotations

import platform
import signal
import time
from multiprocessing import get_context
from multiprocessing.connection import wait
from multiprocessing.context import BaseContext
from multiprocessing.process import BaseProcess
from multiprocessing.synchronize import Event as EventType
from pickle import PicklingError
from typing import Any, List

from .config import Config, Sockets
from .typing import WorkerFunc
from .utils import check_for_updates, files_to_watch, load_application, write_pid_file


def run(config: Config) -> int:
    if config.pid_path is not None:
        write_pid_file(config.pid_path)

    worker_func: WorkerFunc
    if config.worker_class == "asyncio":
        from .asyncio.run import asyncio_worker

        worker_func = asyncio_worker
    elif config.worker_class == "uvloop":
        from .asyncio.run import uvloop_worker

        worker_func = uvloop_worker
    elif config.worker_class == "trio":
        from .trio.run import trio_worker

        worker_func = trio_worker
    else:
        raise ValueError(f"No worker of class {config.worker_class} exists")

    sockets = config.create_sockets()

    if config.use_reloader and config.workers == 0:
        raise RuntimeError("Cannot reload without workers")

    exitcode = 0
    if config.workers == 0:
        worker_func(config, sockets)
    else:
        if config.use_reloader:
            # Load the application so that the correct paths are checked for
            # changes, but only when the reloader is being used.
            load_application(config.application_path, config.wsgi_max_body_size)

        ctx = get_context("spawn")

        active = True
        shutdown_event = ctx.Event()

        def shutdown(*args: Any) -> None:
            nonlocal active, shutdown_event
            shutdown_event.set()
            active = False

        processes: List[BaseProcess] = []
        while active:
            # Ignore SIGINT before creating the processes, so that they
            # inherit the signal handling. This means that the shutdown
            # function controls the shutdown.
            signal.signal(signal.SIGINT, signal.SIG_IGN)

            _populate(processes, config, worker_func, sockets, shutdown_event, ctx)

            for signal_name in {"SIGINT", "SIGTERM", "SIGBREAK"}:
                if hasattr(signal, signal_name):
                    signal.signal(getattr(signal, signal_name), shutdown)

            if config.use_reloader:
                files = files_to_watch()
                while True:
                    finished = wait((process.sentinel for process in processes), timeout=1)
                    updated = check_for_updates(files)
                    if updated:
                        shutdown_event.set()
                        for process in processes:
                            process.join()
                        shutdown_event.clear()
                        break
                    if len(finished) > 0:
                        break
            else:
                wait(process.sentinel for process in processes)

            exitcode = _join_exited(processes)
            if exitcode != 0:
                shutdown_event.set()
                active = False

        for process in processes:
            process.terminate()

        exitcode = _join_exited(processes) if exitcode != 0 else exitcode

        for sock in sockets.secure_sockets:
            sock.close()

        for sock in sockets.insecure_sockets:
            sock.close()

    return exitcode


def _populate(
    processes: List[BaseProcess],
    config: Config,
    worker_func: WorkerFunc,
    sockets: Sockets,
    shutdown_event: EventType,
    ctx: BaseContext,
) -> None:
    for _ in range(config.workers - len(processes)):
        process = ctx.Process(  # type: ignore
            target=worker_func,
            kwargs={"config": config, "shutdown_event": shutdown_event, "sockets": sockets},
        )
        process.daemon = True
        try:
            process.start()
        except PicklingError as error:
            raise RuntimeError(
                "Cannot pickle the config, see https://docs.python.org/3/library/pickle.html#pickle-picklable"  # noqa: E501
            ) from error
        processes.append(process)
        if platform.system() == "Windows":
            time.sleep(0.1)


def _join_exited(processes: List[BaseProcess]) -> int:
    exitcode = 0
    for index in reversed(range(len(processes))):
        worker = processes[index]
        if worker.exitcode is not None:
            worker.join()
            exitcode = worker.exitcode if exitcode == 0 else exitcode
            del processes[index]

    return exitcode
