__all__: list[str] = []

try:
    from . import linux
except ModuleNotFoundError:
    pass
else:
    __all__ += ["linux"]
