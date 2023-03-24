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
"""
Test for GN Java build rules. This test should be executed using ninja.
"""

import json
import os
import unittest
from os import path


class JavaBuildTest(unittest.TestCase):
    chip_root = os.getenv('PW_PROJECT_ROOT')
    local_test_dir = '/build/chip/java/tests'
    test_dir = chip_root + local_test_dir

    jars_dir = 'lib' + local_test_dir
    configs_dir = 'gen' + local_test_dir

    # Target names in the BUILD.gn
    targets_to_check = [
        'java_library',
        'child_library',
        'child_library_2',
        'grandchild_library',
    ]
    prebuilt_targets_to_check = ['java_prebuilt', 'child_prebuilt']

    def testExpectedJarsCreated(self):
        jars_dir = JavaBuildTest.jars_dir
        for target in JavaBuildTest.targets_to_check:
            self.assertTrue(path.exists(jars_dir + '/' + target + '.jar'))

        # Prebuilt jars should have been copied to the output directory.
        self.assertTrue(path.exists(jars_dir + '/prebuilt_jar.jar'))
        self.assertTrue(path.exists(jars_dir + '/child_jar.jar'))

    def testBuildConfigMatchesExpected(self):
        self.maxDiff = None
        configs_dir = JavaBuildTest.configs_dir
        expected_dir = JavaBuildTest.test_dir + '/expected_output'

        for target in (JavaBuildTest.targets_to_check +
                       JavaBuildTest.prebuilt_targets_to_check):
            with open(expected_dir + '/' + target + '_expected.json',
                      'r') as expected_config, open(
                          configs_dir + '/' + target + '.json',
                          'r') as actual_config:
                expected_json = json.load(expected_config)['deps_info']
                actual_json = json.load(actual_config)['deps_info']

                self.assertEqual(expected_json['name'], actual_json['name'])
                self.assertEqual(expected_json['jar_path'],
                                 actual_json['jar_path'])
                self.assertCountEqual(expected_json['deps_configs'],
                                      actual_json['deps_configs'])
                self.assertCountEqual(expected_json['deps_jars'],
                                      actual_json['deps_jars'])


if __name__ == '__main__':
    unittest.main()
