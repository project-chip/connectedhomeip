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
from dataclasses import dataclass
from inspect import Attribute
import inspect
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
from chip.utils import CommissioningBuildingBlocks
from chip.ChipStack import *
import chip.FabricAdmin
import copy
import secrets

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


_configurable_tests = set()
_configurable_test_sets = set()
_enabled_tests = []
_disabled_tests = []


def SetTestSet(enabled_tests, disabled_tests):
    global _enabled_tests, _disabled_tests
    _enabled_tests = enabled_tests[:]
    _disabled_tests = disabled_tests[:]


def TestIsEnabled(test_name: str):
    enabled_len = -1
    disabled_len = -1
    if 'all' in _enabled_tests:
        enabled_len = 0
    if 'all' in _disabled_tests:
        disabled_len = 0

    for test_item in _enabled_tests:
        if test_name.startswith(test_item) and (len(test_item) > enabled_len):
            enabled_len = len(test_item)

    for test_item in _disabled_tests:
        if test_name.startswith(test_item) and (len(test_item) > disabled_len):
            disabled_len = len(test_item)

    return enabled_len > disabled_len


def test_set(cls):
    _configurable_test_sets.add(cls.__qualname__)
    return cls


def test_case(func):
    test_name = func.__qualname__
    _configurable_tests.add(test_name)

    def CheckEnableBeforeRun(*args, **kwargs):
        if TestIsEnabled(test_name=test_name):
            return func(*args, **kwargs)
        elif inspect.iscoroutinefunction(func):
            # noop, so users can use await as usual
            return asyncio.sleep(0)
    return CheckEnableBeforeRun


def configurable_tests():
    res = [v for v in _configurable_test_sets]
    res.sort()
    return res


