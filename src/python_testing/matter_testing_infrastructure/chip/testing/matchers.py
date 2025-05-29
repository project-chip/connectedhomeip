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

"""Type matching utilities for Matter testing infrastructure.

This module provides functionality for validating type compatibility between
received values and expected type specifications.
"""

import typing

from chip.tlv import float32, uint


def is_type(received_value, desired_type):
    """ Checks if a received value matches an expected type.

    Handles unpacking Nullable and Optional types and
    compares list value types for non-empty lists.

    Args:
        received_value: The value to type check
        desired_type: The expected type specification (can be a basic type, Union,
            Optional, or List type)

    Returns:
        bool: True if the received_value matches the desired_type specification

    Examples:
        >>> is_type(42, int)
        True
        >>> from typing import Optional
        >>> is_type(None, Optional[str])
        True
        >>> is_type([1,2,3], list[int])
        True
    """
    if typing.get_origin(desired_type) == typing.Union:
        return any(is_type(received_value, t) for t in typing.get_args(desired_type))
    elif typing.get_origin(desired_type) == list:
        if isinstance(received_value, list):
            # Assume an empty list is of the correct type
            return True if received_value == [] else any(is_type(received_value[0], t) for t in typing.get_args(desired_type))
        else:
            return False
    elif desired_type == uint:
        return isinstance(received_value, int) and received_value >= 0
    elif desired_type == float32:
        return isinstance(received_value, float)
    else:
        return isinstance(received_value, desired_type)


if __name__ == "__main__":
    import doctest
    doctest.testmod()
