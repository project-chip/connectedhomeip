"""Between."""

# standard
from datetime import datetime
from typing import TypeVar, Union

# local
from ._extremes import AbsMax, AbsMin
from .utils import validator

PossibleValueTypes = TypeVar("PossibleValueTypes", int, float, str, datetime, None)


@validator
def between(
    value: PossibleValueTypes,
    /,
    *,
    min_val: Union[PossibleValueTypes, AbsMin, None] = None,
    max_val: Union[PossibleValueTypes, AbsMax, None] = None,
):
    """Validate that a number is between minimum and/or maximum value.

    This will work with any comparable type, such as floats, decimals and dates
    not just integers. This validator is originally based on [WTForms-NumberRange-Validator][1].

    [1]: https://github.com/wtforms/wtforms/blob/master/src/wtforms/validators.py#L166-L220

    Examples:
        >>> from datetime import datetime
        >>> between(5, min_val=2)
        True
        >>> between(13.2, min_val=13, max_val=14)
        True
        >>> between(500, max_val=400)
        ValidationError(func=between, args={'value': 500, 'max_val': 400})
        >>> between(
        ...     datetime(2000, 11, 11),
        ...     min_val=datetime(1999, 11, 11)
        ... )
        True

    Args:
        value:
            Value which is to be compared.
        min_val:
            The minimum required value of the number.
            If not provided, minimum value will not be checked.
        max_val:
            The maximum value of the number.
            If not provided, maximum value will not be checked.

    Returns:
        (Literal[True]): If `value` is in between the given conditions.
        (ValidationError): If `value` is not in between the given conditions.

    Raises:
        (ValueError): If `min_val` is greater than `max_val`.
        (TypeError): If there's a type mismatch during comparison.

    Note:
        - `PossibleValueTypes` = `TypeVar("PossibleValueTypes", int, float, str, datetime)`
        - If neither `min_val` nor `max_val` is provided, result will always be `True`.
    """
    if value is None:
        return False

    if max_val is None:
        max_val = AbsMax()
    if min_val is None:
        min_val = AbsMin()

    try:
        if min_val > max_val:
            raise ValueError("`min_val` cannot be greater than `max_val`")
    except TypeError as err:
        raise TypeError("Comparison type mismatch") from err

    return min_val <= value <= max_val
