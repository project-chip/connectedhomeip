import contextlib

__all__: list[str] = []

with contextlib.suppress(ImportError)
from . import linux
__all__ += ["linux"]
