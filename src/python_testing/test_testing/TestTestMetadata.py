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

"""Unit tests for MatterBaseTest metadata extraction"""

import sys

from mobly import signals

from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, generate_mobly_test_config


def _make_test_instance(test_class):
    config = generate_mobly_test_config(MatterTestConfig())
    return test_class(config)


def test_pics_decorator_only():
    """@pics decorator is used when no pics_* method exists."""

    class MyTest(MatterBaseTest):
        @pics("OO.S", "S.S")
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_pics("test_TC_FOO_1_1")
    assert result == ["OO.S", "S.S"], f"Expected ['OO.S', 'S.S'], got {result}"


def test_pics_method_takes_precedence():
    """Explicit pics_* method takes precedence over @pics decorator."""

    class MyTest(MatterBaseTest):
        def pics_TC_FOO_1_1(self):
            return ["FROM.METHOD"]

        @pics("FROM.DECORATOR")
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_pics("test_TC_FOO_1_1")
    assert result == ["FROM.METHOD"], f"Expected ['FROM.METHOD'], got {result}"


def test_no_pics_at_all():
    """No pics_* method and no @pics decorator returns empty list."""

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_pics("test_TC_FOO_1_1")
    assert result == [], f"Expected [], got {result}"


def test_pics_decorator_below_async_test_body():
    """@pics works when placed below @async_test_body (wraps propagates __dict__)."""

    class MyTest(MatterBaseTest):
        @async_test_body
        @pics("OO.S")
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_pics("test_TC_FOO_1_1")
    assert result == ["OO.S"], f"Expected ['OO.S'], got {result}"


def test_pics_decorator_single_code():
    """@pics with a single PICS code."""

    class MyTest(MatterBaseTest):
        @pics("GRPKEY.S")
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_pics("test_TC_FOO_1_1")
    assert result == ["GRPKEY.S"], f"Expected ['GRPKEY.S'], got {result}"


def test_pics_empty_raises():
    """@pics() with no arguments raises ValueError."""
    try:
        @pics()
        def test_foo(self):
            pass
        assert False, "Expected ValueError for empty @pics"
    except ValueError as e:
        assert "at least one" in str(e).lower()


def test_pics_double_application_raises():
    """Applying @pics twice raises ValueError."""
    try:
        @pics("A.S")
        @pics("B.S")
        def test_foo(self):
            pass
        assert False, "Expected ValueError for double @pics"
    except ValueError as e:
        assert "more than once" in str(e).lower()


def test_desc_from_docstring():
    """Docstring is used as description when no desc_* method exists."""

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            """4.2.4. [TC-FOO-1.1] Scenes Management Cluster Interaction"""
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_desc("test_TC_FOO_1_1")
    assert result == "4.2.4. [TC-FOO-1.1] Scenes Management Cluster Interaction", f"Unexpected desc: {result}"


def test_desc_method_takes_precedence():
    """Explicit desc_* method takes precedence over docstring."""

    class MyTest(MatterBaseTest):
        def desc_TC_FOO_1_1(self):
            return "From method"

        @async_test_body
        async def test_TC_FOO_1_1(self):
            """From docstring"""
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_desc("test_TC_FOO_1_1")
    assert result == "From method", f"Unexpected desc: {result}"


def test_desc_falls_back_to_method_name():
    """No desc_* and no docstring falls back to method name."""

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    result = inst.get_test_desc("test_TC_FOO_1_1")
    assert result == "test_TC_FOO_1_1", f"Unexpected desc: {result}"


def test_steps_from_explicit_method():
    """Explicit steps_* method is used when defined."""

    class MyTest(MatterBaseTest):
        def steps_TC_FOO_1_1(self):
            return [TestStep(1, "Explicit step")]

        @async_test_body
        async def test_TC_FOO_1_1(self):
            self.step(1, "Inline step")

    inst = _make_test_instance(MyTest)
    steps = inst.get_defined_test_steps("test_TC_FOO_1_1")
    assert len(steps) == 1
    assert steps[0].description == "Explicit step", f"Expected explicit method to win, got: {steps[0].description}"


def test_steps_from_ast_fallback():
    """AST extraction is used when no steps_* method exists."""

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            self.step(1, "First inline step")
            self.step(2, "Second inline step")

    inst = _make_test_instance(MyTest)
    steps = inst.get_defined_test_steps("test_TC_FOO_1_1")
    assert steps is not None, "Expected AST fallback to find steps"
    assert len(steps) == 2
    assert steps[0].test_plan_number == 1
    assert steps[0].description == "First inline step"
    assert steps[1].test_plan_number == 2
    assert steps[1].description == "Second inline step"


def test_steps_inline_runtime_validation():
    """Inline step descriptions work end-to-end with step validation."""
    from types import SimpleNamespace

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            self.step(1, "First step")
            self.step(2, "Second step")

    inst = _make_test_instance(MyTest)
    # Manually set up the per-test state that Mobly normally initializes
    # before each test method. We can't call setup_test() because it
    # requires the full Mobly test runner lifecycle and chip stack.
    inst.current_test_info = SimpleNamespace(name="test_TC_FOO_1_1")
    inst.current_step_index = 0
    inst.step_skipped = False
    inst.step_start_time = None

    # These should succeed — steps match the AST-extracted plan
    inst.step(1, "First step")
    inst.step(2, "Second step")

    # Calling a step out of order should fail
    inst.current_step_index = 0
    try:
        inst.step(2, "Wrong order")
        assert False, "Expected TestFailure for out-of-order step"
    except signals.TestFailure:
        pass


def test_steps_none_when_no_steps():
    """No steps_* and no self.step() calls returns None."""

    class MyTest(MatterBaseTest):
        @async_test_body
        async def test_TC_FOO_1_1(self):
            pass

    inst = _make_test_instance(MyTest)
    steps = inst.get_defined_test_steps("test_TC_FOO_1_1")
    assert steps is None, f"Expected None, got: {steps}"


def main():
    failures = []

    test_functions = [
        test_pics_decorator_only,
        test_pics_method_takes_precedence,
        test_no_pics_at_all,
        test_pics_decorator_below_async_test_body,
        test_pics_decorator_single_code,
        test_pics_empty_raises,
        test_pics_double_application_raises,
        test_desc_from_docstring,
        test_desc_method_takes_precedence,
        test_desc_falls_back_to_method_name,
        test_steps_from_explicit_method,
        test_steps_from_ast_fallback,
        test_steps_inline_runtime_validation,
        test_steps_none_when_no_steps,
    ]

    for test_fn in test_functions:
        try:
            test_fn()
            print(f"  PASS: {test_fn.__name__}")
        except Exception as e:
            print(f"  FAIL: {test_fn.__name__}: {e}")
            failures.append(test_fn.__name__)

    print(f"\nTestTestMetadata: {len(failures)} failures out of {len(test_functions)} tests")
    for f in failures:
        print(f"  FAILED: {f}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
