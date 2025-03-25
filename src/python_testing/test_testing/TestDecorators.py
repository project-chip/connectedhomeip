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

import sys
from pathlib import Path
from typing import Optional

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.matter_testing import (MatterBaseTest, MatterTestConfig, async_test_body, has_attribute, has_cluster, has_feature,
                                         run_if_endpoint_matches, run_on_singleton_matching_endpoint, should_run_test_on_endpoint)
from chip.testing.runner import MockTestRunner
from mobly import asserts


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

        msg = "Unexpected evaluation of should_run_test_on_endpoint"
        for e in all_endpoints:
            self.matter_test_config.endpoint = e
            should_run = await should_run_test_on_endpoint(self, has_onoff)
            asserts.assert_true(should_run, msg)

            should_run = await should_run_test_on_endpoint(self, has_onoff_onoff)
            asserts.assert_true(should_run, msg)

            should_run = await should_run_test_on_endpoint(self, has_onoff_ontime)
            asserts.assert_false(should_run, msg)

            should_run = await should_run_test_on_endpoint(self, has_timesync)
            asserts.assert_false(should_run, msg)

            should_run = await should_run_test_on_endpoint(self, has_timesync_utc)
            asserts.assert_false(should_run, msg)

    # This test should be run once per endpoint
    @run_if_endpoint_matches(has_cluster(Clusters.OnOff))
    async def test_endpoint_cluster_yes(self):
        pass

    # This test should be skipped since this cluster isn't on any endpoint
    @run_if_endpoint_matches(has_cluster(Clusters.TimeSynchronization))
    async def test_endpoint_cluster_no(self):
        pass

    # This test should be run once per endpoint
    @run_if_endpoint_matches(has_attribute(Clusters.OnOff.Attributes.OnOff))
    async def test_endpoint_attribute_yes(self):
        pass

    # This test should be skipped since this attribute isn't on the supported cluster
    @run_if_endpoint_matches(has_attribute(Clusters.OnOff.Attributes.OffWaitTime))
    async def test_endpoint_attribute_supported_cluster_no(self):
        pass

    # This test should be skipped since this attribute is part of an unsupported cluster
    @run_if_endpoint_matches(has_attribute(Clusters.TimeSynchronization.Attributes.Granularity))
    async def test_endpoint_attribute_unsupported_cluster_no(self):
        pass

    # This test should be run once per endpoint
    @run_if_endpoint_matches(has_feature(Clusters.OnOff, Clusters.OnOff.Bitmaps.Feature.kLighting))
    async def test_endpoint_feature_yes(self):
        pass

    # This test should be skipped since this attribute is part of an unsupported cluster
    @run_if_endpoint_matches(has_feature(Clusters.TimeSynchronization, Clusters.TimeSynchronization.Bitmaps.Feature.kNTPClient))
    async def test_endpoint_feature_unsupported_cluster_no(self):
        pass

    # This test should be run since both are present
    @run_if_endpoint_matches(has_attribute(Clusters.OnOff.Attributes.OnOff) and has_cluster(Clusters.OnOff))
    async def test_endpoint_boolean_yes(self):
        pass

    # This test should be skipped since we have an OnOff cluster, but no Time sync
    @run_if_endpoint_matches(has_cluster(Clusters.OnOff) and has_cluster(Clusters.TimeSynchronization))
    async def test_endpoint_boolean_no(self):
        pass

    @run_if_endpoint_matches(has_cluster(Clusters.OnOff))
    async def test_fail_on_ep0(self):
        if self.matter_test_config.endpoint == 0:
            asserts.fail("Expected failure")

    @run_if_endpoint_matches(has_cluster(Clusters.OnOff))
    async def test_fail_on_ep1(self):
        if self.matter_test_config.endpoint == 1:
            asserts.fail("Expected failure")

    @run_on_singleton_matching_endpoint(has_cluster(Clusters.OnOff))
    async def test_run_on_singleton_matching_endpoint(self):
        pass

    @run_on_singleton_matching_endpoint(has_cluster(Clusters.OnOff))
    async def test_run_on_singleton_matching_endpoint_failure(self):
        asserts.fail("Expected failure")

    @run_on_singleton_matching_endpoint(has_attribute(Clusters.OnOff.Attributes.OffWaitTime))
    async def test_no_run_on_singleton_matching_endpoint(self):
        pass