def configurable_test_cases():
    res = [v for v in _configurable_tests]
    res.sort()
    return res


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
    def __init__(self, nodeid: int, paaTrustStorePath: str, testCommissioner: bool = False):
        self.chipStack = ChipStack('/tmp/repl_storage.json')
        self.fabricAdmin = chip.FabricAdmin.FabricAdmin(vendorId=0XFFF1,
                                                        fabricId=1, fabricIndex=1)
        self.devCtrl = self.fabricAdmin.NewController(
            nodeid, paaTrustStorePath, testCommissioner)
        self.controllerNodeId = nodeid
        self.logger = logger
        self.paaTrustStorePath = paaTrustStorePath
        logging.getLogger().setLevel(logging.DEBUG)

    async def _GetCommissonedFabricCount(self, nodeid: int):
        data = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.CommissionedFabrics)])
        return data[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CommissionedFabrics]

    def _WaitForOneDiscoveredDevice(self, timeoutSeconds: int = 2):
        print("Waiting for device responses...")
        strlen = 100
        addrStrStorage = ctypes.create_string_buffer(strlen)
        timeout = time.time() + timeoutSeconds
        while (not self.devCtrl.GetIPForDiscoveredDevice(0, addrStrStorage, strlen) and time.time() <= timeout):
            time.sleep(0.2)
        if time.time() > timeout:
            return None
        return ctypes.string_at(addrStrStorage).decode("utf-8")

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

    def TestPaseOnly(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info(
            "Attempting to establish PASE session with device id: {} addr: {}".format(str(nodeid), ip))
        if self.devCtrl.EstablishPASESessionIP(
                ip, setuppin, nodeid) is not None:
            self.logger.info(
                "Failed to establish PASE session with device id: {} addr: {}".format(str(nodeid), ip))
            return False
        self.logger.info(
            "Successfully established PASE session with device id: {} addr: {}".format(str(nodeid), ip))
        return True

    def TestCommissionOnly(self, nodeid: int):
        self.logger.info(
            "Commissioning device with id {}".format(nodeid))
        if not self.devCtrl.Commission(nodeid):
            self.logger.info(
                "Failed to commission device with id {}".format(str(nodeid)))
            return False
        self.logger.info(
            "Successfully commissioned device with id {}".format(str(nodeid)))
        return True

    def TestKeyExchangeBLE(self, discriminator: int, setuppin: int, nodeid: int):
        self.logger.info(
            "Conducting key exchange with device {}".format(discriminator))
        if not self.devCtrl.ConnectBLE(discriminator, setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(discriminator))
            return False
        self.logger.info("Device finished key exchange.")
        return True

    def TestCommissionFailure(self, nodeid: int, failAfter: int):
        self.devCtrl.ResetTestCommissioner()
        a = self.devCtrl.SetTestCommissionerSimulateFailureOnStage(failAfter)
        if not a:
            # We're not going to hit this stage during commissioning so no sense trying, just say it was fine.
            return True

        self.logger.info(
            "Commissioning device, expecting failure after stage {}".format(failAfter))
        self.devCtrl.Commission(nodeid)
        return self.devCtrl.CheckTestCommissionerCallbacks() and self.devCtrl.CheckTestCommissionerPaseConnection(nodeid)

    def TestCommissionFailureOnReport(self, nodeid: int, failAfter: int):
        self.devCtrl.ResetTestCommissioner()
        a = self.devCtrl.SetTestCommissionerSimulateFailureOnReport(failAfter)
        if not a:
            # We're not going to hit this stage during commissioning so no sense trying, just say it was fine.
            return True
        self.logger.info(
            "Commissioning device, expecting failure on report for stage {}".format(failAfter))
        self.devCtrl.Commission(nodeid)
        return self.devCtrl.CheckTestCommissionerCallbacks() and self.devCtrl.CheckTestCommissionerPaseConnection(nodeid)

    def TestCommissioning(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Commissioning device {}".format(ip))
        if not self.devCtrl.CommissionIP(ip, setuppin, nodeid):
            self.logger.info(
                "Failed to finish commissioning device {}".format(ip))
            return False
        self.logger.info("Commissioning finished.")
        return True

    def TestCommissioningWithSetupPayload(self, setupPayload: str, nodeid: int):
        self.logger.info("Commissioning device with setup payload {}".format(setupPayload))
        if not self.devCtrl.CommissionWithCode(setupPayload, nodeid):
            self.logger.info(
                "Failed to finish commissioning device {}".format(setupPayload))
            return False
        self.logger.info("Commissioning finished.")
        return True

    def TestUsedTestCommissioner(self):
        return self.devCtrl.GetTestCommissionerUsed()

    def TestFailsafe(self, nodeid: int):
        self.logger.info("Testing arm failsafe")

        self.logger.info("Setting failsafe on CASE connection")
        err, resp = self.devCtrl.ZCLSend("GeneralCommissioning", "ArmFailSafe", nodeid,
                                         0, 0, dict(expiryLengthSeconds=60, breadcrumb=1), blocking=True)
        if err != 0:
            self.logger.error(
                "Failed to send arm failsafe command error is {} with im response{}".format(err, resp))
            return False

        if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningError.kOk:
            self.logger.error(
                "Incorrect response received from arm failsafe - wanted OK, received {}".format(resp))
            return False

        self.logger.info(
            "Attempting to open basic commissioning window - this should fail since the failsafe is armed")
        try:
            res = asyncio.run(self.devCtrl.SendCommand(
                nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180), timedRequestTimeoutMs=10000))
            # we actually want the exception here because we want to see a failure, so return False here
            self.logger.error(
                'Incorrectly succeeded in opening basic commissioning window')
            return False
        except Exception as ex:
            pass

        # TODO: pipe through the commissioning window opener so we can test enhanced properly. The pake verifier is just garbage because none of of the functions to calculate
        # it or serialize it are available right now. However, this command should fail BEFORE that becomes an issue.
        discriminator = 1111
        salt = secrets.token_bytes(16)
        iterations = 2000
        # not the right size or the right contents, but it won't matter
        verifier = secrets.token_bytes(32)
        self.logger.info(
            "Attempting to open enhanced commissioning window - this should fail since the failsafe is armed")
        try:
            res = asyncio.run(self.devCtrl.SendCommand(nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                commissioningTimeout=180, PAKEVerifier=verifier, discriminator=discriminator, iterations=iterations, salt=salt), timedRequestTimeoutMs=10000))
            # we actually want the exception here because we want to see a failure, so return False here
            self.logger.error(
                'Incorrectly succeeded in opening enhanced commissioning window')
            return False
        except Exception as ex:
            pass

        self.logger.info("Disarming failsafe on CASE connection")
        err, resp = self.devCtrl.ZCLSend("GeneralCommissioning", "ArmFailSafe", nodeid,
                                         0, 0, dict(expiryLengthSeconds=0, breadcrumb=1), blocking=True)
        if err != 0:
            self.logger.error(
                "Failed to send arm failsafe command error is {} with im response{}".format(err, resp))
            return False

        self.logger.info(
            "Opening Commissioning Window - this should succeed since the failsafe was just disarmed")
        try:
            res = asyncio.run(self.devCtrl.SendCommand(
                nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180), timedRequestTimeoutMs=10000))
        except Exception as ex:
            self.logger.error(
                'Failed to open commissioning window after disarming failsafe')
            return False

        self.logger.info(
            "Attempting to arm failsafe over CASE - this should fail since the commissioning window is open")
        err, resp = self.devCtrl.ZCLSend("GeneralCommissioning", "ArmFailSafe", nodeid,
                                         0, 0, dict(expiryLengthSeconds=60, breadcrumb=1), blocking=True)
        if err != 0:
            self.logger.error(
                "Failed to send arm failsafe command error is {} with im response{}".format(err, resp))
            return False
        if resp.errorCode is Clusters.GeneralCommissioning.Enums.CommissioningError.kBusyWithOtherAdmin:
            return True
        return False

    async def TestAddUpdateRemoveFabric(self, nodeid: int):
        logger.info("Testing AddNOC, UpdateNOC and RemoveFabric")

        self.logger.info("Waiting for attribute read for CommissionedFabrics")
        startOfTestFabricCount = await self._GetCommissonedFabricCount(nodeid)

        tempFabric = chip.FabricAdmin.FabricAdmin(vendorId=0xFFF1)
        tempDevCtrl = tempFabric.NewController(self.controllerNodeId, self.paaTrustStorePath)

        self.logger.info("Starting AddNOC using same node ID")
        if not await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(self.devCtrl, tempDevCtrl, nodeid, nodeid):
            self.logger.error("AddNOC failed")
            return False

        expectedFabricCountUntilRemoveFabric = startOfTestFabricCount + 1
        if expectedFabricCountUntilRemoveFabric != await self._GetCommissonedFabricCount(nodeid):
            self.logger.error("Expected commissioned fabric count to change after AddNOC")
            return False

        self.logger.info("Starting UpdateNOC using same node ID")
        if not await CommissioningBuildingBlocks.UpdateNOC(tempDevCtrl, nodeid, nodeid):
            self.logger.error("UpdateNOC using same node ID failed")
            return False

        if expectedFabricCountUntilRemoveFabric != await self._GetCommissonedFabricCount(nodeid):
            self.logger.error("Expected commissioned fabric count to remain unchanged after UpdateNOC")
            return False

        self.logger.info("Starting UpdateNOC using different node ID")
        newNodeIdForUpdateNoc = nodeid + 1
        if not await CommissioningBuildingBlocks.UpdateNOC(tempDevCtrl, nodeid, newNodeIdForUpdateNoc):
            self.logger.error("UpdateNOC using different node ID failed")
            return False

        if expectedFabricCountUntilRemoveFabric != await self._GetCommissonedFabricCount(nodeid):
            self.logger.error("Expected commissioned fabric count to remain unchanged after UpdateNOC with new node ID")
            return False

        # TODO Read using old node ID and expect that it fails.

        currentFabricIndexResponse = await tempDevCtrl.ReadAttribute(newNodeIdForUpdateNoc, [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        updatedNOCFabricIndex = currentFabricIndexResponse[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        removeFabricResponse = await tempDevCtrl.SendCommand(newNodeIdForUpdateNoc, 0, Clusters.OperationalCredentials.Commands.RemoveFabric(updatedNOCFabricIndex))

        if startOfTestFabricCount != await self._GetCommissonedFabricCount(nodeid):
            self.logger.error("Expected fabric count to be the same at the end of test as when it started")
            return False

        tempDevCtrl.Shutdown()
        tempFabric.Shutdown()
        return True

    async def TestCaseEviction(self, nodeid: int):
        self.logger.info("Testing CASE eviction")

        minimumCASESessionsPerFabric = 3
        minimumSupportedFabrics = 16

        #
        # This test exercises the ability to allocate more sessions than are supported in the
        # pool configuration. By going beyond (minimumSupportedFabrics * minimumCASESessionsPerFabric),
        # it starts to test out the eviction logic. This specific test does not validate the specifics
        # of eviction, just that allocation and CASE session establishment proceeds successfully on both
        # the controller and target.
        #
        for x in range(minimumSupportedFabrics * minimumCASESessionsPerFabric * 2):
            self.devCtrl.CloseSession(nodeid)
            await self.devCtrl.ReadAttribute(nodeid, [(Clusters.Basic.Attributes.ClusterRevision)])

        self.logger.info("Testing CASE defunct logic")

        #
        # This tests establishes a subscription on a given CASE session, then marks it defunct (to simulate
        # encountering a transport timeout on the session).
        #
        # Then, we write to the attribute that was subscribed to from a *different* fabric and check to ensure we still get a report
        # on the sub we established previously. Since it was just marked defunct, it should return back to being
        # active and a report should get delivered.
        #
        sawValueChange = False

        def OnValueChange(path: Attribute.TypedAttributePath, transaction: Attribute.SubscriptionTransaction) -> None:
            nonlocal sawValueChange
            self.logger.info("Saw value change!")
            if (path.AttributeType == Clusters.TestCluster.Attributes.Int8u and path.Path.EndpointId == 1):
                sawValueChange = True

        self.logger.info("Testing CASE defunct logic")

        sub = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.TestCluster.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        #
        # This marks the session defunct.
        #
        self.devCtrl.CloseSession(nodeid)

        #
        # Now write the attribute from fabric2, give it some time before checking if the report
        # was received.
        #
        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.Int8u(4))])
        time.sleep(2)

        sub.Shutdown()

        if sawValueChange is False:
            self.logger.error("Didn't see value change in time, likely because sub got terminated due to unexpected session eviction!")
            return False

        #
        # In this test, we're going to setup a subscription on fabric1 through devCtl, then, constantly keep
        # evicting sessions on fabric2 (devCtl2) by cycling through closing sessions followed by issuing a Read. This
        # should result in evictions on the server on fabric2, but not affect any sessions on fabric1. To test this,
        # we're going to setup a subscription to an attribute prior to the cycling reads, and check at the end of the
        # test that it's still valid by writing to an attribute from a *different* fabric, and validating that we see
        # the change on the established subscription. That proves that the session from fabric1 is still valid and untouched.
        #
        self.logger.info("Testing fabric-isolated CASE eviction")

        sawValueChange = False
        sub = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.TestCluster.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        for x in range(minimumSupportedFabrics * minimumCASESessionsPerFabric * 2):
            self.devCtrl2.CloseSession(nodeid)
            await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.Basic.Attributes.ClusterRevision)])

        #
        # Now write the attribute from fabric2, give it some time before checking if the report
        # was received.
        #
        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.Int8u(4))])
        time.sleep(2)

        sub.Shutdown()

        if sawValueChange is False:
            self.logger.error("Didn't see value change in time, likely because sub got terminated due to other fabric (fabric1)")
            return False

        #
        # Do the same test again, but reversing the roles of fabric1 and fabric2.
        #
        self.logger.info("Testing fabric-isolated CASE eviction (reverse)")

        sawValueChange = False
        sub = await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.TestCluster.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        for x in range(minimumSupportedFabrics * minimumCASESessionsPerFabric * 2):
            self.devCtrl.CloseSession(nodeid)
            await self.devCtrl.ReadAttribute(nodeid, [(Clusters.Basic.Attributes.ClusterRevision)])

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.Int8u(4))])
        time.sleep(2)

        sub.Shutdown()

        if sawValueChange is False:
            self.logger.error("Didn't see value change in time, likely because sub got terminated due to other fabric (fabric2)")
            return False

        return True

    async def TestMultiFabric(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Opening Commissioning Window")

        await self.devCtrl.SendCommand(nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180), timedRequestTimeoutMs=10000)

        self.logger.info("Creating 2nd Fabric Admin")
        self.fabricAdmin2 = chip.FabricAdmin.FabricAdmin(vendorId=0xFFF1,
                                                         fabricId=2, fabricIndex=2)

        self.logger.info("Creating Device Controller on 2nd Fabric")
        self.devCtrl2 = self.fabricAdmin2.NewController(
            self.controllerNodeId, self.paaTrustStorePath)

        if not self.devCtrl2.CommissionIP(ip, setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(ip))
            return False

        #
        # Shut-down all the controllers (which will free them up)
        #
        self.logger.info(
            "Shutting down controllers & fabrics and re-initing stack...")

        ChipDeviceCtrl.ChipDeviceController.ShutdownAll()
        chip.FabricAdmin.FabricAdmin.ShutdownAll()

        self.logger.info("Shutdown completed, starting new controllers...")

        self.fabricAdmin = chip.FabricAdmin.FabricAdmin(vendorId=0XFFF1,
                                                        fabricId=1, fabricIndex=1)
        fabricAdmin2 = chip.FabricAdmin.FabricAdmin(vendorId=0xFFF1,
                                                    fabricId=2, fabricIndex=2)

        self.devCtrl = self.fabricAdmin.NewController(
            self.controllerNodeId, self.paaTrustStorePath)
        self.devCtrl2 = fabricAdmin2.NewController(
            self.controllerNodeId, self.paaTrustStorePath)

        self.logger.info("Waiting for attribute reads...")

        data1 = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False)
        data2 = await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False)

        # Read out noclist from each fabric, and each should contain two NOCs.
        nocList1 = data1[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.NOCs]
        nocList2 = data2[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.NOCs]

        if (len(nocList1) != 2 or len(nocList2) != 2):
            self.logger.error("Got back invalid nocList")
            return False

        data1 = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)], fabricFiltered=False)
        data2 = await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)], fabricFiltered=False)

        # Read out current fabric from each fabric, and both should be different.
        self.currentFabric1 = data1[0][Clusters.OperationalCredentials][
            Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        self.currentFabric2 = data2[0][Clusters.OperationalCredentials][
            Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        if (self.currentFabric1 == self.currentFabric2):
            self.logger.error(
                "Got back fabric indices that match for two different fabrics!")
            return False

        self.logger.info("Attribute reads completed...")
        return True

    async def TestFabricSensitive(self, nodeid: int):
        expectedDataFabric1 = [
            Clusters.TestCluster.Structs.TestFabricScoped(),
            Clusters.TestCluster.Structs.TestFabricScoped()
        ]

        expectedDataFabric1[0].fabricIndex = 100
        expectedDataFabric1[0].fabricSensitiveInt8u = 33
        expectedDataFabric1[0].optionalFabricSensitiveInt8u = 34
        expectedDataFabric1[0].nullableFabricSensitiveInt8u = 35
        expectedDataFabric1[0].nullableOptionalFabricSensitiveInt8u = Clusters.Types.NullValue
        expectedDataFabric1[0].fabricSensitiveCharString = "alpha1"
        expectedDataFabric1[0].fabricSensitiveStruct.a = 36
        expectedDataFabric1[0].fabricSensitiveInt8uList = [1, 2, 3, 4]

        expectedDataFabric1[1].fabricIndex = 100
        expectedDataFabric1[1].fabricSensitiveInt8u = 43
        expectedDataFabric1[1].optionalFabricSensitiveInt8u = 44
        expectedDataFabric1[1].nullableFabricSensitiveInt8u = 45
        expectedDataFabric1[1].nullableOptionalFabricSensitiveInt8u = Clusters.Types.NullValue
        expectedDataFabric1[1].fabricSensitiveCharString = "alpha2"
        expectedDataFabric1[1].fabricSensitiveStruct.a = 46
        expectedDataFabric1[1].fabricSensitiveInt8uList = [2, 3, 4, 5]

        self.logger.info("Writing data from fabric1...")

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped(expectedDataFabric1))])

        expectedDataFabric2 = copy.deepcopy(expectedDataFabric1)

        expectedDataFabric2[0].fabricSensitiveInt8u = 133
        expectedDataFabric2[0].optionalFabricSensitiveInt8u = 134
        expectedDataFabric2[0].nullableFabricSensitiveInt8u = 135
        expectedDataFabric2[0].fabricSensitiveCharString = "beta1"
        expectedDataFabric2[0].fabricSensitiveStruct.a = 136
        expectedDataFabric2[0].fabricSensitiveInt8uList = [11, 12, 13, 14]

        expectedDataFabric2[1].fabricSensitiveInt8u = 143
        expectedDataFabric2[1].optionalFabricSensitiveInt8u = 144
        expectedDataFabric2[1].nullableFabricSensitiveInt8u = 145
        expectedDataFabric2[1].fabricSensitiveCharString = "beta2"
        expectedDataFabric2[1].fabricSensitiveStruct.a = 146
        expectedDataFabric2[1].fabricSensitiveStruct.f = 147
        expectedDataFabric2[1].fabricSensitiveInt8uList = [12, 13, 14, 15]

        self.logger.info("Writing data from fabric2...")

        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped(expectedDataFabric2))])

        #
        # Now read the data back filtered from fabric1 and ensure it matches.
        #
        self.logger.info("Reading back data from fabric1...")

        data = await self.devCtrl.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)])
        readListDataFabric1 = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]

        #
        # Update the expected data's fabric index to that we just read back
        # before we attempt to compare the data
        #
        expectedDataFabric1[0].fabricIndex = self.currentFabric1
        expectedDataFabric1[1].fabricIndex = self.currentFabric1

        self.logger.info("Comparing data on fabric1...")
        if (expectedDataFabric1 != readListDataFabric1):
            raise AssertionError("Got back mismatched data")

        self.logger.info("Reading back data from fabric2...")

        data = await self.devCtrl2.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)])
        readListDataFabric2 = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]

        #
        # Update the expected data's fabric index to that we just read back
        # before we attempt to compare the data
        #
        expectedDataFabric2[0].fabricIndex = self.currentFabric2
        expectedDataFabric2[1].fabricIndex = self.currentFabric2

        self.logger.info("Comparing data on fabric2...")
        if (expectedDataFabric2 != readListDataFabric2):
            raise AssertionError("Got back mismatched data")

        self.logger.info(
            "Reading back unfiltered data across all fabrics from fabric1...")

        def CompareUnfilteredData(accessingFabric, otherFabric, expectedData):
            index = 0

            self.logger.info(
                f"Comparing data from accessing fabric {accessingFabric}...")

            for item in readListDataFabric:
                if (item.fabricIndex == accessingFabric):
                    if (index == 2):
                        raise AssertionError(
                            "Got back more data than expected")

                    if (item != expectedData[index]):
                        raise AssertionError("Got back mismatched data")

                    index = index + 1
                else:
                    #
                    # We should not be able to see any fabric sensitive data from the non accessing fabric.
                    # Aside from the fabric index, everything else in TestFabricScoped is marked sensitive so we should
                    # only see defaults for that data. Instantiate an instance of that struct
                    # which should automatically be initialized with defaults and compare that
                    # against what we got back.
                    #
                    expectedDefaultData = Clusters.TestCluster.Structs.TestFabricScoped()
                    expectedDefaultData.fabricIndex = otherFabric

                    if (item != expectedDefaultData):
                        raise AssertionError("Got back mismatched data")

        data = await self.devCtrl.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)], fabricFiltered=False)
        readListDataFabric = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]
        CompareUnfilteredData(self.currentFabric1,
                              self.currentFabric2, expectedDataFabric1)

        data = await self.devCtrl2.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)], fabricFiltered=False)
        readListDataFabric = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]
        CompareUnfilteredData(self.currentFabric2,
                              self.currentFabric1, expectedDataFabric2)

        self.logger.info("Writing smaller list from alpha (again)")

        expectedDataFabric1[0].fabricIndex = 100
        expectedDataFabric1[0].fabricSensitiveInt8u = 53
        expectedDataFabric1[0].optionalFabricSensitiveInt8u = 54
        expectedDataFabric1[0].nullableFabricSensitiveInt8u = 55
        expectedDataFabric1[0].nullableOptionalFabricSensitiveInt8u = Clusters.Types.NullValue
        expectedDataFabric1[0].fabricSensitiveCharString = "alpha3"
        expectedDataFabric1[0].fabricSensitiveStruct.a = 56
        expectedDataFabric1[0].fabricSensitiveInt8uList = [51, 52, 53, 54]

        expectedDataFabric1.pop(1)

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped(expectedDataFabric1))])

        self.logger.info(
            "Reading back data (again) from fabric2 to ensure it hasn't changed")

        data = await self.devCtrl2.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)])
        readListDataFabric2 = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]
        if (expectedDataFabric2 != readListDataFabric2):
            raise AssertionError("Got back mismatched data")

        self.logger.info(
            "Reading back data (again) from fabric1 to ensure it hasn't changed")

        data = await self.devCtrl.ReadAttribute(nodeid, [(1, Clusters.TestCluster.Attributes.ListFabricScoped)])
        readListDataFabric1 = data[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]

        self.logger.info("Comparing data on fabric1...")
        expectedDataFabric1[0].fabricIndex = self.currentFabric1
        if (expectedDataFabric1 != readListDataFabric1):
            raise AssertionError("Got back mismatched data")

    def TestCloseSession(self, nodeid: int):
        self.logger.info(f"Closing sessions with device {nodeid}")
        try:
            err = self.devCtrl.CloseSession(nodeid)
            if err != 0:
                self.logger.exception(
                    f"Failed to close sessions with device {nodeid}: {err}")
                return False
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
            addr = None

            start = time.time()
            while not addr:
                addr = self.devCtrl.GetAddressAndPort(nodeid)
                if time.time() - start > 10:
                    self.logger.exception(f"Timeout waiting for address...")
                    break

                if not addr:
                    time.sleep(0.2)

            if not addr:
                self.logger.exception(f"Addr is missing...")
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
            "SoftwareVersion": 1,
            "SoftwareVersionString": "1.0",
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

            #
            # Clean-up by shutting down the sub. Otherwise, we're going to get callbacks through OnValueChange on what will soon become an invalid
            # execution context above.
            #
            subscription.Shutdown()

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
            clusterInfo = cluster.GetClusterInfoById(0xFFF1FC05)  # TestCluster
            if clusterInfo["clusterName"] != "TestCluster":
                raise Exception(
                    f"Wrong cluster info clusterName: {clusterInfo['clusterName']} expected TestCluster")
        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False
        return True
