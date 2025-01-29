import typing
from binascii import hexlify, unhexlify
from datetime import datetime, timedelta, timezone
from typing import Optional

import chip.clusters as Clusters
from chip.tlv import float32, uint
from mobly import asserts


def type_matches(received_value, desired_type):
    """ Checks if the value received matches the expected type.

        Handles unpacking Nullable and Optional types and
        compares list value types for non-empty lists.
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
    """ Returns the time in matter epoch in us.

        If desired_datetime is None, it will return the current time.
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
    """Returns the given Matter epoch time as a usable Python datetime in UTC."""
    delta_from_epoch = timedelta(microseconds=matter_epoch_us)
    matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)

    return matter_epoch + delta_from_epoch


def utc_datetime_from_posix_time_ms(posix_time_ms: int) -> datetime:
    millis = posix_time_ms % 1000
    seconds = posix_time_ms // 1000
    return datetime.fromtimestamp(seconds, timezone.utc) + timedelta(milliseconds=millis)


def compare_time(received: int, offset: timedelta = timedelta(), utc: Optional[int] = None, tolerance: timedelta = timedelta(seconds=5)) -> None:
    if utc is None:
        utc = utc_time_in_matter_epoch()

    # total seconds includes fractional for microseconds
    expected = utc + offset.total_seconds() * 1000000
    delta_us = abs(expected - received)
    delta = timedelta(microseconds=delta_us)
    asserts.assert_less_equal(delta, tolerance, "Received time is out of tolerance")


def get_wait_seconds_from_set_time(set_time_matter_us: int, wait_seconds: int):
    seconds_passed = (utc_time_in_matter_epoch() - set_time_matter_us) // 1000000
    return wait_seconds - seconds_passed


def bytes_from_hex(hex: str) -> bytes:
    """Converts any `hex` string representation including `01:ab:cd` to bytes

    Handles any whitespace including newlines, which are all stripped.
    """
    return unhexlify("".join(hex.replace(":", "").replace(" ", "").split()))


def hex_from_bytes(b: bytes) -> str:
    """Converts a bytes object `b` into a hex string (reverse of bytes_from_hex)"""
    return hexlify(b).decode("utf-8")


def id_str(id):
    return f'{id} (0x{id:02x})'


def cluster_id_str(id):
    if id in Clusters.ClusterObjects.ALL_CLUSTERS.keys():
        s = Clusters.ClusterObjects.ALL_CLUSTERS[id].__name__
    else:
        s = "Unknown cluster"
    try:
        return f'{id_str(id)} {s}'
    except TypeError:
        return 'HERE IS THE PROBLEM'
