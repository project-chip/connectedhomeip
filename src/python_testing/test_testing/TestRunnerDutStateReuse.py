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

"""Unit tests for the runner's DUT state reuse decisions (scripts/tests/run_python_test.py)."""

import importlib.util
import os
import pathlib
import re
import tempfile
import unittest

from matter.testing.metadata import MetadataReader

REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
RUNNER_PATH = REPO_ROOT / "scripts" / "tests" / "run_python_test.py"


def load_runner():
    spec = importlib.util.spec_from_file_location("run_python_test", RUNNER_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestKeyedKvs(unittest.TestCase):

    def setUp(self):
        self.runner = load_runner()

    def kvs_of(self, app, args):
        return re.search(r"--KVS (\S+)", self.runner.keyed_kvs_app_args(app, args)).group(1)

    def test_kvs_is_keyed_by_app_and_only_keyed_once(self):
        args = "--discriminator 1234 --KVS kvs1 --trace-to json:x.json"
        keyed = self.runner.keyed_kvs_app_args("out/linux/chip-all-clusters-app", args)
        self.assertRegex(keyed, r"--KVS kvs1\.chip-all-clusters-app\.[0-9a-f]{8} ")
        self.assertEqual(self.runner.keyed_kvs_app_args("out/linux/chip-all-clusters-app", keyed), keyed, "idempotent")

    def test_same_binary_name_from_a_different_build_is_a_different_dut(self):
        args = "--discriminator 1234 --KVS kvs1"
        self.assertNotEqual(self.kvs_of("out/all-clusters/chip-all-clusters-app", args),
                            self.kvs_of("out/all-clusters-no-groupcast/chip-all-clusters-app", args),
                            "two builds shipping the same binary name must not share device state")

    def test_device_argument_is_part_of_the_key(self):
        light = self.kvs_of("out/all-devices-app", "--device extended-color-light:1 --KVS kvs1")
        fan = self.kvs_of("out/all-devices-app", "--device fan --KVS kvs1")
        self.assertIn("all-devices-app.extended-color-light-1.", light)
        self.assertIn("all-devices-app.fan.", fan)
        self.assertNotEqual(light, fan)

    def test_no_kvs_or_no_app_is_untouched(self):
        self.assertEqual(self.runner.keyed_kvs_app_args("out/app", "--discriminator 1234"), "--discriminator 1234")
        self.assertEqual(self.runner.keyed_kvs_app_args("", "--KVS kvs1"), "--KVS kvs1")


class TestDecideDutState(unittest.TestCase):

    def setUp(self):
        self.runner = load_runner()
        self.tmp = tempfile.TemporaryDirectory()
        self.kvs = os.path.join(self.tmp.name, "kvs1.app")
        self.storage = os.path.join(self.tmp.name, "admin_storage.json")
        self.app_args = f"--discriminator 1234 --KVS {self.kvs}"
        self.script_args = f"--storage-path {self.storage} --commissioning-method on-network"

    def tearDown(self):
        self.tmp.cleanup()

    def decide(self, factory_reset=True, **policy):
        policy.setdefault("reuse", True)
        return self.runner.decide_dut_state(factory_reset, self.runner.DutStatePolicy(**policy), self.app_args, self.script_args)

    def outcome(self, d):
        return (d.wipe_controller, d.wipe_app, d.force_commissioning, d.restore_golden)

    def snapshot(self):
        """Both halves of the commissioned pair: the app's KVS and the commissioning authority."""
        pathlib.Path(self.runner.commissioned_snapshot_file(self.kvs)).touch()
        pathlib.Path(self.runner.commissioned_snapshot_file(self.storage)).touch()

    def test_no_factory_reset_never_wipes_or_forces(self):
        self.assertEqual(self.outcome(self.decide(factory_reset=False)), (False, False, False, False))

    def test_reuse_off_explicit_and_fresh_dut_wipe_everything(self):
        for policy in ({"reuse": False}, {"factory_reset_explicit": True}, {"fresh_dut": True}):
            self.assertEqual(self.outcome(self.decide(**policy)), (True, True, True, False), policy)

    def test_no_storage_path_wipes_everything(self):
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(
            reuse=True), self.app_args, "--commissioning-method on-network")
        self.assertEqual(self.outcome(d), (True, True, True, False))

    def test_no_kvs_wipes_app_state_only(self):
        pathlib.Path(self.storage).touch()
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), "--discriminator 1234", self.script_args)
        self.assertEqual(self.outcome(d), (False, True, True, False), "controller storage must survive an unkeyable app")

    def test_missing_storage_resets_app_state_and_forces(self):
        pathlib.Path(self.kvs).touch()
        d = self.decide()
        self.assertEqual(self.outcome(d), (False, True, True, False),
                         "a kept KVS is stale without the storage it was commissioned against")

    def test_without_a_snapshot_the_app_is_reset_and_commissioned_to_capture_one(self):
        """The first run of an app resets it, commissions, and the runner captures that state."""
        pathlib.Path(self.storage).touch()
        d = self.decide()
        self.assertEqual(self.outcome(d), (False, True, True, False), "no snapshot yet")
        self.assertIn(self.runner.commissioned_snapshot_file(self.kvs), d.reason)
        pathlib.Path(self.kvs).touch()
        self.assertEqual(self.outcome(self.decide()), (False, True, True, False), "a live KVS is not a snapshot")

    def test_a_class_needing_an_uncommissioned_dut_gets_a_fresh_app(self):
        pathlib.Path(self.storage).touch()
        pathlib.Path(self.kvs).touch()
        self.snapshot()
        d = self.decide(uncommissioned_dut_marker="MatterTestCommissioner")
        self.assertEqual(self.outcome(d), (False, True, True, False))
        self.assertIn("MatterTestCommissioner", d.reason)

    def test_pre_existing_fabric_runs_get_a_fresh_app_and_keep_the_storage(self):
        """The ephemeral fabric is commissioned onto the DUT first, which needs a DUT with no fabric."""
        pathlib.Path(self.storage).touch()
        pathlib.Path(self.kvs).touch()
        self.snapshot()
        self.assertEqual(self.outcome(self.decide(pre_existing_fabric=True)), (False, True, True, False))

    def test_with_a_snapshot_the_app_is_restored_and_not_commissioned(self):
        pathlib.Path(self.storage).touch()
        pathlib.Path(self.kvs).touch()
        self.snapshot()
        d = self.decide()
        self.assertEqual(self.outcome(d), (False, False, False, True))

    def test_half_a_snapshot_is_not_a_snapshot(self):
        """Without the authority that signed it the app snapshot is useless, and vice versa."""
        pathlib.Path(self.storage).touch()
        pathlib.Path(self.kvs).touch()
        for half in (self.kvs, self.storage):
            snapshot = self.runner.commissioned_snapshot_file(half)
            pathlib.Path(snapshot).touch()
            self.assertEqual(self.outcome(self.decide()), (False, True, True, False), half)
            os.unlink(snapshot)

    def test_fresh_dut_and_explicit_resets_ignore_the_snapshot(self):
        pathlib.Path(self.storage).touch()
        pathlib.Path(self.kvs).touch()
        self.snapshot()
        for policy in ({"fresh_dut": True}, {"factory_reset_explicit": True}, {"reuse": False}):
            self.assertEqual(self.outcome(self.decide(**policy)), (True, True, True, False), policy)

    def test_runs_without_a_runner_commissioning_method_get_a_fresh_app(self):
        pathlib.Path(self.kvs).touch()
        pathlib.Path(self.storage).touch()
        for args in (f"--storage-path {self.storage} --in-test-commissioning-method on-network",
                     f"--storage-path {self.storage} --endpoint 0"):
            d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args, args)
            self.assertEqual(self.outcome(d), (False, True, True, False), args)
        self.snapshot()
        with_method = f"--storage-path {self.storage} --commissioning-method on-network --in-test-commissioning-method on-network"
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args, with_method)
        self.assertEqual(self.outcome(d), (False, False, False, True), "with a runner method the snapshot is restored")

    def test_setup_payload_runs_get_a_fresh_app(self):
        pathlib.Path(self.kvs).touch()
        pathlib.Path(self.storage).touch()
        self.snapshot()
        for payload in ("--qr-code MT:-24J0AFN00KA0648G00", "--manual-code 10054912339"):
            d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args,
                                             f"--storage-path {self.storage} --commissioning-method on-network {payload}")
            self.assertEqual(self.outcome(d), (False, True, True, False), payload)


