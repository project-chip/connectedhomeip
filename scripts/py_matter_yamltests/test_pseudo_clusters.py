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

from matter.yamltests.pseudo_clusters.clusters.value_commands import ValueCommands
from matter.yamltests.pseudo_clusters.pseudo_clusters import PseudoCluster, PseudoClusters


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


class MockStepWithArguments(MockStep):
    def __init__(self, cluster: str, command: str, values: list):
        super().__init__(cluster, command)
        self.arguments = {'values': values}


value_clusters = PseudoClusters([ValueCommands()])


class TestValueCommands(unittest.IsolatedAsyncioTestCase):
    async def test_unsigned_number_value_returns_the_argument(self):
        for value in (0, 1, 254):
            step = MockStepWithArguments(
                'ValueCommands', 'UnsignedNumberValue', [{'name': 'Value', 'value': value}])
            status, _ = await value_clusters.execute(step)
            self.assertEqual(status, {'value': {'Value': value}})


if __name__ == '__main__':
    unittest.main()
