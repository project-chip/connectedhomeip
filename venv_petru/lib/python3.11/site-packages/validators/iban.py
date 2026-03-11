"""IBAN."""

# standard
import re

# local
from .utils import validator


def _char_value(char: str):
    """A=10, B=11, ..., Z=35."""
    return char if char.isdigit() else str(10 + ord(char) - ord("A"))


def _mod_check(value: str):
    """Check if the value string passes the mod97-test."""
    # move country code and check numbers to end
    rearranged = value[4:] + value[:4]
    return int("".join(_char_value(char) for char in rearranged)) % 97 == 1


@validator
def iban(value: str, /):
    """Return whether or not given value is a valid IBAN code.

    Examples:
        >>> iban('DE29100500001061045672')
        True
        >>> iban('123456')
        ValidationError(func=iban, args={'value': '123456'})

    Args:
        value:
            IBAN string to validate.

    Returns:
        (Literal[True]): If `value` is a valid IBAN code.
        (ValidationError): If `value` is an invalid IBAN code.
    """
    return (
        (re.match(r"^[a-z]{2}[0-9]{2}[a-z0-9]{11,30}$", value, re.IGNORECASE) and _mod_check(value))
        if value
        else False
    )