class TestUncommissionedDutMarkerScan(unittest.TestCase):

    def test_marker_is_found_in_the_class_definition_only(self):
        runner = load_runner()
        with tempfile.TemporaryDirectory() as tmp:
            def script(body):
                path = os.path.join(tmp, "s.py")
                pathlib.Path(path).write_text(body)
                return runner.script_needs_uncommissioned_dut(path)

            self.assertEqual(script("class TC_X(MatterTestCommissioner):\n    pass\n"), "MatterTestCommissioner")
            self.assertEqual(script("class TC_X(MatterBaseTest, MatterTestUncommissionedDevice):\n    pass\n"),
                             "MatterTestUncommissionedDevice")
            self.assertIsNone(script("class TC_X(MatterTestCommissionedDevice):\n    pass\n"),
                              "a class that wants a commissioned DUT is not a match")
            self.assertIsNone(script("# MatterTestCommissioner is mentioned in a comment\n"),
                              "a mention outside a class definition is not a declaration")

    def test_missing_script_is_harmless(self):
        runner = load_runner()
        self.assertIsNone(runner.script_needs_uncommissioned_dut("/nonexistent/script.py"))


class TestCommissionedSnapshot(unittest.TestCase):

    def test_capture_copies_the_kvs_aside(self):
        runner = load_runner()
        with tempfile.TemporaryDirectory() as tmp:
            kvs = os.path.join(tmp, "kvs1.app")
            pathlib.Path(kvs).write_text("fabric")
            storage = os.path.join(tmp, "admin_storage.json")
            pathlib.Path(storage).write_text("authority")
            runner.capture_commissioned_snapshot(f"--discriminator 1234 --KVS {kvs}", f"--storage-path {storage}")
            self.assertEqual(pathlib.Path(runner.commissioned_snapshot_file(kvs)).read_text(), "fabric")
            self.assertEqual(pathlib.Path(runner.commissioned_snapshot_file(storage)).read_text(), "authority")
            self.assertFalse(os.path.exists(runner.commissioned_snapshot_file(kvs) + ".tmp"), "no leftover temp file")

            # The authority is captured once and shared: every app is commissioned by the same one.
            pathlib.Path(storage).write_text("a later authority")
            runner.capture_commissioned_snapshot(f"--KVS {kvs}", f"--storage-path {storage}")
            self.assertEqual(pathlib.Path(runner.commissioned_snapshot_file(storage)).read_text(), "authority")

    def test_capture_leaves_no_temp_file_behind_when_the_kvs_is_gone(self):
        runner = load_runner()
        with tempfile.TemporaryDirectory() as tmp:
            kvs = os.path.join(tmp, "kvs1.app")
            runner.capture_commissioned_snapshot(f"--KVS {kvs}", f"--storage-path {os.path.join(tmp, 'gone.json')}")
            self.assertEqual(os.listdir(tmp), [], "a failed capture leaves nothing behind")

    def test_capture_without_a_kvs_or_a_missing_file_is_harmless(self):
        runner = load_runner()
        runner.capture_commissioned_snapshot("--discriminator 1234", "--storage-path x")
        with tempfile.TemporaryDirectory() as tmp:
            runner.capture_commissioned_snapshot(f"--KVS {os.path.join(tmp, 'never-created')}",
                                                 f"--storage-path {os.path.join(tmp, 'no-storage')}")


