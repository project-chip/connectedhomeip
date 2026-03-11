"""TRX Address."""

# standard
import hashlib
import re

# local
from validators.utils import validator


def _base58_decode(addr: str) -> bytes:
    """Decode a base58 encoded address."""
    alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
    num = 0
    for char in addr:
        num = num * 58 + alphabet.index(char)
    return num.to_bytes(25, byteorder="big")


def _validate_trx_checksum_address(addr: str) -> bool:
    """Validate TRX type checksum address."""
    if len(addr) != 34:
        return False

    try:
        address = _base58_decode(addr)
    except ValueError:
        return False

    if len(address) != 25 or address[0] != 0x41:
        return False

    check_sum = hashlib.sha256(hashlib.sha256(address[:-4]).digest()).digest()[:4]
    return address[-4:] == check_sum


@validator
def trx_address(value: str, /):
    """Return whether or not given value is a valid tron address.

    Full validation is implemented for TRC20 tron addresses.

    Examples:
        >>> trx_address('TLjfbTbpZYDQ4EoA4N5CLNgGjfbF8ZWz38')
        True
        >>> trx_address('TR2G7Rm4vFqF8EpY4U5xdLdQ7XgJ2U8Vd')
        ValidationError(func=trx_address, args={'value': 'TR2G7Rm4vFqF8EpY4U5xdLdQ7XgJ2U8Vd'})

    Args:
        value:
            Tron address string to validate.

    Returns:
        (Literal[True]): If `value` is a valid tron address.
        (ValidationError): If `value` is an invalid tron address.
    """
    if not value:
        return False

    return re.compile(r"^[T][a-km-zA-HJ-NP-Z1-9]{33}$").match(
        value
    ) and _validate_trx_checksum_address(value)
