# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Keep multiprocessing.Manager proxies usable after a KeyboardInterrupt.

`BaseProxy._callmethod()` sends a request and then blocks in `conn.recv()` with no exception handling. A KeyboardInterrupt raised
while blocked in `recv()` leaves the reply unread, so the connection stays permanently one message out of phase. The connection is
thread-local but keyed by manager address, so a single interrupted call desynchronizes *every* proxy of that manager in that thread,
and each later call returns the previous call's reply.

The patch installed here consumes that in-flight reply before letting the KeyboardInterrupt propagate. It relies on every blocking
wait going through `wait_for_mp_managed()`: because those waits are bounded by its polling interval, the reply is guaranteed to
arrive shortly. An unbounded wait would not reply until notified and would defeat the recovery.

This mutates a stdlib class process-wide. It is a workaround for a standard library shortcoming rather than a design choice, and it
should be removed together with `wait_for_mp_managed()` if the manager is ever replaced by a transport we control.

TODO: To remove this workaround we would need to completely rearchitect the concurrency package to avoid using
      multiprocessing.Manager proxies for any blocking wait. That would require a custom transport for the manager, and a way to
      propagate exceptions from subprocesses to the main process.

"""

import contextlib
import logging
from multiprocessing.connection import Connection
from multiprocessing.managers import BaseProxy
from typing import Any, cast

log = logging.getLogger(__name__)

RESYNC_TIMEOUT_SEC = 2.0
"""How long to wait for the reply of an interrupted call.

Must stay above `wait_for_mp_managed()`'s polling interval, which is what bounds how long the manager can take to answer.
"""

_INSTALLED_MARKER = "_chiptest_resyncing"

_NO_KWDS: dict[Any, Any] = {}
"""Shared empty mapping, so that the stdlib _callmethod() signature can be mirrored without a mutable default."""

_original_callmethod = BaseProxy._callmethod


def _resynchronize(proxy: BaseProxy) -> None:
    """Consume the reply of an interrupted call, so that the connection does not stay out of phase.

    Falls back to dropping the connection if the reply never arrives, so that the next call reconnects instead of reading a stale
    reply. That fallback is the worse outcome of the two: the manager server handles each connection on a dedicated thread and
    `Condition` uses a thread-owned RLock, so abandoning a connection in the middle of a wait can leave that lock held forever.
    """
    tls = getattr(proxy, "_tls", None)
    conn = cast(Connection | None, getattr(tls, "connection", None))
    if conn is None:
        return

    try:
        if conn.poll(RESYNC_TIMEOUT_SEC):
            conn.recv()
            return
    except BaseException:
        # A second interrupt, or a connection that is already broken. Fall through and drop it.
        log.warning("Failed to drain the reply of an interrupted call", exc_info=True)

    with contextlib.suppress(BaseException):
        conn.close()
    with contextlib.suppress(AttributeError):
        delattr(tls, "connection")


def _resyncing_callmethod(self: BaseProxy, methodname: str, args: tuple[Any, ...] = (),
                          kwds: dict[Any, Any] = _NO_KWDS) -> Any:
    """Call the proxied method, resynchronizing the connection if the call is interrupted."""
    try:
        return _original_callmethod(self, methodname, args, kwds)
    except KeyboardInterrupt:
        # Only a KeyboardInterrupt can leave a reply unread. An exception raised by the proxied method itself arrives as a complete
        # '#ERROR' reply that _callmethod() has already consumed, so resynchronizing on it would eat the *next* call's reply. That
        # is not hypothetical: CancellableQueue.get() relies on a remote queue.Empty on every empty-queue check.
        _resynchronize(self)
        raise


def install_proxy_resync() -> None:
    """Patch BaseProxy._callmethod to resynchronize after an interrupted call. Idempotent."""
    if getattr(BaseProxy._callmethod, _INSTALLED_MARKER, False):
        return

    setattr(_resyncing_callmethod, _INSTALLED_MARKER, True)
    setattr(BaseProxy, "_callmethod", _resyncing_callmethod)
