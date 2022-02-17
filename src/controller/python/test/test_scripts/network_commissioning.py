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
from chip.clusters.Types import NullValue
import chip.interaction_model
import asyncio

logger = logging.getLogger('NetworkCommissioning')
logger.setLevel(logging.INFO)

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

TEST_WIFI_SSID = "TestSSID"
TEST_WIFI_PASS = "TestPass"

WIFI_NETWORK_FEATURE_MAP = 1
THREAD_NETWORK_FEATURE_MAP = 2


class NetworkCommissioningTests:
    def __init__(self, devCtrl, nodeid):
        self._devCtrl = devCtrl
        self._nodeid = nodeid

    def log_interface_basic_info(self, values):
        logger.info(f"The interface supports {values.maxNetworks} networks.")
        logger.info(
            f"ScanNetworks should take no more than {values.scanMaxTimeSeconds} seconds.")
        logger.info(
            f"ConnectNetwork should take no more than {values.connectMaxTimeSeconds} seconds.")
        logger.info(
            f"The feature map of this endpoint is {values.featureMap}.")

    async def readLastNetworkingStateAttributes(self, endpointId):
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.LastConnectErrorValue),
                                                                                 (endpointId, Clusters.NetworkCommissioning.Attributes.LastNetworkID),
                                                                                 (endpointId, Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)], returnClusterObject=True)
        values = res[0][endpointId][Clusters.NetworkCommissioning]
        logger.info(f"Got values: {values}")
        return values

    async def test_negative(self, endpointId):
        logger.info(
            f"Running negative test cases for NetworkCommissioning cluster on endpoint {endpointId}")

        try:
            logger.info(
                f"1. Send ConnectNetwork command with a illegal network id")
            req = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
                networkID=b'0' * 254, breadcrumb=0)
            res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
            raise AssertionError(f"Failure expected but got response {res}")
        except chip.interaction_model.InteractionModelError as ex:
            logger.info(f"Received {ex} from server.")

        logger.info(f"Finished negative test cases.")

    async def test_wifi(self, endpointId):
        logger.info(f"Get basic information of the endpoint")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[
            (endpointId,
             Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds),
            (endpointId,
             Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds),
            (endpointId, Clusters.NetworkCommissioning.Attributes.MaxNetworks),
            (endpointId, Clusters.NetworkCommissioning.Attributes.FeatureMap)],
            returnClusterObject=True)
        self.log_interface_basic_info(
            res[0][endpointId][Clusters.NetworkCommissioning])
        logger.info(f"Finished getting basic information of the endpoint")

        # Read Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID != NullValue) or (res.lastNetworkingStatus != NullValue) or (res.lastConnectErrorValue != NullValue):
            raise AssertionError(
                f"LastNetworkID, LastNetworkingStatus and LastConnectErrorValue should be Null")

        # Scan networks
        logger.info(f"Scan networks")
        req = Clusters.NetworkCommissioning.Commands.ScanNetworks(
            ssid=b'', breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        # Verify Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID != NullValue) or (res.lastNetworkingStatus == NullValue) or (res.lastConnectErrorValue != NullValue):
            raise AssertionError(
                f"LastNetworkID and LastConnectErrorValue should be Null and LastNetworkingStatus should not be Null")

        # Remove existing network
        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[0][endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 0:
            logger.info(f"Removing existing network")
            req = Clusters.NetworkCommissioning.Commands.RemoveNetwork(
                networkID=networkList[0].networkID, breadcrumb=0)
            res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
            logger.info(f"Received response: {res}")
            if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
                raise AssertionError(
                    f"Unexpected result: {res.networkingStatus}")

        # Add first network
        logger.info(f"Adding first test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(
            ssid=TEST_WIFI_SSID.encode(), credentials=TEST_WIFI_PASS.encode(), breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[0][endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 1:
            raise AssertionError(
                f"Unexpected result: expect 1 networks, but {len(networkList)} networks received")
        if networkList[0].networkID != TEST_WIFI_SSID.encode():
            raise AssertionError(
                f"Unexpected result: first network ID should be 'TestSSID' got {networkList[0].networkID}")

        logger.info(f"Connect to a network")
        req = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
            networkID=TEST_WIFI_SSID.encode(), breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Got response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")
        logger.info(f"Device connected to a network.")

        # Note: On Linux, when connecting to a connected network, it will return immediately, however, it will try a reconnect. This will make the below attribute read return false negative values.
        await asyncio.sleep(5)

        logger.info(f"Check network is connected")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[0][endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 1:
            raise AssertionError(
                f"Unexpected result: expect 1 networks, but {len(networkList)} networks received")
        if networkList[0].networkID != TEST_WIFI_SSID.encode():
            raise AssertionError(
                f"Unexpected result: first network ID should be 'TestSSID' got {networkList[0].networkID}")
        if not networkList[0].connected:
            raise AssertionError(
                f"Unexpected result: network is not marked as connected")

        # Verify Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID == NullValue) or (res.lastNetworkingStatus == NullValue) or (res.lastConnectErrorValue == NullValue):
            raise AssertionError(
                f"LastNetworkID, LastConnectErrorValue and LastNetworkingStatus should not be Null")

    async def test_thread(self, endpointId):
        logger.info(f"Get basic information of the endpoint")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[
            (endpointId,
             Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds),
            (endpointId,
             Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds),
            (endpointId, Clusters.NetworkCommissioning.Attributes.MaxNetworks),
            (endpointId, Clusters.NetworkCommissioning.Attributes.FeatureMap)],
            returnClusterObject=True)
        self.log_interface_basic_info(
            res[0][endpointId][Clusters.NetworkCommissioning])
        logger.info(f"Finished getting basic information of the endpoint")

        # Read Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID != NullValue) or (res.lastNetworkingStatus != NullValue) or (res.lastConnectErrorValue != NullValue):
            raise AssertionError(
                f"LastNetworkID, LastNetworkingStatus and LastConnectErrorValue should be Null")

        # Scan networks
        logger.info(f"Scan networks")
        req = Clusters.NetworkCommissioning.Commands.ScanNetworks(
            ssid=b'', breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        # Verify Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID != NullValue) or (res.lastNetworkingStatus == NullValue) or (res.lastConnectErrorValue != NullValue):
            raise AssertionError(
                f"LastNetworkID and LastConnectErrorValue should be Null and LastNetworkingStatus should not be Null")

        # Remove existing network
        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[0][endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 0:
            logger.info(f"Removing existing network")
            req = Clusters.NetworkCommissioning.Commands.RemoveNetwork(
                networkID=networkList[0].networkID, breadcrumb=0)
            res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
            logger.info(f"Received response: {res}")
            if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
                raise AssertionError(
                    f"Unexpected result: {res.networkingStatus}")

        # Add first network
        logger.info(f"Adding first test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=TEST_THREAD_NETWORK_DATASET_TLVS[0], breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Received response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")

        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[0][endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 1:
            raise AssertionError(
                f"Unexpected result: expect 1 networks, but {len(networkList.Data.value)} networks received")
        if networkList[0].networkID != TEST_THREAD_NETWORK_IDS[0]:
            raise AssertionError(
                f"Unexpected result: first network ID should be {TEST_THREAD_NETWORK_IDS[0]} got {networkList[0].networkID}")

        logger.info(f"Connect to a network")
        req = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
            networkID=TEST_THREAD_NETWORK_IDS[0], breadcrumb=0)
        res = await self._devCtrl.SendCommand(nodeid=self._nodeid, endpoint=endpointId, payload=req)
        logger.info(f"Got response: {res}")
        if res.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise AssertionError(f"Unexpected result: {res.networkingStatus}")
        logger.info(f"Device connected to a network.")

        # Verify Last* attributes
        logger.info(f"Read Last* attributes")
        res = await self.readLastNetworkingStateAttributes(endpointId=endpointId)
        if (res.lastNetworkID == NullValue) or (res.lastNetworkingStatus == NullValue) or (res.lastConnectErrorValue == NullValue):
            raise AssertionError(
                f"LastNetworkID, LastConnectErrorValue and LastNetworkingStatus should not be Null")

        # TODO: Linux Thread driver cannot get infomation of current connected networks.
        '''
        logger.info(f"Check network list")
        res = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(endpointId, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True)
        networkList = res[endpointId][Clusters.NetworkCommissioning].networks
        logger.info(f"Got network list: {networkList}")
        if len(networkList) != 1:
            raise AssertionError(
                f"Unexpected result: expect 1 networks, but {len(networkList.Data.value)} networks received")
        if networkList[0].networkID != TEST_THREAD_NETWORK_IDS[0]:
            raise AssertionError(
                f"Unexpected result: first network ID should be {TEST_THREAD_NETWORK_IDS[0]} got {networkList[0].networkID}")
        if not networkList[0].connected:
            raise AssertionError(
                f"Unexpected result: network is not marked as connected")
        '''

    async def run(self):
        try:
            endpoints = await self._devCtrl.ReadAttribute(nodeid=self._nodeid, attributes=[(Clusters.NetworkCommissioning.Attributes.FeatureMap)], returnClusterObject=True)
            logger.info(endpoints)
            endpoints = endpoints[0]
            for endpoint, obj in endpoints.items():
                clus = obj[Clusters.NetworkCommissioning]
                if clus.featureMap == WIFI_NETWORK_FEATURE_MAP:
                    logger.info(
                        f"Endpoint {endpoint} is configured as WiFi network, run WiFi commissioning test.")
                    await self.test_negative(endpoint)
                    await self.test_wifi(endpoint)
                elif clus.featureMap == THREAD_NETWORK_FEATURE_MAP:
                    logger.info(
                        f"Endpoint {endpoint} is configured as Thread network, run Thread commissioning test.")
                    await self.test_negative(endpoint)
                    await self.test_thread(endpoint)
                else:
                    logger.info(
                        f"Skip endpoint {endpoint} with featureMap {clus.featureMap}")
        except Exception as ex:
            logger.exception(ex)
            return False
        return True
