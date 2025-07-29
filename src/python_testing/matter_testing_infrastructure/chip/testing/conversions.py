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

"""Conversion utilities for Matter testing infrastructure.

This module provides utility functions for converting between different data
representations commonly used in Matter testing.
"""

from binascii import hexlify, unhexlify

import chip.clusters as Clusters


def bytes_from_hex(hex: str) -> bytes:
    """ Converts hex string to bytes, handling various formats (colons, spaces, newlines).

    Examples:
        >>> bytes_from_hex("01:ab:cd")
        b'\\x01\\xab\\xcd'
        >>> bytes_from_hex("01 ab cd")
        b'\\x01\\xab\\xcd'
        >>> bytes_from_hex("01abcd")
        b'\\x01\\xab\\xcd'
        >>> bytes_from_hex("01\\nab\\ncd")
        b'\\x01\\xab\\xcd'
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

    Examples:
        >>> hex_from_bytes(b'\\x01\\xab\\xcd')
        '01abcd'
        >>> hex_from_bytes(bytes([1, 171, 205]))  # Same bytes, different notation
        '01abcd'
    """
    return hexlify(b).decode("utf-8")


def format_decimal_and_hex(number):
    """ Formats a number showing both decimal and hexadecimal representations.

    Creates a string representation of a number showing both its decimal value
    and its hex representation in parentheses.

    Args:
        number: int, the number to format

    Returns:
        str: A formatted string like "123 (0x7b)"

    Examples:
        >>> format_decimal_and_hex(123)
        '123 (0x7b)'
        >>> format_decimal_and_hex(0)
        '0 (0x00)'
        >>> format_decimal_and_hex(255)
        '255 (0xff)'
    """
    return f'{number} (0x{number:02x})'


def cluster_id_with_name(id):
    """ Formats a Matter cluster ID with its name and numeric representation.

    Uses format_decimal_and_hex() for numeric formatting and looks up cluster name from registry.
    Falls back to "Unknown cluster" if ID not recognized.

    Args:
        id: int, the Matter cluster identifier

    Returns:
        str: A formatted string containing the ID and cluster name

    Examples:
        >>> cluster_id_with_name(6)  # OnOff cluster
        '6 (0x06) OnOff'
        >>> cluster_id_with_name(999999)  # Unknown cluster
        '999999 (0xf423f) Unknown cluster'
        >>> cluster_id_with_name("invalid")  # Invalid input
        'HERE IS THE PROBLEM'
    """
    if id in Clusters.ClusterObjects.ALL_CLUSTERS.keys():
        s = Clusters.ClusterObjects.ALL_CLUSTERS[id].__name__
    else:
        s = "Unknown cluster"
    try:
        return f'{format_decimal_and_hex(id)} {s}'
    except (TypeError, ValueError):
        return 'HERE IS THE PROBLEM'


if __name__ == "__main__":
    import doctest
    doctest.testmod()
