#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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

import string
from dataclasses import dataclass
from pathlib import Path

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.runner import MockTestRunner

c = Clusters.IcdManagement
attr = c.Attributes
uat = c.Bitmaps.UserActiveModeTriggerBitmap


@dataclass
class ICDMData():
    FeatureMap: int
    IdleModeDuration: int
    ActiveModeDuration: int
    ActiveModeThreshold: int
    RegisteredClients: list
    ICDCounter: int
    ClientsSupportedPerFabric: int
    UserActiveModeTriggerHint: int
    UserActiveModeTriggerInstruction: string
    OperatingMode: c.Enums.OperatingModeEnum
    MaximumCheckInBackOff: int
    expect_pass: bool


def test_spec_to_attribute_cache(test_icdm: ICDMData) -> Attribute.AsyncReadTransaction.ReadResponse:
    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {0: {c: {attr.FeatureMap: test_icdm.FeatureMap, attr.IdleModeDuration: test_icdm.IdleModeDuration, attr.ActiveModeDuration: test_icdm.ActiveModeDuration, attr.ActiveModeThreshold: test_icdm.ActiveModeThreshold,
                               attr.RegisteredClients: test_icdm.RegisteredClients, attr.ICDCounter: test_icdm.ICDCounter,
                               attr.ClientsSupportedPerFabric: test_icdm.ClientsSupportedPerFabric, attr.UserActiveModeTriggerHint: test_icdm.UserActiveModeTriggerHint,
                               attr.UserActiveModeTriggerInstruction: test_icdm.UserActiveModeTriggerInstruction, attr.OperatingMode: test_icdm.OperatingMode, attr.MaximumCheckInBackOff: test_icdm.MaximumCheckInBackOff}}}
    return resp


def run_tests(pics, label, test_cases, test_name):
    test_runner = MockTestRunner(Path(__file__).parent / f"../{label}.py",
                                 label, test_name, 0, pics)
    failures = []
    for idx, t in enumerate(test_cases):
        ok = test_runner.run_test_with_mock_read(
            test_spec_to_attribute_cache(t)) == t.expect_pass
        if not ok:
            failures.append(f"Measured test case failure: {idx} {t}")

    test_runner.Shutdown()
    print(
        f"Test of tests: run {len(test_cases)}, test response correct: {len(test_cases) - len(failures)} | test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0