class TestCommissioningDecisionLine(unittest.TestCase):

    def test_decision_is_extracted_from_framework_output(self):
        runner = load_runner()
        skipped = b"[MatterTest] 09-15 14:25:53.366 INFO Skipping commissioning: DUT already commissioned on this fabric (pass --force-commissioning to commission anyway)\n"
        self.assertEqual(runner.commissioning_decision_from_line(skipped),
                         "Skipping commissioning: DUT already commissioned on this fabric (pass --force-commissioning to commission anyway)")
        self.assertEqual(runner.commissioning_decision_from_line(b"INFO Commissioning the DUT first: --force-commissioning given\n"),
                         "Commissioning the DUT first: --force-commissioning given")
        self.assertIsNone(runner.commissioning_decision_from_line(b"INFO Commissioning complete for node ID 0x12344321\n"))

    def test_controller_wipe_removes_every_registered_kvs_and_keeps_the_list(self):
        runner = load_runner()
        with tempfile.TemporaryDirectory() as tmp:
            storage = os.path.join(tmp, "admin_storage.json")
            kvs_a, kvs_b = os.path.join(tmp, "kvs1.app-a"), os.path.join(tmp, "kvs1.app-b")
            for k in (kvs_a, kvs_b, kvs_a):
                pathlib.Path(k).touch()
                runner.register_keyed_kvs(storage, k)
            pathlib.Path(storage).touch()
            self.assertEqual(runner.registered_keyed_kvs(storage), [kvs_a, kvs_b], "listed once each")

            snapshot_a, snapshot_b = runner.commissioned_snapshot_file(kvs_a), runner.commissioned_snapshot_file(kvs_b)
            for f in (snapshot_a, snapshot_b):
                pathlib.Path(f).touch()
            wiped = list(runner.FactoryResetType.AppAndController.config_files("--KVS " + kvs_a, "--storage-path " + storage))
            self.assertTrue({kvs_a, kvs_b, storage, snapshot_a, snapshot_b,
                             runner.commissioned_snapshot_file(storage)} <= set(wiped),
                            "a new controller fabric invalidates every KVS, every snapshot and the authority")
            runner.factory_reset_config_removal("--KVS " + kvs_a, "--storage-path " + storage,
                                                runner.FactoryResetType.AppAndController)
            self.assertFalse(os.path.exists(kvs_a) or os.path.exists(kvs_b) or os.path.exists(storage))
            self.assertEqual(runner.registered_keyed_kvs(storage), [kvs_a, kvs_b], "the list survives the wipe")

            app_only = list(runner.FactoryResetType.AppOnly.config_files("--KVS " + kvs_a, "--storage-path " + storage))
            self.assertNotIn(kvs_b, app_only, "an app-only reset touches only its own KVS")
            self.assertNotIn(snapshot_a, app_only, "the snapshot outlives an app-only reset: the fabric is still valid")


class TestFreshDutHeaderKey(unittest.TestCase):

    def test_fresh_dut_is_parsed(self):
        with tempfile.TemporaryDirectory() as tmp:
            env = os.path.join(tmp, "env.yaml")
            pathlib.Path(env).write_text("ALL_CLUSTERS_APP: /bin/app\n")
            script = os.path.join(tmp, "TC_X.py")
            pathlib.Path(script).write_text(
                "# === BEGIN CI TEST ARGUMENTS ===\n"
                "# test-runner-runs:\n"
                "#   run1:\n"
                "#     app: ${ALL_CLUSTERS_APP}\n"
                "#     factory-reset: true\n"
                "#     fresh-dut: true\n"
                "#   run2:\n"
                "#     app: ${ALL_CLUSTERS_APP}\n"
                "#     factory-reset: true\n"
                "# === END CI TEST ARGUMENTS ===\n")
            runs = MetadataReader(env).parse_script(script)
        self.assertEqual([(r.run, r.factory_reset, r.fresh_dut) for r in runs],
                         [("run1", True, True), ("run2", True, False)])


if __name__ == "__main__":
    unittest.main()
