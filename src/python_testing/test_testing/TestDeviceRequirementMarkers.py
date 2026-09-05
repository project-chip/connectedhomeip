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

"""Structural checks for the device-requirement marker base classes.

These verify the marker hierarchy itself (empty, mutually exclusive, orthogonal to
the wildcard subscription) and that the tests reclassified in this first PR declare
the expected marker. The lists below are the explicit scope of that reclassification;
the assertions inspect actual class inheritance (via device_requirement), not file
paths, so they stay meaningful if a class is moved or renamed.
"""

import ast
import asyncio
import importlib.util
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

from mobly import signals

from matter.testing import matter_testing as matter_testing_module
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import (CertificationUnitTestNoDevice, MatterBaseTest, MatterTestCommissionedDevice,
                                           MatterTestCommissioner, MatterTestUncommissionedDevice, device_requirement)

_MARKERS = (
    MatterTestCommissionedDevice,
    MatterTestUncommissionedDevice,
    MatterTestCommissioner,
    CertificationUnitTestNoDevice,
)

# src/python_testing (parent of test_testing/). Both are placed on sys.path so the
# reclassified modules and their sibling helpers import cleanly when loaded by file.
_PY_TESTING = Path(__file__).resolve().parent.parent

# Reclassified-in-this-PR scope: expected marker -> module files (relative to _PY_TESTING).
_EXPECTED_BY_MARKER: dict[type, list[str]] = {
    CertificationUnitTestNoDevice: [
        "test_testing/TestChoiceConformanceSupport.py",
        "test_testing/TestConformanceSupport.py",
        "test_testing/TestSpecParsingDataType.py",
        "test_testing/TestSpecParsingNamespace.py",
        "test_testing/TestSpecParsingSupport.py",
        "test_testing/TestIdChecks.py",
        "test_testing/TestDefaultWarnings.py",
        "test_testing/TestPics.py",
        "test_testing/TestMatterTestingSupport.py",
        "test_testing/TestMatterDeviceGraph.py",
        "test_testing/TestDecorators.py",
        # Controller-lifecycle unit test: creates and shuts down fabric admins locally and
        # never performs DUT I/O. test_metadata.yaml and CI both run it with no app.
        "test_testing/TestCreateNewController.py",
    ],
    MatterTestCommissionedDevice: [
        "test_testing/TestBatchInvoke.py",
        "test_testing/TestBdxTransfer.py",
        "test_testing/TestCheckCommandFlags.py",
        "test_testing/TestGroupTableReports.py",
        "test_testing/TestInvokeReturnCodes.py",
        "test_testing/TestUnitTestingErrorPath.py",
        "test_testing/TestWriteReadOnlyAttributes.py",
        "test_testing/TestTimeSyncTrustedTimeSource.py",
        "test_testing/test_manufacturer_specific_cluster.py",
        "test_testing/test_ota_version.py",
        "test_testing/TestCommissioningTimeSync.py",
        "test_testing/TestCleanupFramework.py",
        # Representatives of the support-module bases: mixed bases carry the marker on the
        # concrete class (CADMIN/ICDB_1_x), uniform bases carry it on the base and the tests
        # single-inherit (SMOKECO/BINFO). All must resolve to Commissioned.
        "TC_CADMIN_1_10.py",
        "TC_ICDB_1_1.py",
        "TC_SMOKECO_2_1.py",
        "TC_BINFO_2_1.py",
    ],
    MatterTestCommissioner: [
        "test_testing/TestCommissioningStatusDetectionIntegration.py",
        "test_testing/TestFactoryResetRequests.py",
        "TC_CGEN_2_5.py",
        "TC_CGEN_2_6.py",
        "TC_CGEN_2_7.py",
        "TC_CGEN_2_8.py",
        "TC_CGEN_2_9.py",
        "TC_CGEN_2_10.py",
        "TC_CGEN_2_11.py",
        "TC_CNET_4_23.py",
        "TC_DA_1_9.py",
        "TC_DD_3_23.py",
        "TC_DD_3_24.py",
        "TC_SC_7_1.py",
        "TC_SC_TC_2_1.py",
        "TC_SC_TC_2_2.py",
        "TC_SC_TC_3_1.py",
        "TC_SC_TC_4_1.py",
        # commission the primary DUT themselves (ICDB via commission_device; JF via joint-fabric
        # pairing; SC_3_5 is a DUT-as-commissioner test). ICDB/JFADMIN_2_2 also exercise the mixed
        # support bases (ICDBaseTest/CADMINBaseTest) that must stay marker-free.
        "TC_ICDB_2_1_2_2.py",
        "TC_ICDB_2_3.py",
        "TC_ICDB_2_4.py", "TC_ICDB_2_5.py",
        "TC_JFADMIN_1_1.py",
        "TC_JFADMIN_1_2.py",
        "TC_JFADMIN_1_4.py",
        "TC_JFADMIN_2_1.py",
        "TC_JFADMIN_2_2.py",
        "TC_JFDS_2_1.py",
        "TC_JFDS_2_2.py",
        "TC_JFDS_2_3.py",
        "TC_JFDS_2_4.py",
        "TC_SC_3_5.py",
    ],
    MatterTestUncommissionedDevice: [
        "TC_DD_1_16_17.py",
        "TC_DD_1_5.py",
    ],
}


