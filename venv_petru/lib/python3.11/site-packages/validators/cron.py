"""Cron."""

# local
from .utils import validator


def _validate_cron_component(component: str, min_val: int, max_val: int):
    if component == "*":
        return True

    if component.isdecimal():
        return min_val <= int(component) <= max_val

    if "/" in component:
        parts = component.split("/")
        if len(parts) != 2 or not parts[1].isdecimal() or int(parts[1]) < 1:
            return False
        if parts[0] == "*":
            return True
        return parts[0].isdecimal() and min_val <= int(parts[0]) <= max_val

    if "-" in component:
        parts = component.split("-")
        if len(parts) != 2 or not parts[0].isdecimal() or not parts[1].isdecimal():
            return False
        start, end = int(parts[0]), int(parts[1])
        return min_val <= start <= max_val and min_val <= end <= max_val and start <= end

    if "," in component:
        for item in component.split(","):
            if not _validate_cron_component(item, min_val, max_val):
                return False
        return True
        # return all(
        #   _validate_cron_component(item, min_val, max_val) for item in component.split(",")
        # ) # throws type error. why?

    return False


@validator
def cron(value: str, /):
    """Return whether or not given value is a valid cron string.

    Examples:
        >>> cron('*/5 * * * *')
        True
        >>> cron('30-20 * * * *')
        ValidationError(func=cron, args={'value': '30-20 * * * *'})

    Args:
        value:
            Cron string to validate.

    Returns:
        (Literal[True]): If `value` is a valid cron string.
        (ValidationError): If `value` is an invalid cron string.
    """
    if not value:
        return False

    try:
        minutes, hours, days, months, weekdays = value.strip().split()
    except ValueError as err:
        raise ValueError("Badly formatted cron string") from err

    if not _validate_cron_component(minutes, 0, 59):
        return False
    if not _validate_cron_component(hours, 0, 23):
        return False
    if not _validate_cron_component(days, 1, 31):
        return False
    if not _validate_cron_component(months, 1, 12):
        return False
    if not _validate_cron_component(weekdays, 0, 6):
        return False

    return True
