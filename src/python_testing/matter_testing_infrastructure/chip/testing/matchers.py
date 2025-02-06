import typing

from chip.tlv import float32, uint


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
