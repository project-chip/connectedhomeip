#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2022 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import unittest

from matter_yamltests.pseudo_clusters.pseudo_clusters import PseudoCluster, PseudoClusters


class MockStep:
    def __init__(self, cluster: str, command: str):
        self.cluster = cluster
        self.command = command


class MyCluster(PseudoCluster):
    name = 'MyCluster'

    async def MyCommand(self, request):
        pass

    async def MyCommandWithCustomSuccess(self, request):
        return 'CustomSuccess'


unsupported_cluster_step = MockStep('UnsupportedCluster', 'MyCommand')
unsupported_command_step = MockStep('MyCluster', 'UnsupportedCommand')
supported_step = MockStep('MyCluster', 'MyCommand')
supported_step_with_custom_success = MockStep(
    'MyCluster', 'MyCommandWithCustomSuccess')

default_failure = ({'error': 'FAILURE'}, [])
default_success = ({}, [])
custom_success = ('CustomSuccess', [])

clusters = PseudoClusters([MyCluster()])


class TestPseudoClusters(unittest.IsolatedAsyncioTestCase):
    def test_supports(self):
        self.assertFalse(clusters.supports(unsupported_cluster_step))
        self.assertFalse(clusters.supports(unsupported_command_step))
        self.assertTrue(clusters.supports(supported_step))
        self.assertTrue(clusters.supports(supported_step_with_custom_success))

    async def test_execute_return_value(self):
        self.assertEqual(await clusters.execute(unsupported_cluster_step), default_failure)
        self.assertEqual(await clusters.execute(unsupported_command_step), default_failure)
        self.assertEqual(await clusters.execute(supported_step), default_success)
        self.assertEqual(await clusters.execute(supported_step_with_custom_success), custom_success)


if __name__ == '__main__':
    unittest.main()
