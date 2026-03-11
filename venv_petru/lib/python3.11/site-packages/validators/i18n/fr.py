"""France."""

# standard
from functools import lru_cache
import re
import typing

# local
from validators.utils import validator


@lru_cache
def _ssn_pattern():
    """SSN Pattern."""
    return re.compile(
        r"^([1,2])"  # gender (1=M, 2=F)
        r"\s(\d{2})"  # year of birth
        r"\s(0[1-9]|1[0-2])"  # month of birth
        r"\s(\d{2,3}|2[A,B])"  # department of birth
        r"\s(\d{2,3})"  # town of birth
        r"\s(\d{3})"  # registration number
        r"(?:\s(\d{2}))?$",  # control key (may or may not be provided)
        re.VERBOSE,
    )


@validator
def fr_department(value: typing.Union[str, int]):
    """Validate a french department number.

    Examples:
        >>> fr_department(20)  # can be an integer
        ValidationError(func=fr_department, args={'value': 20})
        >>> fr_department("20")
        ValidationError(func=fr_department, args={'value': '20'})
        >>> fr_department("971")  # Guadeloupe
        True
        >>> fr_department("00")
        ValidationError(func=fr_department, args={'value': '00'})
        >>> fr_department('2A')  # Corsica
        True
        >>> fr_department('2B')
        True
        >>> fr_department('2C')
        ValidationError(func=fr_department, args={'value': '2C'})

    Args:
        value:
            French department number to validate.

    Returns:
        (Literal[True]): If `value` is a valid french department number.
        (ValidationError): If `value` is an invalid french department number.
    """
    if not value:
        return False
    if isinstance(value, str):
        if value in ("2A", "2B"):  # Corsica
            return True
        try:
            value = int(value)
        except ValueError:
            return False
    return 1 <= value <= 19 or 21 <= value <= 95 or 971 <= value <= 976  # Overseas departments


@validator
def fr_ssn(value: str):
    """Validate a french Social Security Number.

    Each french citizen has a distinct Social Security Number.
    For more information see [French Social Security Number][1] (sadly unavailable in english).

    [1]: https://fr.wikipedia.org/wiki/Num%C3%A9ro_de_s%C3%A9curit%C3%A9_sociale_en_France

    Examples:
        >>> fr_ssn('1 84 12 76 451 089 46')
        True
        >>> fr_ssn('1 84 12 76 451 089')  # control key is optional
        True
        >>> fr_ssn('3 84 12 76 451 089 46')  # wrong gender number
        ValidationError(func=fr_ssn, args={'value': '3 84 12 76 451 089 46'})
        >>> fr_ssn('1 84 12 76 451 089 47')  # wrong control key
        ValidationError(func=fr_ssn, args={'value': '1 84 12 76 451 089 47'})

    Args:
        value:
            French Social Security Number string to validate.

    Returns:
        (Literal[True]): If `value` is a valid french Social Security Number.
        (ValidationError): If `value` is an invalid french Social Security Number.
    """
    if not value:
        return False
    matched = re.match(_ssn_pattern(), value)
    if not matched:
        return False
    groups = list(matched.groups())
    control_key = groups[-1]
    department = groups[3]
    if department != "99" and not fr_department(department):
        # 99 stands for foreign born people
        return False
    if control_key is None:
        # no control key provided, no additional check needed
        return True
    if len(department) == len(groups[4]):
        # if the department number is 3 digits long (overseas departments),
        # the town number must be 2 digits long
        # and vice versa
        return False
    if department in ("2A", "2B"):
        # Corsica's department numbers are not in the same range as the others
        # thus 2A and 2B are replaced by 19 and 18 respectively to compute the control key
        groups[3] = "19" if department == "2A" else "18"
    # the control key is valid if it is equal to 97 - (the first 13 digits modulo 97)
    digits = int("".join(groups[:-1]))
    return int(control_key) == (97 - (digits % 97))
