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

"""Decide how a test runner commissions the DUT for a test, the way the Test Harness does.

The Test Harness (TH) decides per test function whether the DUT is commissioned before
the test starts:

| Test function                                      | TH behaviour                                |
|----------------------------------------------------|---------------------------------------------|
| In TH_OPERATOR_CHOICE_TESTS, or only the step      | The operator chooses                        |
| "Run entire test" (LEGACY)                         |                                             |
| Class has a device-requirement marker              | MatterTestCommissionedDevice: commissioned; |
|                                                    | the other markers: not commissioned         |
| No marker                                          | Step 1's is_commissioning flag decides      |

The TH commissions in a separate commission-only process and starts each test with
--in-test-commissioning-method, never --commissioning-method. plan_run() turns a run's
script arguments into that shape. The script's test class is read in a separate process
(read_script_info()), so the runner never imports test modules itself.
"""

import enum
import importlib.util
import json
import pathlib
import subprocess
import sys
from dataclasses import asdict, dataclass, field
from types import ModuleType

# Names of the device-requirement markers in matter.testing.matter_testing, kept as strings so the
# runner can use this module without importing the Matter controller.
COMMISSIONED_DEVICE = "MatterTestCommissionedDevice"
UNCOMMISSIONED_DEVICE = "MatterTestUncommissionedDevice"
COMMISSIONER = "MatterTestCommissioner"
NO_DEVICE = "CertificationUnitTestNoDevice"
_MARKER_NEEDS_COMMISSIONING = {
    COMMISSIONED_DEVICE: True,
    UNCOMMISSIONED_DEVICE: False,
    COMMISSIONER: False,
    NO_DEVICE: False,
}

# Test cases in the TH's "Mandatories" suite, where the operator chooses whether to commission
# (certification-tool-backend: python_test_parser.py, mandatory_python_tcs_public_id).
TH_OPERATOR_CHOICE_TESTS = frozenset({"TC_IDM_10_2", "TC_IDM_10_3", "TC_IDM_10_4", "TC_IDM_10_5", "TC_IDM_12_1"})

_LEGACY_STEP_DESCRIPTION = "Run entire test"
_RESULT_PREFIX = "DEVICE_REQUIREMENT_INFO "
_QUERY_TIMEOUT_S = 120


class CommissioningPlanError(Exception):
    """A run can't be planned: the script's test class or the run's arguments are not usable."""


@dataclass(frozen=True)
class TestFunctionInfo:
    """What the TH reads about one test function of a script."""
    name: str
    step_count: int
    first_step_description: str
    first_step_is_commissioning: bool

    @property
    def test_case(self) -> str:
        """The test case name, e.g. TC_IDM_4_2 for test_TC_IDM_4_2."""
        return self.name.removeprefix("test_")


@dataclass(frozen=True)
class ScriptInfo:
    """The device-requirement marker of a script's test class and its selected test functions."""
    device_requirement: str | None
    functions: list[TestFunctionInfo]


class Commissioning(enum.Enum):
    """Whether the TH commissions the DUT before a test function starts."""
    REQUIRED = "commissioned"
    NOT_REQUIRED = "not commissioned"
    OPERATOR_CHOICE = "operator chooses"


@dataclass(frozen=True)
class FunctionCommissioning:
    test_case: str
    commissioning: Commissioning
    # What decided it: a marker name, "step 1 is_commissioning", "LEGACY" or "MANDATORY".
    reason: str
    # Set when step 1's is_commissioning flag says commissioned but the marker says not. The TH
    # follows the marker and logs a warning.
    contradiction: str | None = None


class RunLabel(enum.Enum):
    """How a run was executed. Only the TH_STYLE labels ran the way the TH runs the test."""
    COMMISSIONED_IN_SEPARATE_STEP = "TH-style: commissioned in a separate step"
    LEFT_UNCOMMISSIONED = "TH-style: DUT left uncommissioned"
    COMMISSIONED_BY_EARLIER_RUN = "TH-style test args: DUT commissioned by an earlier run"
    OPERATOR_CHOICE = "As written: TH asks the operator"
    MIXED = "As written: mixed requirements"
    MODE_OFF = "As written: mode off"

    @property
    def th_style(self) -> bool:
        return self in (RunLabel.COMMISSIONED_IN_SEPARATE_STEP, RunLabel.LEFT_UNCOMMISSIONED,
                        RunLabel.COMMISSIONED_BY_EARLIER_RUN)


