#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Declarative PIXIT (Protocol Implementation eXtra Information for Testing) support.

This module provides a mechanism for Matter tests to formally declare their
required and optional PIXIT values via decorators. PIXITs are test parameters
that provide extra information needed to run the test, such as device paths,
timeouts, or configuration flags.

Usage example:

    from matter.testing.pixit import pixit

    class TC_Example(MatterBaseTest):

        @pixit("th_server_app_path", str, "Path to TH server application")
        @pixit("timeout_sec", int, "Timeout in seconds", required=False, default=30)
        @async_test_body
        async def test_TC_Example_1_1(self):
            app_path = self.pixit("th_server_app_path")
            timeout = self.pixit("timeout_sec")
            ...

    For standard commissioning flags (discriminator, Wi-Fi, etc.) declared on
    the test harness, see :mod:`matter.testing.harness_params` and
    ``MatterBaseTest.harness_param``.
"""

from dataclasses import dataclass
from typing import Any

# Sentinel object to distinguish "no default provided" from None
_PIXIT_NO_DEFAULT = object()


@dataclass
class PixitDefinition:
    """Declares a PIXIT parameter for a test method.

    Attributes:
        name: The key used to pass the PIXIT value via command-line args
              (e.g., --string-arg name:value, --int-arg name:value).
        type: Expected Python type of the value (str, int, bool, float). Enforced at
              setup via ``isinstance``; use the matching ``--*-arg`` flag when passing
              values on the command line.
        description: Human-readable description shown in error messages.
        required: If True, the test fails early when the PIXIT is not provided.
                  If False, the PIXIT is optional and uses the default value.
        default: Default value for optional PIXITs. Ignored when required=True.
                 Uses _PIXIT_NO_DEFAULT sentinel to distinguish "no default" from None.
        sensitive: If True, the value is redacted in failure-parameter dumps.
    """

    name: str
    type: type
    description: str
    required: bool = True
    default: Any = _PIXIT_NO_DEFAULT
    sensitive: bool = False


def pixit(
    name: str,
    value_type: type,
    description: str,
    required: bool = True,
    default: Any = _PIXIT_NO_DEFAULT,
    sensitive: bool = False,
):
    """Decorator that declares a PIXIT parameter requirement for a test method.

    This decorator attaches PIXIT metadata to the test method. When the test runs,
    MatterBaseTest.setup_test() automatically validates that all required PIXITs
    are present in the test's user_params.

    Multiple @pixit decorators can be stacked on a single test method
    to declare multiple PIXIT parameters.

    The decorator should be placed ABOVE @async_test_body and
    @run_if_endpoint_matches decorators so that PIXIT validation runs first.

    Args:
        name: The parameter name (key in user_params).
        value_type: Expected Python type (str, int, bool, float). Enforced at setup;
                    pass values with the matching ``--*-arg`` flag (e.g. ``--int-arg``).
        description: Human-readable description for error messages.
        required: Whether this PIXIT must be provided. Defaults to True.
        default: Default value when not provided. Only used when required=False.
                 If required=True, this argument is ignored.
        sensitive: If True, redact the value in failure-parameter dumps.

    Example:
        @pixit("app_path", str, "Path to the server application")
        @pixit("retry_count", int, "Number of retries", required=False, default=3)
        @async_test_body
        async def test_TC_Example_1_1(self):
            path = self.pixit("app_path")      # guaranteed to exist
            retries = self.pixit("retry_count") # returns 3 if not provided
    """
    pixit_def = PixitDefinition(
        name=name,
        type=value_type,
        description=description,
        required=required,
        default=default,
        sensitive=sensitive,
    )

    def decorator(func):
        # Initialize the list if this is the first @pixit on this function
        if not hasattr(func, "_pixit_definitions"):
            func._pixit_definitions = []
        func._pixit_definitions.insert(0, pixit_def)
        return func

    return decorator


def get_pixit_definitions(test_method) -> list[PixitDefinition]:
    """Extract PIXIT definitions from a decorated test method.

    Args:
        test_method: A test method potentially decorated with @pixit.

    Returns:
        List of PixitDefinition objects attached to the method, or empty list
        if the method has no PIXIT declarations or test_method is None.
    """
    if test_method is None:
        return []
    return getattr(test_method, "_pixit_definitions", [])


def validate_pixits(
    pixit_definitions: list[PixitDefinition], user_params: dict
) -> tuple[list[PixitDefinition], list[PixitDefinition]]:
    """Validate that all required PIXIT values are present in user_params.

    Args:
        pixit_definitions: List of PixitDefinition objects to validate.
        user_params: Dictionary of user-provided test parameters.

    Returns:
        A tuple of (missing_required, available_optional) where:
        - missing_required: List of required PIXITs not found in user_params.
        - available_optional: List of optional PIXITs (for informational display).
    """
    missing_required = []
    available_optional = []
    seen_names = set()

    for pixit_def in pixit_definitions:
        if pixit_def.name in seen_names:
            continue
        seen_names.add(pixit_def.name)
        if pixit_def.required:
            if pixit_def.name not in user_params or user_params[pixit_def.name] is None:
                missing_required.append(pixit_def)
        else:
            available_optional.append(pixit_def)

    return missing_required, available_optional


def _pixit_type_mismatch_message(name: str, value: Any, expected_type: type) -> str:
    flag = _type_to_arg_flag(expected_type)
    return (
        f"PIXIT {name!r} expected {expected_type.__name__}, "
        f"got {type(value).__name__} ({value!r}). "
        f"Provide via: --{flag} {name}:<value>"
    )


def validate_pixit_types(pixit_definitions: list[PixitDefinition], user_params: dict) -> list[str]:
    """Return error messages for PIXIT values that do not match declared types."""
    errors: list[str] = []
    seen_names: set[str] = set()

    for pixit_def in pixit_definitions:
        if pixit_def.name in seen_names:
            continue
        seen_names.add(pixit_def.name)
        if pixit_def.name not in user_params or user_params[pixit_def.name] is None:
            continue
        value = user_params[pixit_def.name]
        if not isinstance(value, pixit_def.type):
            errors.append(_pixit_type_mismatch_message(pixit_def.name, value, pixit_def.type))

    return errors


def format_pixit_type_errors(test_name: str, errors: list[str]) -> str:
    """Format PIXIT type validation errors for setup."""
    lines = [
        f"Test '{test_name}' has PIXIT parameter type error(s):",
        "",
    ]
    for err in errors:
        lines.append(f"  - {err}")
    return "\n".join(lines)


def format_pixit_value_for_dump(pdef: PixitDefinition, user_params: dict) -> str:
    """Format a PIXIT value for failure dumps (redacts when ``sensitive``)."""
    value = user_params[pdef.name]
    if pdef.sensitive:
        return "***REDACTED***"
    if isinstance(value, bytes):
        return f"<{len(value)} bytes>"
    return repr(value)


def format_pixit_error(test_name: str, missing_required: list[PixitDefinition], available_optional: list[PixitDefinition]) -> str:
    """Format a clear error message for missing PIXIT values.

    Args:
        test_name: Name of the test method.
        missing_required: List of required PIXITs that are missing.
        available_optional: List of optional PIXITs available.

    Returns:
        A formatted error string.
    """
    lines = [
        f"Test '{test_name}' is missing required PIXIT value(s):",
        "",
        "Missing required PIXITs:",
    ]

    for pixit_def in missing_required:
        type_name = pixit_def.type.__name__
        lines.append(f"  - {pixit_def.name} ({type_name}): {pixit_def.description}")
        lines.append(f"    Provide via: --{_type_to_arg_flag(pixit_def.type)} {pixit_def.name}:<value>")

    if available_optional:
        lines.append("")
        lines.append("Available optional PIXITs:")
        for pixit_def in available_optional:
            type_name = pixit_def.type.__name__
            lines.append(f"  - {pixit_def.name} ({type_name}): {pixit_def.description}")
            if pixit_def.default is not _PIXIT_NO_DEFAULT:
                lines.append(f"    Default: {pixit_def.default!r}")
            lines.append(f"    Override via: --{_type_to_arg_flag(pixit_def.type)} {pixit_def.name}:<value>")

    return "\n".join(lines)


def _type_to_arg_flag(pixit_type: type) -> str:
    """Map a Python type to the corresponding command-line argument flag.

    Args:
        pixit_type: Python type (str, int, bool, float).

    Returns:
        The corresponding flag name (e.g., 'int-arg', 'string-arg').
    """
    type_map = {
        int: "int-arg",
        str: "string-arg",
        bool: "bool-arg",
        float: "float-arg",
        bytes: "hex-arg",
        list: "json-arg",
        dict: "json-arg",
    }
    return type_map.get(pixit_type, "string-arg")
