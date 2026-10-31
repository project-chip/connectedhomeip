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

"""Unit tests for matter.testing.device_requirement_commissioning.

Fixture test scripts are written to a temporary directory, so the CI test-argument scanners
and the marker checks in TestDeviceRequirementMarkers never see them.
"""

import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

from matter.testing import device_requirement_commissioning as drc
from matter.testing.matter_testing import _DEVICE_REQUIREMENT_MARKERS

_SCRIPT_TEMPLATE = '''
from matter.testing.matter_testing import {base_imports}
from matter.testing.runner import TestStep, default_matter_test_main
{extra_imports}

class {class_name}({bases}):
{body}

if __name__ == "__main__":
    raise SystemExit("the main guard ran")
'''


def _test_function(name: str, first_step_is_commissioning: bool | None) -> str:
    """Source for a test function; first_step_is_commissioning None gives a test without steps."""
    steps = ""
    if first_step_is_commissioning is not None:
        steps = (f"    def steps_{name}(self):\n"
                 f"        return [TestStep(1, 'Commissioning, already done', is_commissioning={first_step_is_commissioning}),\n"
                 f"                TestStep(2, 'Read an attribute')]\n\n")
    return steps + f"    async def test_{name}(self):\n        pass\n"


def _info(marker: str | None, *functions: tuple[str, bool | None]) -> drc.ScriptInfo:
    """A ScriptInfo; each function is (test case, step 1 is_commissioning), or None for LEGACY."""
    return drc.ScriptInfo(marker, [
        drc.TestFunctionInfo(f"test_{name}", 1, "Run entire test", False) if flag is None
        else drc.TestFunctionInfo(f"test_{name}", 2, "Commissioning, already done", flag)
        for name, flag in functions])


_COMMISSIONING_ARGS = ["--storage-path", "admin_storage.json", "--commissioning-method", "on-network",
                       "--discriminator", "1234", "--passcode", "20202021"]
_IN_TEST_ARGS = ["--storage-path", "admin_storage.json", "--in-test-commissioning-method", "on-network",
                 "--discriminator", "1234", "--passcode", "20202021"]


class TestReadScriptInfo(unittest.TestCase):

    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.dir = Path(self._dir.name)
        self._saved_path = list(sys.path)

    def tearDown(self):
        sys.path[:] = self._saved_path
        self._dir.cleanup()

    def write(self, file_name: str, source: str) -> str:
        path = self.dir / file_name
        path.write_text(textwrap.dedent(source))
        return str(path)

    def write_script(self, class_name: str, bases: str, functions: dict[str, bool | None],
                     base_imports: str | None = None, extra_imports: str = "") -> str:
        body = "\n".join(_test_function(name, flag) for name, flag in functions.items())
        return self.write(f"{class_name}.py", _SCRIPT_TEMPLATE.format(
            base_imports=base_imports or bases, extra_imports=extra_imports, class_name=class_name, bases=bases,
            body=body))

    def info(self, script: str, tests: list[str] | None = None) -> drc.ScriptInfo:
        return drc.collect_script_info(drc.load_test_class(script), tests or [])

    def test_each_marker(self):
        for marker in (drc.COMMISSIONED_DEVICE, drc.UNCOMMISSIONED_DEVICE, drc.COMMISSIONER, drc.NO_DEVICE):
            with self.subTest(marker=marker):
                script = self.write_script(f"TC_FIX{marker}_1_1", marker, {f"TC_FIX{marker}_1_1": False})
                info = self.info(script)
                self.assertEqual(info.device_requirement, marker)
                self.assertEqual(info.functions, [drc.TestFunctionInfo(
                    f"test_TC_FIX{marker}_1_1", 2, "Commissioning, already done", False)])

    def test_no_marker_reports_step_one_flag(self):
        script = self.write_script("TC_FIXNONE_1_1", "MatterBaseTest", {"TC_FIXNONE_1_1": True})
        info = self.info(script)
        self.assertIsNone(info.device_requirement)
        self.assertTrue(info.functions[0].first_step_is_commissioning)

    def test_test_without_steps(self):
        script = self.write_script("TC_FIXLEGACY_1_1", "MatterTestCommissionedDevice", {"TC_FIXLEGACY_1_1": None})
        self.assertEqual(self.info(script).functions,
                         [drc.TestFunctionInfo("test_TC_FIXLEGACY_1_1", 1, "Run entire test", False)])

    def test_tests_selects_functions(self):
        script = self.write_script("TC_FIXTWO_1_1", "MatterTestCommissionedDevice",
                                   {"TC_FIXTWO_1_1": False, "TC_FIXTWO_1_2": True})
        self.assertEqual([f.name for f in self.info(script).functions], ["test_TC_FIXTWO_1_1", "test_TC_FIXTWO_1_2"])
        self.assertEqual([f.name for f in self.info(script, ["test_TC_FIXTWO_1_2"]).functions], ["test_TC_FIXTWO_1_2"])

    def test_marker_inherited_from_sibling_module(self):
        self.write("fixture_sibling_base.py", """
            from matter.testing.matter_testing import MatterTestCommissioner

            class SiblingBase(MatterTestCommissioner):
                pass
            """)
        script = self.write_script("TC_FIXSIB_1_1", "SiblingBase", {"TC_FIXSIB_1_1": False},
                                   base_imports="MatterBaseTest", extra_imports="from fixture_sibling_base import SiblingBase")
        self.assertEqual(self.info(script).device_requirement, drc.COMMISSIONER)

    def test_two_leaf_classes_raise(self):
        script = self.write("TC_FIXLEAVES_1_1.py", """
            from matter.testing.matter_testing import MatterTestCommissionedDevice

            class First(MatterTestCommissionedDevice):
                pass

            class Second(MatterTestCommissionedDevice):
                pass
            """)
        with self.assertRaisesRegex(ValueError, "Exactly one subclass"):
            drc.load_test_class(script)

    def test_conflicting_markers_raise(self):
        script = self.write_script("TC_FIXCONFLICT_1_1", "MatterTestCommissionedDevice, MatterTestCommissioner",
                                   {"TC_FIXCONFLICT_1_1": False})
        with self.assertRaisesRegex(ValueError, "conflicting device-requirement markers"):
            self.info(script)

    def test_query_process(self):
        script = self.write_script("TC_FIXPROC_1_1", "MatterTestCommissionedDevice",
                                   {"TC_FIXPROC_1_1": True, "TC_FIXPROC_1_2": False})
        info = drc.read_script_info(script, ["test_TC_FIXPROC_1_2"])
        self.assertEqual(info, drc.ScriptInfo(drc.COMMISSIONED_DEVICE, [drc.TestFunctionInfo(
            "test_TC_FIXPROC_1_2", 2, "Commissioning, already done", False)]))

    def test_query_process_does_not_run_main_guard(self):
        script = self.write_script("TC_FIXGUARD_1_1", "MatterTestCommissionedDevice", {"TC_FIXGUARD_1_1": False})
        self.assertEqual(drc.read_script_info(script, []).device_requirement, drc.COMMISSIONED_DEVICE)

    def test_query_process_reports_errors(self):
        conflict = self.write_script("TC_FIXPCONF_1_1", "MatterTestCommissionedDevice, MatterTestCommissioner",
                                     {"TC_FIXPCONF_1_1": False})
        with self.assertRaisesRegex(drc.CommissioningPlanError, "conflicting device-requirement markers"):
            drc.read_script_info(conflict, [])
        missing_import = self.write("TC_FIXMISSING_1_1.py", "import fixture_module_that_does_not_exist\n")
        with self.assertRaisesRegex(drc.CommissioningPlanError, "fixture_module_that_does_not_exist"):
            drc.read_script_info(missing_import, [])


