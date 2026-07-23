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

"""Declarative harness (runner) parameters for Matter Python tests.

Standard commissioning and test-harness flags are populated on
``MatterTestConfig`` by ``runner.py`` (e.g. ``--discriminator``, ``--wifi-ssid``).
This module lets tests declare which of those values they rely on, validates
them in ``setup_test``, and formats them in failure output alongside PIXITs.

PIXITs remain in :mod:`matter.testing.pixit` (``user_params`` / ``--*-arg``).
This module imports ``pixit`` only for combined validation and failure messages;
``pixit`` must not import this module.

Usage:

    from matter.testing.harness_params import harness_params
    from matter.testing.pixit import pixit

    class TC_Example(MatterBaseTest):

        @harness_params("discriminator", "passcode", optional=("endpoint",))
        @pixit("th_server_app_path", str, "Path to TH server application")
        @async_test_body
        async def test_TC_Example_1_1(self):
            disc = self.harness_param("discriminator")
            path = self.pixit("th_server_app_path")
            ...
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Any

from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.pixit import (_PIXIT_NO_DEFAULT, PixitDefinition, _type_to_arg_flag, format_pixit_error,
                                  format_pixit_type_errors, format_pixit_value_for_dump, get_pixit_definitions,
                                  validate_pixit_types, validate_pixits)


@dataclass(frozen=True)
class HarnessParamDefinition:
    """One harness parameter attached to a test method."""

    name: str
    required: bool


@dataclass(frozen=True)
class _HarnessParamSpec:
    """Registry entry: how to validate and display a logical harness name."""

    description: str
    cli_hint: str
    sensitive: bool
    is_satisfied: Callable[[MatterTestConfig], bool]
    resolve_runtime: Callable[[MatterTestConfig], Any]
    resolve_display: Callable[[MatterTestConfig], Any]


def _commissioning_credentials_satisfied(cfg: MatterTestConfig) -> bool:
    return bool(cfg.discriminators) or bool(cfg.qr_code_content) or bool(cfg.manual_code)


def _passcode_satisfied(cfg: MatterTestConfig) -> bool:
    return bool(cfg.setup_passcodes) or bool(cfg.qr_code_content) or bool(cfg.manual_code)


def _resolve_discriminator_runtime(cfg: MatterTestConfig) -> Any:
    if cfg.discriminators:
        return cfg.discriminators[0]
    return None


def _resolve_discriminator_display(cfg: MatterTestConfig) -> Any:
    if cfg.discriminators:
        return cfg.discriminators[0]
    if cfg.qr_code_content:
        return "<from --qr-code>"
    if cfg.manual_code:
        return "<from --manual-code>"
    return None


def _resolve_passcode_runtime(cfg: MatterTestConfig) -> Any:
    if cfg.setup_passcodes:
        return cfg.setup_passcodes[0]
    return None


def _resolve_passcode_display(cfg: MatterTestConfig) -> Any:
    if cfg.setup_passcodes:
        return cfg.setup_passcodes[0]
    if cfg.qr_code_content or cfg.manual_code:
        return "<from setup code>"
    return None


HARNESS_PARAM_REGISTRY: dict[str, _HarnessParamSpec] = {
    "discriminator": _HarnessParamSpec(
        description="Long discriminator for commissioning (or use --qr-code / --manual-code).",
        cli_hint="--discriminator <value>  or  --qr-code <payload>  or  --manual-code <code>",
        sensitive=False,
        is_satisfied=_commissioning_credentials_satisfied,
        resolve_runtime=_resolve_discriminator_runtime,
        resolve_display=_resolve_discriminator_display,
    ),
    "passcode": _HarnessParamSpec(
        description="PAKE passcode for commissioning (or use --qr-code / --manual-code).",
        cli_hint="--passcode <value>  or  --qr-code / --manual-code",
        sensitive=True,
        is_satisfied=_passcode_satisfied,
        resolve_runtime=_resolve_passcode_runtime,
        resolve_display=_resolve_passcode_display,
    ),
    "wifi_ssid": _HarnessParamSpec(
        description="Wi-Fi network SSID (e.g. for ble-wifi commissioning).",
        cli_hint="--wifi-ssid <SSID>",
        sensitive=False,
        is_satisfied=lambda cfg: cfg.wifi_ssid is not None,
        resolve_runtime=lambda cfg: cfg.wifi_ssid,
        resolve_display=lambda cfg: cfg.wifi_ssid,
    ),
    "wifi_passphrase": _HarnessParamSpec(
        description="Wi-Fi network passphrase.",
        cli_hint="--wifi-passphrase <passphrase>",
        sensitive=True,
        is_satisfied=lambda cfg: cfg.wifi_passphrase is not None,
        resolve_runtime=lambda cfg: cfg.wifi_passphrase,
        resolve_display=lambda cfg: cfg.wifi_passphrase,
    ),
    "commissioning_method": _HarnessParamSpec(
        description="Commissioning method name (see runner --commissioning-method choices).",
        cli_hint="--commissioning-method <method>",
        sensitive=False,
        is_satisfied=lambda cfg: cfg.commissioning_method is not None and str(cfg.commissioning_method).strip() != "",
        resolve_runtime=lambda cfg: cfg.commissioning_method,
        resolve_display=lambda cfg: cfg.commissioning_method,
    ),
    "dut_node_id": _HarnessParamSpec(
        description="DUT node id on the fabric.",
        cli_hint="--dut-node-id <id>",
        sensitive=False,
        is_satisfied=lambda cfg: bool(cfg.dut_node_ids),
        resolve_runtime=lambda cfg: cfg.dut_node_ids[0] if cfg.dut_node_ids else None,
        resolve_display=lambda cfg: cfg.dut_node_ids[0] if cfg.dut_node_ids else None,
    ),
    "endpoint": _HarnessParamSpec(
        description="Endpoint under test (may be implicit; declare if the test requires an explicit value).",
        cli_hint="--endpoint <n>",
        sensitive=False,
        is_satisfied=lambda cfg: cfg.endpoint is not None,
        resolve_runtime=lambda cfg: cfg.endpoint,
        resolve_display=lambda cfg: cfg.endpoint,
    ),
    "thread_dataset": _HarnessParamSpec(
        description="Thread operational dataset hex (ble-thread / similar).",
        cli_hint="--thread-dataset-hex <hex>",
        sensitive=True,
        is_satisfied=lambda cfg: cfg.thread_operational_dataset is not None,
        resolve_runtime=lambda cfg: cfg.thread_operational_dataset,
        resolve_display=lambda cfg: cfg.thread_operational_dataset,
    ),
    "commissionee_ip": _HarnessParamSpec(
        description="Commissionee IP (on-network-ip commissioning).",
        cli_hint="--ip-addr <address>",
        sensitive=False,
        is_satisfied=lambda cfg: cfg.commissionee_ip_address_just_for_testing is not None,
        resolve_runtime=lambda cfg: cfg.commissionee_ip_address_just_for_testing,
        resolve_display=lambda cfg: cfg.commissionee_ip_address_just_for_testing,
    ),
}


def harness_params(*required: str, optional: tuple[str, ...] = ()) -> Callable:
    """Declare harness (runner) parameters required or optional for a test method.

    Names must exist in ``HARNESS_PARAM_REGISTRY``. Descriptions and CLI hints
    come from the registry (not repeated on each test).

    Args:
        *required: Logical harness parameter names that must be satisfied.
        optional: Names that are declared for documentation / failure dumps but
            do not fail validation if absent.

    Raises:
        ValueError: If any name is not in the registry.
    """

    defs = [HarnessParamDefinition(n, True) for n in required]
    defs.extend(HarnessParamDefinition(n, False) for n in optional)

    for d in defs:
        if d.name not in HARNESS_PARAM_REGISTRY:
            raise ValueError(
                f"Unknown harness parameter {d.name!r}. Valid names: {sorted(HARNESS_PARAM_REGISTRY)!r}"
            )

    def decorator(func: Callable) -> Callable:
        if not hasattr(func, "_harness_param_definitions"):
            func._harness_param_definitions = []
        # Outermost decorator first (same convention as @pixit).
        func._harness_param_definitions = defs + func._harness_param_definitions
        return func

    return decorator


def get_harness_param_definitions(test_method: Any) -> list[HarnessParamDefinition]:
    if test_method is None:
        return []
    return getattr(test_method, "_harness_param_definitions", [])


def validate_harness_params(
    definitions: list[HarnessParamDefinition], config: MatterTestConfig
) -> tuple[list[HarnessParamDefinition], list[HarnessParamDefinition]]:
    """Return (missing_required, optional_declared) for harness declarations."""
    missing_required: list[HarnessParamDefinition] = []
    optional_declared: list[HarnessParamDefinition] = []
    seen: set[str] = set()

    for hdef in definitions:
        if hdef.name in seen:
            continue
        seen.add(hdef.name)
        spec = HARNESS_PARAM_REGISTRY[hdef.name]
        if hdef.required:
            if not spec.is_satisfied(config):
                missing_required.append(hdef)
        else:
            optional_declared.append(hdef)

    return missing_required, optional_declared


def format_harness_error(
    test_name: str,
    missing_required: list[HarnessParamDefinition],
    optional_declared: list[HarnessParamDefinition],
) -> str:
    lines = [
        f"Test '{test_name}' is missing required harness parameter(s):",
        "",
        "Missing required harness (pass via command line):",
    ]
    for hdef in missing_required:
        spec = HARNESS_PARAM_REGISTRY[hdef.name]
        lines.append(f"  - {hdef.name}: {spec.description}")
        lines.append(f"    Provide via: {spec.cli_hint}")

    if optional_declared:
        lines.append("")
        lines.append("Declared optional harness parameters (not required to be set):")
        for hdef in optional_declared:
            spec = HARNESS_PARAM_REGISTRY[hdef.name]
            lines.append(f"  - {hdef.name}: {spec.description}")
            lines.append(f"    Set via: {spec.cli_hint}")

    return "\n".join(lines)


def format_missing_test_parameters(
    test_name: str,
    test_method: Any,
    user_params: dict,
    matter_test_config: MatterTestConfig,
) -> str | None:
    """Return a combined error message if PIXITs or harness params are missing.

    Returns None if nothing is missing.
    """
    pixit_defs = get_pixit_definitions(test_method)
    harness_defs = get_harness_param_definitions(test_method)

    pixit_missing: list[PixitDefinition] = []
    pixit_optional: list[PixitDefinition] = []
    if pixit_defs:
        pixit_missing, pixit_optional = validate_pixits(pixit_defs, user_params)

    harness_missing: list[HarnessParamDefinition] = []
    harness_optional: list[HarnessParamDefinition] = []
    if harness_defs:
        harness_missing, harness_optional = validate_harness_params(harness_defs, matter_test_config)

    if not pixit_missing and not harness_missing:
        if pixit_defs:
            type_errors = validate_pixit_types(pixit_defs, user_params)
            if type_errors:
                return format_pixit_type_errors(test_name, type_errors)
        return None

    parts: list[str] = []
    if pixit_missing:
        parts.append(format_pixit_error(test_name, pixit_missing, pixit_optional))
    if harness_missing:
        parts.append(format_harness_error(test_name, harness_missing, harness_optional))

    return "\n\n".join(parts)


def resolve_harness_value(name: str, config: MatterTestConfig) -> Any:
    """Resolve a logical harness name to a runtime value from ``MatterTestConfig``.

    Returns the direct config value when available. For commissioning credentials
    satisfied only via ``--qr-code`` / ``--manual-code``, returns ``None`` (the
    decoded value is not stored on ``MatterTestConfig``). Use failure-dump display
    or explicit ``--discriminator`` / ``--passcode`` when a numeric value is needed.
    """
    if name not in HARNESS_PARAM_REGISTRY:
        raise ValueError(
            f"Unknown harness parameter {name!r}. Valid names: {sorted(HARNESS_PARAM_REGISTRY)!r}"
        )
    return HARNESS_PARAM_REGISTRY[name].resolve_runtime(config)


def _format_value_for_dump(name: str, raw: Any, satisfied: bool) -> str:
    spec = HARNESS_PARAM_REGISTRY[name]
    if not satisfied:
        return "(missing)"
    if raw is None:
        return "(missing)"
    if spec.sensitive:
        return "***REDACTED***"
    if isinstance(raw, bytes):
        return f"<{len(raw)} bytes>"
    return repr(raw)


def format_declared_parameters_for_failure(
    test_method: Any,
    user_params: dict,
    matter_test_config: MatterTestConfig,
) -> str:
    """Human-readable block listing all declared PIXIT and harness parameters."""
    pixit_defs = get_pixit_definitions(test_method)
    harness_defs = get_harness_param_definitions(test_method)
    if not pixit_defs and not harness_defs:
        return ""

    lines = ["Declared test parameters at failure:", ""]

    seen_pixit: set[str] = set()
    for pdef in pixit_defs:
        if pdef.name in seen_pixit:
            continue
        seen_pixit.add(pdef.name)
        type_name = pdef.type.__name__
        if pdef.name in user_params and user_params[pdef.name] is not None:
            val_repr = format_pixit_value_for_dump(pdef, user_params)
        elif not pdef.required and pdef.default is not _PIXIT_NO_DEFAULT:
            val_repr = f"{repr(pdef.default)} (default)"
        else:
            val_repr = "(missing)"
        lines.append(f"  [pixit] {pdef.name} ({type_name}): {pdef.description}")
        lines.append(f"          value: {val_repr}")
        lines.append(f"          Override via: --{_type_to_arg_flag(pdef.type)} {pdef.name}:<value>")
        lines.append("")

    seen_h: set[str] = set()
    for hdef in harness_defs:
        if hdef.name in seen_h:
            continue
        seen_h.add(hdef.name)
        spec = HARNESS_PARAM_REGISTRY[hdef.name]
        satisfied = spec.is_satisfied(matter_test_config)
        raw = spec.resolve_display(matter_test_config)
        val = _format_value_for_dump(hdef.name, raw, satisfied)
        req_label = "required" if hdef.required else "optional"
        lines.append(f"  [harness] {hdef.name} ({req_label}): {spec.description}")
        lines.append(f"            value: {val}")
        lines.append(f"            Set via: {spec.cli_hint}")
        lines.append("")

    return "\n".join(lines).rstrip() + "\n"
