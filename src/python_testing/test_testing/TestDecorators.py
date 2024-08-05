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


# Hooks:
# If this is a per-endpoint test
# - If test is run, hook object will get one test_start and one test_stop call per endpoint on which the test is run
# - If the test is skipped, hook object will get test_start, test_skipped, test_stop
# If this is a whole-node test:
# - test will always be run, so hook object will get test_start, test_stop
#
# You will get step_* calls as appropriate in between the test_start and test_stop calls if the test is not skipped.

import os
import sys

import chip.clusters as Clusters
from chip.clusters import Attribute

try:
    from matter_testing_support import (MatterBaseTest, async_test_body, get_accepted_endpoints_for_test, has_attribute,
                                        has_cluster, has_feature, per_endpoint_test, per_node_test)
except ImportError:
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
    from matter_testing_support import (MatterBaseTest, async_test_body, get_accepted_endpoints_for_test, has_attribute,
                                        has_cluster, has_feature, per_endpoint_test, per_node_test)

from typing import Optional

from mobly import asserts
from MockTestRunner import MockTestRunner


def get_clusters(endpoints: list[int]) -> Attribute.AsyncReadTransaction.ReadResponse:
    c = Clusters.OnOff
    attr = c.Attributes
    # We're JUST populating the globals here because that's all that matters for this particular test
    feature_map = c.Bitmaps.Feature.kLighting
    # Only supported attributes - globals and OnOff. This isn't a compliant device. Doesn't matter for this test.
    attribute_list = [attr.FeatureMap.attribute_id, attr.AttributeList.attribute_id,
                      attr.AcceptedCommandList.attribute_id, attr.GeneratedCommandList.attribute_id, attr.OnOff.attribute_id]
    accepted_commands = [c.Commands.Off, c.Commands.On]
    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    for e in endpoints:
        resp.attributes[e] = {c: {attr.FeatureMap: feature_map,
                                  attr.AttributeList: attribute_list, attr.AcceptedCommandList: accepted_commands}}
    return resp


class DecoratorTestRunnerHooks:
    def __init__(self):
        self.started = []
        self.skipped = []
        self.stopped = 0

    def start(self, count: int):
        pass

    def stop(self, duration: int):
        pass

    def test_start(self, filename: str, name: str, count: int, steps: list[str] = []):
        self.started.append(name)

    def test_skipped(self, filename: str, name: str):
        self.skipped.append(name)

    def test_stop(self, exception: Exception, duration: int):
        self.stopped += 1

    def step_skipped(self, name: str, expression: str):
        pass

    def step_start(self, name: str):
        pass

    def step_success(self, logger, logs, duration: int, request):
        pass

    def step_failure(self, logger, logs, duration: int, request, received):
        pass

    def step_unknown(self):
        pass

    def show_prompt(self,
                    msg: str,
                    placeholder: Optional[str] = None,
                    default_value: Optional[str] = None) -> None:
        pass


