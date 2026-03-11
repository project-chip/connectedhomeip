"""BSC Address."""

# standard
import re

# local
from validators.utils import validator


@validator
def bsc_address(value: str, /):
    """Return whether or not given value is a valid binance smart chain address.

    Full validation is implemented for BSC addresses.

    Examples:
        >>> bsc_address('0x4e5acf9684652BEa56F2f01b7101a225Ee33d23f')
        True
        >>> bsc_address('0x4g5acf9684652BEa56F2f01b7101a225Eh33d23z')
        ValidationError(func=bsc_address, args={'value': '0x4g5acf9684652BEa56F2f01b7101a225Eh33d23z'})

    Args:
        value:
            BSC address string to validate.

    Returns:
        (Literal[True]): If `value` is a valid bsc address.
        (ValidationError): If `value` is an invalid bsc address.
    """  # noqa: E501
    if not value:
        return False

    if not re.fullmatch(r"0x[a-fA-F0-9]{40}", value):
        return False

    return True
