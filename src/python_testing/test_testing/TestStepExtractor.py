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

"""Unit tests for the AST step extractor module."""

import sys

from matter.testing.runner import TestStep
from matter.testing.step_extractor import extract_steps_from_method, extract_steps_from_source


def test_simple_sequential_steps():
    source = '''
    def test_foo(self):
        self.step(1, "First step")
        self.step(2, "Second step")
        self.step(3, "Third step")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 3
    assert steps[0] == TestStep(test_plan_number=1, description="First step")
    assert steps[1] == TestStep(test_plan_number=2, description="Second step")
    assert steps[2] == TestStep(test_plan_number=3, description="Third step")


def test_string_step_numbers():
    source = '''
    def test_foo(self):
        self.step("4a", "Sub-step a")
        self.step("4b", "Sub-step b")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 2
    assert steps[0].test_plan_number == "4a"
    assert steps[1].test_plan_number == "4b"


def test_conditional_if_step():
    source = '''
    def test_foo(self):
        self.step(1, "Always")
        if self.step(2, "Conditional"):
            pass
        self.step(3, "Always again")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 3


def test_duplicate_steps_in_if_else():
    source = '''
    def test_foo(self):
        if condition:
            self.step(1, "Branch A")
        else:
            self.step(1, "Branch B")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 1
    assert steps[0].test_plan_number == 1
    assert steps[0].description == "Branch A"


def test_duplicate_prefers_description():
    """When merging duplicates, prefer the entry with a description."""
    source = '''
    def test_foo(self):
        if condition:
            self.step(1)
        else:
            self.step(1, "Has description")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 1
    assert steps[0].description == "Has description"


def test_duplicate_prefers_metadata():
    """When merging duplicates, prefer the entry with metadata."""
    source = '''
    def test_foo(self):
        if condition:
            self.step(1)
        else:
            self.step(1, "Commissioning", is_commissioning=True, expectation="Verify success")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 1
    assert steps[0].description == "Commissioning"
    assert steps[0].is_commissioning is True
    assert steps[0].expectation == "Verify success"


def test_no_description():
    source = '''
    def test_foo(self):
        self.step(1)
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 1
    assert steps[0].description == ""


def test_dynamic_step_number_raises():
    source = '''
    def test_foo(self):
        self.step(1, "Static")
        self.step(step_var, "Dynamic")
        self.step(3, "Static again")
    '''
    try:
        extract_steps_from_source(source)
        assert False, "Expected ValueError for dynamic step number"
    except ValueError as e:
        assert "must be a constant" in str(e).lower()


def test_keyword_description():
    source = '''
    def test_foo(self):
        self.step(1, description="Keyword desc")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 1
    assert steps[0].description == "Keyword desc"


def test_nested_for_loop():
    source = '''
    def test_foo(self):
        self.step(1, "Before loop")
        for i in range(3):
            self.step(2, "In loop")
        self.step(3, "After loop")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 3


def test_empty_method():
    source = '''
    def test_foo(self):
        pass
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 0


def test_extract_from_live_method():
    class FakeTest:
        def test_example(self):
            self.step(1, "Live step")
            self.step(2, "Another live step")

    steps = extract_steps_from_method(FakeTest.test_example)
    assert len(steps) == 2
    assert steps[0] == TestStep(test_plan_number=1, description="Live step")


def test_current_style_no_description():
    source = '''
    def test_foo(self):
        self.step(0)
        self.step(1)
        self.step(2)
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 3
    assert all(s.description == "" for s in steps)


def test_if_else_branches():
    """Steps from both branches of an if/else are extracted and deduplicated."""
    source = '''
    def test_TC_SC_5_1(self):
        self.step("0", "Commissioning")
        self.step("1", "TH writes ACL")
        if groupcast_enabled:
            self.step("3", "Groupcast bind")
            self.step("4", "Groupcast remove")
        else:
            self.step("3", "Legacy bind")
            self.step("4", "Legacy remove")
        self.step("10", "KeySetRead")
    '''
    steps = extract_steps_from_source(source)
    numbers = [s.test_plan_number for s in steps]
    assert numbers == ["0", "1", "3", "4", "10"]


def test_keyword_only_args():
    source = '''
    def test_foo(self):
        self.step(1, "Commissioning", is_commissioning=True)
        self.step(2, "Do something", expectation="Verify success")
        self.step(3, "Plain step")
    '''
    steps = extract_steps_from_source(source)
    assert len(steps) == 3
    assert steps[0].is_commissioning is True
    assert steps[1].expectation == "Verify success"
    assert steps[1].is_commissioning is False
    assert steps[2].expectation == ""


def main():
    failures = []

    test_functions = [
        test_simple_sequential_steps,
        test_string_step_numbers,
        test_conditional_if_step,
        test_duplicate_steps_in_if_else,
        test_duplicate_prefers_description,
        test_duplicate_prefers_metadata,
        test_no_description,
        test_dynamic_step_number_raises,
        test_keyword_description,
        test_nested_for_loop,
        test_empty_method,
        test_extract_from_live_method,
        test_current_style_no_description,
        test_if_else_branches,
        test_keyword_only_args,
    ]

    for test_fn in test_functions:
        try:
            test_fn()
            print(f"  PASS: {test_fn.__name__}")
        except Exception as e:
            print(f"  FAIL: {test_fn.__name__}: {e}")
            failures.append(test_fn.__name__)

    print(f"\nTest of StepExtractor: {len(failures)} failures out of {len(test_functions)} tests")
    for f in failures:
        print(f"  FAILED: {f}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
