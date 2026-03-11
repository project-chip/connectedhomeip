"""
Verify service identities.
"""

from . import cryptography, hazmat, pyopenssl
from .exceptions import (
    CertificateError,
    SubjectAltNameWarning,
    VerificationError,
)


__title__ = "service-identity"

__author__ = "Hynek Schlawack"

__license__ = "MIT"
__copyright__ = "Copyright (c) 2014 " + __author__


__all__ = [
    "CertificateError",
    "SubjectAltNameWarning",
    "VerificationError",
    "hazmat",
    "cryptography",
    "pyopenssl",
]


def __getattr__(name: str) -> str:
    if name != "__version__":
        msg = f"module {__name__} has no attribute {name}"
        raise AttributeError(msg)

    from importlib.metadata import version

    return version("service-identity")
