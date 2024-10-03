#!/usr/bin/env python3

#
#    Copyright (c) 2021 Project CHIP Authors
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

# Commissioning test.

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --log-level INFO
#       --timeout 3600
#       --disable-test ClusterObjectTests.TestTimedRequestTimeout
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import os

import base
import chip.logging
import click
import coloredlogs
from base import BaseTestHelper, FailIfNot, SetTestSet, TestFail, TestTimeout, logger
from chip.tracing import TracingContext
from cluster_objects import ClusterObjectTests
from network_commissioning import NetworkCommissioningTests

# The thread network dataset tlv for testing, splitted into T-L-V.

TEST_THREAD_NETWORK_DATASET_TLV = "0e080000000000010000" + \
    "000300000c" + \
    "35060004001fffe0" + \
    "0208fedcba9876543210" + \
    "0708fd00000000001234" + \
    "0510ffeeddccbbaa99887766554433221100" + \
    "030e54657374696e674e6574776f726b" + \
    "0102d252" + \
    "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"
# Network id, for the thread network, current a const value, will be changed to XPANID of the thread network.
TEST_THREAD_NETWORK_ID = "fedcba9876543210"
TEST_DISCRIMINATOR = 3840
TEST_SETUPPIN = 20202021

ENDPOINT_ID = 0
LIGHTING_ENDPOINT_ID = 1
GROUP_ID = 0

TEST_CONTROLLER_NODE_ID = 112233
TEST_DEVICE_NODE_ID = 1

ALL_TESTS = ['network_commissioning', 'datamodel']


async def ethernet_commissioning(test: BaseTestHelper, discriminator: int, setup_pin: int, address_override: str, device_nodeid: int):
    logger.info("Testing discovery")
    device = await test.TestDiscovery(discriminator=discriminator)
    FailIfNot(device, "Failed to discover any devices.")

    address = device.addresses[0]

    # FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
    #           "Failed to finish network commissioning")

    if address_override:
        address = address_override

    logger.info("Testing commissioning")
    FailIfNot(await test.TestCommissioning(ip=address,
                                           setuppin=setup_pin,
                                           nodeid=device_nodeid),
              "Failed to finish key exchange")

    logger.info("Testing multi-controller setup on the same fabric")
    FailIfNot(await test.TestMultiControllerFabric(nodeid=device_nodeid), "Failed the multi-controller test")

    logger.info("Testing CATs used on controllers")
    FailIfNot(await test.TestControllerCATValues(nodeid=device_nodeid), "Failed the controller CAT test")

    ok = await test.TestMultiFabric(ip=address,
                                    setuppin=20202021,
                                    nodeid=1)
    FailIfNot(ok, "Failed to commission multi-fabric")

    FailIfNot(await test.TestAddUpdateRemoveFabric(nodeid=device_nodeid),
              "Failed AddUpdateRemoveFabric test")

    logger.info("Testing CASE Eviction")
    FailIfNot(await test.TestCaseEviction(device_nodeid), "Failed TestCaseEviction")

    logger.info("Testing closing sessions")
    FailIfNot(test.TestCloseSession(nodeid=device_nodeid), "Failed to close sessions")


@base.test_case
def TestDatamodel(test: BaseTestHelper, device_nodeid: int):
    logger.info("Testing datamodel functions")

    logger.info("Testing on off cluster")
    FailIfNot(asyncio.run(test.TestOnOffCluster(nodeid=device_nodeid,
                                                endpoint=LIGHTING_ENDPOINT_ID)), "Failed to test on off cluster")

    logger.info("Testing level control cluster")
    FailIfNot(asyncio.run(test.TestLevelControlCluster(nodeid=device_nodeid,
                                                       endpoint=LIGHTING_ENDPOINT_ID)),
              "Failed to test level control cluster")

    logger.info("Testing sending commands to non exist endpoint")
    FailIfNot(not asyncio.run(test.TestOnOffCluster(nodeid=device_nodeid,
                                                    endpoint=233)), "Failed to test on off cluster on non-exist endpoint")

    # Test experimental Python cluster objects API
    logger.info("Testing cluster objects API")
    FailIfNot(asyncio.run(ClusterObjectTests.RunTest(test.devCtrl)),
              "Failed when testing Python Cluster Object APIs")

    logger.info("Testing attribute reading")
    FailIfNot(asyncio.run(test.TestReadBasicAttributes(nodeid=device_nodeid,
                                                       endpoint=ENDPOINT_ID)),
              "Failed to test Read Basic Attributes")

    logger.info("Testing attribute writing")
    FailIfNot(asyncio.run(test.TestWriteBasicAttributes(nodeid=device_nodeid,
                                                        endpoint=ENDPOINT_ID)),
              "Failed to test Write Basic Attributes")

    logger.info("Testing attribute reading basic again")
    FailIfNot(asyncio.run(test.TestReadBasicAttributes(nodeid=1,
                                                       endpoint=ENDPOINT_ID)),
              "Failed to test Read Basic Attributes")

    logger.info("Testing subscription")
    FailIfNot(asyncio.run(test.TestSubscription(nodeid=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID)),
              "Failed to subscribe attributes.")

    logger.info("Testing another subscription that kills previous subscriptions")
    FailIfNot(asyncio.run(test.TestSubscription(nodeid=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID)),
              "Failed to subscribe attributes.")

    logger.info("Testing re-subscription")
    FailIfNot(asyncio.run(test.TestResubscription(nodeid=device_nodeid)),
              "Failed to validated re-subscription")

    logger.info("Testing on off cluster over resolved connection")
    FailIfNot(asyncio.run(test.TestOnOffCluster(nodeid=device_nodeid,
                                                endpoint=LIGHTING_ENDPOINT_ID)), "Failed to test on off cluster")

    logger.info("Testing writing/reading fabric sensitive data")
    asyncio.run(test.TestFabricSensitive(nodeid=device_nodeid))