def _load_module(rel_path: str):
    """Import a test module by file path, returning the loaded module object.

    Raises ImportError if the spec can't be created or the module fails to execute; the
    caller uses that to fall back to a source (AST) check. On failure the partially
    initialized module is removed from sys.modules so it can't leak into later imports.
    """
    abs_path = _PY_TESTING / rel_path
    name = abs_path.stem
    spec = importlib.util.spec_from_file_location(name, abs_path)
    if spec is None or spec.loader is None:
        raise ImportError(f"Cannot create import spec for {abs_path}")
    module = importlib.util.module_from_spec(spec)
    # Register before exec so intra-module dataclass/self references resolve.
    sys.modules[name] = module
    try:
        spec.loader.exec_module(module)
    except BaseException:
        sys.modules.pop(name, None)
        raise
    return module


def _concrete_test_classes(module):
    """Return MatterBaseTest subclasses defined in `module` (excludes the markers themselves)."""
    classes = []
    for obj in vars(module).values():
        if (isinstance(obj, type) and issubclass(obj, MatterBaseTest)
                and obj.__module__ == module.__name__ and obj not in _MARKERS):
            classes.append(obj)
    return classes


_MARKER_NAMES = {m.__name__ for m in _MARKERS}

# Optional third-party dependencies that some reclassified modules import at load time
# (absent in minimal local environments, present in CI). When one of these is missing we
# verify the marker from source instead of importing; ANY other import failure is a real
# problem (missing required dep, bad import, typo) and must fail the test.
_OPTIONAL_IMPORT_DEPS = frozenset({"ndef", "graphviz"})


def _marker_bases_from_source(rel_path: str) -> dict[str, set[str]]:
    """Fallback used when a module can't be imported (e.g. an optional dependency such as
    ndeflib is unavailable). Parses the source and returns, per class that declares a
    marker base, the set of marker names in its base list. Detects both "which marker"
    and "more than one marker" without executing the module.
    """
    def _base_name(base) -> str | None:
        # Handle both `class X(Marker)` (ast.Name) and `class X(module.Marker)` (ast.Attribute).
        if isinstance(base, ast.Name):
            return base.id
        if isinstance(base, ast.Attribute):
            return base.attr
        return None

    tree = ast.parse((_PY_TESTING / rel_path).read_text())
    result: dict[str, set[str]] = {}
    for node in tree.body:
        if isinstance(node, ast.ClassDef):
            base_names = {name for name in (_base_name(b) for b in node.bases) if name}
            markers = base_names & _MARKER_NAMES
            if markers:
                result[node.name] = markers
    return result


