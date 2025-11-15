try:
    from typing import Protocol, Any  # type: ignore
except ImportError:
    from typing_extensions import Protocol, Any  # type: ignore


# fmt: off
class Data(Protocol):
    def keys(self) -> Any: ...
    def __getitem__(self, *args, **kwargs) -> Any: ...
    def __contains__(self, *args, **kwargs) -> bool: ...
# fmt: on
