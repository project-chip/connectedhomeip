#!/usr/bin/env python3

# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from collections.abc import Iterator
import contextlib
import difflib
import io
import os
import subprocess
import sys
import unittest

SCRIPT_ROOT = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_ROOT not in sys.path:
    sys.path.insert(0, SCRIPT_ROOT)

from builders.builder import OutDirLock  # noqa: E402
from builders.esp32 import ESP32_GENERATE_LOCK_KEY, Esp32App, Esp32Board, Esp32Builder  # noqa: E402
from runner import PrintOnlyRunner  # noqa: E402


def build_expected_output(source: str, root: str, out: str) -> Iterator[str]:
    with open(os.path.join(SCRIPT_ROOT, source)) as f:
        for line in f.readlines():
            yield line.replace("{root}", root).replace("{out}", out)


def build_actual_output(root: str, out: str, args: list[str]) -> list[str]:
    # Fake out that we have a project root
    binary = os.path.join(SCRIPT_ROOT, 'build_examples.py')

    runenv = {}
    runenv.update(os.environ)
    runenv.update({
        'PW_PROJECT_ROOT': root,
        'ANDROID_NDK_HOME': 'TEST_ANDROID_NDK_HOME',
        'ANDROID_HOME': 'TEST_ANDROID_HOME',
        'TEST_ANDROID_SDK_MANAGER_PATH': 'TEST_ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager',
        'TIZEN_SDK_ROOT': 'TEST_TIZEN_SDK_ROOT',
        'TIZEN_SDK_SYSROOT': 'TEST_TIZEN_SDK_SYSROOT',
        'TELINK_ZEPHYR_SDK_DIR': 'TELINK_ZEPHYR_SDK_DIR',
        'SYSROOT_AARCH64': 'SYSROOT_AARCH64',
        'IMX_SDK_ROOT': 'IMX_SDK_ROOT',
        'TI_SYSCONFIG_ROOT': 'TEST_TI_SYSCONFIG_ROOT',
        'JAVA_HOME': 'TEST_JAVA_HOME',
        'GSDK_ROOT': 'TEST_GSDK_ROOT',
        'WIFI_SDK_ROOT': 'TEST_WIFI_SDK_ROOT',
    })

    retval = subprocess.run([
        binary,
        '--log-level', 'FATAL',
        '--dry-run',
        '--repo', root,
        '--out-prefix', out,
    ] + args, stdout=subprocess.PIPE, check=True, encoding='UTF-8', env=runenv)

    result = [line + '\n' for line in retval.stdout.split('\n')]

    # ensure a single terminating newline: easier to edit since autoformat
    # often strips ending double newlines on text files
    while result[-1] == '\n':
        result = result[:-1]

    return result


class TestBuilder(unittest.TestCase):

    def assertCommandOutput(self, expected_file: str, args: list[str]):
        ROOT = '/TEST/BUILD/ROOT'
        OUT = '/OUTPUT/DIR'

        expected = list(build_expected_output(expected_file, ROOT, OUT))
        actual = list(build_actual_output(ROOT, OUT, args))

        diffs = list(difflib.unified_diff(expected, actual))

        if diffs:
            reference = os.path.basename(expected_file) + '.actual'
            with open(reference, "w") as fo:
                for line in build_actual_output(ROOT, OUT, args):
                    fo.write(line.replace(ROOT, '{root}').replace(OUT, '{out}'))

            msg = f"DIFFERENCE between expected and generated output in {expected_file}\n"
            msg += f"Expected file can be found in {reference}"
            for line in diffs:
                msg += ("\n   " + line.replace(ROOT,
                                               '{root}').replace(OUT, '{out}').strip())
            self.fail(msg)

    @unittest.skipUnless(sys.platform == 'linux', 'Build on linux test')
    @unittest.skipUnless(os.uname().machine == 'x86_64', 'Validation x64 and crosscompile, requires linux x64')
    def test_linux64_targets(self):
        self.assertCommandOutput(
            os.path.join('testdata', 'all_targets_linux_x64.txt'),
            'targets'.split(' ')
        )

    def test_general_dry_runs(self):
        # A sampling of targets and variants to validate that
        # build options do not change too much
        TARGETS = [
            'esp32-devkitc-light-rpc',
            'android-arm64-chip-tool',
            'nrf-nrf52840dk-pump',
            'efr32-brd4187c-light-rpc',
        ]

        for target in TARGETS:
            expected = os.path.join('testdata', f'dry_run_{target}.txt')
            self.assertCommandOutput(expected, f'--target {target} --dry-run build'.split(' '))

    @unittest.skipUnless(sys.platform == 'linux', 'Build on linux test')
    @unittest.skipUnless(os.uname().machine == 'x86_64', 'Validation x64 and crosscompile, requires linux x64')
    def test_linux_dry_runs(self):
        TARGETS = [
            'linux-arm64-chip-tool-ipv6only-clang',
            'linux-arm64-ota-requestor-nodeps-ipv6only',
            'linux-x64-all-clusters-coverage',
        ]

        for target in TARGETS:
            expected = os.path.join('testdata', f'dry_run_{target}.txt')
            self.assertCommandOutput(expected, f'--target {target} --dry-run build'.split(' '))

    def test_esp32_generate_serializes_across_apps(self):
        class RecordingOutDirLock(OutDirLock):
            def __init__(self) -> None:
                super().__init__()
                self.locked_keys: list[str] = []

            @contextlib.contextmanager
            def lock_dir(self, dir_path: str | None) -> Iterator[None]:
                if dir_path is not None:
                    self.locked_keys.append(dir_path)
                with super().lock_dir(dir_path):
                    yield

        root = '/TEST/BUILD/ROOT'
        lock = RecordingOutDirLock()
        runner = PrintOnlyRunner(io.StringIO(), root=root)

        all_clusters = Esp32Builder(root=root, runner=runner, output_dir_lock=lock,
                                    board=Esp32Board.M5Stack, app=Esp32App.ALL_CLUSTERS)
        all_clusters.identifier = 'esp32-m5stack-all-clusters'
        all_clusters.output_dir = '/OUTPUT/DIR/esp32-m5stack-all-clusters'

        ota_requestor = Esp32Builder(root=root, runner=runner, output_dir_lock=lock,
                                     board=Esp32Board.M5Stack, app=Esp32App.OTA_REQUESTOR)
        ota_requestor.identifier = 'esp32-m5stack-ota-requestor'
        ota_requestor.output_dir = '/OUTPUT/DIR/esp32-m5stack-ota-requestor'

        all_clusters.generate()
        ota_requestor.generate()
        self.assertEqual(lock.locked_keys.count(ESP32_GENERATE_LOCK_KEY), 2)

        lock.locked_keys.clear()
        all_clusters._build()
        ota_requestor._build()
        self.assertNotIn(ESP32_GENERATE_LOCK_KEY, lock.locked_keys)
        self.assertIn(os.path.join(root, all_clusters.ExamplePath), lock.locked_keys)
        self.assertIn(os.path.join(root, ota_requestor.ExamplePath), lock.locked_keys)


if __name__ == '__main__':
    unittest.main()
