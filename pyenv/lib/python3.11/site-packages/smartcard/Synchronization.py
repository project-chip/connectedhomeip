"""
from Thinking in Python, Bruce Eckel
https://python-3-patterns-idioms-test.readthedocs.io/en/latest/Observer.html

(c) Copyright 2008, Creative Commons Attribution-Share Alike 3.0.

Simple emulation of Java's 'synchronized'
keyword, from Peter Norvig.
"""

from __future__ import annotations

import functools
import sys
import threading
from collections.abc import Iterable
from typing import Any, Callable, Protocol, TypeVar

# pylint: disable=too-few-public-methods

if sys.version_info >= (3, 10):
    from typing import ParamSpec
else:
    from typing_extensions import ParamSpec


T = TypeVar("T")
P = ParamSpec("P")


def synchronized(method: Callable[P, T]) -> Callable[P, T]:
    """Synchronize methods with the same mutex"""

    @functools.wraps(method)
    def f(self: _SynchronizationProtocol, *args: Any, **kwargs: Any) -> Any:
        with self.mutex:
            return method(self, *args, **kwargs)

    return f


def synchronize(klass: type, names: str | Iterable[str] | None = None) -> None:
    """Synchronize methods in the given class.
    Only synchronize the methods whose names are
    given, or all methods if names=None."""

    if isinstance(names, str):
        names = names.split()
    for name, val in list(klass.__dict__.items()):
        if callable(val) and name != "__init__" and (names is None or name in names):
            setattr(klass, name, synchronized(val))


class _SynchronizationProtocol(Protocol):
    mutex: threading.Lock | threading.RLock


class Synchronization(_SynchronizationProtocol):
    """You can create your own self.mutex, or inherit from this class"""

    def __init__(self):
        self.mutex = threading.RLock()