@dataclass(frozen=True)
class RunPlan:
    """How to run one run block.

    commission_only_args is None when no commission-only step runs. Otherwise the runner runs
    the script with these arguments (they include --commission-only) before the test.
    ambiguities lists why the run isn't TH-style, or contradicts the TH's inputs; a strict
    runner fails on any of them.
    """
    label: RunLabel
    detail: str
    test_args: list[str]
    commission_only_args: list[str] | None = None
    warnings: list[str] = field(default_factory=list)
    ambiguities: list[str] = field(default_factory=list)


def load_test_class(script_path: str) -> type:
    """Import a test script by path, without running its main guard, and return its test class.

    The script's directory is put first on sys.path if it isn't there, so the script's sibling
    imports resolve. Raises ValueError when the script doesn't define exactly one leaf
    MatterBaseTest subclass.
    """
    from matter.testing.runner import find_test_class

    path = pathlib.Path(script_path).resolve()
    if str(path.parent) not in sys.path:
        sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location(path.stem, path)
    if spec is None or spec.loader is None:
        raise ImportError(f"Cannot create an import spec for {path}")
    module: ModuleType = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return find_test_class(module)


def collect_script_info(test_class: type, tests: list[str]) -> ScriptInfo:
    """Collect the marker and, per selected test function, the step information the TH reads.

    tests are test function names as given to --tests; empty selects every test function.
    Raises ValueError when the class declares conflicting markers.
    """
    from matter.testing.matter_testing import device_requirement
    from matter.testing.runner import convert_args_to_matter_config, get_test_info, matter_test_args_parser

    marker = device_requirement(test_class)
    args = ["--tests", *tests] if tests else []
    config = convert_args_to_matter_config(matter_test_args_parser().parse_args(args))
    functions = []
    for info in get_test_info(test_class, config):
        first_step = info.steps[0] if info.steps else None
        functions.append(TestFunctionInfo(
            name=info.function,
            step_count=len(info.steps),
            first_step_description=first_step.description if first_step else "",
            first_step_is_commissioning=bool(first_step and first_step.is_commissioning),
        ))
    return ScriptInfo(device_requirement=marker.__name__ if marker else None, functions=functions)


def read_script_info(script_path: str, tests: list[str]) -> ScriptInfo:
    """Run this module in a new process to read a script's ScriptInfo.

    Raises CommissioningPlanError with the process's error output when the script can't be
    imported, or its test class has no single leaf class or has conflicting markers.
    """
    command = [sys.executable, "-m", __name__, script_path, *tests]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=_QUERY_TIMEOUT_S, check=False)
    except subprocess.TimeoutExpired as e:
        raise CommissioningPlanError(f"Reading the test class of {script_path} took over {_QUERY_TIMEOUT_S} s") from e
    lines = [line for line in result.stdout.splitlines() if line.startswith(_RESULT_PREFIX)]
    if result.returncode != 0 or not lines:
        error_tail = "\n".join((result.stderr or result.stdout).strip().splitlines()[-15:])
        raise CommissioningPlanError(f"Cannot read the test class of {script_path}:\n{error_tail}")
    data = json.loads(lines[-1].removeprefix(_RESULT_PREFIX))
    return ScriptInfo(device_requirement=data["device_requirement"],
                      functions=[TestFunctionInfo(**f) for f in data["functions"]])


def classify_functions(info: ScriptInfo) -> list[FunctionCommissioning]:
    """Decide, per test function, whether the TH commissions the DUT before it starts."""
    result = []
    for function in info.functions:
        if function.test_case in TH_OPERATOR_CHOICE_TESTS:
            result.append(FunctionCommissioning(function.test_case, Commissioning.OPERATOR_CHOICE, "MANDATORY"))
            continue
        if function.step_count == 1 and function.first_step_description == _LEGACY_STEP_DESCRIPTION:
            result.append(FunctionCommissioning(function.test_case, Commissioning.OPERATOR_CHOICE, "LEGACY"))
            continue
        marker = info.device_requirement
        if marker not in _MARKER_NEEDS_COMMISSIONING:
            commissioning = Commissioning.REQUIRED if function.first_step_is_commissioning else Commissioning.NOT_REQUIRED
            result.append(FunctionCommissioning(function.test_case, commissioning, "step 1 is_commissioning"))
            continue
        needs_commissioning = _MARKER_NEEDS_COMMISSIONING[marker]
        contradiction = None
        if function.first_step_is_commissioning and not needs_commissioning:
            contradiction = (f"{function.test_case}: step 1 sets is_commissioning but the test class is {marker}; "
                             f"the TH uses {marker}")
        commissioning = Commissioning.REQUIRED if needs_commissioning else Commissioning.NOT_REQUIRED
        result.append(FunctionCommissioning(function.test_case, commissioning, marker, contradiction))
    return result


