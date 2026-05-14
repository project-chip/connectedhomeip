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

"""AST-based extraction of TestStep objects from test method source code.

When a test class does not define an explicit steps_* method, this module
extracts step information by parsing the test method's source code. It finds
all self.step() calls and returns them as TestStep objects. skip_step() calls
are ignored since they should always have a corresponding step() in another
branch that carries the description and metadata.

Step numbers and descriptions must be constants (string or int literals).
Dynamic step numbers (e.g. self.step(variable)) will raise a ValueError,
since the extracted plan would be silently incomplete. Tests that need
dynamic steps should define an explicit steps_* method instead.
"""


import ast
import inspect
import textwrap
from typing import Optional, Union

from matter.testing.runner import TestStep


class _StepExtractorVisitor(ast.NodeVisitor):
    """Walks the AST finding self.step() calls."""

    def __init__(self):
        self.steps: list[TestStep] = []

    def _is_step_call(self, node: ast.Call) -> bool:
        return (isinstance(node.func, ast.Attribute)
                and isinstance(node.func.value, ast.Name)
                and node.func.value.id == "self"
                and node.func.attr == "step")

    def _require_constant(self, node: ast.expr, name: str, lineno: int) -> object:
        """Extract a constant value from an AST node, raising if it's dynamic."""
        if not isinstance(node, ast.Constant):
            raise ValueError(
                f"self.step() at line {lineno}: {name} must be a constant. "
                "Define an explicit steps_* method for tests with dynamic step arguments.")
        return node.value

    def _extract_step(self, node: ast.Call) -> Optional[TestStep]:
        if not node.args:
            return None

        number = self._require_constant(node.args[0], "step number", node.lineno)

        description = ""
        if len(node.args) >= 2:
            description = str(self._require_constant(node.args[1], "description", node.lineno))

        kw_values: dict[str, object] = {}
        for kw in node.keywords:
            if kw.arg is None:
                continue
            kw_values[kw.arg] = self._require_constant(kw.value, kw.arg, node.lineno)
        if not description:
            description = str(kw_values.get("description", ""))

        return TestStep(
            test_plan_number=number,
            description=description,
            is_commissioning=bool(kw_values.get("is_commissioning", False)),
            expectation=str(kw_values.get("expectation", "")),
        )

    def visit_Call(self, node: ast.Call):
        if self._is_step_call(node):
            step = self._extract_step(node)
            if step:
                self.steps.append(step)
        self.generic_visit(node)


def _merge_duplicate_steps(steps: list[TestStep]) -> list[TestStep]:
    """Deduplicate steps with the same number (e.g. from if/else branches).

    When a step appears in multiple branches, the entry with a description is preferred.
    """
    seen: dict[Union[int, str], TestStep] = {}
    order: list[Union[int, str]] = []
    for s in steps:
        key = s.test_plan_number
        if key not in seen:
            seen[key] = s
            order.append(key)
        elif not seen[key].description and s.description:
            seen[key] = s
    return [seen[k] for k in order]


def _extract_steps_from_ast(tree: ast.AST) -> list[TestStep]:
    visitor = _StepExtractorVisitor()
    visitor.visit(tree)
    if not visitor.steps:
        return []
    return _merge_duplicate_steps(visitor.steps)


def extract_steps_from_method(method) -> list[TestStep]:
    """Extract a TestStep list from a test method using AST analysis."""
    name = getattr(method, '__name__', repr(method))
    try:
        source = textwrap.dedent(inspect.getsource(method))
    except (OSError, TypeError) as e:
        raise ValueError(
            f"Failed to get source for {name}: {e}. "
            "Define an explicit steps_* method for tests whose source is not available.") from e

    try:
        tree = ast.parse(source)
    except SyntaxError as e:
        raise ValueError(
            f"Failed to parse source for {name}: {e}. "
            "Define an explicit steps_* method if AST extraction cannot handle the source.") from e

    return _extract_steps_from_ast(tree)


def extract_steps_from_source(source: str) -> list[TestStep]:
    """Extract TestStep list from a source code string (convenience for testing)."""
    return _extract_steps_from_ast(ast.parse(textwrap.dedent(source)))
