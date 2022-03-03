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
from logging import disable
import os
import sys
import click
import coloredlogs
import chip.logging
import logging
from base import TestFail, TestTimeout, BaseTestHelper, FailIfNot, logger, TestIsEnabled, SetTestSet
from cluster_objects import NODE_ID, ClusterObjectTests
from network_commissioning import NetworkCommissioningTests
import asyncio

# The thread network dataset tlv for testing, splited into T-L-V.

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


def ethernet_commissioning(test: BaseTestHelper, discriminator, setup_pin, device_nodeid):
    logger.info("Testing discovery")
    address = test.TestDiscovery(discriminator=discriminator)
    FailIfNot(address, "Failed to discover any devices.")

    # FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
    #           "Failed to finish network commissioning")

    logger.info("Testing key exchange")
    FailIfNot(test.TestKeyExchange(ip=address.decode("utf-8"),
                                   setuppin=setup_pin,
                                   nodeid=device_nodeid),
              "Failed to finish key exchange")

    #
    # Disable this test for now since it's exposing some bugs
    # in the underlying minimal mDNS component on Linux and triggering crashes.
    #
    # Issue: #15688
    #
    # asyncio.run(test.TestMultiFabric(ip=address.decode("utf-8"),
    #                                  setuppin=20202021,
    #                                  nodeid=1))

    logger.info("Testing closing sessions")
    FailIfNot(test.TestCloseSession(nodeid=device_nodeid),
              "Failed to close sessions")


@click.command()
@click.option("--controller-nodeid", default=TEST_CONTROLLER_NODE_ID, type=int, help="NodeId of the controller.")
@click.option("--device-nodeid", default=TEST_DEVICE_NODE_ID, type=int, help="NodeId of the device.")
@click.option("--timeout", "-t", default=240, type=int, help="The program will return with timeout after specified seconds.")
@click.option("--discriminator", default=TEST_DISCRIMINATOR, type=int, help="Discriminator of the device.")
@click.option("--setup-pin", default=TEST_SETUPPIN, type=int, help="Setup pincode of the device.")
@click.option('--enable-test', default=['all'], multiple=True, help='The tests to be executed.')
@click.option('--disable-test', default=[], multiple=True, help='The tests to be excluded.')
@click.option('--log-level', default='WARN', type=click.Choice(['ERROR', 'WARN', 'INFO', 'DEBUG']), help="The log level of the test.")
@click.option('--log-format', default=None, type=str, help="Override logging format")
def main(controller_nodeid, device_nodeid, timeout, discriminator, setup_pin, enable_test, disable_test, log_level, log_format):
    coloredlogs.install(level=log_level, fmt=log_format)
    logger.info("Test Parameters:")
    logger.info(f"\tController NodeId: {controller_nodeid}")
    logger.info(f"\tDevice NodeId:     {device_nodeid}")
    logger.info(f"\tTest Timeout:      {timeout}s")
    logger.info(f"\tDiscriminator:     {discriminator}")
    logger.info(f"\tEnabled Tests:     {enable_test}")
    logger.info(f"\tDisabled Tests:    {disable_test}")
    SetTestSet(enable_test, disable_test)
    do_tests(controller_nodeid, device_nodeid, timeout,
             discriminator, setup_pin)


def test_datamodel(test: BaseTestHelper, device_nodeid: int):
    logger.info("Testing on off cluster")
    FailIfNot(test.TestOnOffCluster(nodeid=device_nodeid,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

    logger.info("Testing level control cluster")
    FailIfNot(test.TestLevelControlCluster(nodeid=device_nodeid,
                                           endpoint=LIGHTING_ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test level control cluster")

    logger.info("Testing sending commands to non exist endpoint")
    FailIfNot(not test.TestOnOffCluster(nodeid=device_nodeid,
                                        endpoint=233,
                                        group=GROUP_ID), "Failed to test on off cluster on non-exist endpoint")

    # Test experimental Python cluster objects API
    logger.info("Testing cluster objects API")
    FailIfNot(asyncio.run(ClusterObjectTests.RunTest(test.devCtrl)),
              "Failed when testing Python Cluster Object APIs")

    logger.info("Testing attribute reading")
    FailIfNot(test.TestReadBasicAttributes(nodeid=device_nodeid,
                                           endpoint=ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test Read Basic Attributes")

    logger.info("Testing attribute writing")
    FailIfNot(test.TestWriteBasicAttributes(nodeid=device_nodeid,
                                            endpoint=ENDPOINT_ID,
                                            group=GROUP_ID),
              "Failed to test Write Basic Attributes")

    logger.info("Testing attribute reading basic again")
    FailIfNot(test.TestReadBasicAttributes(nodeid=1,
                                           endpoint=ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test Read Basic Attributes")

    logger.info("Testing subscription")
    FailIfNot(test.TestSubscription(nodeid=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID),
              "Failed to subscribe attributes.")

    logger.info("Testing another subscription that kills previous subscriptions")
    FailIfNot(test.TestSubscription(nodeid=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID),
              "Failed to subscribe attributes.")

    logger.info("Testing on off cluster over resolved connection")
    FailIfNot(test.TestOnOffCluster(nodeid=device_nodeid,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")


def do_tests(controller_nodeid, device_nodeid, timeout, discriminator, setup_pin):
    timeoutTicker = TestTimeout(timeout)
    timeoutTicker.start()

    test = BaseTestHelper(nodeid=controller_nodeid)

    chip.logging.RedirectToPythonLogging()

    commissioning_method = ethernet_commissioning

    commissioning_method(test, discriminator, setup_pin,
                         device_nodeid)

    logger.info("Testing resolve")
    FailIfNot(test.TestResolve(nodeid=device_nodeid),
              "Failed to resolve nodeid")

    # Still test network commissioning
    if TestIsEnabled('network_commissioning'):
        logger.info("Testing network commissioning")
        FailIfNot(asyncio.run(NetworkCommissioningTests(devCtrl=test.devCtrl, nodeid=device_nodeid).run()),
                  "Failed to finish network commissioning")

    if TestIsEnabled('datamodel'):
        logger.info("Testing datamodel functions")
        test_datamodel(test, device_nodeid)

    logger.info("Testing non-controller APIs")
    FailIfNot(test.TestNonControllerAPIs(), "Non controller API test failed")

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


if __name__ == "__main__":
    try:
        main()
    except Exception as ex:
        logger.exception(ex)
        TestFail("Exception occurred when running tests.")
