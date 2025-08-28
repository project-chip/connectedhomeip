#
#    Copyright (c) 2025 Project CHIP Authors
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

"""Decorator utilities for Matter testing infrastructure.

This module provides decorator functions for Matter testing to handle async operations
and endpoint matching.
"""

import asyncio
import logging
from enum import IntFlag
from functools import partial
from typing import TYPE_CHECKING, Callable, Type

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Attribute
from matter.clusters import ClusterObjects as ClusterObjects
from matter.testing.global_attribute_ids import GlobalAttributeIds

# conditional import to avoid circular dependency but still allow type checking
if TYPE_CHECKING:
    from matter.testing.matter_testing import MatterBaseTest

EndpointCheckFunction = Callable[[
    Clusters.Attribute.AsyncReadTransaction.ReadResponse, int], bool]


def _has_cluster(wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int, cluster: ClusterObjects.ClusterObjectDescriptor) -> bool:
    """Check if a cluster exists on a specific endpoint.

    Args:
        wildcard: A wildcard read result containing endpoint attributes mapping
        endpoint: The endpoint ID to check
        cluster: The Cluster object to look for

    Returns:
        bool: True if the cluster exists on the endpoint, False otherwise
            Returns False if endpoint is not found in wildcard attributes
    """
    return endpoint in wildcard.attributes and cluster in wildcard.attributes[endpoint]


