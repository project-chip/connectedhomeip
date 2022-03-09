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
import os
import sys
from optparse import OptionParser
from base import TestFail, TestTimeout, BaseTestHelper, FailIfNot, logger
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

ENDPOINT_ID = 0
LIGHTING_ENDPOINT_ID = 1
GROUP_ID = 0


def main():
    optParser = OptionParser()
    optParser.add_option(
        "-t",
        "--timeout",
        action="store",
        dest="testTimeout",
        default=75,
        type='int',
        help="The program will return with timeout after specified seconds.",
        metavar="<timeout-second>",
    )
    optParser.add_option(
        "-a",
        "--address",
        action="store",
        dest="deviceAddress",
        default='',
        type='str',
        help="Address of the device",
        metavar="<device-addr>",
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = BaseTestHelper(nodeid=112233)

    logger.info("Testing discovery")
    FailIfNot(test.TestDiscovery(discriminator=TEST_DISCRIMINATOR),
              "Failed to discover any devices.")

    # FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
    #           "Failed to finish network commissioning")

    logger.info("Testing key exchange")
    FailIfNot(test.TestKeyExchange(ip=options.deviceAddress,
                                   setuppin=20202021,
                                   nodeid=1),
              "Failed to finish key exchange")

    #
    # Disable this test for now since it's exposing some bugs
    # in the underlying minimal mDNS component on Linux and triggering crashes.
    #
    # Issue: #15688
    #
    # asyncio.run(test.TestMultiFabric(ip=options.deviceAddress,
    #                                  setuppin=20202021,
    #                                  nodeid=1))
    #
    # logger.info("Testing writing/reading fabric sensitive data")
    # asyncio.run(test.TestFabricSensitive(nodeid=1))

    logger.info("Testing closing sessions")
    FailIfNot(test.TestCloseSession(nodeid=1), "Failed to close sessions")

    logger.info("Testing resolve")
    FailIfNot(test.TestResolve(nodeid=1),
              "Failed to resolve nodeid")

    # Still test network commissioning
    logger.info("Testing network commissioning")
    FailIfNot(asyncio.run(NetworkCommissioningTests(devCtrl=test.devCtrl, nodeid=1).run()),
              "Failed to finish network commissioning")

    logger.info("Testing on off cluster")
    FailIfNot(test.TestOnOffCluster(nodeid=1,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

    logger.info("Testing level control cluster")
    FailIfNot(test.TestLevelControlCluster(nodeid=1,
                                           endpoint=LIGHTING_ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test level control cluster")

    logger.info("Testing sending commands to non exist endpoint")
    FailIfNot(not test.TestOnOffCluster(nodeid=1,
                                        endpoint=233,
                                        group=GROUP_ID), "Failed to test on off cluster on non-exist endpoint")

    # Test experimental Python cluster objects API
    logger.info("Testing cluster objects API")
    FailIfNot(asyncio.run(ClusterObjectTests.RunTest(test.devCtrl)),
              "Failed when testing Python Cluster Object APIs")

    logger.info("Testing attribute reading")
    FailIfNot(test.TestReadBasicAttributes(nodeid=1,
                                           endpoint=ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test Read Basic Attributes")

    logger.info("Testing attribute writing")
    FailIfNot(test.TestWriteBasicAttributes(nodeid=1,
                                            endpoint=ENDPOINT_ID,
                                            group=GROUP_ID),
              "Failed to test Write Basic Attributes")

    logger.info("Testing attribute reading basic again")
    FailIfNot(test.TestReadBasicAttributes(nodeid=1,
                                           endpoint=ENDPOINT_ID,
                                           group=GROUP_ID),
              "Failed to test Read Basic Attributes")

    logger.info("Testing subscription")
    FailIfNot(test.TestSubscription(nodeid=1, endpoint=LIGHTING_ENDPOINT_ID),
              "Failed to subscribe attributes.")

    logger.info("Testing another subscription that kills previous subscriptions")
    FailIfNot(test.TestSubscription(nodeid=1, endpoint=LIGHTING_ENDPOINT_ID),
              "Failed to subscribe attributes.")

    logger.info("Testing on off cluster over resolved connection")
    FailIfNot(test.TestOnOffCluster(nodeid=1,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

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
