"""Finland."""

# standard
from functools import lru_cache
import re

# local
from validators.utils import validator


@lru_cache
def _business_id_pattern():
    """Business ID Pattern."""
    return re.compile(r"^[0-9]{7}-[0-9]$")


@lru_cache
def _ssn_pattern(ssn_check_marks: str):
    """SSN Pattern."""
    return re.compile(
        r"""^
        (?P<date>(0[1-9]|[1-2]\d|3[01])
        (0[1-9]|1[012])
        (\d{{2}}))
        [ABCDEFYXWVU+-]
        (?P<serial>(\d{{3}}))
        (?P<checksum>[{check_marks}])$""".format(check_marks=ssn_check_marks),
        re.VERBOSE,
    )


@validator
def fi_business_id(value: str, /):
    """Validate a Finnish Business ID.

    Each company in Finland has a distinct business id. For more
    information see [Finnish Trade Register][1]

    [1]: http://en.wikipedia.org/wiki/Finnish_Trade_Register

    Examples:
        >>> fi_business_id('0112038-9')  # Fast Monkeys Ltd
        True
        >>> fi_business_id('1234567-8')  # Bogus ID
        ValidationError(func=fi_business_id, args={'value': '1234567-8'})

    Args:
        value:
            Business ID string to be validated.

    Returns:
        (Literal[True]): If `value` is a valid finnish business id.
        (ValidationError): If `value` is an invalid finnish business id.
    """
    if not value:
        return False
    if not re.match(_business_id_pattern(), value):
        return False
    factors = [7, 9, 10, 5, 8, 4, 2]
    numbers = map(int, value[:7])
    checksum = int(value[8])
    modulo = sum(f * n for f, n in zip(factors, numbers)) % 11
    return (11 - modulo == checksum) or (modulo == checksum == 0)


@validator
def fi_ssn(value: str, /, *, allow_temporal_ssn: bool = True):
    """Validate a Finnish Social Security Number.

    This validator is based on [django-localflavor-fi][1].

    [1]: https://github.com/django/django-localflavor-fi/

    Examples:
        >>> fi_ssn('010101-0101')
        True
        >>> fi_ssn('101010-0102')
        ValidationError(func=fi_ssn, args={'value': '101010-0102'})

    Args:
        value:
            Social Security Number to be validated.
        allow_temporal_ssn:
            Whether to accept temporal SSN numbers. Temporal SSN numbers are the
            ones where the serial is in the range [900-999]. By default temporal
            SSN numbers are valid.

    Returns:
        (Literal[True]): If `value` is a valid finnish SSN.
        (ValidationError): If `value` is an invalid finnish SSN.
    """
    if not value:
        return False
    ssn_check_marks = "0123456789ABCDEFHJKLMNPRSTUVWXY"
    if not (result := re.match(_ssn_pattern(ssn_check_marks), value)):
        return False
    gd = result.groupdict()
    checksum = int(gd["date"] + gd["serial"])
    return (
        int(gd["serial"]) >= 2
        and (allow_temporal_ssn or int(gd["serial"]) <= 899)
        and ssn_check_marks[checksum % len(ssn_check_marks)] == gd["checksum"]
    )
