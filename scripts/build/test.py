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

import difflib
import os
import subprocess
import sys
import unittest
from typing import List

SCRIPT_ROOT = os.path.dirname(__file__)


def build_expected_output(source: str, root: str, out: str) -> List[str]:
    with open(os.path.join(SCRIPT_ROOT, source), 'rt') as f:
        for line in f.readlines():
            yield line.replace("{root}", root).replace("{out}", out)


def build_actual_output(root: str, out: str, args: List[str]) -> List[str]:
    # Fake out that we have a project root
    binary = os.path.join(SCRIPT_ROOT, 'build_examples.py')

    runenv = {}
    runenv.update(os.environ)
    runenv.update({
        'PW_PROJECT_ROOT': root,
        'ANDROID_NDK_HOME': 'TEST_ANDROID_NDK_HOME',
        'ANDROID_HOME': 'TEST_ANDROID_HOME',
        'TIZEN_SDK_ROOT': 'TEST_TIZEN_SDK_ROOT',
        'TIZEN_SDK_SYSROOT': 'TEST_TIZEN_SDK_SYSROOT',
        'TELINK_ZEPHYR_SDK_DIR': 'TELINK_ZEPHYR_SDK_DIR',
        'SYSROOT_AARCH64': 'SYSROOT_AARCH64',
        'NXP_K32W0_SDK_ROOT': 'TEST_NXP_K32W0_SDK_ROOT',
        'IMX_SDK_ROOT': 'IMX_SDK_ROOT',
        'TI_SYSCONFIG_ROOT': 'TEST_TI_SYSCONFIG_ROOT',
        'JAVA_HOME': 'TEST_JAVA_HOME',
        'GSDK_ROOT': 'TEST_GSDK_ROOT',
        'WISECONNECT_SDK_ROOT': 'TEST_WISECONNECT_SDK_ROOT',
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

    def assertCommandOutput(self, expected_file: str, args: List[str]):
        ROOT = '/TEST/BUILD/ROOT'
        OUT = '/OUTPUT/DIR'

        expected = [line for line in build_expected_output(expected_file, ROOT, OUT)]
        actual = [line for line in build_actual_output(ROOT, OUT, args)]

        diffs = [line for line in difflib.unified_diff(expected, actual)]

        if diffs:
            reference = os.path.basename(expected_file) + '.actual'
            with open(reference, 'wt') as fo:
                for line in build_actual_output(ROOT, OUT, args):
                    fo.write(line.replace(ROOT, '{root}').replace(OUT, '{out}'))

            msg = "DIFFERENCE between expected and generated output in %s\n" % expected_file
            msg += "Expected file can be found in %s" % reference
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
            'efr32-brd4187c-light-rpc-no-version',
            'openiotsdk-lock-mbedtls',
            'openiotsdk-shell-mbedtls'
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


if __name__ == '__main__':
    unittest.main()
