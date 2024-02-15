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

from base import BaseTestHelper, FailIfNot, TestFail, TestTimeout, logger

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
TEST_DISCOVERY_TYPE = 2

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
    optParser.add_option(
        "--setup-payload",
        action="store",
        dest="setupPayload",
        default='',
        type='str',
        help="Setup Payload (manual pairing code or QR code content)",
        metavar="<setup-payload>"
    )
    optParser.add_option(
        "--nodeid",
        action="store",
        dest="nodeid",
        default=1,
        type=int,
        help="The Node ID issued to the device",
        metavar="<nodeid>"
    )
    optParser.add_option(
        "--discriminator",
        action="store",
        dest="discriminator",
        default=TEST_DISCRIMINATOR,
        type=int,
        help="Discriminator of the device",
        metavar="<nodeid>"
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
    optParser.add_option(
        "--discovery-type",
        action="store",
        dest="discoveryType",
        default=TEST_DISCOVERY_TYPE,
        type=int,
        help="Discovery type of commissioning. (0: networkOnly 1: networkOnlyWithoutPASEAutoRetry 2: All<Ble & Network>)",
        metavar="<discovery-type>"
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = BaseTestHelper(
        nodeid=112233, paaTrustStorePath=options.paaTrustStorePath, testCommissioner=True)

    logger.info("Testing discovery")
    FailIfNot(test.TestDiscovery(discriminator=options.discriminator),
              "Failed to discover any devices.")

    FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
              "Failed to finish network commissioning")

    if options.deviceAddress:
        logger.info("Testing commissioning (IP)")
        FailIfNot(test.TestCommissioning(ip=options.deviceAddress,
                                         setuppin=20202021,
                                         nodeid=options.nodeid),
                  "Failed to finish commissioning")
    elif options.setupPayload:
        logger.info("Testing commissioning (w/ Setup Payload)")
        FailIfNot(test.TestCommissioningWithSetupPayload(setupPayload=options.setupPayload,
                                                         nodeid=options.nodeid,
                                                         discoveryType=options.discoveryType),
                  "Failed to finish commissioning")
    else:
        TestFail("Must provide device address or setup payload to commissioning the device")

    logger.info("Testing on off cluster")
    FailIfNot(test.TestOnOffCluster(nodeid=options.nodeid,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

    FailIfNot(test.TestUsedTestCommissioner(),
              "Test commissioner check failed")

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
