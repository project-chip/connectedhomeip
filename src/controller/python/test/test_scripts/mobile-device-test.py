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
from base import TestTimeout, BaseTestHelper, FailIfNot, logger

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

    logger.info("Testing key exchange")
    FailIfNot(test.TestKeyExchange(ip=options.deviceAddress,
                                   setuppin=20202021,
                                   nodeid=1),
              "Failed to finish key exchange")

    logger.info("Testing network commissioning")
    FailIfNot(test.TestNetworkCommissioning(nodeid=1,
                                            endpoint=ENDPOINT_ID,
                                            group=GROUP_ID,
                                            dataset=TEST_THREAD_NETWORK_DATASET_TLV,
                                            network_id=TEST_THREAD_NETWORK_ID),
              "Failed to finish network commissioning")

    logger.info("Testing on off cluster")
    FailIfNot(test.TestOnOffCluster(nodeid=1,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

    logger.info("Testing sending commands to non exist endpoint")
    FailIfNot(not test.TestOnOffCluster(nodeid=1,
                                        endpoint=233,
                                        group=GROUP_ID), "Failed to test on off cluster on non-exist endpoint")

    logger.info("Testing attribute reading")
    FailIfNot(test.TestReadBasicAttribiutes(nodeid=1,
                                            endpoint=ENDPOINT_ID,
                                            group=GROUP_ID),
              "Failed to test Read Basic Attributes")

    logger.info("Testing attribute writing")
    FailIfNot(test.TestWriteBasicAttributes(nodeid=1,
                                            endpoint=ENDPOINT_ID,
                                            group=GROUP_ID),
              "Failed to test Write Basic Attributes")

    logger.info("Testing closing sessions")
    FailIfNot(test.TestCloseSession(nodeid=1), "Failed to close sessions")

    logger.info("Testing resolve")
    FailIfNot(test.TestResolve(nodeid=1),
              "Failed to resolve nodeid")

    logger.info("Testing on off cluster over resolved connection")
    FailIfNot(test.TestOnOffCluster(nodeid=1,
                                    endpoint=LIGHTING_ENDPOINT_ID,
                                    group=GROUP_ID), "Failed to test on off cluster")

    logger.info("Testing non-controller APIs")
    FailIfNot(test.TestNonControllerAPIs(), "Non controller API test failed")

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


if __name__ == "__main__":
    main()