class TestDeviceRequirementMarkers(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        for path in (_PY_TESTING, _PY_TESTING / "test_testing"):
            p = str(path)
            if p not in sys.path:
                sys.path.insert(0, p)

    def test_device_state_markers_do_not_touch_subscription(self):
        """The three device-STATE markers stay orthogonal to the wildcard subscription: they set
        neither requires_dut nor disable_wildcard_subscription."""
        for marker in (MatterTestCommissionedDevice, MatterTestUncommissionedDevice, MatterTestCommissioner):
            self.assertNotIn("requires_dut", marker.__dict__, f"{marker.__name__} must not set requires_dut")
            self.assertNotIn("disable_wildcard_subscription", marker.__dict__,
                             f"{marker.__name__} must not set disable_wildcard_subscription")

    def test_no_device_marker_disables_subscription(self):
        """CertificationUnitTestNoDevice is the sanctioned exception: it sets requires_dut = False
        on the base (a no-device test can never use the subscription) so no-device tests inherit
        it without each having to set it. It sets nothing else subscription-related."""
        self.assertIn("requires_dut", CertificationUnitTestNoDevice.__dict__,
                      "CertificationUnitTestNoDevice must set requires_dut on the base")
        self.assertFalse(CertificationUnitTestNoDevice.requires_dut,
                         "CertificationUnitTestNoDevice.requires_dut must be False")
        self.assertNotIn("disable_wildcard_subscription", CertificationUnitTestNoDevice.__dict__)

    def test_base_is_requirement_neutral(self):
        """MatterBaseTest itself declares no device requirement, and BasicCompositionTests is a
        distinct dual-state base rather than one of the markers."""
        self.assertIsNone(device_requirement(MatterBaseTest))
        self.assertNotIn(BasicCompositionTests, _MARKERS)
        self.assertIsNone(device_requirement(BasicCompositionTests),
                          "BasicCompositionTests must not carry a device-requirement marker")

    def test_conflicting_markers_are_rejected(self):
        """A class cannot declare two device requirements at once."""
        class _Conflicting(MatterTestCommissioner, MatterTestCommissionedDevice):
            pass
        with self.assertRaises(ValueError):
            device_requirement(_Conflicting)

    def test_reclassified_tests_declare_expected_marker(self):
        """Every module reclassified in this PR exposes a concrete test class carrying exactly
        the expected marker (and only one).

        Importing each module here intentionally doubles as a load smoke-test: a broken import
        in a reclassified module (typo, bad base change, missing required dep) is re-raised and
        fails this test rather than being masked.

        Modules import cleanly -> checked at runtime via device_requirement(). Modules whose
        optional dependencies are absent in this environment fall back to a source (AST) check
        of the declared base; such modules are reported, never silently skipped.
        """
        source_fallback = []
        for expected_marker, paths in _EXPECTED_BY_MARKER.items():
            for rel_path in paths:
                with self.subTest(module=rel_path):
                    try:
                        module = _load_module(rel_path)
                    except ImportError as e:
                        # Only fall back for a known-optional missing dependency; re-raise any
                        # other import failure so it fails the test rather than being masked.
                        if e.name not in _OPTIONAL_IMPORT_DEPS:
                            raise
                        source_fallback.append(f"{rel_path} ({e.name})")
                        declared = _marker_bases_from_source(rel_path)
                        self.assertTrue(declared, f"{rel_path}: no class declares a device-requirement marker")
                        for cls_name, markers in declared.items():
                            self.assertEqual(
                                markers, {expected_marker.__name__},
                                f"{rel_path}:{cls_name} should declare exactly {expected_marker.__name__}, got {markers}")
                        continue

                    concrete = _concrete_test_classes(module)
                    self.assertTrue(concrete, f"{rel_path}: no concrete MatterBaseTest subclass found")
                    for cls in concrete:
                        # device_requirement raises if two markers are present, covering exclusivity.
                        self.assertIs(
                            device_requirement(cls), expected_marker,
                            f"{rel_path}:{cls.__name__} should declare {expected_marker.__name__}")

        if source_fallback:
            print(f"\n[TestDeviceRequirementMarkers] source-only (import deps unavailable): {source_fallback}",
                  file=sys.stderr)

    def test_no_concrete_test_inherits_matterbasetest_directly(self):
        """Coverage guard: every concrete test must declare its device requirement through a
        marker (or an intermediate base such as BasicCompositionTests), never bare MatterBaseTest.

        AST-scans src/python_testing/TC_*.py and test_testing/*.py (no imports needed, so it is
        independent of optional dependencies). Any top-level class that both defines a test
        method (test_/steps_/desc_) and lists MatterBaseTest as a *direct* base is flagged --
        that is a test sitting on the raw base with no device classification. Mixin/base helpers
        (no such methods) may still derive from MatterBaseTest directly.
        """
        def base_names(class_def):
            return {base.id if isinstance(base, ast.Name) else base.attr
                    for base in class_def.bases if isinstance(base, (ast.Name, ast.Attribute))}

        def defines_test(class_def):
            return any(isinstance(body_node, (ast.FunctionDef, ast.AsyncFunctionDef))
                       and body_node.name.startswith(("test_", "steps_", "desc_")) for body_node in class_def.body)

        offenders = []
        for path in sorted(_PY_TESTING.glob("TC_*.py")) + sorted((_PY_TESTING / "test_testing").glob("*.py")):
            tree = ast.parse(path.read_text())
            for node in tree.body:
                if isinstance(node, ast.ClassDef) and defines_test(node) and "MatterBaseTest" in base_names(node):
                    offenders.append(f"{path.name}:{node.name}")
        self.assertEqual(offenders, [],
                         "concrete tests must derive from a device-requirement marker, not bare "
                         f"MatterBaseTest: {offenders}")


_MATTER_TESTING_LOGGER = "matter.testing.matter_testing"


def _make_stub(loop, case_reachable: bool = False):
    """Minimal stand-in for a MatterBaseTest instance, avoiding the heavy Mobly class setup.

    The real _run_blocking / _resolve_dut_commissioned / _is_dut_commissioned_blocking are
    bound through, so the running-loop detection and the DNS-SD-then-CASE resolution are
    exercised rather than stubbed out.

    case_reachable controls whether the simulated GetConnectedDevice succeeds, i.e. whether
    the DUT is reachable over CASE when the DNS-SD probe comes back negative.
    """
    controller = mock.MagicMock()
    if case_reachable:
        controller.GetConnectedDevice = mock.AsyncMock(return_value=object())
    else:
        controller.GetConnectedDevice = mock.AsyncMock(side_effect=TimeoutError("no CASE session"))

    stub = SimpleNamespace(
        event_loop=loop,
        default_controller=controller,
        dut_node_id=1,
        matter_test_config=SimpleNamespace(commissioning_method=None),
        _dut_confirmed_available=False,
    )
    stub._run_blocking = lambda coro: MatterBaseTest._run_blocking(stub, coro)
    stub._resolve_dut_commissioned = lambda: MatterBaseTest._resolve_dut_commissioned(stub)
    stub._is_dut_commissioned_blocking = lambda: MatterBaseTest._is_dut_commissioned_blocking(stub)
    return stub


class TestCommissioningPreconditionChecks(unittest.TestCase):
    """Behavioral checks for the commissioning-state precondition the device-state markers enforce.

    The precondition delegates to matter.testing.commissioning.is_commissioned (a passive DNS-SD
    probe). These tests mock it so no device or network is required, and drive the helper directly
    (avoiding the heavy Mobly class setup) plus verify each marker's setup_class passes the right
    expectation.
    """

    def _run_precondition(self, is_commissioned_result: bool, expect_commissioned: bool):
        """Invoke MatterBaseTest._assert_device_commissioning_precondition against a stub self with
        is_commissioned mocked to return is_commissioned_result. Raises signals.TestFailure if the
        precondition fails."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop)
            with mock.patch.object(matter_testing_module, "is_commissioned",
                                   new=mock.AsyncMock(return_value=is_commissioned_result)):
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=expect_commissioned)
        finally:
            loop.close()

    def test_commissioned_precondition_passes_when_commissioned(self):
        """MatterTestCommissionedDevice: a commissioned DUT satisfies the precondition."""
        self._run_precondition(is_commissioned_result=True, expect_commissioned=True)

    def test_commissioned_precondition_fails_when_not_commissioned(self):
        """MatterTestCommissionedDevice: an uncommissioned DUT hard-fails class setup."""
        with self.assertRaises(signals.TestFailure):
            self._run_precondition(is_commissioned_result=False, expect_commissioned=True)

    def test_uncommissioned_precondition_passes_when_not_commissioned(self):
        """MatterTestUncommissionedDevice: an uncommissioned DUT satisfies the precondition."""
        self._run_precondition(is_commissioned_result=False, expect_commissioned=False)

    def test_uncommissioned_precondition_fails_when_commissioned(self):
        """MatterTestUncommissionedDevice: a DUT already on the fabric hard-fails class setup."""
        with self.assertRaises(signals.TestFailure):
            self._run_precondition(is_commissioned_result=True, expect_commissioned=False)

    def test_assert_dut_commissioned_helper(self):
        """assert_dut_commissioned passes when the DUT is commissioned and fails otherwise."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop)
            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=True)):
                MatterBaseTest.assert_dut_commissioned(stub)
            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=False)), \
                    self.assertRaises(signals.TestFailure):
                MatterBaseTest.assert_dut_commissioned(stub)
        finally:
            loop.close()

    def test_precondition_works_when_event_loop_already_running(self):
        """Regression: a setup_class override decorated with @async_test_body calls
        super().setup_class() from inside a running event loop. The precondition must still
        run there instead of raising "This event loop is already running"."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop)

            async def call_from_within_loop():
                # Mirrors @async_test_body: sync framework code reached from a running loop.
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)

            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=True)):
                loop.run_until_complete(call_from_within_loop())
        finally:
            loop.close()

    def test_precondition_still_fails_when_event_loop_already_running(self):
        """The running-loop path must report a genuine precondition failure, not swallow it."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop)

            async def call_from_within_loop():
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)

            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=False)), \
                    self.assertRaises(signals.TestFailure):
                loop.run_until_complete(call_from_within_loop())
        finally:
            loop.close()

    def test_dnssd_miss_is_confirmed_over_case(self):
        """Regression: the DNS-SD probe is IPv6-only and skips loopback, so a commissioned DUT on
        the same host reads as not commissioned. A reachable CASE session must override that
        negative rather than hard-failing an already-commissioned DUT."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop, case_reachable=True)
            # assertLogs keeps the expected diagnostic out of this suite's own output, and doubles
            # as a check that the CASE override is reported rather than applied silently.
            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=False)), \
                    self.assertLogs(_MATTER_TESTING_LOGGER, level="WARNING") as logs:
                # Would raise before the CASE confirmation was added.
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)
            self.assertIn("reachable over CASE", "\n".join(logs.output))
            stub.default_controller.GetConnectedDevice.assert_awaited_once()
            self.assertFalse(stub.default_controller.GetConnectedDevice.await_args.kwargs["allowPASE"],
                             "the confirmation must not fall back to PASE, which would mask an "
                             "uncommissioned DUT in a pairing window as commissioned")
        finally:
            loop.close()

    def test_dnssd_hit_skips_the_case_confirmation(self):
        """A positive DNS-SD result is conclusive, so the CASE round trip must be skipped. This is
        what keeps the common path (334 MatterTestCommissionedDevice tests) cheap."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop, case_reachable=True)
            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=True)):
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)
            stub.default_controller.GetConnectedDevice.assert_not_awaited()
        finally:
            loop.close()

    def test_uncommissioned_marker_rejects_a_case_reachable_dut(self):
        """A DUT that is invisible to DNS-SD but answers CASE is commissioned, so it must fail the
        MatterTestUncommissionedDevice precondition instead of silently passing it."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop, case_reachable=True)
            with mock.patch.object(matter_testing_module, "is_commissioned", new=mock.AsyncMock(return_value=False)), \
                    self.assertLogs(_MATTER_TESTING_LOGGER, level="WARNING"), \
                    self.assertRaises(signals.TestFailure):
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=False)
        finally:
            loop.close()

    def test_dnssd_probe_failure_falls_through_to_case(self):
        """Regression: is_commissioned imports mdns_discovery, which ships in src/python_testing
        rather than in the matter package, so tests under test_testing/ hit ModuleNotFoundError.
        An unavailable DNS-SD probe is inconclusive and must defer to CASE, not fail setup."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop, case_reachable=True)
            with mock.patch.object(matter_testing_module, "is_commissioned",
                                   new=mock.AsyncMock(side_effect=ModuleNotFoundError("No module named 'mdns_discovery'"))), \
                    self.assertLogs(_MATTER_TESTING_LOGGER, level="INFO") as logs:
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)
            self.assertIn("mdns_discovery", "\n".join(logs.output),
                          "the unusable probe should be named in the log so the cause is diagnosable")
            stub.default_controller.GetConnectedDevice.assert_awaited_once()
        finally:
            loop.close()

    def test_dnssd_probe_failure_with_unreachable_dut_still_fails(self):
        """A broken DNS-SD probe must not turn into a free pass: with no CASE session either, the
        precondition still has to fail."""
        loop = asyncio.new_event_loop()
        try:
            stub = _make_stub(loop, case_reachable=False)
            with mock.patch.object(matter_testing_module, "is_commissioned",
                                   new=mock.AsyncMock(side_effect=ModuleNotFoundError("No module named 'mdns_discovery'"))), \
                    self.assertRaises(signals.TestFailure):
                MatterBaseTest._assert_device_commissioning_precondition(stub, expect_commissioned=True)
        finally:
            loop.close()

    def test_commissioned_marker_setup_class_enforces_precondition(self):
        """MatterTestCommissionedDevice.setup_class calls super().setup_class() then asserts the
        DUT is commissioned."""
        stub = mock.MagicMock(spec=MatterTestCommissionedDevice)
        with mock.patch.object(MatterBaseTest, "setup_class") as super_setup:
            MatterTestCommissionedDevice.setup_class(stub)
        super_setup.assert_called_once()
        stub._assert_device_commissioning_precondition.assert_called_once_with(expect_commissioned=True)

    def test_uncommissioned_marker_setup_class_enforces_precondition(self):
        """MatterTestUncommissionedDevice.setup_class calls super().setup_class() then asserts the
        DUT is NOT commissioned."""
        stub = mock.MagicMock(spec=MatterTestUncommissionedDevice)
        with mock.patch.object(MatterBaseTest, "setup_class") as super_setup:
            MatterTestUncommissionedDevice.setup_class(stub)
        super_setup.assert_called_once()
        stub._assert_device_commissioning_precondition.assert_called_once_with(expect_commissioned=False)

    def test_commissioner_marker_has_no_setup_class_precondition(self):
        """MatterTestCommissioner does not enforce a setup_class precondition (its DUT is not yet
        present at class setup); it relies on the opt-in assert_dut_commissioned instead."""
        self.assertNotIn("setup_class", MatterTestCommissioner.__dict__)


if __name__ == "__main__":
    unittest.main()