class TestDecorators(MatterBaseTest):
    def test_checkers(self):
        has_onoff = has_cluster(Clusters.OnOff)
        has_onoff_onoff = has_attribute(Clusters.OnOff.Attributes.OnOff)
        has_onoff_ontime = has_attribute(Clusters.OnOff.Attributes.OnTime)
        has_timesync = has_cluster(Clusters.TimeSynchronization)
        has_timesync_utc = has_attribute(Clusters.TimeSynchronization.Attributes.UTCTime)

        wildcard = get_clusters([0, 1])

        def check_endpoints(f, expect_true, expectation: str):
            asserts.assert_equal(f(wildcard, 0), expect_true, f"Expected {expectation} == {expect_true} on EP0")
            asserts.assert_equal(f(wildcard, 1), expect_true, f"Expected {expectation} == {expect_true} on EP1")
            asserts.assert_false(f(wildcard, 2), f"Expected {expectation} == False on EP2")

        check_endpoints(has_onoff, True, "OnOff Cluster")
        check_endpoints(has_onoff_onoff, True, "OnOff attribute")
        check_endpoints(has_onoff_ontime, False, "OnTime attribute")
        check_endpoints(has_timesync, False, "TimeSynchronization Cluster")
        check_endpoints(has_timesync_utc, False, "UTC attribute")

    @async_test_body
    async def test_endpoints(self):
        has_onoff = has_cluster(Clusters.OnOff)
        has_onoff_onoff = has_attribute(Clusters.OnOff.Attributes.OnOff)
        has_onoff_ontime = has_attribute(Clusters.OnOff.Attributes.OnTime)
        has_timesync = has_cluster(Clusters.TimeSynchronization)
        has_timesync_utc = has_attribute(Clusters.TimeSynchronization.Attributes.UTCTime)

        all_endpoints = await self.default_controller.Read(self.dut_node_id, [()])
        all_endpoints = list(all_endpoints.attributes.keys())

        msg = "Unexpected endpoint list returned"

        endpoints = await get_accepted_endpoints_for_test(self, has_onoff)
        asserts.assert_equal(endpoints, all_endpoints, msg)

        endpoints = await get_accepted_endpoints_for_test(self, has_onoff_onoff)
        asserts.assert_equal(endpoints, all_endpoints, msg)

        endpoints = await get_accepted_endpoints_for_test(self, has_onoff_ontime)
        asserts.assert_equal(endpoints, [], msg)

        endpoints = await get_accepted_endpoints_for_test(self, has_timesync)
        asserts.assert_equal(endpoints, [], msg)

        endpoints = await get_accepted_endpoints_for_test(self, has_timesync_utc)
        asserts.assert_equal(endpoints, [], msg)

    # This test should cause an assertion because it has pics_ method
    @per_node_test
    async def test_whole_node_with_pics(self):
        pass

    # This method returns the top level pics for test_whole_node_with_pics
    # It is used to test that test_whole_node_with_pics will fail since you can't have a whole node test gated on a PICS.
    def pics_whole_node_with_pics(self):
        return ['EXAMPLE.S']

    # This test should cause an assertion because it has a pics_ method
    @per_endpoint_test(has_cluster(Clusters.OnOff))
    async def test_per_endpoint_with_pics(self):
        pass

    # This method returns the top level pics for test_per_endpoint_with_pics
    # It is used to test that test_per_endpoint_with_pics will fail since you can't have a per endpoint test gated on a PICS.
    def pics_per_endpoint_with_pics(self):
        return ['EXAMPLE.S']

    # This test should be run once
    @per_node_test
    async def test_whole_node_ok(self):
        pass

    # This test should be run once per endpoint
    @per_endpoint_test(has_cluster(Clusters.OnOff))
    async def test_endpoint_cluster_yes(self):
        pass

    # This test should be skipped since this cluster isn't on any endpoint
    @per_endpoint_test(has_cluster(Clusters.TimeSynchronization))
    async def test_endpoint_cluster_no(self):
        pass

    # This test should be run once per endpoint
    @per_endpoint_test(has_attribute(Clusters.OnOff.Attributes.OnOff))
    async def test_endpoint_attribute_yes(self):
        pass

    # This test should be skipped since this attribute isn't on the supported cluster
    @per_endpoint_test(has_attribute(Clusters.OnOff.Attributes.OffWaitTime))
    async def test_endpoint_attribute_supported_cluster_no(self):
        pass

    # This test should be skipped since this attribute is part of an unsupported cluster
    @per_endpoint_test(has_attribute(Clusters.TimeSynchronization.Attributes.Granularity))
    async def test_endpoint_attribute_unsupported_cluster_no(self):
        pass

    # This test should be run once per endpoint
    @per_endpoint_test(has_feature(Clusters.OnOff, Clusters.OnOff.Bitmaps.Feature.kLighting))
    async def test_endpoint_feature_yes(self):
        pass

    # This test should be skipped since this attribute is part of an unsupported cluster
    @per_endpoint_test(has_feature(Clusters.TimeSynchronization, Clusters.TimeSynchronization.Bitmaps.Feature.kNTPClient))
    async def test_endpoint_feature_unsupported_cluster_no(self):
        pass

    # This test should be run since both are present
    @per_endpoint_test(has_attribute(Clusters.OnOff.Attributes.OnOff) and has_cluster(Clusters.OnOff))
    async def test_endpoint_boolean_yes(self):
        pass

    # This test should be skipped since we have an OnOff cluster, but no Time sync
    @per_endpoint_test(has_cluster(Clusters.OnOff) and has_cluster(Clusters.TimeSynchronization))
    async def test_endpoint_boolean_no(self):
        pass

    @per_endpoint_test(has_cluster(Clusters.OnOff))
    async def test_fail_on_ep0(self):
        if self.matter_test_config.endpoint == 0:
            asserts.fail("Expected failure")

    @per_endpoint_test(has_cluster(Clusters.OnOff))
    async def test_fail_on_ep1(self):
        if self.matter_test_config.endpoint == 1:
            asserts.fail("Expected failure")

    @per_node_test
    async def test_fail_on_whole_node(self):
        asserts.fail("Expected failure")


