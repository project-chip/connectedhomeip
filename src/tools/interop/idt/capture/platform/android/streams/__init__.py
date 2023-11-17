from capture.loader import CaptureImplsLoader

from .base import AndroidStream

impl_loader = CaptureImplsLoader(
    __path__[0],
    "capture.platform.android.streams",
    AndroidStream
)

for impl_name, impl in impl_loader.impls.items():
    globals()[impl_name] = impl

__all__ = impl_loader.impl_names
