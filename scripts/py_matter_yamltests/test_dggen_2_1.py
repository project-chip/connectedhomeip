# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import copy
import tempfile
import unittest
from pathlib import Path

import yaml

from matter.yamltests.definitions import ParseSource, SpecDefinitions
from matter.yamltests.parser import TestParser, TestParserConfig
from matter.yamltests.pics_checker import PICSChecker


class TestGeneralDiagnosticsCertification(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[2]
        path = root / 'src/app/tests/suites/certification/Test_TC_DGGEN_2_1.yaml'
        cls.document = yaml.safe_load(path.read_text(encoding='utf-8'))
        xml_root = root / 'src/app/zap-templates/zcl/data-model/chip'
        cls.definitions = SpecDefinitions([
            ParseSource(source=str(xml_root / name))
            for name in ('global-attributes.xml', 'general-diagnostics-cluster.xml')
        ])

    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.directory = Path(directory.name)
        self.pics = self.directory / 'pics.txt'
        self.pics.write_text(
            '\n'.join(f'DGGEN.S.A{index:04X}=1' for index in range(9)), encoding='utf-8')

    def _attribute_steps(self, attribute: str) -> list[dict]:
        return [step for step in self.document['tests'] if step.get('attribute') == attribute]

    def _accepts(self, step: dict, value: object, **variables) -> bool:
        config = copy.deepcopy(self.document['config'])
        config.update(boot_count1=0, uptime1=25,
                      uptime2=25, operational_hours1=5)
        config.update(variables)
        fragment = {'name': 'DGGEN 回归测试', 'config': config,
                    'tests': [copy.deepcopy(step)]}
        path = self.directory / 'test.yaml'
        path.write_text(yaml.safe_dump(fragment), encoding='utf-8')
        parser = TestParser(str(path), TestParserConfig(
            pics=str(self.pics), definitions=self.definitions))
        request = next(iter(parser.tests))
        self.assertTrue(request.is_pics_enabled)
        return not request.post_process_response({'value': value}).is_failure()

    def test_operational_hours_must_increase(self):
        step = self._attribute_steps('TotalOperationalHours')[1]
        for value, accepted in ((4, False), (5, False), (6, True), (7, True)):
            with self.subTest(value=value):
                self.assertEqual(self._accepts(step, value), accepted)

    def test_uptime_must_increase(self):
        step = self._attribute_steps('UpTime')[1]
        for value, accepted in ((24, False), (25, False), (26, True), (35, True)):
            with self.subTest(value=value):
                self.assertEqual(self._accepts(step, value), accepted)

    def test_uptime_must_decrease_after_reboot(self):
        step = self._attribute_steps('UpTime')[2]
        for value, accepted in ((0, True), (24, True), (25, False), (26, False)):
            with self.subTest(value=value):
                self.assertEqual(self._accepts(step, value), accepted)

    def test_nonnullable_attributes_reject_null(self):
        for step in self.document['tests']:
            if step.get('command') not in ('readAttribute', 'subscribeAttribute'):
                continue
            with self.subTest(attribute=step['attribute'], label=step['label']):
                self.assertFalse(self._accepts(step, None))

    def test_fault_lists_validate_every_enum(self):
        for attribute, maximum in (('ActiveHardwareFaults', 10), ('ActiveRadioFaults', 6), ('ActiveNetworkFaults', 3)):
            for step in self._attribute_steps(attribute):
                for value in ([], [0], [maximum], [0, maximum], list(range(maximum + 1))):
                    with self.subTest(attribute=attribute, command=step['command'], value=value):
                        self.assertTrue(self._accepts(step, value))
                for value in ([-1], [maximum + 1], [255], [0, 255], [None], [True], [0.5], ['0'],
                              [*range(maximum + 1), 0]):
                    with self.subTest(attribute=attribute, command=step['command'], value=value):
                        self.assertFalse(self._accepts(step, value))

    def test_network_interfaces_have_an_automatic_read(self):
        steps = self._attribute_steps('NetworkInterfaces')
        self.assertEqual(len(steps), 1)
        interface = {'Name': 'eth0', 'IsOperational': True, 'OffPremiseServicesReachableIPv4': None,
                     'OffPremiseServicesReachableIPv6': None, 'HardwareAddress': bytes(6),
                     'IPv4Addresses': [], 'IPv6Addresses': [], 'Type': 2}
        for value, accepted in (([], True), ([interface] * 8, True), ([interface] * 9, False), (None, False)):
            with self.subTest(length=len(value) if value is not None else None):
                self.assertEqual(self._accepts(steps[0], value), accepted)

    def test_operational_hours_accept_the_uint32_range(self):
        step = self._attribute_steps('TotalOperationalHours')[0]
        for value, accepted in ((0, True), (0xFFFFFFFF, True), (-1, False), (0x100000000, False)):
            with self.subTest(value=value):
                self.assertEqual(self._accepts(step, value), accepted)

    def test_optional_attribute_operations_follow_pics(self):
        steps = self.document['tests']
        groups = (
            ('DGGEN.S.A0004', next(i for i, step in enumerate(steps) if step['label'].startswith('Step 5a:')),
             next(i for i, step in enumerate(steps) if step['label'].startswith('Step 6a:'))),
            ('DGGEN.S.A0003', next(i for i, step in enumerate(steps)
             if step['label'].startswith('Step 10a:')), len(steps)),
        )
        for attribute, start, stop in groups:
            for automated in (False, True):
                for supported in (False, True):
                    self.pics.write_text(f'{attribute}={int(supported)}\nPICS_SDK_CI_ONLY={int(automated)}\n'
                                         f'PICS_USER_PROMPT={int(not automated)}\n', encoding='utf-8')
                    checker = PICSChecker(str(self.pics))
                    commands = [step['command'] for step in steps[start:stop] if checker.check(
                        step.get('PICS'))]
                    with self.subTest(attribute=attribute, automated=automated, supported=supported):
                        if supported:
                            self.assertIn('readAttribute', commands)
                            self.assertIn(
                                'Reboot' if automated else 'UserPrompt', commands)
                            if attribute == 'DGGEN.S.A0003':
                                self.assertIn('PairWithCode', commands)
                                if automated:
                                    self.assertIn('FactoryReset', commands)
                        else:
                            self.assertEqual(commands, [])


if __name__ == '__main__':
    unittest.main()
