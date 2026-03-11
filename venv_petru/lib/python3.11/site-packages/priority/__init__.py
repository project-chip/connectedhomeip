# -*- coding: utf-8 -*-
"""
priority: HTTP/2 priority implementation for Python
"""
from .priority import (  # noqa
    Stream,
    PriorityTree,
    DeadlockError,
    PriorityLoop,
    PriorityError,
    DuplicateStreamError,
    MissingStreamError,
    TooManyStreamsError,
    BadWeightError,
    PseudoStreamError,
)


__version__ = "2.0.0"
