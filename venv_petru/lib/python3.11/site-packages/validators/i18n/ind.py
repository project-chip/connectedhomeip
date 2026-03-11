"""India."""

# standard
import re

# local
from validators.utils import validator


@validator
def ind_aadhar(value: str):
    """Validate an indian aadhar card number.

    Examples:
        >>> ind_aadhar('3675 9834 6015')
        True
        >>> ind_aadhar('3675 ABVC 2133')
        ValidationError(func=ind_aadhar, args={'value': '3675 ABVC 2133'})

    Args:
        value: Aadhar card number string to validate.

    Returns:
        (Literal[True]): If `value` is a valid aadhar card number.
        (ValidationError): If `value` is an invalid aadhar card number.
    """
    return re.match(r"^[2-9]{1}\d{3}\s\d{4}\s\d{4}$", value)


@validator
def ind_pan(value: str):
    """Validate a pan card number.

    Examples:
        >>> ind_pan('ABCDE9999K')
        True
        >>> ind_pan('ABC5d7896B')
        ValidationError(func=ind_pan, args={'value': 'ABC5d7896B'})

    Args:
        value: PAN card number string to validate.

    Returns:
        (Literal[True]): If `value` is a valid PAN card number.
        (ValidationError): If `value` is an invalid PAN card number.
    """
    return re.match(r"[A-Z]{5}\d{4}[A-Z]{1}", value)
