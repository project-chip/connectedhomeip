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

import asyncio
# Commissioning test.
import os
import random
import sys
from optparse import OptionParser

import example_python_commissioning_flow
from base import BaseTestHelper, TestFail, TestTimeout, logger
from chip import ChipDeviceCtrl
from chip import clusters as Clusters
from chip import commissioning
from chip.crypto import p256keypair

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
        "--bad-cert-issuer",
        action="store_true",
        dest="badCertIssuer",
        default=False,
        help="Simulate a bad certificate issuer, the commissioning should fail when sending OpCreds.",
    )
    optParser.add_option(
        "-d",
        "--discriminator",
        action="store",
        dest="discriminator",
        default='',
        type='str',
        help="The long discriminator of the device",
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
        '--paa-trust-store-path',
        dest="paaPath",
        default='',
        type='str',
        help="Path that contains valid and trusted PAA Root Certificates."
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = BaseTestHelper(
        nodeid=112233, paaTrustStorePath=options.paaPath, testCommissioner=True, keypair=p256keypair.TestP256Keypair())

    class BadCredentialProvider:
        def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceController):
            self._devCtrl = devCtrl

        async def get_attestation_nonce(self) -> bytes:
            return os.urandom(32)

        async def get_csr_nonce(self) -> bytes:
            return os.urandom(32)

        async def get_commissionee_credentials(self, request: commissioning.GetCommissioneeCredentialsRequest) -> commissioning.GetCommissioneeCredentialsResponse:
            node_id = random.randint(100000, 999999)
            nocChain = self._devCtrl.IssueNOCChain(Clusters.OperationalCredentials.Commands.CSRResponse(
                NOCSRElements=request.csr_elements, attestationSignature=request.attestation_signature), nodeId=node_id)
            return commissioning.GetCommissioneeCredentialsResponse(
                rcac=nocChain.rcacBytes[1:],
                noc=nocChain.nocBytes[1:],
                icac=nocChain.icacBytes[1:],
                ipk=nocChain.ipkBytes[1:],
                case_admin_node=self._devCtrl.nodeId,
                admin_vendor_id=self._devCtrl.fabricAdmin.vendorId,
                node_id=node_id,
                fabric_id=self._devCtrl.fabricId)

    flow = example_python_commissioning_flow.ExampleCustomMatterCommissioningFlow(
        devCtrl=test.devCtrl,
        credential_provider=BadCredentialProvider(
            test.devCtrl) if options.badCertIssuer else example_python_commissioning_flow.ExampleCredentialProvider(test.devCtrl),
        logger=logger)

    try:
        asyncio.run(flow.commission(commissioning.Parameters(
            pase_param=commissioning.PaseOverIPParameters(
                long_discriminator=options.discriminator,
                setup_pin=20202021, temporary_nodeid=options.nodeid
            ),
            regulatory_config=commissioning.RegulatoryConfig(
                location_type=commissioning.RegulatoryLocationType.INDOOR_OUTDOOR, country_code='US'),
            fabric_label="TestFabric",
            commissionee_info=commissioning.CommissioneeInfo(
                endpoints={},
                is_thread_device=True,
                is_ethernet_device=False,
                is_wifi_device=False,
            ),
            wifi_credentials=None,
            thread_credentials=bytes.fromhex(TEST_THREAD_NETWORK_DATASET_TLV))))
        if options.badCertIssuer:
            raise AssertionError("The commission is expected to fail. (BadCredentialProvider used)")
    except Exception as ex:
        if options.badCertIssuer:
            logger.exception("Got exception and the test is expected to fail (BadCredentialProvider used)")
        else:
            raise ex

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
