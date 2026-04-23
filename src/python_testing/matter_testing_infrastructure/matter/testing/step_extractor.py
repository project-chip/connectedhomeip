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

"""
AST-based extraction of TestStep objects from test method source code.

When a test class does not define an explicit steps_* method, this module
extracts step information by parsing the test method's source code. It finds
all self.step() and self.skip_step() calls and returns them as TestStep objects.

Step numbers and descriptions must be constants (string or int literals).
Dynamic step numbers (e.g. self.step(variable)) will raise a ValueError,
since the extracted plan would be silently incomplete. Tests that need
dynamic steps should define an explicit steps_* method instead.
"""


import ast
import inspect
import textwrap
from typing import Optional

from matter.testing.runner import TestStep


class _StepExtractorVisitor(ast.NodeVisitor):
    """
    Walks the AST finding self.step() and self.skip_step() calls.

    When the same step number appears in multiple branches of an if/else,
    the merge logic deduplicates them (keeping the first description found).
    """

    def __init__(self):
        self.steps: list[dict] = []

    def _is_step_call(self, node: ast.Call) -> bool:
        return (isinstance(node.func, ast.Attribute)
                and isinstance(node.func.value, ast.Name)
                and node.func.value.id == "self"
                and node.func.attr in ("step", "skip_step"))

    def _extract_step_data(self, node: ast.Call) -> Optional[dict]:
        if not node.args:
            return None

        # Step number must be a constant (int or str literal)
        num_node = node.args[0]
        if not isinstance(num_node, ast.Constant):
            raise ValueError(f"self.step() at line {node.lineno} has a dynamic step number. "
                             "AST step extraction requires constant step numbers. "
                             "Define an explicit steps_* method for tests with dynamic steps.")
        number = num_node.value

        # Description is optional; must be a constant if provided
        description = ""
        if len(node.args) >= 2 and isinstance(node.args[1], ast.Constant):
            description = str(node.args[1].value)

        # Extract keyword-only arguments
        kw_values: dict[str, object] = {}
        for kw in node.keywords:
            if isinstance(kw.value, ast.Constant):
                kw_values[kw.arg] = kw.value.value
        if not description:
            description = str(kw_values.get("description", ""))

        is_commissioning = bool(kw_values.get("is_commissioning", False))
        expectation = str(kw_values.get("expectation", ""))

        return {
            "number": number,
            "description": description,
            "is_commissioning": is_commissioning,
            "expectation": expectation,
        }

    def visit_Call(self, node: ast.Call):
        if self._is_step_call(node):
            data = self._extract_step_data(node)
            if data:
                self.steps.append(data)
        self.generic_visit(node)


def _merge_duplicate_steps(steps: list[dict]) -> list[dict]:
    """Deduplicate steps with the same number (e.g. from if/else branches)."""
    seen: dict = {}
    order: list = []
    for s in steps:
        key = str(s["number"])
        if key not in seen:
            seen[key] = s
            order.append(key)
        else:
            existing = seen[key]
            if not existing["description"] and s["description"]:
                existing["description"] = s["description"]
    return [seen[k] for k in order]


def _extract_steps_from_ast(tree: ast.AST) -> list[TestStep]:
    visitor = _StepExtractorVisitor()
    visitor.visit(tree)
    if not visitor.steps:
        return []
    merged = _merge_duplicate_steps(visitor.steps)
    return [TestStep(
        test_plan_number=s["number"],
        description=s["description"],
        is_commissioning=s.get("is_commissioning", False),
        expectation=s.get("expectation", ""),
    ) for s in merged]


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