def main():
    failures = []
    hooks = DecoratorTestRunnerHooks()
    test_runner = MockTestRunner('TestDecorators.py', 'TestDecorators', 'test_checkers')
    read_resp = get_clusters([0, 1])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append("Test case failure: test_checkers")

    test_runner.set_test('TestDecorators.py', 'TestDecorators', 'test_endpoints')
    read_resp = get_clusters([0, 1])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append("Test case failure: test_endpoints")

    read_resp = get_clusters([0])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append("Test case failure: test_endpoints")

    test_name = 'test_whole_node_with_pics'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    test_name = 'test_per_endpoint_with_pics'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    # Test should run once for the whole node, regardless of the number of endpoints
    def run_check(test_name: str, read_response: Attribute.AsyncReadTransaction.ReadResponse, expected_runs: int, expect_skip: bool) -> None:
        nonlocal failures
        test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
        hooks = DecoratorTestRunnerHooks()
        ok = test_runner.run_test_with_mock_read(read_response, hooks)
        started_ok = len(hooks.started) == expected_runs
        skipped_ok = (hooks.skipped != []) == expect_skip
        stopped_ok = hooks.stopped == expected_runs
        if not ok or not started_ok or not skipped_ok or not stopped_ok:
            failures.append(
                f'Expected {expected_runs} run of {test_name}, skips expected: {expect_skip}. Runs: {hooks.started}, skips: {hooks.skipped} stops: {hooks.stopped}')

    def check_once_per_node(test_name: str):
        run_check(test_name, get_clusters([0]), 1, False)
        run_check(test_name, get_clusters([0, 1]), 1, False)

    def check_once_per_endpoint(test_name: str):
        run_check(test_name, get_clusters([0]), 1, False)
        run_check(test_name, get_clusters([0, 1]), 2, False)

    def check_skipped(test_name: str):
        run_check(test_name, get_clusters([0]), 1, True)
        run_check(test_name, get_clusters([0, 1]), 1, True)

    check_once_per_node('test_whole_node_ok')
    check_once_per_endpoint('test_endpoint_cluster_yes')
    check_skipped('test_endpoint_cluster_no')
    check_once_per_endpoint('test_endpoint_attribute_yes')
    check_skipped('test_endpoint_attribute_supported_cluster_no')
    check_skipped('test_endpoint_attribute_unsupported_cluster_no')
    check_once_per_endpoint('test_endpoint_feature_yes')
    check_skipped('test_endpoint_feature_unsupported_cluster_no')
    check_once_per_endpoint('test_endpoint_boolean_yes')
    check_skipped('test_endpoint_boolean_no')

    test_name = 'test_fail_on_ep0'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0, 1])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    test_name = 'test_fail_on_ep1'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0, 1])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    test_name = 'test_fail_on_ep1'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append(f"Unexpected failure on {test_name}")

    test_name = 'test_fail_on_whole_node'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0, 1])
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    test_runner.Shutdown()
    print(
        f"Test of Decorators: test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
