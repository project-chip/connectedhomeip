#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
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
        "--address1",
        action="store",
        dest="deviceAddress1",
        default='',
        type='str',
        help="Address of the first device",
    )
    optParser.add_option(
        "--address2",
        action="store",
        dest="deviceAddress2",
        default='',
        type='str',
        help="Address of the second device",
    )
    optParser.add_option(
        "-p",
        "--paa-trust-store-path",
        action="store",
        dest="paaTrustStorePath",
        default='',
        type='str',
        help="Path that contains valid and trusted PAA Root Certificates.",
        metavar="<paa-trust-store-path>"
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = BaseTestHelper(
        nodeid=112233, paaTrustStorePath=options.paaTrustStorePath, testCommissioner=False)

    FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
              "Failed to finish network commissioning")

    logger.info("Testing PASE connection to device 1")
    FailIfNot(test.TestPaseOnly(ip=options.deviceAddress1,
                                setuppin=20202021,
                                nodeid=1),
              "Failed to establish PASE connection with device 1")

    logger.info("Testing PASE connection to device 2")
    FailIfNot(test.TestPaseOnly(ip=options.deviceAddress2,
                                setuppin=20202021,
                                nodeid=2),
              "Failed to establish PASE connection with device 2")

    logger.info("Attempting to execute a fabric-scoped command during PASE before AddNOC")
    FailIfNot(test.TestFabricScopedCommandDuringPase(nodeid=1),
              "Did not get UNSUPPORTED_ACCESS for fabric-scoped command during PASE")

    FailIfNot(test.TestCommissionOnly(nodeid=1),
              "Failed to commission device 1")

    FailIfNot(test.TestCommissionOnly(nodeid=2),
              "Failed to commission device 2")

    logger.info("Testing on off cluster on device 1")
    FailIfNot(test.TestOnOffCluster(nodeid=1,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster on device 1")

    logger.info("Testing on off cluster on device 2")
    FailIfNot(test.TestOnOffCluster(nodeid=2,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster on device 2")

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
