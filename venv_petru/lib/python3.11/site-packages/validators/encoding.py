"""Encoding."""

# standard
import re

# local
from .utils import validator


@validator
def base16(value: str, /):
    """Return whether or not given value is a valid base16 encoding.

    Examples:
        >>> base16('a3f4b2')
        True
        >>> base16('a3f4Z1')
        ValidationError(func=base16, args={'value': 'a3f4Z1'})

    Args:
        value:
            base16 string to validate.

    Returns:
        (Literal[True]): If `value` is a valid base16 encoding.
        (ValidationError): If `value` is an invalid base16 encoding.
    """
    return re.match(r"^[0-9A-Fa-f]+$", value) if value else False


@validator
def base32(value: str, /):
    """Return whether or not given value is a valid base32 encoding.

    Examples:
        >>> base32('MFZWIZLTOQ======')
        True
        >>> base32('MfZW3zLT9Q======')
        ValidationError(func=base32, args={'value': 'MfZW3zLT9Q======'})

    Args:
        value:
            base32 string to validate.

    Returns:
        (Literal[True]): If `value` is a valid base32 encoding.
        (ValidationError): If `value` is an invalid base32 encoding.
    """
    return re.match(r"^[A-Z2-7]+=*$", value) if value else False


@validator
def base58(value: str, /):
    """Return whether or not given value is a valid base58 encoding.

    Examples:
        >>> base58('14pq6y9H2DLGahPsM4s7ugsNSD2uxpHsJx')
        True
        >>> base58('cUSECm5YzcXJwP')
        True

    Args:
        value:
            base58 string to validate.

    Returns:
        (Literal[True]): If `value` is a valid base58 encoding.
        (ValidationError): If `value` is an invalid base58 encoding.
    """
    return re.match(r"^[1-9A-HJ-NP-Za-km-z]+$", value) if value else False


@validator
def base64(value: str, /):
    """Return whether or not given value is a valid base64 encoding.

    Examples:
        >>> base64('Y2hhcmFjdGVyIHNldA==')
        True
        >>> base64('cUSECm5YzcXJwP')
        ValidationError(func=base64, args={'value': 'cUSECm5YzcXJwP'})

    Args:
        value:
            base64 string to validate.

    Returns:
        (Literal[True]): If `value` is a valid base64 encoding.
        (ValidationError): If `value` is an invalid base64 encoding.
    """
    return (
        re.match(r"^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$", value)
        if value
        else False
    )