class TestClassifyFunctions(unittest.TestCase):

    def classify(self, info: drc.ScriptInfo) -> list[tuple[str, drc.Commissioning, str]]:
        return [(f.test_case, f.commissioning, f.reason) for f in drc.classify_functions(info)]

    def test_marker_names_match_the_markers(self):
        names = {marker.__name__ for marker in _DEVICE_REQUIREMENT_MARKERS}
        self.assertEqual({drc.COMMISSIONED_DEVICE, drc.UNCOMMISSIONED_DEVICE, drc.COMMISSIONER, drc.NO_DEVICE}, names)
        self.assertEqual(set(drc._MARKER_NEEDS_COMMISSIONING), names)

    def test_marker_decides(self):
        expected = {
            drc.COMMISSIONED_DEVICE: drc.Commissioning.REQUIRED,
            drc.UNCOMMISSIONED_DEVICE: drc.Commissioning.NOT_REQUIRED,
            drc.COMMISSIONER: drc.Commissioning.NOT_REQUIRED,
            drc.NO_DEVICE: drc.Commissioning.NOT_REQUIRED,
        }
        for marker, commissioning in expected.items():
            with self.subTest(marker=marker):
                self.assertEqual(self.classify(_info(marker, ("TC_X_1_1", False))),
                                 [("TC_X_1_1", commissioning, marker)])

    def test_step_one_flag_decides_without_a_marker(self):
        self.assertEqual(self.classify(_info(None, ("TC_X_1_1", True), ("TC_X_1_2", False))), [
            ("TC_X_1_1", drc.Commissioning.REQUIRED, "step 1 is_commissioning"),
            ("TC_X_1_2", drc.Commissioning.NOT_REQUIRED, "step 1 is_commissioning"),
        ])

    def test_operator_chooses_for_legacy_and_mandatory(self):
        self.assertEqual(self.classify(_info(drc.COMMISSIONED_DEVICE, ("TC_X_1_1", None), ("TC_IDM_12_1", True))), [
            ("TC_X_1_1", drc.Commissioning.OPERATOR_CHOICE, "LEGACY"),
            ("TC_IDM_12_1", drc.Commissioning.OPERATOR_CHOICE, "MANDATORY"),
        ])

    def test_marker_wins_over_contradicting_step_one_flag(self):
        [function] = drc.classify_functions(_info(drc.COMMISSIONER, ("TC_X_1_1", True)))
        self.assertEqual(function.commissioning, drc.Commissioning.NOT_REQUIRED)
        self.assertIn("step 1 sets is_commissioning", function.contradiction)
        [function] = drc.classify_functions(_info(drc.COMMISSIONED_DEVICE, ("TC_X_1_1", False)))
        self.assertIsNone(function.contradiction)