def has_cluster(cluster: ClusterObjects.ClusterObjectDescriptor) -> EndpointCheckFunction:
    """" EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified cluster. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D, E
        EP2, cluster D
        EP3, cluster E

        And the following test specification:
        @run_if_endpoint_matches(has_cluster(Clusters.D))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_cluster, cluster=cluster)


def _has_attribute(wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int, attribute: ClusterObjects.ClusterAttributeDescriptor) -> bool:
    """Check if an attribute exists in a cluster's AttributeList on a specific endpoint.

    Args:
        wildcard: A wildcard read result containing endpoint attributes mapping
        endpoint: The endpoint ID to check
        attribute: The ClusterAttributeDescriptor to look for

    Returns:
        bool: True if the attribute ID exists in the cluster's AttributeList, False otherwise
            Returns False if endpoint, cluster, or AttributeList is not found

    Raises:
        ValueError: If AttributeList value is not a list type
        KeyError: If attribute's cluster_id is not found in ALL_CLUSTERS
    """
    cluster: Type[ClusterObjects.Cluster] = ClusterObjects.ALL_CLUSTERS[attribute.cluster_id]

    if endpoint not in wildcard.attributes:
        return False

    if cluster not in wildcard.attributes[endpoint]:
        return False

    # Mypy can't verify that cluster subclasses have Attributes, because the Attribute subclass does not
    # appear in the base class. However, cluster classes are generated code and all derived cluster classes
    # are guaranteed to have attributes because the global attributes are included.
    if cluster.Attributes.AttributeList not in wildcard.attributes[endpoint][cluster]:  # type: ignore[attr-defined]
        return False

    # Mypy can't verify that cluster subclasses have Attributes, but they do at runtime
    attr_list = wildcard.attributes[endpoint][cluster][cluster.Attributes.AttributeList]  # type: ignore[attr-defined]
    if not isinstance(attr_list, list):
        raise ValueError(
            f"Failed to read mandatory AttributeList attribute value for cluster {cluster} on endpoint {endpoint}: {attr_list}.")

    return attribute.attribute_id in attr_list


def has_attribute(attribute: ClusterObjects.ClusterAttributeDescriptor) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified attribute. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with attribute d, E
        EP2, cluster D with attribute d
        EP3, cluster D without attribute d

        And the following test specification:
        @run_if_endpoint_matches(has_attribute(Clusters.D.Attributes.d))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_attribute, attribute=attribute)


def _has_command(wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int, command: ClusterObjects.ClusterCommand) -> bool:
    """Check if a command exists in a cluster's AcceptedCommandList on a specific endpoint.

    Args:
        wildcard: A wildcard read result containing endpoint attributes mapping
        endpoint: The endpoint ID to check
        command: The ClusterCommand to look for

    Returns:
        bool: True if the command ID exists in the cluster's AcceptedCommandList, False otherwise
            Returns False if endpoint, cluster, or AcceptedCommandList is not found

    Raises:
        ValueError: If AcceptedCommandList value is not a list type
        KeyError: If command's cluster_id is not found in ALL_CLUSTERS
    """
    cluster: Type[ClusterObjects.Cluster] = ClusterObjects.ALL_CLUSTERS[command.cluster_id]

    if endpoint not in wildcard.attributes:
        return False

    if cluster not in wildcard.attributes[endpoint]:
        return False

    # Mypy can't verify that cluster subclasses have Attributes, but they do at runtime
    if cluster.Attributes.AcceptedCommandList not in wildcard.attributes[endpoint][cluster]:  # type: ignore[attr-defined]
        return False

    # Mypy can't verify that cluster subclasses have Attributes, but they do at runtime
    cmd_list = wildcard.attributes[endpoint][cluster][cluster.Attributes.AcceptedCommandList]  # type: ignore[attr-defined]
    if not isinstance(cmd_list, list):
        raise ValueError(
            f"Failed to read mandatory AcceptedCommandList command value for cluster {cluster} on endpoint {endpoint}: {cmd_list}.")

    return command.command_id in cmd_list


def has_command(command: ClusterObjects.ClusterCommand) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified attribute. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with command d, E
        EP2, cluster D with command d
        EP3, cluster D without command d

        And the following test specification:
        @run_if_endpoint_matches(has_command(Clusters.D.Commands.d))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_command, command=command)


def _has_feature(wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int, cluster: ClusterObjects.ClusterObjectDescriptor, feature: IntFlag) -> bool:
    if endpoint not in wildcard.attributes:
        return False

    if cluster not in wildcard.attributes[endpoint]:
        return False

    # Mypy can't verify that cluster subclasses have Attributes, but they do at runtime
    if cluster.Attributes.FeatureMap not in wildcard.attributes[endpoint][cluster]:  # type: ignore[attr-defined]
        return False

    # Mypy can't verify that cluster subclasses have Attributes, but they do at runtime
    feature_map = wildcard.attributes[endpoint][cluster][cluster.Attributes.FeatureMap]  # type: ignore[attr-defined]
    if not isinstance(feature_map, int):
        raise ValueError(
            f"Failed to read mandatory FeatureMap attribute value for cluster {cluster} on endpoint {endpoint}: {feature_map}.")

    return (feature & feature_map) != 0


def has_feature(cluster: ClusterObjects.ClusterObjectDescriptor, feature: IntFlag) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified feature. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with feature F0
        EP2: cluster D with feature F0
        EP3: cluster D without feature F0

        And the following test specification:
        @run_if_endpoint_matches(has_feature(Clusters.D.Bitmaps.Feature.F0))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_feature, cluster=cluster, feature=feature)


def _async_runner(body, test_instance, *args, **kwargs):
    timeout = getattr(test_instance.matter_test_config,
                      'timeout', None) or test_instance.default_timeout
    return test_instance.event_loop.run_until_complete(asyncio.wait_for(body(test_instance, *args, **kwargs), timeout=timeout))


def async_test_body(body):
    """Decorator required to be applied whenever a `test_*` method is `async def`.

    Since Mobly doesn't support asyncio directly, and the test methods are called
    synchronously, we need a mechanism to allow an `async def` to be converted to
    a asyncio-run synchronous method. This decorator does the wrapping.
    """

    def async_runner(self: "MatterBaseTest", *args, **kwargs):
        return _async_runner(body, self, *args, **kwargs)
    return async_runner


async def _get_all_matching_endpoints(test_instance, accept_function: EndpointCheckFunction) -> list[int]:
    """ Returns a list of endpoints matching the accept condition. """
    wildcard = await test_instance.default_controller.Read(test_instance.dut_node_id, [(Clusters.Descriptor), Attribute.AttributePath(None, None, GlobalAttributeIds.ATTRIBUTE_LIST_ID), Attribute.AttributePath(None, None, GlobalAttributeIds.FEATURE_MAP_ID), Attribute.AttributePath(None, None, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)])
    matching = [e for e in wildcard.attributes.keys()
                if accept_function(wildcard, e)]
    return matching


async def should_run_test_on_endpoint(test_instance, accept_function: EndpointCheckFunction) -> bool:
    """ Helper function for the run_if_endpoint_matches decorator.

        Returns True if test_instance.matter_test_config.endpoint matches the accept function.
    """
    if test_instance.matter_test_config.endpoint is None:
        msg = """
              The --endpoint flag is required for this test.
              """
        asserts.fail(msg)
    matching = await (_get_all_matching_endpoints(test_instance, accept_function))
    return test_instance.matter_test_config.endpoint in matching


def run_on_singleton_matching_endpoint(accept_function: EndpointCheckFunction):
    """ Test decorator for a test that needs to be run on the endpoint that matches the given accept function.

        This decorator should be used for tests where the endpoint is not known a-priori (dynamic endpoints).
        Note that currently this test is limited to devices with a SINGLE matching endpoint.
    """
    def run_on_singleton_matching_endpoint_internal(body):
        def matching_runner(self: "MatterBaseTest", *args, **kwargs):
            # Import locally to avoid circular dependency
            from matter.testing.matter_testing import MatterBaseTest
            assert isinstance(self, MatterBaseTest)

            runner_with_timeout = asyncio.wait_for(
                _get_all_matching_endpoints(self, accept_function), timeout=30)
            matching = self.event_loop.run_until_complete(runner_with_timeout)
            asserts.assert_less_equal(
                len(matching), 1, "More than one matching endpoint found for singleton test.")
            if not matching:
                logging.info(
                    "Test is not applicable to any endpoint - skipping test")
                asserts.skip('No endpoint matches test requirements')
                return
            try:
                old_endpoint = self.matter_test_config.endpoint
                self.matter_test_config.endpoint = matching[0]
                logging.info(
                    f'Running test on endpoint {self.matter_test_config.endpoint}')
                timeout = getattr(self.matter_test_config,
                                  'timeout', None) or self.default_timeout
                self.event_loop.run_until_complete(asyncio.wait_for(
                    body(self, *args, **kwargs), timeout=timeout))
            finally:
                self.matter_test_config.endpoint = old_endpoint
        return matching_runner
    return run_on_singleton_matching_endpoint_internal


def run_if_endpoint_matches(accept_function: EndpointCheckFunction):
    """ Test decorator for a test that needs to be run only if the endpoint meets the accept_function criteria.

        Place this decorator above the test_ method to have the test framework run this test only if the endpoint matches.
        This decorator takes an EndpointCheckFunction to assess whether a test needs to be run on a particular
        endpoint.

        For example, given the following device conformance:

        EP0: cluster A, B, C
        EP1: cluster D, E
        EP2, cluster D
        EP3, cluster E

        And the following test specification:
        @run_if_endpoint_matches(has_cluster(Clusters.D))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.

        Tests that use this decorator cannot use a pics_ method for test selection and should not reference any
        PICS values internally.
    """
    def run_if_endpoint_matches_internal(body):
        def per_endpoint_runner(test_instance, *args, **kwargs):
            runner_with_timeout = asyncio.wait_for(
                should_run_test_on_endpoint(test_instance, accept_function), timeout=60)
            should_run_test = test_instance.event_loop.run_until_complete(
                runner_with_timeout)
            if not should_run_test:
                logging.info(
                    "Test is not applicable to this endpoint - skipping test")
                asserts.skip('Endpoint does not match test requirements')
                return
            logging.info(
                f'Running test on endpoint {test_instance.matter_test_config.endpoint}')
            timeout = getattr(test_instance.matter_test_config,
                              'timeout', None) or test_instance.default_timeout
            test_instance.event_loop.run_until_complete(asyncio.wait_for(
                body(test_instance, *args, **kwargs), timeout=timeout))
        return per_endpoint_runner
    return run_if_endpoint_matches_internal
