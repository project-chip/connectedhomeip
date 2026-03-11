"""Finance."""

from .utils import validator


def _cusip_checksum(cusip: str):
    check, val = 0, None

    for idx in range(9):
        c = cusip[idx]
        if c >= "0" and c <= "9":
            val = ord(c) - ord("0")
        elif c >= "A" and c <= "Z":
            val = 10 + ord(c) - ord("A")
        elif c >= "a" and c <= "z":
            val = 10 + ord(c) - ord("a")
        elif c == "*":
            val = 36
        elif c == "@":
            val = 37
        elif c == "#":
            val = 38
        else:
            return False

        if idx & 1:
            val += val

        check = check + (val // 10) + (val % 10)

    return (check % 10) == 0


def _isin_checksum(value: str):
    check, val = 0, None

    for idx in range(12):
        c = value[idx]
        if c >= "0" and c <= "9" and idx > 1:
            val = ord(c) - ord("0")
        elif c >= "A" and c <= "Z":
            val = 10 + ord(c) - ord("A")
        elif c >= "a" and c <= "z":
            val = 10 + ord(c) - ord("a")
        else:
            return False

        if idx & 1:
            val += val

    return (check % 10) == 0


@validator
def cusip(value: str):
    """Return whether or not given value is a valid CUSIP.

    Checks if the value is a valid [CUSIP][1].
    [1]: https://en.wikipedia.org/wiki/CUSIP

    Examples:
        >>> cusip('037833DP2')
        True
        >>> cusip('037833DP3')
        ValidationError(func=cusip, args={'value': '037833DP3'})

    Args:
        value: CUSIP string to validate.

    Returns:
        (Literal[True]): If `value` is a valid CUSIP string.
        (ValidationError): If `value` is an invalid CUSIP string.
    """
    return len(value) == 9 and _cusip_checksum(value)


@validator
def isin(value: str):
    """Return whether or not given value is a valid ISIN.

    Checks if the value is a valid [ISIN][1].
    [1]: https://en.wikipedia.org/wiki/International_Securities_Identification_Number

    Examples:
        >>> isin('037833DP2')
        ValidationError(func=isin, args={'value': '037833DP2'})
        >>> isin('037833DP3')
        ValidationError(func=isin, args={'value': '037833DP3'})

    Args:
        value: ISIN string to validate.

    Returns:
        (Literal[True]): If `value` is a valid ISIN string.
        (ValidationError): If `value` is an invalid ISIN string.
    """
    return len(value) == 12 and _isin_checksum(value)


@validator
def sedol(value: str):
    """Return whether or not given value is a valid SEDOL.

    Checks if the value is a valid [SEDOL][1].
    [1]: https://en.wikipedia.org/wiki/SEDOL

    Examples:
        >>> sedol('2936921')
        True
        >>> sedol('29A6922')
        ValidationError(func=sedol, args={'value': '29A6922'})

    Args:
        value: SEDOL string to validate.

    Returns:
        (Literal[True]): If `value` is a valid SEDOL string.
        (ValidationError): If `value` is an invalid SEDOL string.
    """
    if len(value) != 7:
        return False

    weights = [1, 3, 1, 7, 3, 9, 1]
    check = 0
    for idx in range(7):
        c = value[idx]
        if c in "AEIOU":
            return False

        val = None
        if c >= "0" and c <= "9":
            val = ord(c) - ord("0")
        elif c >= "A" and c <= "Z":
            val = 10 + ord(c) - ord("A")
        else:
            return False
        check += val * weights[idx]

    return (check % 10) == 0
