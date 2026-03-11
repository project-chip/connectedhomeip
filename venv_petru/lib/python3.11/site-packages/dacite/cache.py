from functools import lru_cache
from typing import TypeVar, Callable, Optional

T = TypeVar("T", bound=Callable)

__MAX_SIZE: Optional[int] = 2048


@lru_cache(maxsize=None)
def cache(function: T) -> T:
    return lru_cache(maxsize=get_cache_size(), typed=True)(function)  # type: ignore


def set_cache_size(size: Optional[int]) -> None:
    global __MAX_SIZE  # pylint: disable=global-statement
    __MAX_SIZE = size


def get_cache_size() -> Optional[int]:
    global __MAX_SIZE  # pylint: disable=global-variable-not-assigned
    return __MAX_SIZE


def clear_cache() -> None:
    cache.cache_clear()
