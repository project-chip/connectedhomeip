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

import asyncio
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

ENDPOINT_ID = 0
LIGHTING_ENDPOINT_ID = 1
GROUP_ID = 0


async def main():
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
        dest="deviceAddress1",
        default='',
        type='str',
        help="Address of the first device",
    )
    optParser.add_option(
        '--paa-trust-store-path',
        dest="paaPath",
        default='',
        type='str',
        help="Path that contains valid and trusted PAA Root Certificates."
    )
    optParser.add_option(
        '--fail-on-report',
        action="store_true",
        dest="report",
        default=False,
        help='Use this flag to simulate a failure handling the report. Without this flag, failure is simulated on the stage'
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = BaseTestHelper(nodeid=112233, testCommissioner=True,
                          paaTrustStorePath=options.paaPath)

    FailIfNot(test.SetNetworkCommissioningParameters(dataset=TEST_THREAD_NETWORK_DATASET_TLV),
              "Failed to set network commissioning parameters")

    logger.info("Testing PASE connection to device")

    # TODO: Start at stage 2 once handling for arming failsafe on pase is done.
    if options.report:
        for testFailureStage in range(3, 21):
            FailIfNot(await test.TestPaseOnly(ip=options.deviceAddress1,
                                              setuppin=20202021,
                                              nodeid=1),
                      "Failed to establish PASE connection with device")
            FailIfNot(await test.TestCommissionFailureOnReport(1, testFailureStage),
                      "Commissioning failure tests failed for simulated report failure on stage {}".format(testFailureStage))

    else:
        for testFailureStage in range(3, 21):
            FailIfNot(await test.TestPaseOnly(ip=options.deviceAddress1,
                                              setuppin=20202021,
                                              nodeid=1),
                      "Failed to establish PASE connection with device")
            FailIfNot(await test.TestCommissionFailure(1, testFailureStage),
                      "Commissioning failure tests failed for simulated stage failure on stage {}".format(testFailureStage))

    # Ensure we can still commission for real
    FailIfNot(await test.TestPaseOnly(ip=options.deviceAddress1,
                                      setuppin=20202021,
                                      nodeid=1),
              "Failed to establish PASE connection with device")
    FailIfNot(await test.TestCommissionFailure(1, 0), "Failed to commission device")

    logger.info("Testing on off cluster")
    FailIfNot(await test.TestOnOffCluster(nodeid=1,
                                          endpoint=LIGHTING_ENDPOINT_ID), "Failed to test on off cluster")

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except Exception as ex:
        logger.exception(ex)
        TestFail("Exception occurred when running tests.")
