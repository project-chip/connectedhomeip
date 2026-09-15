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

    def test_kvs_is_keyed_by_app_basename_once(self):
        args = "--discriminator 1234 --KVS kvs1 --trace-to json:x.json"
        keyed = self.runner.keyed_kvs_app_args("out/linux/chip-all-clusters-app", args)
        self.assertEqual(keyed, "--discriminator 1234 --KVS kvs1.chip-all-clusters-app --trace-to json:x.json")
        self.assertEqual(self.runner.keyed_kvs_app_args("out/linux/chip-all-clusters-app", keyed), keyed, "idempotent")

    def test_device_argument_is_part_of_the_key(self):
        args = "--device extended-color-light:1 --discriminator 1234 --KVS kvs1"
        keyed = self.runner.keyed_kvs_app_args("out/all-devices-app", args)
        self.assertEqual(keyed, "--device extended-color-light:1 --discriminator 1234 --KVS kvs1.all-devices-app.extended-color-light-1")
        other = self.runner.keyed_kvs_app_args("out/all-devices-app", "--device fan --KVS kvs1")
        self.assertEqual(other, "--device fan --KVS kvs1.all-devices-app.fan")

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
        return (d.wipe_controller, d.wipe_app, d.force_commissioning)

    def test_no_factory_reset_never_wipes_or_forces(self):
        self.assertEqual(self.outcome(self.decide(factory_reset=False)), (False, False, False))

    def test_reuse_off_explicit_and_fresh_dut_wipe_everything(self):
        for policy in ({"reuse": False}, {"factory_reset_explicit": True}, {"fresh_dut": True}):
            self.assertEqual(self.outcome(self.decide(**policy)), (True, True, True), policy)

    def test_no_storage_path_wipes_everything(self):
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(
            reuse=True), self.app_args, "--commissioning-method on-network")
        self.assertEqual(self.outcome(d), (True, True, True))

    def test_no_kvs_wipes_app_state_only(self):
        pathlib.Path(self.storage).touch()
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), "--discriminator 1234", self.script_args)
        self.assertEqual(self.outcome(d), (False, True, True), "controller storage must survive an unkeyable app")

    def test_missing_storage_resets_app_state_and_forces(self):
        pathlib.Path(self.kvs).touch()
        d = self.decide()
        self.assertEqual(self.outcome(d), (False, True, True),
                         "a kept KVS is stale without the storage it was commissioned against")

    def test_missing_kvs_keeps_storage_and_forces(self):
        pathlib.Path(self.storage).touch()
        d = self.decide()
        self.assertEqual(self.outcome(d), (False, False, True))
        self.assertIn(self.kvs, d.reason)

    def test_present_state_keeps_and_probes(self):
        pathlib.Path(self.kvs).touch()
        pathlib.Path(self.storage).touch()
        self.assertEqual(self.outcome(self.decide()), (False, False, False))

    def test_in_test_commissioning_runs_get_a_fresh_app(self):
        pathlib.Path(self.kvs).touch()
        pathlib.Path(self.storage).touch()
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args,
                                         f"--storage-path {self.storage} --in-test-commissioning-method on-network")
        self.assertEqual(self.outcome(d), (False, True, True))
        d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args,
                                         f"--storage-path {self.storage} --commissioning-method on-network "
                                         "--in-test-commissioning-method on-network")
        self.assertEqual(self.outcome(d), (False, False, False), "with a runner method the DUT may be reused")

    def test_setup_payload_runs_get_a_fresh_app(self):
        pathlib.Path(self.kvs).touch()
        pathlib.Path(self.storage).touch()
        for payload in ("--qr-code MT:-24J0AFN00KA0648G00", "--manual-code 10054912339"):
            d = self.runner.decide_dut_state(True, self.runner.DutStatePolicy(reuse=True), self.app_args,
                                             f"--storage-path {self.storage} --commissioning-method on-network {payload}")
            self.assertEqual(self.outcome(d), (False, True, True), payload)


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

            wiped = list(runner.FactoryResetType.AppAndController.config_files("--KVS " + kvs_a, "--storage-path " + storage))
            self.assertTrue({kvs_a, kvs_b, storage} <= set(wiped), wiped)
            runner.factory_reset_config_removal("--KVS " + kvs_a, "--storage-path " + storage,
                                                runner.FactoryResetType.AppAndController)
            self.assertFalse(os.path.exists(kvs_a) or os.path.exists(kvs_b) or os.path.exists(storage))
            self.assertEqual(runner.registered_keyed_kvs(storage), [kvs_a, kvs_b], "the list survives the wipe")

            app_only = list(runner.FactoryResetType.AppOnly.config_files("--KVS " + kvs_a, "--storage-path " + storage))
            self.assertNotIn(kvs_b, app_only, "an app-only reset touches only its own KVS")


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