def main():
    failures = []
    hooks = DecoratorTestRunnerHooks()
    test_runner = MockTestRunner(Path(__file__).parent / 'TestDecorators.py',
                                 'TestDecorators', 'test_checkers')
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

    # Test should run once for the whole node, regardless of the number of endpoints
    def run_check(test_name: str, read_response: Attribute.AsyncReadTransaction.ReadResponse, expect_skip: bool) -> None:
        nonlocal failures
        test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
        hooks = DecoratorTestRunnerHooks()
        num_endpoints = 2
        for e in [0, 1]:
            test_runner.set_test_config(MatterTestConfig(endpoint=e))
            ok = test_runner.run_test_with_mock_read(read_response, hooks)
        started_ok = len(hooks.started) == num_endpoints
        expected_num_skips = 2 if expect_skip else 0
        skipped_ok = len(hooks.skipped) == expected_num_skips
        stopped_ok = hooks.stopped == num_endpoints
        if not ok or not started_ok or not skipped_ok or not stopped_ok:
            failures.append(
                f'Expected {num_endpoints} run of {test_name}, skips expected: {expect_skip}. Runs: {hooks.started}, skips: {hooks.skipped} stops: {hooks.stopped}')

    def check_once_per_endpoint(test_name: str):
        run_check(test_name, get_clusters([0, 1]), False)

    def check_all_skipped(test_name: str):
        run_check(test_name, get_clusters([0, 1]), True)

    check_once_per_endpoint('test_endpoint_cluster_yes')
    check_all_skipped('test_endpoint_cluster_no')
    check_once_per_endpoint('test_endpoint_attribute_yes')
    check_all_skipped('test_endpoint_attribute_supported_cluster_no')
    check_all_skipped('test_endpoint_attribute_unsupported_cluster_no')
    check_once_per_endpoint('test_endpoint_feature_yes')
    check_all_skipped('test_endpoint_feature_unsupported_cluster_no')
    check_once_per_endpoint('test_endpoint_boolean_yes')
    check_all_skipped('test_endpoint_boolean_no')

    test_name = 'test_fail_on_ep0'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0, 1])
    # fail on EP0, pass on EP1
    test_runner.set_test_config(MatterTestConfig(endpoint=0))
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")
    test_runner.set_test_config(MatterTestConfig(endpoint=1))
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append(f"Unexpected failure on {test_name}")

    test_name = 'test_fail_on_ep1'
    test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
    read_resp = get_clusters([0, 1])
    # pass on EP0, fail on EP1
    test_runner.set_test_config(MatterTestConfig(endpoint=0))
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if not ok:
        failures.append(f"Unexpected failure on {test_name}")
    test_runner.set_test_config(MatterTestConfig(endpoint=1))
    ok = test_runner.run_test_with_mock_read(read_resp, hooks)
    if ok:
        failures.append(f"Did not get expected test assertion on {test_name}")

    def run_singleton_dynamic(test_name: str, cluster_list: list[int]) -> tuple[bool, DecoratorTestRunnerHooks]:
        nonlocal failures
        read_resp = get_clusters(cluster_list)
        test_runner.set_test('TestDecorators.py', 'TestDecorators', test_name)
        test_runner.set_test_config(MatterTestConfig(endpoint=2))
        hooks = DecoratorTestRunnerHooks()
        ok = test_runner.run_test_with_mock_read(read_resp, hooks)
        # for all tests, we need to ensure the endpoint was set back to the prior values
        if test_runner.config.endpoint != 2:
            failures.append(f"Dynamic tests {test_name} with clusters {cluster_list} did not set endpoint back to prior")
        # All tests should have a start and a stop
        started_ok = len(hooks.started) == 1
        stopped_ok = hooks.stopped == 1
        if not started_ok or not stopped_ok:
            failures.append(
                f'Hooks failure on {test_name}, Runs: {hooks.started}, skips: {hooks.skipped} stops: {hooks.stopped}')
        return ok, hooks

    def expect_success_dynamic(test_name: str, cluster_list: list[int]):
        ok, hooks = run_singleton_dynamic(test_name, cluster_list)
        if not ok:
            failures.append(f"Unexpected failure on {test_name} with cluster list {cluster_list}")
        if hooks.skipped:
            failures.append(f'Unexpected skip call on {test_name} with cluster list {cluster_list}')

    def expect_failure_dynamic(test_name: str, cluster_list: list[int]):
        ok, hooks = run_singleton_dynamic(test_name, cluster_list)
        if ok:
            failures.append(f"Unexpected success on {test_name} with cluster list {cluster_list}")
        if hooks.skipped:
            # We don't expect a skip call because the test actually failed.
            failures.append(f'Skip called for {test_name} with cluster list {cluster_list}')

    def expect_skip_dynamic(test_name: str, cluster_list: list[int]):
        ok, hooks = run_singleton_dynamic(test_name, cluster_list)
        if not ok:
            failures.append(f"Unexpected failure on {test_name} with cluster list {cluster_list}")
        if not hooks.skipped:
            # We don't expect a skip call because the test actually failed.
            failures.append(f'Skip not called for {test_name} with cluster list {cluster_list}')

    test_name = 'test_run_on_singleton_matching_endpoint'
    expect_success_dynamic(test_name, [0])
    expect_success_dynamic(test_name, [1])
    # expect failure because there is more than 1 endpoint
    expect_failure_dynamic(test_name, [0, 1])

    test_name = 'test_run_on_singleton_matching_endpoint_failure'
    expect_failure_dynamic(test_name, [0])
    expect_failure_dynamic(test_name, [1])
    expect_failure_dynamic(test_name, [0, 1])

    test_name = 'test_no_run_on_singleton_matching_endpoint'
    # no failure, no matches, expect skips on all endpoints
    expect_skip_dynamic(test_name, [0])
    expect_skip_dynamic(test_name, [1])
    expect_skip_dynamic(test_name, [0, 1])

    test_runner.Shutdown()
    print(
        f"Test of Decorators: test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
