#!/usr/bin/env -S python3 -B
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
import importlib.util
import sys
import unittest
from pathlib import Path

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
        "test_testing/TestCreateNewController.py",
        "test_testing/TestCommissioningTimeSync.py",
        "test_testing/TestCleanupFramework.py",
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
    ],
    MatterTestUncommissionedDevice: [
        "TC_DD_1_16_17.py",
        "TC_DD_1_5.py",
    ],
}


def _load_module(rel_path: str):
    """Import a test module by file path, returning the loaded module object."""
    abs_path = _PY_TESTING / rel_path
    name = abs_path.stem
    spec = importlib.util.spec_from_file_location(name, abs_path)
    module = importlib.util.module_from_spec(spec)
    # Register before exec so intra-module dataclass/self references resolve.
    sys.modules[name] = module
    spec.loader.exec_module(module)
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


def _marker_bases_from_source(rel_path: str) -> dict[str, set[str]]:
    """Fallback used when a module can't be imported (e.g. an optional dependency such as
    ndeflib is unavailable). Parses the source and returns, per class that declares a
    marker base, the set of marker names in its base list. Detects both "which marker"
    and "more than one marker" without executing the module.
    """
    tree = ast.parse((_PY_TESTING / rel_path).read_text())
    result: dict[str, set[str]] = {}
    for node in tree.body:
        if isinstance(node, ast.ClassDef):
            base_names = {b.id for b in node.bases if isinstance(b, ast.Name)}
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

    def test_markers_are_empty_subclasses_of_base(self):
        """Each marker is an empty subclass of MatterBaseTest (pure declaration, no behavior)."""
        allowed = {"__doc__", "__module__", "__firstlineno__", "__static_attributes__", "__qualname__"}
        for marker in _MARKERS:
            self.assertTrue(issubclass(marker, MatterBaseTest), f"{marker.__name__} must subclass MatterBaseTest")
            self.assertIs(marker.__base__, MatterBaseTest, f"{marker.__name__} must derive directly from MatterBaseTest")
            extra = set(marker.__dict__) - allowed
            self.assertEqual(extra, set(), f"{marker.__name__} must not define members, found: {extra}")

    def test_markers_do_not_touch_subscription(self):
        """Device classification is orthogonal to the wildcard subscription: markers must not
        set requires_dut or disable_wildcard_subscription."""
        for marker in _MARKERS:
            self.assertNotIn("requires_dut", marker.__dict__, f"{marker.__name__} must not set requires_dut")
            self.assertNotIn("disable_wildcard_subscription", marker.__dict__,
                             f"{marker.__name__} must not set disable_wildcard_subscription")

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
                        # Optional dependency missing (e.g. ndeflib for NFC tests): verify from source.
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


if __name__ == "__main__":
    unittest.main()