def in_test_commissioning_args(tokens: list[str]) -> list[str]:
    """Replace --commissioning-method METHOD (or -m METHOD) with --in-test-commissioning-method METHOD.

    Raises CommissioningPlanError if the result still sets --commissioning-method, for example
    through an abbreviated option name.
    """
    from matter.testing.runner import matter_test_args_parser

    result = []
    remaining = iter(tokens)
    for token in remaining:
        if token in ("--commissioning-method", "-m"):
            result += ["--in-test-commissioning-method", next(remaining, "")]
        elif token.startswith("--commissioning-method="):
            result += ["--in-test-commissioning-method", token.split("=", 1)[1]]
        elif token.startswith("-m") and not token.startswith("--") and len(token) > 2:
            result += ["--in-test-commissioning-method", token[2:]]
        else:
            result.append(token)

    args, _ = matter_test_args_parser().parse_known_args(result)
    if args.commissioning_method is not None:
        raise CommissioningPlanError(f"Cannot remove --commissioning-method from: {' '.join(tokens)}")
    return result


def plan_run(info: ScriptInfo, tokens: list[str]) -> RunPlan:
    """Plan a run of a script with these script arguments (shlex-split), TH-style where possible."""
    from matter.testing.runner import matter_test_args_parser

    args, _ = matter_test_args_parser().parse_known_args(tokens)
    if args.commissioning_method is not None and args.in_test_commissioning_method is not None:
        raise CommissioningPlanError("Script arguments set both --commissioning-method and --in-test-commissioning-method")
    if not info.functions:
        raise CommissioningPlanError("The test class has no test functions to run")

    functions = classify_functions(info)
    warnings = [f.contradiction for f in functions if f.contradiction]
    ambiguities = list(warnings)

    def names(commissioning: Commissioning) -> list[str]:
        return [f.test_case for f in functions if f.commissioning == commissioning]

    def plan(label: RunLabel, detail: str, test_args: list[str], commission_only_args: list[str] | None = None) -> RunPlan:
        return RunPlan(label, detail, test_args, commission_only_args, warnings, ambiguities)

    operator_choice = [f for f in functions if f.commissioning == Commissioning.OPERATOR_CHOICE]
    if operator_choice:
        detail = ", ".join(f"{f.test_case} ({f.reason})" for f in operator_choice)
        ambiguities.append(f"the TH asks the operator whether to commission for {detail}")
        return plan(RunLabel.OPERATOR_CHOICE, detail, tokens)

    required = names(Commissioning.REQUIRED)
    not_required = names(Commissioning.NOT_REQUIRED)
    if required and not_required:
        detail = f"commissioned: {', '.join(required)}; not commissioned: {', '.join(not_required)}"
        warnings.append(f"Mixed commissioning requirements ({detail}); the TH runs these in different suites. "
                        "Select one group with --tests to run it TH-style")
        ambiguities.append(f"mixed commissioning requirements ({detail})")
        return plan(RunLabel.MIXED, detail, tokens)

    reasons = ", ".join(sorted({f.reason for f in functions}))
    has_method = args.commissioning_method is not None
    if not_required:
        test_args = in_test_commissioning_args(tokens) if has_method else tokens
        return plan(RunLabel.LEFT_UNCOMMISSIONED, reasons, test_args)
    if has_method:
        return plan(RunLabel.COMMISSIONED_IN_SEPARATE_STEP, reasons, in_test_commissioning_args(tokens),
                    tokens + ["--commission-only"])
    return plan(RunLabel.COMMISSIONED_BY_EARLIER_RUN, reasons, tokens)


def main(argv: list[str]) -> int:
    """`python3 -m matter.testing.device_requirement_commissioning <script> [test function ...]`

    Prints the script's ScriptInfo as JSON on a line that starts with _RESULT_PREFIX.
    """
    if not argv:
        print(main.__doc__, file=sys.stderr)
        return 2
    # As for `python3 <script>`: the script's directory instead of the working directory.
    sys.path[0] = str(pathlib.Path(argv[0]).resolve().parent)
    try:
        info = collect_script_info(load_test_class(argv[0]), argv[1:])
    except Exception as e:
        print(f"{type(e).__name__}: {e}", file=sys.stderr)
        return 1
    print(_RESULT_PREFIX + json.dumps(asdict(info)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
