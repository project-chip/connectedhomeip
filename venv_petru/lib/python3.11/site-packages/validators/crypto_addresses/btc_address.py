"""BTC Address."""

# standard
from hashlib import sha256
import re

# local
from validators.utils import validator


def _decode_base58(addr: str):
    """Decode base58."""
    alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
    return sum((58**enm) * alphabet.index(idx) for enm, idx in enumerate(addr[::-1]))


def _validate_old_btc_address(addr: str):
    """Validate P2PKH and P2SH type address."""
    if len(addr) not in range(25, 35):
        return False
    decoded_bytes = _decode_base58(addr).to_bytes(25, "big")
    header, checksum = decoded_bytes[:-4], decoded_bytes[-4:]
    return checksum == sha256(sha256(header).digest()).digest()[:4]


@validator
def btc_address(value: str, /):
    """Return whether or not given value is a valid bitcoin address.

    Full validation is implemented for P2PKH and P2SH addresses.
    For segwit addresses a regexp is used to provide a reasonable
    estimate on whether the address is valid.

    Examples:
        >>> btc_address('3Cwgr2g7vsi1bXDUkpEnVoRLA9w4FZfC69')
        True
        >>> btc_address('1BvBMsEYstWetqTFn5Au4m4GFg7xJaNVN2')
        ValidationError(func=btc_address, args={'value': '1BvBMsEYstWetqTFn5Au4m4GFg7xJaNVN2'})

    Args:
        value:
            Bitcoin address string to validate.

    Returns:
        (Literal[True]): If `value` is a valid bitcoin address.
        (ValidationError): If `value` is an invalid bitcoin address.
    """
    if not value:
        return False

    return (
        # segwit pattern
        re.compile(r"^(bc|tc)[0-3][02-9ac-hj-np-z]{14,74}$").match(value)
        if value[:2] in ("bc", "tb")
        else _validate_old_btc_address(value)
    )
