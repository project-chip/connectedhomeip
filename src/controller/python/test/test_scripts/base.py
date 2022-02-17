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
from inspect import Attribute
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
import chip.clusters as Clusters
import chip.clusters.Attribute as Attribute
from chip.ChipStack import *
import chip.FabricAdmin

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
    def __init__(self, nodeid: int, testCommissioner: bool = False):
        self.chipStack = ChipStack('/tmp/repl_storage.json')
        self.fabricAdmin = chip.FabricAdmin.FabricAdmin(
            fabricId=1, fabricIndex=1)
        self.devCtrl = self.fabricAdmin.NewController(nodeid, testCommissioner)
        self.controllerNodeId = nodeid
        self.logger = logger

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
        if not self.devCtrl.CommissionIP(ip.encode("utf-8"), setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(ip))
            return False
        self.logger.info("Device finished key exchange.")
        return True

    def TestUsedTestCommissioner(self):
        return self.devCtrl.GetTestCommissionerUsed()

    async def TestMultiFabric(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Opening Commissioning Window")

        await self.devCtrl.SendCommand(nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(100), timedRequestTimeoutMs=10000)

        self.logger.info("Creating 2nd Fabric Admin")
        fabricAdmin2 = chip.FabricAdmin.FabricAdmin(fabricId=2, fabricIndex=2)

        self.logger.info("Creating Device Controller on 2nd Fabric")
        devCtrl2 = fabricAdmin2.NewController(self.controllerNodeId)

        if not devCtrl2.CommissionIP(ip.encode("utf-8"), setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(ip))
            return False

        #
        # Shutting down controllers results in races where the resolver still delivers
        # resolution results to a now destroyed controller. Add a sleep for now to work around
        # it while #14555 is being resolved.
        time.sleep(1)

        #
        # Shut-down all the controllers (which will free them up as well as de-initialize the
        # stack as well.
        #
        self.logger.info(
            "Shutting down controllers & fabrics and re-initing stack...")

        ChipDeviceCtrl.ChipDeviceController.ShutdownAll()
        chip.FabricAdmin.FabricAdmin.ShutdownAll()

        self.fabricAdmin = chip.FabricAdmin.FabricAdmin(
            fabricId=1, fabricIndex=1)
        fabricAdmin2 = chip.FabricAdmin.FabricAdmin(fabricId=2, fabricIndex=2)

        self.devCtrl = self.fabricAdmin.NewController(self.controllerNodeId)
        devCtrl2 = fabricAdmin2.NewController(self.controllerNodeId)

        data1 = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False)
        data2 = await devCtrl2.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False)

        # Read out noclist from each fabric, and each should contain two NOCs.
        nocList1 = data1[0][0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.NOCs]
        nocList2 = data2[0][0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.NOCs]

        if (len(nocList1) != 2 or len(nocList2) != 2):
            self.logger.error("Got back invalid nocList")
            return False

        data1 = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)], fabricFiltered=False)
        data2 = await devCtrl2.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)], fabricFiltered=False)

        # Read out current fabric from each fabric, and both should be different.
        currentFabric1 = data1[0][0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        currentFabric2 = data2[0][0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        if (currentFabric1 == currentFabric2):
            self.logger.error(
                "Got back fabric indices that match for two different fabrics!")
            return False

        devCtrl2.Shutdown()
        fabricAdmin2.Shutdown()

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

    def SetNetworkCommissioningParameters(self, dataset: str):
        self.logger.info("Setting network commissioning parameters")
        self.devCtrl.SetThreadOperationalDataset(bytes.fromhex(dataset))
        return True

    def TestOnOffCluster(self, nodeid: int, endpoint: int, group: int):
        self.logger.info(
            "Sending On/Off commands to device {} endpoint {}".format(nodeid, endpoint))
        err, resp = self.devCtrl.ZCLSend("OnOff", "On", nodeid,
                                         endpoint, group, {}, blocking=True)
        if err != 0:
            self.logger.error(
                "failed to send OnOff.On: error is {} with im response{}".format(err, resp))
            return False
        err, resp = self.devCtrl.ZCLSend("OnOff", "Off", nodeid,
                                         endpoint, group, {}, blocking=True)
        if err != 0:
            self.logger.error(
                "failed to send OnOff.Off: error is {} with im response {}".format(err, resp))
            return False
        return True

    def TestLevelControlCluster(self, nodeid: int, endpoint: int, group: int):
        self.logger.info(
            f"Sending MoveToLevel command to device {nodeid} endpoint {endpoint}")
        try:
            commonArgs = dict(transitionTime=0, optionMask=1, optionOverride=1)

            # Move to 1
            self.devCtrl.ZCLSend("LevelControl", "MoveToLevel", nodeid,
                                 endpoint, group, dict(**commonArgs, level=1), blocking=True)
            res = self.devCtrl.ZCLReadAttribute(cluster="LevelControl",
                                                attribute="CurrentLevel",
                                                nodeid=nodeid,
                                                endpoint=endpoint,
                                                groupid=group)
            TestResult("Read attribute LevelControl.CurrentLevel",
                       res).assertValueEqual(1)

            # Move to 254
            self.devCtrl.ZCLSend("LevelControl", "MoveToLevel", nodeid,
                                 endpoint, group, dict(**commonArgs, level=254), blocking=True)
            res = self.devCtrl.ZCLReadAttribute(cluster="LevelControl",
                                                attribute="CurrentLevel",
                                                nodeid=nodeid,
                                                endpoint=endpoint,
                                                groupid=group)
            TestResult("Read attribute LevelControl.CurrentLevel",
                       res).assertValueEqual(254)

            return True
        except Exception as ex:
            self.logger.exception(f"Level cluster test failed: {ex}")
            return False

    def TestResolve(self, nodeid):
        self.logger.info(
            "Resolve: node id = {:08x}".format(nodeid))
        try:
            self.devCtrl.ResolveNode(nodeid=nodeid)
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
            "VendorID": 0xFFF1,
            "ProductName": "TEST_PRODUCT",
            "ProductID": 0x8001,
            "NodeLabel": "Test",
            "Location": "XX",
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
            expected_status: IM.Status = IM.Status.Success

        requests = [
            AttributeWriteRequest("Basic", "NodeLabel", "Test"),
            AttributeWriteRequest("Basic", "Location",
                                  "a pretty loooooooooooooog string", IM.Status.InvalidValue),
        ]
        failed_zcl = []
        for req in requests:
            try:
                try:
                    self.devCtrl.ZCLWriteAttribute(cluster=req.cluster,
                                                   attribute=req.attribute,
                                                   nodeid=nodeid,
                                                   endpoint=endpoint,
                                                   groupid=group,
                                                   value=req.value)
                    if req.expected_status != IM.Status.Success:
                        raise AssertionError(
                            f"Write attribute {req.cluster}.{req.attribute} expects failure but got success response")
                except Exception as ex:
                    if req.expected_status != IM.Status.Success:
                        continue
                    else:
                        raise ex
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
        desiredPath = None
        receivedUpdate = 0
        updateLock = threading.Lock()
        updateCv = threading.Condition(updateLock)

        def OnValueChange(path: Attribute.TypedAttributePath, transaction: Attribute.SubscriptionTransaction) -> None:
            nonlocal desiredPath, updateCv, updateLock, receivedUpdate
            if path.Path != desiredPath:
                return

            data = transaction.GetAttribute(path)
            logger.info(
                f"Received report from server: path: {path.Path}, value: {data}")
            with updateLock:
                receivedUpdate += 1
                updateCv.notify_all()

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
            desiredPath = Clusters.Attribute.AttributePath(
                EndpointId=1, ClusterId=6, AttributeId=0)
            # OnOff Cluster, OnOff Attribute
            subscription = self.devCtrl.ZCLSubscribeAttribute(
                "OnOff", "OnOff", nodeid, endpoint, 1, 10)
            subscription.SetAttributeUpdateCallback(OnValueChange)
            changeThread = _conductAttributeChange(
                self.devCtrl, nodeid, endpoint)
            # Reset the number of subscriptions received as subscribing causes a callback.
            changeThread.start()
            with updateCv:
                while receivedUpdate < 5:
                    # We should observe 5 attribute changes
                    # The changing thread will change the value after 3 seconds. If we're waiting more than 10, assume something
                    # is really wrong and bail out here with some information.
                    if not updateCv.wait(10.0):
                        self.logger.error(
                            f"Failed to receive subscription update")
                        break

            # thread changes 5 times, and sleeps for 3 seconds in between. Add an additional 3 seconds of slack. Timeout is in seconds.
            changeThread.join(18.0)
            if changeThread.is_alive():
                # Thread join timed out
                self.logger.error(f"Failed to join change thread")
                return False
            return True if receivedUpdate == 5 else False

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
