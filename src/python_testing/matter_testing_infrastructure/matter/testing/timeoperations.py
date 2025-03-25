#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Time conversion and validation utilities for Matter testing infrastructure.

This module provides functionality for handling time-related operations in Matter
testing, particularly focusing on conversions between different time formats and
epochs.

Examples:
    To run the doctests, execute the module as a script:
    $ python3 timeoperations.py
"""

from datetime import datetime, timedelta, timezone
from typing import Optional

from mobly import asserts

# TODO(#31177): Need to add unit tests for all time conversion methods.


def utc_time_in_matter_epoch(desired_datetime: Optional[datetime] = None):
    """ Converts a datetime to microseconds since Matter epoch.

    The Matter epoch is defined as January 1, 2000, 00:00:00 UTC. This function
    calculates the number of microseconds beчtween the input datetime and the
    Matter epoch. If no datetime is provided, the current UTC time is used.

    Args:
        desired_datetime: Optional datetime to convert, uses current UTC if None.
        Should be timezone-aware.

    Returns:
        int: Microseconds since Matter epoch

    Examples:
        >>> from datetime import datetime, timezone
        >>> utc_time_in_matter_epoch(datetime(2000, 1, 1, 0, 0, 0, 0, tzinfo=timezone.utc))
        0
        >>> utc_time_in_matter_epoch(datetime(2000, 1, 1, 0, 0, 0, 1, tzinfo=timezone.utc))
        1
    """
    if desired_datetime is None:
        utc_native = datetime.now(tz=timezone.utc)
    else:
        utc_native = desired_datetime
    # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
    utc_th_delta = utc_native - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)
    utc_th_us = int(utc_th_delta.total_seconds() * 1000000)
    return utc_th_us


def utc_datetime_from_matter_epoch_us(matter_epoch_us: int) -> datetime:
    """ Converts microseconds since Matter epoch to UTC datetime.

    Inverse of utc_time_in_matter_epoch().
    It converts a microsecond timestamp relative to the Matter epoch
    (Jan 1, 2000, 00:00:00 UTC) into a timezone-aware Python datetime object.

    Args:
        matter_epoch_us: int, microseconds since Matter epoch

    Returns:
        datetime: UTC datetime

    Examples:
        >>> utc_datetime_from_matter_epoch_us(0)
        datetime.datetime(2000, 1, 1, 0, 0, tzinfo=datetime.timezone.utc)
        >>> utc_datetime_from_matter_epoch_us(123456789)
        datetime.datetime(2000, 1, 1, 0, 2, 3, 456789, tzinfo=datetime.timezone.utc)
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

    Examples:
        >>> utc_datetime_from_posix_time_ms(0)
        datetime.datetime(1970, 1, 1, 0, 0, tzinfo=datetime.timezone.utc)
        >>> utc_datetime_from_posix_time_ms(1609459200000) # 2021-01-01 00:00:00 UTC
        datetime.datetime(2021, 1, 1, 0, 0, tzinfo=datetime.timezone.utc)
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

    Examples:
        >>> compare_time(5000000, utc=0, tolerance=timedelta(seconds=5)) # Passes (exactly 5s)
        >>> from mobly import signals
        >>> try:
        ...     compare_time(6000000, utc=0, tolerance=timedelta(seconds=5))
        ... except signals.TestFailure as e:
        ...     print("AssertionError: Received time is out of tolerance")
        AssertionError: Received time is out of tolerance
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
        >>> import unittest.mock
        >>> start_time = utc_time_in_matter_epoch(datetime(2000, 1, 1, 0, 0, 0, tzinfo=timezone.utc)) # 0
        >>> with unittest.mock.patch(__name__ + '.utc_time_in_matter_epoch') as mock_time:
        ...     mock_time.return_value = 2000000  # Simulate current time 2 seconds after start
        ...     get_wait_seconds_from_set_time(start_time, 5)  # 5 - (2000000 / 1e6) = 5 - 2 = 3
        3
    """
    seconds_passed = (utc_time_in_matter_epoch() - set_time_matter_us) // 1000000
    return wait_seconds - seconds_passed


if __name__ == "__main__":
    import doctest
    doctest.testmod()