class TestInTestCommissioningArgs(unittest.TestCase):

    def test_rewrites_each_form(self):
        for method_args in (["--commissioning-method", "on-network"], ["-m", "on-network"],
                            ["--commissioning-method=on-network"], ["-mon-network"]):
            with self.subTest(method_args=method_args):
                self.assertEqual(drc.in_test_commissioning_args(["--storage-path", "a.json", *method_args, "-d", "1234"]),
                                 ["--storage-path", "a.json", "--in-test-commissioning-method", "on-network", "-d", "1234"])

    def test_keeps_other_tokens(self):
        tokens = ["--storage-path", "a b.json", "--string-arg", "name:value with spaces", "--tests", "test_TC_X_1_1"]
        self.assertEqual(drc.in_test_commissioning_args(tokens), tokens)

    def test_abbreviated_option_raises(self):
        with self.assertRaises(drc.CommissioningPlanError):
            drc.in_test_commissioning_args(["--commissioning", "on-network"])


class TestPlanRun(unittest.TestCase):

    def test_commissioned_in_separate_step(self):
        plan = drc.plan_run(_info(drc.COMMISSIONED_DEVICE, ("TC_X_1_1", False)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.COMMISSIONED_IN_SEPARATE_STEP)
        self.assertTrue(plan.label.th_style)
        self.assertEqual(plan.commission_only_args, _COMMISSIONING_ARGS + ["--commission-only"])
        self.assertEqual(plan.test_args, _IN_TEST_ARGS)
        self.assertEqual(plan.ambiguities, [])

    def test_step_one_flag_commissions_without_a_marker(self):
        plan = drc.plan_run(_info(None, ("TC_X_1_1", True)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.COMMISSIONED_IN_SEPARATE_STEP)
        self.assertEqual(plan.detail, "step 1 is_commissioning")

    def test_commissioned_by_earlier_run(self):
        tokens = ["--storage-path", "admin_storage.json"]
        plan = drc.plan_run(_info(drc.COMMISSIONED_DEVICE, ("TC_X_1_1", False)), tokens)
        self.assertEqual(plan.label, drc.RunLabel.COMMISSIONED_BY_EARLIER_RUN)
        self.assertIsNone(plan.commission_only_args)
        self.assertEqual(plan.test_args, tokens)

    def test_left_uncommissioned(self):
        plan = drc.plan_run(_info(drc.COMMISSIONER, ("TC_X_1_1", False)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.LEFT_UNCOMMISSIONED)
        self.assertIsNone(plan.commission_only_args)
        self.assertEqual(plan.test_args, _IN_TEST_ARGS)
        self.assertEqual(drc.plan_run(_info(drc.COMMISSIONER, ("TC_X_1_1", False)), _IN_TEST_ARGS).test_args,
                         _IN_TEST_ARGS)

    def test_operator_choice_runs_as_written(self):
        plan = drc.plan_run(_info(None, ("TC_X_1_1", None)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.OPERATOR_CHOICE)
        self.assertFalse(plan.label.th_style)
        self.assertEqual(plan.detail, "TC_X_1_1 (LEGACY)")
        self.assertEqual(plan.test_args, _COMMISSIONING_ARGS)
        self.assertIsNone(plan.commission_only_args)
        self.assertEqual(len(plan.ambiguities), 1)

    def test_mixed_requirements_run_as_written(self):
        plan = drc.plan_run(_info(None, ("TC_X_1_1", True), ("TC_X_1_2", False)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.MIXED)
        self.assertFalse(plan.label.th_style)
        self.assertEqual(plan.detail, "commissioned: TC_X_1_1; not commissioned: TC_X_1_2")
        self.assertEqual(plan.test_args, _COMMISSIONING_ARGS)
        self.assertEqual(len(plan.ambiguities), 1)
        self.assertIn("--tests", plan.warnings[0])

    def test_contradiction_is_an_ambiguity(self):
        plan = drc.plan_run(_info(drc.COMMISSIONER, ("TC_X_1_1", True)), _COMMISSIONING_ARGS)
        self.assertEqual(plan.label, drc.RunLabel.LEFT_UNCOMMISSIONED)
        self.assertEqual(len(plan.warnings), 1)
        self.assertEqual(plan.ambiguities, plan.warnings)

    def test_errors(self):
        info = _info(drc.COMMISSIONED_DEVICE, ("TC_X_1_1", False))
        with self.assertRaisesRegex(drc.CommissioningPlanError, "both"):
            drc.plan_run(info, _COMMISSIONING_ARGS + ["--in-test-commissioning-method", "on-network"])
        with self.assertRaisesRegex(drc.CommissioningPlanError, "no test functions"):
            drc.plan_run(drc.ScriptInfo(drc.COMMISSIONED_DEVICE, []), _COMMISSIONING_ARGS)


if __name__ == "__main__":
    unittest.main()
