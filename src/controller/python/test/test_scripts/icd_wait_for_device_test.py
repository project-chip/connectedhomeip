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

import asyncio
import os
import sys
from optparse import OptionParser

from base import BaseTestHelper, FailIfNot, TestFail, TestTimeout, logger

TEST_DISCRIMINATOR = 3840
TEST_DISCOVERY_TYPE = 2

ENDPOINT_ID = 0
LIGHTING_ENDPOINT_ID = 1
GROUP_ID = 0


async def waitForActiveAndTriggerCheckIn(test, nodeid):
    coro = test.TestWaitForActive(nodeid=nodeid)
    await test.TestTriggerTestEventHandler(nodeid, bytes.fromhex("00112233445566778899aabbccddeeff"), 0x0046 << 48)
    return await coro


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

    devCtrl = test.devCtrl
    devCtrl.EnableICDRegistration(devCtrl.GenerateICDRegistrationParameters())
    logger.info("Testing commissioning")
    FailIfNot(await test.TestCommissioning(ip=options.deviceAddress,
                                           setuppin=20202021,
                                           nodeid=options.nodeid),
              "Failed to finish key exchange")
    logger.info("Commissioning completed")
    logger.info("Testing wait for active")
    FailIfNot(await waitForActiveAndTriggerCheckIn(test, nodeid=options.nodeid), "Failed to test wait for active")
    logger.info('Successfully handled wait-for-active')

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


if __name__ == "__main__":
    try:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(main())
        loop.close()
    except Exception as ex:
        logger.exception(ex)
        TestFail("Exception occurred when running tests.")
