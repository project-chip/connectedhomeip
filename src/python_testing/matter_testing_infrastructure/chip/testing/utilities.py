import typing
from binascii import hexlify, unhexlify
from datetime import datetime, timedelta, timezone
from typing import Optional

import chip.clusters as Clusters
from chip.tlv import float32, uint
from mobly import asserts


def type_matches(received_value, desired_type):
    """ Checks if a received value matches an expected type.

    Handles unpacking Nullable and Optional types and
    compares list value types for non-empty lists.

    Args:
        received_value: The value to type check
        desired_type: The expected type specification (can be a basic type, Union,
            Optional, or List type)

    Returns:
        bool: True if the received_value matches the desired_type specification
    """
    if typing.get_origin(desired_type) == typing.Union:
        return any(type_matches(received_value, t) for t in typing.get_args(desired_type))
    elif typing.get_origin(desired_type) == list:
        if isinstance(received_value, list):
            # Assume an empty list is of the correct type
            return True if received_value == [] else any(type_matches(received_value[0], t) for t in typing.get_args(desired_type))
        else:
            return False
    elif desired_type == uint:
        return isinstance(received_value, int) and received_value >= 0
    elif desired_type == float32:
        return isinstance(received_value, float)
    else:
        return isinstance(received_value, desired_type)

# TODO(#31177): Need to add unit tests for all time conversion methods.


def utc_time_in_matter_epoch(desired_datetime: Optional[datetime] = None):
    """ Converts a datetime to microseconds since Matter epoch.

    The Matter epoch is defined as January 1, 2000, 00:00:00 UTC. This function
    calculates the number of microseconds between the input datetime and the
    Matter epoch. If no datetime is provided, the current UTC time is used.

    Args:
        desired_datetime: Optional datetime to convert, uses current UTC if None.
        Should be timezone-aware.

    Returns:
        int: Microseconds since Matter epoch
    """
    if desired_datetime is None:
        utc_native = datetime.now(tz=timezone.utc)
    else:
        utc_native = desired_datetime
    # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
    utc_th_delta = utc_native - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)
    utc_th_us = int(utc_th_delta.total_seconds() * 1000000)
    return utc_th_us


matter_epoch_us_from_utc_datetime = utc_time_in_matter_epoch


def utc_datetime_from_matter_epoch_us(matter_epoch_us: int) -> datetime:
    """ Converts microseconds since Matter epoch to UTC datetime.

    Inverse of utc_time_in_matter_epoch().
    It converts a microsecond timestamp relative to the Matter epoch
    (Jan 1, 2000, 00:00:00 UTC) into a timezone-aware Python datetime object.

    Args:
        matter_epoch_us: int, microseconds since Matter epoch

    Returns:
        datetime: UTC datetime
    """
    delta_from_epoch = timedelta(microseconds=matter_epoch_us)
    matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)

    return matter_epoch + delta_from_epoch


def utc_datetime_from_posix_time_ms(posix_time_ms: int) -> datetime:
    """ Converts POSIX timestamp in milliseconds to UTC datetime.

    This function converts a POSIX timestamp (milliseconds since January 1, 1970, 00:00:00 UTC)
    to a timezone-aware Python datetime object.

    Args:
        posix_time_ms: int, Unix timestamp in milliseconds since Jan 1, 1970

    Returns:
        datetime: UTC datetime   
    """
    millis = posix_time_ms % 1000
    seconds = posix_time_ms // 1000
    return datetime.fromtimestamp(seconds, timezone.utc) + timedelta(milliseconds=millis)


def compare_time(received: int, offset: timedelta = timedelta(), utc: Optional[int] = None, tolerance: timedelta = timedelta(seconds=5)) -> None:
    """ Validates a Matter timestamp against expected time within tolerance.

    Args:
        received: int, Matter timestamp in microseconds
        offset: timedelta, Optional offset from reference time (default: 0s)
        utc: Optional[int], Reference time in Matter microseconds (default: current time)
        tolerance: timedelta, Maximum allowed time difference (default: 5s)

    Raises:
        AssertionError: If the received time differs from the expected time by
            more than the specified tolerance
    """
    if utc is None:
        utc = utc_time_in_matter_epoch()

    # total seconds includes fractional for microseconds
    expected = utc + offset.total_seconds() * 1000000
    delta_us = abs(expected - received)
    delta = timedelta(microseconds=delta_us)
    asserts.assert_less_equal(delta, tolerance, "Received time is out of tolerance")


def get_wait_seconds_from_set_time(set_time_matter_us: int, wait_seconds: int):
    """ Calculates remaining wait time based on a previously set timestamp.

    This function determines how many seconds remain from an original wait duration,
    accounting for time that has already elapsed since a Matter timestamp. Useful
    for implementing timeouts or delays that need to account for already elapsed time.

    Args:
        set_time_matter_us: int, Start time in Matter microseconds
        wait_seconds: int, Total seconds to wait from start time

    Returns:
        int: Remaining seconds (negative if period expired)

    Examples:
        >>> start_time = utc_time_in_matter_epoch()
        >>> # After 2 seconds...
        >>> get_wait_seconds_from_set_time(start_time, 5)
        3  # Returns remaining 3 seconds from original 5 second wait
    """
    seconds_passed = (utc_time_in_matter_epoch() - set_time_matter_us) // 1000000
    return wait_seconds - seconds_passed


def bytes_from_hex(hex: str) -> bytes:
    """ Converts hex string to bytes, handling various formats (colons, spaces, newlines).

    Examples:
        "01:ab:cd" -> b'\x01\xab\xcd'
        "01 ab cd" -> b'\x01\xab\xcd'
    """
    return unhexlify("".join(hex.replace(":", "").replace(" ", "").split()))


def hex_from_bytes(b: bytes) -> str:
    """ Converts a bytes object to a hexadecimal string.

    This function performs the inverse operation of bytes_from_hex(). It converts
    a bytes object into a continuous hexadecimal string without any separators.

    Args:
        b: bytes, the bytes object to convert to hexadecimal

    Returns:
        str: A string containing the hexadecimal representation of the bytes,
            using lowercase letters a-f for hex digits

    Example: b'\x01\xab\xcd' -> '01abcd'
    """
    return hexlify(b).decode("utf-8")


def id_str(id):
    """ Formats a numeric ID as both decimal and hex.

    Creates a string representation of an ID showing both its decimal value
    and its hex representation in parentheses.

    Args:
        id: int, the numeric identifier to format

    Returns:
        str: A formatted string like "123 (0x7b)"
    """
    return f'{id} (0x{id:02x})'


def cluster_id_str(id):
    """ Formats a Matter cluster ID with its name and numeric representation.

    Uses id_str() for numeric formatting and looks up cluster name from registry.
    Falls back to "Unknown cluster" if ID not recognized.

    Args:
        id: int, the Matter cluster identifier

    Returns:
        str: A formatted string containing the ID and cluster name, like
            "6 (0x06) OnOff", or "Unknown cluster" if ID not recognized
    """
    if id in Clusters.ClusterObjects.ALL_CLUSTERS.keys():
        s = Clusters.ClusterObjects.ALL_CLUSTERS[id].__name__
    else:
        s = "Unknown cluster"
    try:
        return f'{id_str(id)} {s}'
    except TypeError:
        return 'HERE IS THE PROBLEM'