def do_tests(controller_nodeid, device_nodeid, address, timeout, discriminator, setup_pin, paa_trust_store_path):
    timeoutTicker = TestTimeout(timeout)
    timeoutTicker.start()

    test = BaseTestHelper(nodeid=controller_nodeid,
                          paaTrustStorePath=paa_trust_store_path)

    chip.logging.RedirectToPythonLogging()

    asyncio.run(ethernet_commissioning(test, discriminator, setup_pin, address,
                                       device_nodeid))

    logger.info("Testing resolve")
    FailIfNot(test.TestResolve(nodeid=device_nodeid),
              "Failed to resolve nodeid")

    # Still test network commissioning
    FailIfNot(asyncio.run(NetworkCommissioningTests(devCtrl=test.devCtrl, nodeid=device_nodeid).run()),
              "Failed to finish network commissioning")

    TestDatamodel(test, device_nodeid)

    logger.info("Testing non-controller APIs")
    FailIfNot(test.TestNonControllerAPIs(), "Non controller API test failed")

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


@click.command()
@click.option("--controller-nodeid",
              default=TEST_CONTROLLER_NODE_ID,
              type=int,
              help="NodeId of the controller.")
@click.option("--device-nodeid",
              default=TEST_DEVICE_NODE_ID,
              type=int,
              help="NodeId of the device.")
@click.option("--address", "-a",
              default='',
              type=str,
              help="Skip commissionee discovery, commission the device with the IP directly.")
@click.option("--timeout", "-t",
              default=240,
              type=int,
              help="The program will return with timeout after specified seconds.")
@click.option("--discriminator",
              default=TEST_DISCRIMINATOR,
              type=int,
              help="Discriminator of the device.")
@click.option("--setup-pin",
              default=TEST_SETUPPIN,
              type=int,
              help="Setup pincode of the device.")
@click.option('--enable-test',
              default=['all'],
              type=str,
              multiple=True,
              help='The tests to be executed. By default, all tests will be executed, use this option to run a '
              'specific set of tests. Use --print-test-list for a list of appliable tests.')
@click.option('--disable-test',
              default=[],
              type=str,
              multiple=True,
              help='The tests to be excluded from the set of enabled tests. Use --print-test-list for a list of '
              'appliable tests.')
@click.option('--log-level',
              default='WARN',
              type=click.Choice(['ERROR', 'WARN', 'INFO', 'DEBUG']),
              help="The log level of the test.")
@click.option('--log-format',
              default=None,
              type=str,
              help="Override logging format")
@click.option('--print-test-list',
              is_flag=True,
              help="Print a list of test cases and test sets that can be toggled via --enable-test and --disable-test, then exit")
@click.option('--paa-trust-store-path',
              default='',
              type=str,
              help="Path that contains valid and trusted PAA Root Certificates.")
@click.option('--trace-to',
              multiple=True,
              default=[],
              help="Trace location")
@click.option('--app-pid',
              type=int,
              default=0,
              help="The PID of the app against which the test is going to run")
def run(controller_nodeid, device_nodeid, address, timeout, discriminator, setup_pin, enable_test, disable_test, log_level,
        log_format, print_test_list, paa_trust_store_path, trace_to, app_pid):
    coloredlogs.install(level=log_level, fmt=log_format, logger=logger)

    if print_test_list:
        print("Test sets:")
        for name in base.configurable_tests():
            print(f"\t{name}")
        print("Test cases:")
        for name in base.configurable_test_cases():
            print(f"\t{name}")
        return

    logger.info("Test Parameters:")
    logger.info(f"\tController NodeId: {controller_nodeid}")
    logger.info(f"\tDevice NodeId:     {device_nodeid}")
    logger.info(f"\tTest Timeout:      {timeout}s")
    logger.info(f"\tDiscriminator:     {discriminator}")
    logger.info(f"\tEnabled Tests:     {enable_test}")
    logger.info(f"\tDisabled Tests:    {disable_test}")
    SetTestSet(enable_test, disable_test)
    with TracingContext() as tracing_ctx:
        for destination in trace_to:
            tracing_ctx.StartFromString(destination)

        do_tests(controller_nodeid, device_nodeid, address, timeout,
                 discriminator, setup_pin, paa_trust_store_path)


if __name__ == "__main__":
    try:
        run()
    except Exception as ex:
        logger.exception(ex)
        TestFail("Exception occurred when running tests.")
