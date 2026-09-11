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

"""Guards the layout of src/python_testing: shared code belongs in support_modules/.

Both CI test runners (scripts/tests/local.py and
src/python_testing/execute_python_tests.py) discover tests by globbing
src/python_testing/*.py and then executing each match as a standalone script. A
shared test base or helper module dropped in that directory is therefore picked
up as if it were a test, and the only way to keep it out of CI used to be a
not_automated entry in test_metadata.yaml excusing it.

Neither runner recurses, so moving shared code into support_modules/ keeps it
out of discovery for good. This test enforces that placement.
"""

import ast
import pathlib
import unittest

PYTHON_TESTING_DIR = pathlib.Path(__file__).resolve().parent.parent
SUPPORT_MODULES_DIR = PYTHON_TESTING_DIR / "support_modules"


def _has_main_block(path: pathlib.Path) -> bool:
    """Return whether the module has a top-level `if __name__ == "__main__":` block.

    Parsed rather than grepped so that a mention of __main__ in a docstring or
    comment does not count as an entry point.
    """
    for node in ast.parse(path.read_text()).body:
        if not isinstance(node, ast.If):
            continue
        test = node.test
        if not isinstance(test, ast.Compare) or len(test.ops) != 1 or not isinstance(test.ops[0], ast.Eq):
            continue
        # Accept either operand order: `__name__ == "__main__"` or the reverse.
        operands = (test.left, test.comparators[0])
        names = {node.id for node in operands if isinstance(node, ast.Name)}
        constants = {node.value for node in operands if isinstance(node, ast.Constant)}
        if "__name__" in names and "__main__" in constants:
            return True
    return False


def _misplacement_report(path: pathlib.Path) -> str:
    module = path.stem
    return f"""
{path.name} is shared support code, but it lives in src/python_testing/ where only
runnable tests belong. Move it to src/python_testing/support_modules/.

Why this fails: the CI test runners (scripts/tests/local.py and
src/python_testing/execute_python_tests.py) glob src/python_testing/*.py and run
every file they find as a standalone test. {path.name} has no
`if __name__ == "__main__":` block, so it cannot run standalone -- CI would
either fail on it or need a test_metadata.yaml entry excusing it.

How to fix it:

  1. Move the file:
       git mv src/python_testing/{path.name} src/python_testing/support_modules/

  2. Update every importer:
       from {module} import Something
     becomes
       from support_modules.{module} import Something

     A module imported as a whole (`import {module}`) becomes
       from support_modules import {module}
     which leaves its call sites unchanged.

  3. Do NOT add it to test_metadata.yaml. The runners do not recurse into
     support_modules/, so the file is already invisible to them and needs no
     not_automated entry.

If this file really is a standalone test, give it an
`if __name__ == "__main__":` block that invokes the test runner, the way the
other tests in src/python_testing/ do.
"""


class TestSupportModuleLocation(unittest.TestCase):

    def test_support_modules_are_not_in_python_testing_root(self):
        """Every src/python_testing/*.py must be runnable standalone; shared code belongs in support_modules/."""
        misplaced = sorted(
            path for path in PYTHON_TESTING_DIR.glob("*.py") if not _has_main_block(path)
        )
        if misplaced:
            self.fail("".join(_misplacement_report(path) for path in misplaced))

    def test_support_modules_directory_exists(self):
        """A missing support_modules/ would make the check above vacuous."""
        self.assertTrue(
            SUPPORT_MODULES_DIR.is_dir(),
            f"Expected shared test code to live in {SUPPORT_MODULES_DIR}, but that directory does not exist.",
        )


if __name__ == "__main__":
    unittest.main()
