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

from dataclasses import dataclass
from typing import Any
import typing
from chip import ChipDeviceCtrl
from chip import ChipCommissionableNodeCtrl
import chip.interaction_model as IM
import threading
import os
import sys
import logging
import time
import ctypes

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
sh.setStream(sys.stdout)
logger.addHandler(sh)


def TestFail(message):
    logger.fatal("Testfail: {}".format(message))
    os._exit(1)


def FailIfNot(cond, message):
    if not cond:
        TestFail(message)


class TestTimeout(threading.Thread):
    def __init__(self, timeout: int):
        threading.Thread.__init__(self)
        self._timeout = timeout
        self._should_stop = False
        self._cv = threading.Condition()

    def stop(self):
        with self._cv:
            self._should_stop = True
            self._cv.notify_all()
        self.join()

    def run(self):
        stop_time = time.time() + self._timeout
        logger.info("Test timeout set to {} seconds".format(self._timeout))
        with self._cv:
            wait_time = stop_time - time.time()
            while wait_time > 0 and not self._should_stop:
                self._cv.wait(wait_time)
                wait_time = stop_time - time.time()
        if time.time() > stop_time:
            TestFail("Timeout")


class TestResult:
    def __init__(self, operationName, result):
        self.operationName = operationName
        self.result = result

    def assertStatusEqual(self, expected):
        if self.result is None:
            raise Exception(f"{self.operationName}: no result got")
        if self.result.status != expected:
            raise Exception(
                f"{self.operationName}: expected status {expected}, got {self.result.status}")
        return self

    def assertValueEqual(self, expected):
        self.assertStatusEqual(0)
        if self.result is None:
            raise Exception(f"{self.operationName}: no result got")
        if self.result.value != expected:
            raise Exception(
                f"{self.operationName}: expected value {expected}, got {self.result.value}")
        return self


