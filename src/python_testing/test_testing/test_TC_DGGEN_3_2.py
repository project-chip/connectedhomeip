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

import sys
from dataclasses import dataclass
from pathlib import Path

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.runner import MockTestRunner


@dataclass
class TestSpec():
    max_paths: int
    dmtest_feature_map: int
    expect_pass: bool


TEST_CASES = [
    TestSpec(1, 0, True),
    TestSpec(1, 1, True),
    TestSpec(2, 0, False),
    TestSpec(2, 1, True),
]


def test_spec_to_attribute_cache(test_spec: TestSpec) -> Attribute.AsyncReadTransaction.ReadResponse:
    bi = Clusters.BasicInformation
    bi_attr = bi.Attributes
    gd = Clusters.GeneralDiagnostics
    gd_attr = gd.Attributes
    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {0: {bi: {bi_attr.MaxPathsPerInvoke: test_spec.max_paths},
                           gd: {gd_attr.FeatureMap: test_spec.dmtest_feature_map}}}
    return resp


def main():
    test_runner = MockTestRunner(Path(__file__).parent / '../TC_DGGEN_3_2.py',
                                 'TC_DGGEN_3_2', 'test_TC_DGGEN_3_2', 0)
    failures = []
    for idx, t in enumerate(TEST_CASES):
        ok = test_runner.run_test_with_mock_read(test_spec_to_attribute_cache(t)) == t.expect_pass
        if not ok:
            failures.append(f"Test case failure: {idx} {t}")

    test_runner.Shutdown()
    print(
        f"Test of tests: run {len(TEST_CASES)}, test response correct: {len(TEST_CASES) - len(failures)} test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
