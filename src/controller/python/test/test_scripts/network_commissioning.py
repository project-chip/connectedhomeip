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

import chip.clusters as Clusters
import logging
from chip.clusters.Attribute import AttributePath, AttributeReadResult, AttributeStatus, ValueDecodeFailure
import chip.interaction_model
import asyncio

logger = logging.getLogger('NetworkCommissioning')
logger.setLevel(logging.INFO)

NODE_ID = 1
THREAD_ENDPOINT_ID = 0
WIFI_ENDPOINT_ID = 1

TEST_THREAD_NETWORK_DATASET_TLVS = [bytes.fromhex("0e080000000000010000" +
                                                  "000300000c" +
                                                  "35060004001fffe0" +
                                                  "0208fedcba9876543210" +
                                                  "0708fd00000000001234" +
                                                  "0510ffeeddccbbaa99887766554433221100" +
                                                  "030e54657374696e674e6574776f726b" +
                                                  "0102d252" +
                                                  "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"),
                                    # End of first TLV
                                    ]
# Network id, for the thread network, current a const value, will be changed to XPANID of the thread network.
TEST_THREAD_NETWORK_IDS = [
    bytes.fromhex("fedcba9876543210"),
]


class NetworkCommissioningTests:
    def __init__(self, devCtrl, nodeid):
        self._devCtrl = devCtrl
        self._nodeid = nodeid

    def log_interface_basic_info(self, endpoint, values):
        connectMaxTimeSeconds = values[AttributePath(
            EndpointId=endpoint, Attribute=Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds)].Data.value
        scanMaxTimeSeconds = values[AttributePath(
            EndpointId=endpoint, Attribute=Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds)].Data.value
        maxNetworks = values[AttributePath(
            EndpointId=endpoint, Attribute=Clusters.NetworkCommissioning.Attributes.MaxNetworks)].Data.value
        featureMap = values[AttributePath(
            EndpointId=endpoint, Attribute=Clusters.NetworkCommissioning.Attributes.FeatureMap)].Data.value
        logger.info(f"The interface supports {maxNetworks} networks.")
        logger.info(
            f"ScanNetworks should take no more than {scanMaxTimeSeconds} seconds.")
        logger.info(
            f"ConnectNetwork should take no more than {connectMaxTimeSeconds} seconds.")
        logger.info(
            f"The feature map of this endpoint is {featureMap}.")

    async def test_wifi(self):
        logger.info(f"Get basic infomation of the endpoint")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[
            (WIFI_ENDPOINT_ID,
             Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds),
            (WIFI_ENDPOINT_ID,
             Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds),
            (WIFI_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.MaxNetworks),
            (WIFI_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.FeatureMap)])
        self.log_interface_basic_info(WIFI_ENDPOINT_ID, res["Attributes"])
        logger.info(f"Finished getting basic infomation of the endpoint")

        # Scan networks
        logger.info(f"Scan networks")
        req = Clusters.NetworkCommissioning.Commands.ScanNetworks(
            ssid=b'', breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=WIFI_ENDPOINT_ID, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        # Add first network
        logger.info(f"Adding first test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(
            ssid=b"TestSSID", credentials=b"TestPass", breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=WIFI_ENDPOINT_ID, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(WIFI_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)])
        networkList = res['Attributes'][AttributePath(
            EndpointId=WIFI_ENDPOINT_ID, Attribute=Clusters.NetworkCommissioning.Attributes.Networks)]
        logger.info(f"Got network list: {networkList}")
        if len(networkList.Data.value) != 2:
            raise AssertionError(
                f"Unexpected result: expect 2 networks, but {len(networkList.Data.value)} networks received")
        if networkList.Data.value[0].networkID != b"TestSSID":
            raise AssertionError(
                f"Unexpected result: first network ID should be 'TestSSID' got {networkList.Data.value[0].networkID}")

        logger.info(f"Connect to a network")
        req = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
            networkID=b'TestSSID', breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=WIFI_ENDPOINT_ID, payload=req)
        logger.info(f"Got response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")
        logger.info(f"Device connected to a network.")

    async def test_thread(self):
        logger.info(f"Get basic infomation of the endpoint")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[
            (THREAD_ENDPOINT_ID,
             Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds),
            (THREAD_ENDPOINT_ID,
             Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds),
            (THREAD_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.MaxNetworks),
            (THREAD_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.FeatureMap)])
        self.log_interface_basic_info(THREAD_ENDPOINT_ID, res["Attributes"])
        logger.info(f"Finished getting basic infomation of the endpoint")

        # Scan networks
        logger.info(f"Scan networks")
        req = Clusters.NetworkCommissioning.Commands.ScanNetworks(
            ssid=b'', breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=THREAD_ENDPOINT_ID, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        # Add first network
        logger.info(f"Adding first test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=TEST_THREAD_NETWORK_DATASET_TLVS[0], breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=THREAD_ENDPOINT_ID, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(THREAD_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)])
        networkList = res['Attributes'][AttributePath(
            EndpointId=THREAD_ENDPOINT_ID, Attribute=Clusters.NetworkCommissioning.Attributes.Networks)]
        logger.info(f"Got network list: {networkList}")
        if len(networkList.Data.value) != 2:
            raise AssertionError(
                f"Unexpected result: expect 2 networks, but {len(networkList.Data.value)} networks received")
        if networkList.Data.value[0].networkID != TEST_THREAD_NETWORK_IDS[0]:
            raise AssertionError(
                f"Unexpected result: first network ID should be {TEST_THREAD_NETWORK_IDS[0]} got {networkList.Data.value[0].networkID}")

        logger.info(f"Connect to a network")
        req = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
            networkID=TEST_THREAD_NETWORK_IDS[0], breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=THREAD_ENDPOINT_ID, payload=req)
        logger.info(f"Got response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")
        logger.info(f"Device connected to a network.")

    async def run(self, testThread=True, testWiFi=False):
        if testThread:
            try:
                await self.test_thread()
            except Exception as ex:
                logger.exception(ex)
                return False
        if testWiFi:
            try:
                await self.test_wifi()
            except Exception as ex:
                logger.exception(ex)
                return False
        return True
