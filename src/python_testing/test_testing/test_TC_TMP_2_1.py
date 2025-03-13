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
import typing
from dataclasses import dataclass
from pathlib import Path

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.clusters.Types import NullValue
from chip.testing.runner import MockTestRunner


@dataclass
class TestSpec():
    min: typing.Optional[int]
    max: typing.Optional[int]
    measured: int
    tolerance: int
    expect_pass: bool


TEST_CASES = [
    # ==============================
    # Measured Test cases
    # ==============================
    # --------
    # Neither min nor max specified
    # --------
    # Measured lowest
    TestSpec(NullValue, NullValue, -27315, 10, True),
    # Measured highest
    TestSpec(NullValue, NullValue, 32767, 10, True),
    # Measured below
    TestSpec(NullValue, NullValue, -27316, 10, False),
    # Measured above
    TestSpec(NullValue, NullValue, 32768, 10, False),
    # Measured null
    TestSpec(NullValue, NullValue, NullValue, 10, True),

    # --------
    # min only
    # --------
    # Measured lowest
    TestSpec(5, NullValue, 5, 10, True),
    # Measured highest
    TestSpec(5, NullValue, 32767, 10, True),
    # Measured below
    TestSpec(5, NullValue, 4, 10, False),
    # Measured above
    TestSpec(5, NullValue, 32768, 10, False),
    # Measured null
    TestSpec(5, NullValue, NullValue, 10, True),

    # --------
    # max only
    # --------
    # Measured lowest
    TestSpec(NullValue, 5, -27315, 10, True),
    # Measured highest
    TestSpec(NullValue, 5, 5, 10, True),
    # Measured below
    TestSpec(NullValue, 5, -27316, 10, False),
    # Measured above
    TestSpec(NullValue, 5, 6, 10, False),
    # Measured null
    TestSpec(NullValue, 5, NullValue, 10, True),

    # --------
    # both
    # --------
    # Measured lowest
    TestSpec(-5, 5, -5, 10, True),
    # Measured highest
    TestSpec(-5, 5, 5, 10, True),
    # Measured below
    TestSpec(-5, 5, -6, 10, False),
    # Measured above
    TestSpec(-5, 5, 6, 10, False),
    # Measured null
    TestSpec(-5, 5, NullValue, 10, True),

    # ==============================
    # Min Test cases
    # ==============================
    # Max not specified, min OK bottom
    TestSpec(-27315, NullValue, 0, 10, True),
    # Max not specified, min OK top
    TestSpec(32766, NullValue, 32767, 10, True),
    # Max not specified, min out of range below
    TestSpec(-27316, NullValue, 0, 10, False),
    # Max not specified, min out of range above
    TestSpec(32767, NullValue, 32767, 10, False),
    # Max specified, min OK bottom
    TestSpec(-27315, 5, 0, 10, True),
    # Max specified, min OK top
    TestSpec(4, 5, 4, 10, True),
    # Max specified, min out of range below
    TestSpec(-27316, 5, 0, 10, False),
    # Max specified, min out of range above
    TestSpec(5, 5, 5, 10, False),

    # ==============================
    # Min Test cases
    # ==============================
    # min not specified, max OK bottom
    TestSpec(NullValue, -27314, -27315, 10, True),
    # min not specified, max OK top
    TestSpec(NullValue, 32767, 0, 10, True),
    # min not specified, max out of range bottom
    TestSpec(NullValue, -27315, -27315, 10, False),
    # min not specified, max out of range top
    TestSpec(NullValue, 32768, 0, 10, False),
    # min specified, max OK bottom
    TestSpec(0, 1, 0, 10, True),
    # min specified, max OK top
    TestSpec(0, 32767, 0, 10, True),
    # min specified, max out of range bottom
    TestSpec(0, 0, 0, 10, False),
    # min specified, max out of range top
    TestSpec(0, 32768, 0, 10, False),

    # ==============================
    # Tolerance test cases
    # ==============================
    # Tolerance OK bottom
    TestSpec(NullValue, NullValue, 0, 0, True),
    # Tolerance OK top
    TestSpec(NullValue, NullValue, 0, 2048, True),
    # Tolerance out of range bottom
    TestSpec(NullValue, NullValue, 0, -1, False),
    # Tolerance out of range top
    TestSpec(NullValue, NullValue, 0, 2049, False),

]


def test_spec_to_attribute_cache(test_spec: TestSpec) -> Attribute.AsyncReadTransaction.ReadResponse:
    c = Clusters.TemperatureMeasurement
    attr = Clusters.TemperatureMeasurement.Attributes
    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {1: {c: {attr.MaxMeasuredValue: test_spec.max,
                               attr.MinMeasuredValue: test_spec.min, attr.MeasuredValue: test_spec.measured, attr.Tolerance: test_spec.tolerance}}}
    return resp


def main():
    test_runner = MockTestRunner(Path(__file__).parent / '../TC_TMP_2_1.py',
                                 'TC_TMP_2_1', 'test_TC_TMP_2_1', 1)
    failures = []
    for idx, t in enumerate(TEST_CASES):
        ok = test_runner.run_test_with_mock_read(test_spec_to_attribute_cache(t)) == t.expect_pass
        if not ok:
            failures.append(f"Measured test case failure: {idx} {t}")

    test_runner.Shutdown()
    print(
        f"Test of tests: run {len(TEST_CASES)}, test response correct: {len(TEST_CASES) - len(failures)} test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