class BaseTestHelper:
    def __init__(self, nodeid: int):
        self.devCtrl = ChipDeviceCtrl.ChipDeviceController(
            controllerNodeId=nodeid)
        self.logger = logger
        self.commissionableNodeCtrl = ChipCommissionableNodeCtrl.ChipCommissionableNodeController()

    def _WaitForOneDiscoveredDevice(self, timeoutSeconds: int = 2):
        print("Waiting for device responses...")
        strlen = 100
        addrStrStorage = ctypes.create_string_buffer(strlen)
        timeout = time.time() + timeoutSeconds
        while (not self.devCtrl.GetIPForDiscoveredDevice(0, addrStrStorage, strlen) and time.time() <= timeout):
            time.sleep(0.2)
        if time.time() > timeout:
            return None
        return ctypes.string_at(addrStrStorage)

    def TestDiscovery(self, discriminator: int):
        self.logger.info(
            f"Discovering commissionable nodes with discriminator {discriminator}")
        self.devCtrl.DiscoverCommissionableNodesLongDiscriminator(
            ctypes.c_uint16(int(discriminator)))
        res = self._WaitForOneDiscoveredDevice()
        if not res:
            self.logger.info(
                f"Device not found")
            return False
        self.logger.info(f"Found device at {res}")
        return res

    def TestKeyExchange(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Conducting key exchange with device {}".format(ip))
        if not self.devCtrl.ConnectIP(ip.encode("utf-8"), setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(ip))
            return False
        self.logger.info("Device finished key exchange.")
        return True

    def TestCloseSession(self, nodeid: int):
        self.logger.info(f"Closing sessions with device {nodeid}")
        try:
            self.devCtrl.CloseSession(nodeid)
            return True
        except Exception as ex:
            self.logger.exception(
                f"Failed to close sessions with device {nodeid}: {ex}")
            return False

    def TestNetworkCommissioning(self, nodeid: int, endpoint: int, group: int, dataset: str, network_id: str):
        self.logger.info("Commissioning network to device {}".format(nodeid))
        try:
            self.devCtrl.ZCLSend("NetworkCommissioning", "AddThreadNetwork", nodeid, endpoint, group, {
                "operationalDataset": bytes.fromhex(dataset),
                "breadcrumb": 0,
                "timeoutMs": 1000}, blocking=True)
        except Exception as ex:
            self.logger.exception("Failed to send AddThreadNetwork command")
            return False
        self.logger.info(
            "Send EnableNetwork command to device {}".format(nodeid))
        try:
            err, resp = self.devCtrl.ZCLSend("NetworkCommissioning", "EnableNetwork", nodeid, endpoint, group, {
                "networkID": bytes.fromhex(network_id),
                "breadcrumb": 0,
                "timeoutMs": 1000}, blocking=True)
            if err != 0 or resp is None or resp.ProtocolCode != 0:
                self.logger.error(
                    "failed to send enable network: error is {} with im response{}".format(err, resp))
                return False
        except Exception as ex:
            self.logger.exception("Failed to send EnableNetwork command")
            return False
        return True

    def TestOnOffCluster(self, nodeid: int, endpoint: int, group: int):
        self.logger.info(
            "Sending On/Off commands to device {} endpoint {}".format(nodeid, endpoint))
        err, resp = self.devCtrl.ZCLSend("OnOff", "On", nodeid,
                                         endpoint, group, {}, blocking=True)
        if err != 0 or resp is None or resp.ProtocolCode != 0:
            self.logger.error(
                "failed to send OnOff.On: error is {} with im response{}".format(err, resp))
            return False
        err, resp = self.devCtrl.ZCLSend("OnOff", "Off", nodeid,
                                         endpoint, group, {}, blocking=True)
        if err != 0 or resp is None or resp.ProtocolCode != 0:
            self.logger.error(
                "failed to send OnOff.Off: error is {} with im response {}".format(err, resp))
            return False
        return True

    def TestLevelControlCluster(self, nodeid: int, endpoint: int, group: int):
        self.logger.info(
            f"Sending MoveToLevel command to device {nodeid} endpoint {endpoint}")
        try:
            commonArgs = dict(transitionTime=0, optionMask=0, optionOverride=0)

            # Move to 0
            self.devCtrl.ZCLSend("LevelControl", "MoveToLevel", nodeid,
                                 endpoint, group, dict(**commonArgs, level=0), blocking=True)
            res = self.devCtrl.ZCLReadAttribute(cluster="LevelControl",
                                                attribute="CurrentLevel",
                                                nodeid=nodeid,
                                                endpoint=endpoint,
                                                groupid=group)
            TestResult("Read attribute LevelControl.CurrentLevel",
                       res).assertValueEqual(0)

            # Move to 255
            self.devCtrl.ZCLSend("LevelControl", "MoveToLevel", nodeid,
                                 endpoint, group, dict(**commonArgs, level=255), blocking=True)
            res = self.devCtrl.ZCLReadAttribute(cluster="LevelControl",
                                                attribute="CurrentLevel",
                                                nodeid=nodeid,
                                                endpoint=endpoint,
                                                groupid=group)
            TestResult("Read attribute LevelControl.CurrentLevel",
                       res).assertValueEqual(255)

            return True
        except Exception as ex:
            self.logger.exception(f"Level cluster test failed: {ex}")
            return False

    def TestResolve(self, nodeid):
        fabricid = self.devCtrl.GetCompressedFabricId()
        self.logger.info(
            "Resolve: node id = {:08x} (compressed) fabric id = {:08x}".format(nodeid, fabricid))
        try:
            self.devCtrl.ResolveNode(fabricid=fabricid, nodeid=nodeid)
            addr = self.devCtrl.GetAddressAndPort(nodeid)
            if not addr:
                return False
            self.logger.info(f"Resolved address: {addr[0]}:{addr[1]}")
            return True
        except Exception as ex:
            self.logger.exception("Failed to resolve. {}".format(ex))
            return False

    def TestReadBasicAttributes(self, nodeid: int, endpoint: int, group: int):
        basic_cluster_attrs = {
            "VendorName": "TEST_VENDOR",
            "VendorID": 9050,
            "ProductName": "TEST_PRODUCT",
            "ProductID": 65279,
            "UserLabel": "",
            "Location": "",
            "HardwareVersion": 0,
            "HardwareVersionString": "TEST_VERSION",
            "SoftwareVersion": 0,
            "SoftwareVersionString": "prerelease",
        }
        failed_zcl = {}
        for basic_attr, expected_value in basic_cluster_attrs.items():
            try:
                res = self.devCtrl.ZCLReadAttribute(cluster="Basic",
                                                    attribute=basic_attr,
                                                    nodeid=nodeid,
                                                    endpoint=endpoint,
                                                    groupid=group)
                TestResult(f"Read attribute {basic_attr}", res).assertValueEqual(
                    expected_value)
            except Exception as ex:
                failed_zcl[basic_attr] = str(ex)
        if failed_zcl:
            self.logger.exception(f"Following attributes failed: {failed_zcl}")
            return False
        return True

    def TestWriteBasicAttributes(self, nodeid: int, endpoint: int, group: int):
        @dataclass
        class AttributeWriteRequest:
            cluster: str
            attribute: str
            value: Any
            expected_status: IM.ProtocolCode = IM.ProtocolCode.Success

        requests = [
            AttributeWriteRequest("Basic", "UserLabel", "Test"),
            AttributeWriteRequest("Basic", "Location",
                                  "a pretty loooooooooooooog string", IM.ProtocolCode.InvalidValue),
        ]
        failed_zcl = []
        for req in requests:
            try:
                res = self.devCtrl.ZCLWriteAttribute(cluster=req.cluster,
                                                     attribute=req.attribute,
                                                     nodeid=nodeid,
                                                     endpoint=endpoint,
                                                     groupid=group,
                                                     value=req.value)
                TestResult(f"Write attribute {req.cluster}.{req.attribute}", res).assertStatusEqual(
                    req.expected_status)
                if req.expected_status != IM.ProtocolCode.Success:
                    # If the write interaction is expected to success, proceed to verify it.
                    continue
                res = self.devCtrl.ZCLReadAttribute(
                    cluster=req.cluster, attribute=req.attribute, nodeid=nodeid, endpoint=endpoint, groupid=group)
                TestResult(f"Read attribute {req.cluster}.{req.attribute}", res).assertValueEqual(
                    req.value)
            except Exception as ex:
                failed_zcl.append(str(ex))
        if failed_zcl:
            self.logger.exception(f"Following attributes failed: {failed_zcl}")
            return False
        return True

    def TestSubscription(self, nodeid: int, endpoint: int):
        class _subscriptionHandler(IM.OnSubscriptionReport):
            def __init__(self, path: IM.AttributePath, logger: logging.Logger):
                super(_subscriptionHandler, self).__init__()
                self.subscriptionReceived = 0
                self.path = path
                self.countLock = threading.Lock()
                self.cv = threading.Condition(self.countLock)
                self.logger = logger

            def OnData(self, path: IM.AttributePath, subscriptionId: int, data: typing.Any) -> None:
                if path != self.path:
                    return
                logger.info(
                    f"Received report from server: path: {path}, value: {data}, subscriptionId: {subscriptionId}")
                with self.countLock:
                    self.subscriptionReceived += 1
                    self.cv.notify_all()

        class _conductAttributeChange(threading.Thread):
            def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceController, nodeid: int, endpoint: int):
                super(_conductAttributeChange, self).__init__()
                self.nodeid = nodeid
                self.endpoint = endpoint
                self.devCtrl = devCtrl

            def run(self):
                for i in range(5):
                    time.sleep(3)
                    self.devCtrl.ZCLSend(
                        "OnOff", "Toggle", self.nodeid, self.endpoint, 0, {})

        try:
            subscribedPath = IM.AttributePath(
                nodeId=nodeid, endpointId=endpoint, clusterId=6, attributeId=0)
            # OnOff Cluster, OnOff Attribute
            handler = _subscriptionHandler(subscribedPath, self.logger)
            IM.SetAttributeReportCallback(subscribedPath, handler)
            self.devCtrl.ZCLSubscribeAttribute(
                "OnOff", "OnOff", nodeid, endpoint, 1, 10)
            changeThread = _conductAttributeChange(
                self.devCtrl, nodeid, endpoint)
            changeThread.start()
            with handler.cv:
                while handler.subscriptionReceived < 5:
                    # We should observe 10 attribute changes
                    handler.cv.wait()
            return True
        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False

        return True

    def TestNonControllerAPIs(self):
        '''
        This function validates various APIs provided by chip package which is not related to controller.
        TODO: Add more tests for APIs
        '''
        try:
            cluster = self.devCtrl.GetClusterHandler()
            clusterInfo = cluster.GetClusterInfoById(0x50F)  # TestCluster
            if clusterInfo["clusterName"] != "TestCluster":
                raise Exception(
                    f"Wrong cluster info clusterName: {clusterInfo['clusterName']} expected TestCluster")
        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False
        return True
