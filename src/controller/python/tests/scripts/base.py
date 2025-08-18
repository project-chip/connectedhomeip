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
import copy
import ctypes
import faulthandler
import hashlib
import inspect
import logging
import os
import secrets
import struct
import sys
import threading
import time
from dataclasses import dataclass
from typing import Any

from cirque_restart_remote_device import restartRemoteDevice
from ecdsa import NIST256p

import matter.CertificateAuthority
import matter.clusters as Clusters
import matter.clusters.Attribute as Attribute
import matter.discovery
import matter.FabricAdmin
import matter.interaction_model as IM
import matter.native
from matter import ChipDeviceCtrl
from matter.ChipStack import ChipStack
from matter.crypto import p256keypair
from matter.exceptions import ChipStackException
from matter.storage import PersistentStorageJSON
from matter.utils import CommissioningBuildingBlocks

DEFAULT_REPL_STORAGE_PATH = '/tmp/repl-storage.json'

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
sh.setStream(sys.stdout)
logger.addHandler(sh)


def GenerateVerifier(passcode: int, salt: bytes, iterations: int) -> bytes:
    ws_len = NIST256p.baselen + 8
    ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', passcode), salt, iterations, ws_len * 2)
    w0 = int.from_bytes(ws[:ws_len], byteorder='big') % NIST256p.order
    w1 = int.from_bytes(ws[ws_len:], byteorder='big') % NIST256p.order
    L = NIST256p.generator * w1

    return w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')


def TestFail(message, doCrash=False):
    logger.fatal("Testfail: {}".format(message))

    if (doCrash):
        logger.fatal("--------------------------------")
        logger.fatal("Backtrace of all Python threads:")
        logger.fatal("--------------------------------")

        #
        # Let's dump the Python backtrace for all threads, since the backtrace we'll
        # get from gdb (if one is attached) won't give us good Python symbol information.
        #
        faulthandler.dump_traceback()

        #
        # Cause a crash to happen so that we can actually get a meaningful
        # backtrace when run through GDB.
        #
        matter.native.GetLibraryHandle().pychip_CauseCrash()
    else:
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
    res = sorted([v for v in _configurable_test_sets])
    return res


def configurable_test_cases():
    res = sorted([v for v in _configurable_tests])
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
            TestFail("Timeout", doCrash=True)


class BaseTestHelper:
    def __init__(self, nodeid: int, paaTrustStorePath: str, testCommissioner: bool = False,
                 keypair: p256keypair.P256Keypair = None):
        matter.native.Init()

        self.chipStack = ChipStack(PersistentStorageJSON(DEFAULT_REPL_STORAGE_PATH), enableServerInteractions=True)
        self.certificateAuthorityManager = matter.CertificateAuthority.CertificateAuthorityManager(chipStack=self.chipStack)
        self.certificateAuthority = self.certificateAuthorityManager.NewCertificateAuthority()
        self.fabricAdmin = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
        self.devCtrl = self.fabricAdmin.NewController(
            nodeid, paaTrustStorePath, testCommissioner, keypair=keypair)
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

    async def TestDiscovery(self, discriminator: int):
        self.logger.info(
            f"Discovering commissionable nodes with discriminator {discriminator}")
        res = await self.devCtrl.DiscoverCommissionableNodes(
            matter.discovery.FilterType.LONG_DISCRIMINATOR, discriminator, stopOnFirst=True, timeoutSecond=3)
        if not res:
            self.logger.info(
                "Device not found")
            return False
        self.logger.info(f"Found device {res[0]}")
        return res[0]

    def CreateNewFabricController(self):
        self.logger.info("Creating 2nd Fabric Admin")
        self.fabricAdmin2 = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)

        self.logger.info("Creating Device Controller on 2nd Fabric")
        self.devCtrl2 = self.fabricAdmin2.NewController(
            self.controllerNodeId, self.paaTrustStorePath)
        return True

    async def TestRevokeCommissioningWindow(self, ip: str, setuppin: int, nodeid: int):
        await self.devCtrl.SendCommand(
            nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180), timedRequestTimeoutMs=10000)
        if not await self.TestPaseOnly(ip=ip, setuppin=setuppin, nodeid=nodeid, devCtrl=self.devCtrl2):
            return False

        await self.devCtrl2.SendCommand(
            nodeid, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=180, breadcrumb=0))

        await self.devCtrl.SendCommand(
            nodeid, 0, Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(), timedRequestTimeoutMs=10000)
        await self.devCtrl.SendCommand(
            nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180), timedRequestTimeoutMs=10000)
        await self.devCtrl.SendCommand(
            nodeid, 0, Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(), timedRequestTimeoutMs=10000)
        return True

    async def TestEnhancedCommissioningWindow(self, ip: str, nodeid: int):
        params = await self.devCtrl.OpenCommissioningWindow(nodeid=nodeid, timeout=600, iteration=10000, discriminator=3840, option=1)
        return await self.TestPaseOnly(ip=ip, nodeid=nodeid, setuppin=params.setupPinCode, devCtrl=self.devCtrl2)

    async def TestPaseOnly(self, ip: str, setuppin: int, nodeid: int, devCtrl=None):
        if devCtrl is None:
            devCtrl = self.devCtrl
        self.logger.info(
            "Attempting to establish PASE session with device id: {} addr: {}".format(str(nodeid), ip))
        try:
            await devCtrl.EstablishPASESessionIP(ip, setuppin, nodeid)
        except ChipStackException:
            self.logger.info(
                "Failed to establish PASE session with device id: {} addr: {}".format(str(nodeid), ip))
            return False
        self.logger.info(
            "Successfully established PASE session with device id: {} addr: {}".format(str(nodeid), ip))
        return True

    async def TestCommissionOnly(self, nodeid: int):
        self.logger.info(
            "Commissioning device with id {}".format(nodeid))
        try:
            await self.devCtrl.Commission(nodeid)
        except ChipStackException:
            self.logger.info(
                "Failed to commission device with id {}".format(str(nodeid)))
            return False
        self.logger.info(
            "Successfully commissioned device with id {}".format(str(nodeid)))
        return True

    async def TestKeyExchangeBLE(self, discriminator: int, setuppin: int, nodeid: int):
        self.logger.info(
            "Conducting key exchange with device {}".format(discriminator))
        if not await self.devCtrl.ConnectBLE(discriminator, setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(discriminator))
            return False
        self.logger.info("Device finished key exchange.")
        return True

    async def TestCommissionFailure(self, nodeid: int, failAfter: int):
        self.devCtrl.ResetTestCommissioner()
        a = self.devCtrl.SetTestCommissionerSimulateFailureOnStage(failAfter)
        if not a:
            # We're not going to hit this stage during commissioning so no sense trying, just say it was fine.
            return True

        self.logger.info(
            "Commissioning device, expecting failure after stage {}".format(failAfter))
        await self.devCtrl.Commission(nodeid)
        return self.devCtrl.CheckTestCommissionerCallbacks() and self.devCtrl.CheckTestCommissionerPaseConnection(nodeid)

    async def TestCommissionFailureOnReport(self, nodeid: int, failAfter: int):
        self.devCtrl.ResetTestCommissioner()
        a = self.devCtrl.SetTestCommissionerSimulateFailureOnReport(failAfter)
        if not a:
            # We're not going to hit this stage during commissioning so no sense trying, just say it was fine.
            return True
        self.logger.info(
            "Commissioning device, expecting failure on report for stage {}".format(failAfter))
        await self.devCtrl.Commission(nodeid)
        return self.devCtrl.CheckTestCommissionerCallbacks() and self.devCtrl.CheckTestCommissionerPaseConnection(nodeid)

    async def TestCommissioning(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Commissioning device {}".format(ip))
        try:
            await self.devCtrl.CommissionIP(ip, setuppin, nodeid)
        except ChipStackException:
            self.logger.exception(
                "Failed to finish commissioning device {}".format(ip))
            return False
        self.logger.info("Commissioning finished.")
        return True

    async def TestCommissioningWithSetupPayload(self, setupPayload: str, nodeid: int, discoveryType: int = 2):
        self.logger.info("Commissioning device with setup payload {}".format(setupPayload))
        try:
            await self.devCtrl.CommissionWithCode(setupPayload, nodeid, matter.discovery.DiscoveryType(discoveryType))
        except ChipStackException:
            self.logger.exception(
                "Failed to finish commissioning device {}".format(setupPayload))
            return False
        self.logger.info("Commissioning finished.")
        return True

    async def TestOnNetworkCommissioning(self, discriminator: int, setuppin: int, nodeid: int, ip_override: str = None):
        self.logger.info("Testing discovery")
        device = await self.TestDiscovery(discriminator=discriminator)
        if not device:
            self.logger.info("Failed to discover any devices.")
            return False
        address = device.addresses[0]
        if ip_override:
            address = ip_override
        self.logger.info("Testing commissioning")
        if not await self.TestCommissioning(address, setuppin, nodeid):
            self.logger.info("Failed to finish commissioning")
            return False
        return True

    def TestUsedTestCommissioner(self):
        return self.devCtrl.GetTestCommissionerUsed()

    async def TestFailsafe(self, nodeid: int):
        self.logger.info("Testing arm failsafe")

        self.logger.info("Setting failsafe on CASE connection")
        try:
            resp = await self.devCtrl.SendCommand(nodeid, 0,
                                                  Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=60, breadcrumb=1))
        except IM.InteractionModelError as ex:
            self.logger.error(
                "Failed to send arm failsafe command error is {}".format(ex.status))
            return False

        if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk:
            self.logger.error(
                "Incorrect response received from arm failsafe - wanted OK, received {}".format(resp))
            return False

        self.logger.info(
            "Attempting to open basic commissioning window - this should fail since the failsafe is armed")
        try:
            await self.devCtrl.SendCommand(
                nodeid,
                0,
                Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180),
                timedRequestTimeoutMs=10000
            )
            # we actually want the exception here because we want to see a failure, so return False here
            self.logger.error(
                'Incorrectly succeeded in opening basic commissioning window')
            return False
        except IM.InteractionModelError:
            pass

        # TODO:
        ''' Pipe through the commissioning window opener so we can test enhanced properly.
            The pake verifier is just garbage because none of of the functions to calculate
            it or serialize it are available right now. However, this command should fail BEFORE that becomes an issue.
        '''
        discriminator = 1111
        salt = secrets.token_bytes(16)
        iterations = 2000
        # not the right size or the right contents, but it won't matter
        verifier = secrets.token_bytes(32)
        self.logger.info(
            "Attempting to open enhanced commissioning window - this should fail since the failsafe is armed")
        try:
            await self.devCtrl.SendCommand(
                nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                    commissioningTimeout=180,
                    PAKEPasscodeVerifier=verifier,
                    discriminator=discriminator,
                    iterations=iterations,
                    salt=salt), timedRequestTimeoutMs=10000)

            # we actually want the exception here because we want to see a failure, so return False here
            self.logger.error(
                'Incorrectly succeeded in opening enhanced commissioning window')
            return False
        except IM.InteractionModelError:
            pass

        self.logger.info("Disarming failsafe on CASE connection")
        try:
            resp = await self.devCtrl.SendCommand(nodeid, 0,
                                                  Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))
        except IM.InteractionModelError as ex:
            self.logger.error(
                "Failed to send arm failsafe command error is {}".format(ex.status))
            return False

        self.logger.info(
            "Opening Commissioning Window - this should succeed since the failsafe was just disarmed")
        try:
            await self.devCtrl.SendCommand(
                nodeid,
                0,
                Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180),
                timedRequestTimeoutMs=10000
            )
        except Exception:
            self.logger.error(
                'Failed to open commissioning window after disarming failsafe')
            return False

        self.logger.info(
            "Attempting to arm failsafe over CASE - this should fail since the commissioning window is open")
        try:
            resp = await self.devCtrl.SendCommand(nodeid, 0,
                                                  Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=60, breadcrumb=1))
        except IM.InteractionModelError as ex:
            self.logger.error(
                "Failed to send arm failsafe command error is {}".format(ex.status))
            return False
        if resp.errorCode is Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kBusyWithOtherAdmin:
            return True
        return False

    async def TestControllerCATValues(self, nodeid: int):
        ''' This tests controllers using CAT Values
        '''
        # Allocate a new controller instance with a CAT tag.
        newControllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(
            fabricAdmin=self.fabricAdmin,
            adminDevCtrl=self.devCtrl,
            controllerNodeIds=[300],
            targetNodeId=nodeid,
            privilege=None, catTags=[0x0001_0001])

        # Read out an attribute using the new controller. It has no privileges, so this should fail with an UnsupportedAccess error.
        res = await newControllers[0].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (res[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl].Reason.status != IM.Status.UnsupportedAccess):
            self.logger.error(f"1: Received data instead of an error:{res}")
            return False

        # Grant the new controller privilege by adding the CAT tag to the subject.
        await CommissioningBuildingBlocks.GrantPrivilege(
            adminCtrl=self.devCtrl,
            grantedCtrl=newControllers[0],
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            targetNodeId=nodeid, targetCatTags=[0x0001_0001])

        # Read out the attribute again - this time, it should succeed.
        res = await newControllers[0].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (not isinstance(res[0][
                Clusters.AccessControl][
                Clusters.AccessControl.Attributes.Acl][0], Clusters.AccessControl.Structs.AccessControlEntryStruct)):
            self.logger.error(f"2: Received something other than data:{res}")
            return False

        # Reset the privilege back to pre-test.
        await CommissioningBuildingBlocks.GrantPrivilege(
            adminCtrl=self.devCtrl,
            grantedCtrl=newControllers[0],
            privilege=None,
            targetNodeId=nodeid
        )

        newControllers[0].Shutdown()

        return True

    async def TestMultiControllerFabric(self, nodeid: int):
        ''' This tests having multiple controller instances on the same fabric.
        '''

        # Create two new controllers on the same fabric with no privilege on the target node.
        newControllers = await CommissioningBuildingBlocks.CreateControllersOnFabric(
            fabricAdmin=self.fabricAdmin,
            adminDevCtrl=self.devCtrl,
            controllerNodeIds=[100, 200],
            targetNodeId=nodeid,
            privilege=None
        )

        #
        # Read out the ACL list from one of the newly minted controllers which has no access. This should return an IM error.
        #
        res = await newControllers[0].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (res[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl].Reason.status != IM.Status.UnsupportedAccess):
            self.logger.error(f"1: Received data instead of an error:{res}")
            return False

        #
        # Read out the ACL list from an existing controller with admin privileges. This should return back valid data.
        # Doing this ensures that we're not somehow aliasing the CASE sessions.
        #
        res = await self.devCtrl.ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (not isinstance(res[0][
                Clusters.AccessControl][
                Clusters.AccessControl.Attributes.Acl][0], Clusters.AccessControl.Structs.AccessControlEntryStruct)):
            self.logger.error(f"2: Received something other than data:{res}")
            return False

        #
        # Re-do the previous read from the unprivileged controller
        # just to do an ABA test to prove we haven't switched the CASE sessions
        # under-neath.
        #
        res = await newControllers[0].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (res[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl].Reason.status != IM.Status.UnsupportedAccess):
            self.logger.error(f"3: Received data instead of an error:{res}")
            return False

        #
        # Grant the new controller admin privileges. Reading out the ACL cluster should now yield data.
        #
        await CommissioningBuildingBlocks.GrantPrivilege(
            adminCtrl=self.devCtrl,
            grantedCtrl=newControllers[0],
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            targetNodeId=nodeid
        )
        res = await newControllers[0].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (not isinstance(res[0][
                Clusters.AccessControl][
                Clusters.AccessControl.Attributes.Acl][0], Clusters.AccessControl.Structs.AccessControlEntryStruct)):
            self.logger.error(f"4: Received something other than data:{res}")
            return False

        #
        # Grant the second new controller admin privileges as well. Reading out the ACL cluster should now yield data.
        #
        await CommissioningBuildingBlocks.GrantPrivilege(
            adminCtrl=self.devCtrl,
            grantedCtrl=newControllers[1],
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            targetNodeId=nodeid
        )
        res = await newControllers[1].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (not isinstance(res[0][
                Clusters.AccessControl][
                Clusters.AccessControl.Attributes.Acl][0], Clusters.AccessControl.Structs.AccessControlEntryStruct)):
            self.logger.error(f"5: Received something other than data:{res}")
            return False

        #
        # Grant the second new controller just view privilege. Reading out the ACL cluster should return no data.
        #
        await CommissioningBuildingBlocks.GrantPrivilege(
            adminCtrl=self.devCtrl,
            grantedCtrl=newControllers[1],
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            targetNodeId=nodeid)
        res = await newControllers[1].ReadAttribute(nodeid=nodeid, attributes=[(0, Clusters.AccessControl.Attributes.Acl)])
        if (res[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl].Reason.status != IM.Status.UnsupportedAccess):
            self.logger.error(f"6: Received data5 instead of an error:{res}")
            return False

        #
        # Read the Basic cluster from the 2nd controller. This is possible with just view privileges.
        #
        res = await newControllers[1].ReadAttribute(nodeid=nodeid,
                                                    attributes=[(0, Clusters.BasicInformation.Attributes.ClusterRevision)])
        if (not isinstance(res[0][
                Clusters.BasicInformation][
                Clusters.BasicInformation.Attributes.ClusterRevision], Clusters.BasicInformation.Attributes.ClusterRevision.attribute_type.Type)):
            self.logger.error(f"7: Received something other than data:{res}")
            return False

        newControllers[0].Shutdown()
        newControllers[1].Shutdown()

        return True

    async def TestAddUpdateRemoveFabric(self, nodeid: int):
        logger.info("Testing AddNOC, UpdateNOC and RemoveFabric")

        self.logger.info("Waiting for attribute read for CommissionedFabrics")
        startOfTestFabricCount = await self._GetCommissonedFabricCount(nodeid)

        tempCertificateAuthority = self.certificateAuthorityManager.NewCertificateAuthority()
        tempFabric = tempCertificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
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

        currentFabricIndexResponse = await tempDevCtrl.ReadAttribute(
            newNodeIdForUpdateNoc,
            [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)]
        )
        updatedNOCFabricIndex = currentFabricIndexResponse[0][Clusters.OperationalCredentials][
            Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        # Remove Fabric Response
        await tempDevCtrl.SendCommand(
            newNodeIdForUpdateNoc, 0,
            Clusters.OperationalCredentials.Commands.RemoveFabric(updatedNOCFabricIndex))

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
            self.devCtrl.MarkSessionDefunct(nodeid)
            await self.devCtrl.ReadAttribute(nodeid, [(Clusters.BasicInformation.Attributes.ClusterRevision)])

        self.logger.info("Testing CASE defunct logic")

        #
        # This tests establishes a subscription on a given CASE session, then marks it defunct (to simulate
        # encountering a transport timeout on the session).
        #
        # Then, we write to the attribute that was subscribed to from a *different* fabric and check to ensure we still get a report
        # on the sub we established previously. Since it was just marked defunct, it should return back to being
        # active and a report should get delivered.
        #
        sawValueChangeEvent = asyncio.Event()
        loop = asyncio.get_running_loop()

        def OnValueChange(path: Attribute.TypedAttributePath, transaction: Attribute.SubscriptionTransaction) -> None:
            self.logger.info("Saw value change!")
            if (path.AttributeType == Clusters.UnitTesting.Attributes.Int8u and path.Path.EndpointId == 1):
                loop.call_soon_threadsafe(sawValueChangeEvent.set)

        self.logger.info("Testing CASE defunct logic")

        sub = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.UnitTesting.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        #
        # This marks the session defunct.
        #
        self.devCtrl.MarkSessionDefunct(nodeid)

        #
        # Now write the attribute from fabric2, give it some time before checking if the report
        # was received.
        #
        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.Int8u(4))])

        try:
            await asyncio.wait_for(sawValueChangeEvent.wait(), 2)
        except TimeoutError:
            self.logger.error(
                "Didn't see value change in time, likely because sub got terminated due to unexpected session eviction!")
            return False
        finally:
            sub.Shutdown()

        #
        # In this test, we're going to setup a subscription on fabric1 through devCtl, then, constantly keep
        # evicting sessions on fabric2 (devCtl2) by cycling through closing sessions followed by issuing a Read. This
        # should result in evictions on the server on fabric2, but not affect any sessions on fabric1. To test this,
        # we're going to setup a subscription to an attribute prior to the cycling reads, and check at the end of the
        # test that it's still valid by writing to an attribute from a *different* fabric, and validating that we see
        # the change on the established subscription. That proves that the session from fabric1 is still valid and untouched.
        #
        self.logger.info("Testing fabric-isolated CASE eviction")

        sawValueChangeEvent.clear()
        sub = await self.devCtrl.ReadAttribute(nodeid, [(Clusters.UnitTesting.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        for x in range(minimumSupportedFabrics * minimumCASESessionsPerFabric * 2):
            self.devCtrl2.MarkSessionDefunct(nodeid)
            await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.BasicInformation.Attributes.ClusterRevision)])

        #
        # Now write the attribute from fabric2, give it some time before checking if the report
        # was received.  Use a different value from before, so there is an actual change.
        #
        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.Int8u(5))])

        try:
            await asyncio.wait_for(sawValueChangeEvent.wait(), 2)
        except TimeoutError:
            self.logger.error("Didn't see value change in time, likely because sub got terminated due to other fabric (fabric1)")
            return False
        finally:
            sub.Shutdown()

        #
        # Do the same test again, but reversing the roles of fabric1 and fabric2.  And again
        # writing a different value, so there is an actual value change.
        #
        self.logger.info("Testing fabric-isolated CASE eviction (reverse)")

        sawValueChangeEvent.clear()
        sub = await self.devCtrl2.ReadAttribute(nodeid, [(Clusters.UnitTesting.Attributes.Int8u)], reportInterval=(0, 1))
        sub.SetAttributeUpdateCallback(OnValueChange)

        for x in range(minimumSupportedFabrics * minimumCASESessionsPerFabric * 2):
            self.devCtrl.MarkSessionDefunct(nodeid)
            await self.devCtrl.ReadAttribute(nodeid, [(Clusters.BasicInformation.Attributes.ClusterRevision)])

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.Int8u(6))])
        try:
            await asyncio.wait_for(sawValueChangeEvent.wait(), 2)
        except TimeoutError:
            self.logger.error("Didn't see value change in time, likely because sub got terminated due to other fabric (fabric2)")
            return False
        finally:
            sub.Shutdown()

        return True

    async def TestMultiFabric(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Opening Commissioning Window")

        await self.devCtrl.SendCommand(
            nodeid,
            0,
            Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180),
            timedRequestTimeoutMs=10000
        )

        self.logger.info("Creating 2nd Fabric Admin")
        self.fabricAdmin2 = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)

        self.logger.info("Creating Device Controller on 2nd Fabric")
        self.devCtrl2 = self.fabricAdmin2.NewController(
            self.controllerNodeId, self.paaTrustStorePath)

        try:
            await self.devCtrl2.CommissionIP(ip, setuppin, nodeid)
        except ChipStackException:
            self.logger.exception(
                "Failed to finish key exchange with device {}".format(ip))
            return False

        #
        # Shut-down all the controllers (which will free them up)
        #
        self.logger.info(
            "Shutting down controllers & fabrics and re-initing stack...")

        self.certificateAuthorityManager.Shutdown()

        self.logger.info("Shutdown completed, starting new controllers...")

        self.certificateAuthorityManager = matter.CertificateAuthority.CertificateAuthorityManager(chipStack=self.chipStack)
        self.certificateAuthority = self.certificateAuthorityManager.NewCertificateAuthority()
        self.fabricAdmin = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)

        fabricAdmin2 = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)

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

        data1 = await self.devCtrl.ReadAttribute(
            nodeid,
            [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)],
            fabricFiltered=False
        )
        data2 = await self.devCtrl2.ReadAttribute(
            nodeid,
            [(Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)],
            fabricFiltered=False
        )

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
            Clusters.UnitTesting.Structs.TestFabricScoped(),
            Clusters.UnitTesting.Structs.TestFabricScoped()
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

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped(expectedDataFabric1))])

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

        await self.devCtrl2.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped(expectedDataFabric2))])

        #
        # Now read the data back filtered from fabric1 and ensure it matches.
        #
        self.logger.info("Reading back data from fabric1...")

        data = await self.devCtrl.ReadAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)])
        readListDataFabric1 = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]

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

        data = await self.devCtrl2.ReadAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)])
        readListDataFabric2 = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]

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
                    expectedDefaultData = Clusters.UnitTesting.Structs.TestFabricScoped()
                    expectedDefaultData.fabricIndex = otherFabric

                    if (item != expectedDefaultData):
                        raise AssertionError("Got back mismatched data")

        data = await self.devCtrl.ReadAttribute(nodeid,
                                                [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)], fabricFiltered=False)
        readListDataFabric = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]
        CompareUnfilteredData(self.currentFabric1,
                              self.currentFabric2, expectedDataFabric1)

        data = await self.devCtrl2.ReadAttribute(nodeid,
                                                 [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)], fabricFiltered=False)
        readListDataFabric = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]
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

        await self.devCtrl.WriteAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped(expectedDataFabric1))])

        self.logger.info(
            "Reading back data (again) from fabric2 to ensure it hasn't changed")

        data = await self.devCtrl2.ReadAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)])
        readListDataFabric2 = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]
        if (expectedDataFabric2 != readListDataFabric2):
            raise AssertionError("Got back mismatched data")

        self.logger.info(
            "Reading back data (again) from fabric1 to ensure it hasn't changed")

        data = await self.devCtrl.ReadAttribute(nodeid, [(1, Clusters.UnitTesting.Attributes.ListFabricScoped)])
        readListDataFabric1 = data[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]

        self.logger.info("Comparing data on fabric1...")
        expectedDataFabric1[0].fabricIndex = self.currentFabric1
        if (expectedDataFabric1 != readListDataFabric1):
            raise AssertionError("Got back mismatched data")

    async def TestResubscription(self, nodeid: int):
        ''' This validates the re-subscription logic by triggering a liveness failure caused by the expiration
            of the underlying CASE session and the resultant failure to receive reports from the server. This should
            trigger CASE session establishment and subscription reestablishment. Both the attempt and successful
            reestablishment of the subscription are validated.
        '''
        cv = asyncio.Condition()
        resubAttempted = False
        resubSucceeded = True

        async def OnResubscriptionAttempted(transaction, errorEncountered: int, nextResubscribeIntervalMsec: int):
            self.logger.info("Re-subscription Attempted")
            nonlocal resubAttempted
            resubAttempted = True

        async def OnResubscriptionSucceeded(transaction):
            self.logger.info("Re-subscription Succeeded")
            nonlocal cv
            async with cv:
                cv.notify()

        subscription = await self.devCtrl.ReadAttribute(
            nodeid,
            [(Clusters.BasicInformation.Attributes.ClusterRevision)],
            reportInterval=(0, 5)
        )

        #
        # Register async callbacks that will fire when a re-sub is attempted or succeeds.
        #
        subscription.SetResubscriptionAttemptedCallback(OnResubscriptionAttempted, True)
        subscription.SetResubscriptionSucceededCallback(OnResubscriptionSucceeded, True)

        #
        # Over-ride the default liveness timeout (which is set quite high to accommodate for
        # transport delays) to something very small. This ensures that our liveness timer will
        # fire quickly and cause a re-subscription to occur naturally.
        #
        subscription.OverrideLivenessTimeoutMs(100)

        async with cv:
            if (not (resubAttempted) or not (resubSucceeded)):
                res = await asyncio.wait_for(cv.wait(), 3)
                if not res:
                    self.logger.error("Timed out waiting for resubscription to succeed")
                    return False

        subscription.Shutdown()
        return True

    def TestCloseSession(self, nodeid: int):
        self.logger.info(f"Closing sessions with device {nodeid}")
        try:
            self.devCtrl.MarkSessionDefunct(nodeid)
            return True
        except Exception as ex:
            self.logger.exception(
                f"Failed to close sessions with device {nodeid}: {ex}")
            return False

    def SetNetworkCommissioningParameters(self, dataset: str):
        self.logger.info("Setting network commissioning parameters")
        self.devCtrl.SetThreadOperationalDataset(bytes.fromhex(dataset))
        return True

    async def TestOnOffCluster(self, nodeid: int, endpoint: int):
        self.logger.info(
            "Sending On/Off commands to device {} endpoint {}".format(nodeid, endpoint))

        try:
            await self.devCtrl.SendCommand(nodeid, endpoint,
                                           Clusters.OnOff.Commands.On())
        except IM.InteractionModelError as ex:
            self.logger.error(
                "failed to send OnOff.On: error is {}".format(ex.status))
            return False

        try:
            await self.devCtrl.SendCommand(nodeid, endpoint,
                                           Clusters.OnOff.Commands.Off())
        except IM.InteractionModelError as ex:
            self.logger.error(
                "failed to send OnOff.Off: error is {}".format(ex.status))
            return False
        return True

    async def TestLevelControlCluster(self, nodeid: int, endpoint: int):
        self.logger.info(
            f"Sending MoveToLevel command to device {nodeid} endpoint {endpoint}")

        commonArgs = dict(transitionTime=0, optionsMask=1, optionsOverride=1)

        async def _moveClusterLevel(setLevel):
            await self.devCtrl.SendCommand(nodeid,
                                           endpoint,
                                           Clusters.LevelControl.Commands.MoveToLevel(**commonArgs, level=setLevel))
            res = await self.devCtrl.ReadAttribute(nodeid, [(endpoint, Clusters.LevelControl.Attributes.CurrentLevel)])
            readVal = res[endpoint][Clusters.LevelControl][Clusters.LevelControl.Attributes.CurrentLevel]
            if readVal != setLevel:
                raise Exception(f"Read attribute LevelControl.CurrentLevel: expected value {setLevel}, got {readVal}")

        try:
            # Move to 1
            await _moveClusterLevel(1)

            # Move to 254
            await _moveClusterLevel(254)

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
                    self.logger.exception("Timeout waiting for address...")
                    break

                if not addr:
                    time.sleep(0.2)

            if not addr:
                self.logger.exception("Addr is missing...")
                return False
            self.logger.info(f"Resolved address: {addr[0]}:{addr[1]}")
            return True
        except Exception as ex:
            self.logger.exception("Failed to resolve. {}".format(ex))
            return False

    async def TestTriggerTestEventHandler(self, nodeid, enable_key, event_trigger):
        self.logger.info("Test trigger test event handler for device = %08x trigger = %016x", nodeid, event_trigger)
        try:
            await self.devCtrl.SendCommand(nodeid, 0, Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enable_key, eventTrigger=event_trigger))
            return True
        except Exception as ex:
            self.logger.exception("Failed to trigger test event handler {}".format(ex))
            return False

    async def TestWaitForActive(self, nodeid, stayActiveDurationMs=30000):
        self.logger.info("Test wait for device = %08x", nodeid)
        try:
            await self.devCtrl.WaitForActive(nodeid, stayActiveDurationMs=stayActiveDurationMs)
            return True
        except Exception as ex:
            self.logger.exception("Failed to wait for active. {}".format(ex))
            return False

    async def TestReadBasicAttributes(self, nodeid: int, endpoint: int):
        attrs = Clusters.BasicInformation.Attributes
        basic_cluster_attrs = {
            attrs.VendorName: "TEST_VENDOR",
            attrs.VendorID: 0xFFF1,
            attrs.ProductName: "TEST_PRODUCT",
            attrs.ProductID: 0x8001,
            attrs.NodeLabel: "Test",
            attrs.Location: "XX",
            attrs.HardwareVersion: 0,
            attrs.HardwareVersionString: "TEST_VERSION",
            attrs.SoftwareVersion: 1,
            attrs.SoftwareVersionString: "1.0",
        }
        failed_zcl = {}
        for basic_attr, expected_value in basic_cluster_attrs.items():
            try:
                res = await self.devCtrl.ReadAttribute(nodeid, [(endpoint, basic_attr)])
                readVal = res[endpoint][Clusters.BasicInformation][basic_attr]
                if readVal != expected_value:
                    raise Exception(f"Read attribute: expected value {expected_value}, got {readVal}")
            except Exception as ex:
                failed_zcl[basic_attr] = str(ex)
        if failed_zcl:
            self.logger.exception(f"Following attributes failed: {failed_zcl}")
            return False
        return True

    async def TestWriteBasicAttributes(self, nodeid: int, endpoint: int):
        @dataclass
        class AttributeWriteRequest:
            cluster: Clusters.ClusterObjects.Cluster
            attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor
            value: Any
            expected_status: IM.Status = IM.Status.Success

        requests = [
            AttributeWriteRequest(Clusters.BasicInformation, Clusters.BasicInformation.Attributes.NodeLabel, "Test"),
            AttributeWriteRequest(Clusters.BasicInformation, Clusters.BasicInformation.Attributes.Location,
                                  "a pretty loooooooooooooog string", IM.Status.ConstraintError),
        ]
        failed_attribute_write = []
        for req in requests:
            try:
                # Errors tested here is in the per-attribute result list (type AttributeStatus)
                write_res = await self.devCtrl.WriteAttribute(nodeid, [(endpoint, req.attribute(req.value))])
                status = write_res[0].Status
                if req.expected_status != status:
                    raise AssertionError(
                        f"Write attribute {req.attribute.__qualname__} expects {req.expected_status} but got {status}")

                # Only execute read tests where write is successful.
                if req.expected_status != IM.Status.Success:
                    continue

                res = await self.devCtrl.ReadAttribute(nodeid, [(endpoint, req.attribute)])
                val = res[endpoint][req.cluster][req.attribute]
                if val != req.value:
                    raise Exception(
                        f"Read attribute {req.attribute.__qualname__}: expected value {req.value}, got {val}")
            except Exception as ex:
                failed_attribute_write.append(str(ex))
        if failed_attribute_write:
            self.logger.exception(f"Following attributes failed: {failed_attribute_write}")
            return False
        return True

    async def TestSubscription(self, nodeid: int, endpoint: int):
        desiredPath = None
        receivedUpdate = 0
        updateEvent = asyncio.Event()
        loop = asyncio.get_running_loop()

        def OnValueChange(path: Attribute.TypedAttributePath, transaction: Attribute.SubscriptionTransaction) -> None:
            nonlocal desiredPath, updateEvent, receivedUpdate
            if path.Path != desiredPath:
                return

            data = transaction.GetAttribute(path)
            logger.info(
                f"Received report from server: path: {path.Path}, value: {data}")
            receivedUpdate += 1
            loop.call_soon_threadsafe(updateEvent.set)

        async def _conductAttributeChange(devCtrl: ChipDeviceCtrl.ChipDeviceController, nodeid: int, endpoint: int):
            for i in range(5):
                await asyncio.sleep(3)
                await self.devCtrl.SendCommand(nodeid, endpoint, Clusters.OnOff.Commands.Toggle())

        try:
            desiredPath = Clusters.Attribute.AttributePath(
                EndpointId=1, ClusterId=6, AttributeId=0)
            # OnOff Cluster, OnOff Attribute
            subscription = await self.devCtrl.ReadAttribute(nodeid, [(endpoint, Clusters.OnOff.Attributes.OnOff)], None, False, reportInterval=(1, 10),
                                                            keepSubscriptions=False, autoResubscribe=True)
            subscription.SetAttributeUpdateCallback(OnValueChange)
            # Reset the number of subscriptions received as subscribing causes a callback.
            taskAttributeChange = loop.create_task(_conductAttributeChange(self.devCtrl, nodeid, endpoint))

            while receivedUpdate < 5:
                # We should observe 5 attribute changes
                # The changing thread will change the value after 3 seconds. If we're waiting more than 10, assume something
                # is really wrong and bail out here with some information.
                try:
                    await asyncio.wait_for(updateEvent.wait(), 10)
                    updateEvent.clear()
                except TimeoutError:
                    self.logger.error(
                        "Failed to receive subscription update")
                    break

            # At this point the task should really have done the three attribute,
            # otherwise something is wrong. Wait for just 1s in case of a race
            # condition between the last attribute update and the callback.
            try:
                await asyncio.wait_for(taskAttributeChange, 1)
            except asyncio.TimeoutError:
                # If attribute change task did not finish something is wrong. Cancel
                # the task.
                taskAttributeChange.cancel()
                # This will throw a asyncio.CancelledError and makes sure the test
                # is declared failed.
                await taskAttributeChange

            return True if receivedUpdate == 5 else False

        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False
        finally:
            #
            # Clean-up by shutting down the sub. Otherwise, we're going to get callbacks through
            # OnValueChange on what will soon become an invalid
            # execution context above.
            #
            subscription.Shutdown()

        return True

    def TestNonControllerAPIs(self):
        '''
        This function validates various APIs provided by chip package which is not related to controller.
        TODO: Add more tests for APIs
        '''
        try:
            cluster = self.devCtrl.GetClusterHandler()
            clusterInfo = cluster.GetClusterInfoById(0xFFF1FC05)  # TestCluster
            if clusterInfo["clusterName"] != "UnitTesting":
                raise Exception(
                    f"Wrong cluster info clusterName: {clusterInfo['clusterName']} expected 'UnitTesting'")
        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False
        return True

    async def TestFabricScopedCommandDuringPase(self, nodeid: int):
        '''Validates that fabric-scoped commands fail during PASE with UNSUPPORTED_ACCESS

        The nodeid is the PASE pseudo-node-ID used during PASE establishment
        '''
        status = None
        try:
            await self.devCtrl.SendCommand(
                nodeid, 0, Clusters.OperationalCredentials.Commands.UpdateFabricLabel("roboto"))
        except IM.InteractionModelError as ex:
            status = ex.status

        return status == IM.Status.UnsupportedAccess

    async def TestSubscriptionResumption(self, nodeid: int, endpoint: int, remote_ip: str, ssh_port: int, remote_server_app: str):
        '''
        This test validates that the device can resume the subscriptions after restarting.
        It is executed in Linux Cirque tests and the steps of this test are:
        1. Subscription the NodeLable attribute on BasicInformation cluster with the controller
        2. Restart the remote server app
        3. Validate that the controller can receive a report from the remote server app
        '''
        desiredPath = None
        updateEvent = asyncio.Event()
        loop = asyncio.get_running_loop()

        def OnValueReport(path: Attribute.TypedAttributePath, transaction: Attribute.SubscriptionTransaction) -> None:
            nonlocal desiredPath, updateEvent, receivedUpdate
            if path.Path != desiredPath:
                return

            data = transaction.GetAttribute(path)
            logger.info(
                f"Received report from server: path: {path.Path}, value: {data}")
            loop.call_soon_threadsafe(updateEvent.set)

        try:
            desiredPath = Clusters.Attribute.AttributePath(
                EndpointId=0, ClusterId=0x28, AttributeId=5)
            # BasicInformation Cluster, NodeLabel Attribute
            subscription = await self.devCtrl.ReadAttribute(nodeid, [(endpoint, Clusters.BasicInformation.Attributes.NodeLabel)], None, False, reportInterval=(1, 50),
                                                            keepSubscriptions=True, autoResubscribe=False)
            subscription.SetAttributeUpdateCallback(OnValueReport)

            self.logger.info("Restart remote device")
            restartRemoteThread = restartRemoteDevice(
                remote_ip, ssh_port, "root", "admin", remote_server_app, "--thread --discriminator 3840")
            restartRemoteThread.start()
            # After device restarts, the attribute will be set dirty so the subscription can receive
            # the update
            receivedUpdate = False
            try:
                await asyncio.wait_for(updateEvent.wait(), 10)
                receivedUpdate = True
            except TimeoutError:
                self.logger.error(
                    "Failed to receive subscription resumption report")

            restartRemoteThread.join(10.0)

            #
            # Clean-up by shutting down the sub. Otherwise, we're going to get callbacks through
            # OnValueChange on what will soon become an invalid execution context above.
            #
            subscription.Shutdown()

            if restartRemoteThread.is_alive():
                # Thread join timed out
                self.logger.error("Failed to join change thread")
                return False

            return receivedUpdate

        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False

        return True

    '''
    The SubscriptionResumptionCapacity Cirque Test is to verify that the device can still handle new subscription
    requests when resuming the maximum subscriptions. The steps for this test are:
    1. Commission the server app to the first fabric and send maximum subscription requests from the controller in
    the first fabric to establish maximum subscriptions.
    2. Open the commissioning window to make the server app can be commissioned to the second fabric.
    3. Shutdown the controller in the first fabric to extend the time of resuming subscriptions. The server app will
    keep resolving the address of the first controller for a while after rebooting.
    4. Commission the server app to the second fabric.
    5. Restart the server app and the server app will start resuming subscriptions. Since the first controller is
    shutdown, the server app will keep resolving the address of the first controller for a while and the subscription
    resumption will not fail so quickly.
    6. When the server app is resuming subscriptions, send a new subscription request from the second controller.
    Verify that the device can still handle this subscription.

    BaseTestHelper provides two controllers. However, if using the two controller (devCtrl and devCtrl2) in one
    MobileDevice to execute this Cirque test, the CHIPEndDevice can still resolve the address for first controller
    even if the first controller is shutdown by 'self.devCtrl.Shutdown()'. And the server will fail to establish the
    subscriptions immediately, which makes it hard to send the new subscription request from the second controller
    at the time of server app resuming maximum subscriptions.
    So we will use two controller containers for this test and divide the test to two steps. The Step1 is executed in
    controller 1 in container 1 while the Step2 is executed in controller 2 in container 2
    '''

    async def TestSubscriptionResumptionCapacityStep1(self, nodeid: int, endpoint: int, passcode: int, subscription_capacity: int):
        try:
            # BasicInformation Cluster, NodeLabel Attribute
            for i in range(subscription_capacity):
                await self.devCtrl.ReadAttribute(nodeid, [(endpoint, Clusters.BasicInformation.Attributes.NodeLabel)], None,
                                                 False, reportInterval=(1, 50),
                                                 keepSubscriptions=True, autoResubscribe=False)

            logger.info("Send OpenCommissioningWindow command on fist controller")
            discriminator = 3840
            salt = secrets.token_bytes(16)
            iterations = 2000
            verifier = GenerateVerifier(passcode, salt, iterations)
            await self.devCtrl.SendCommand(
                nodeid, 0, Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                    commissioningTimeout=180,
                    PAKEPasscodeVerifier=verifier,
                    discriminator=discriminator,
                    iterations=iterations,
                    salt=salt), timedRequestTimeoutMs=10000)
            return True

        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False

        return True

    async def TestSubscriptionResumptionCapacityStep2(self, nodeid: int, endpoint: int, remote_ip: str, ssh_port: int,
                                                      remote_server_app: str, subscription_capacity: int):
        try:
            self.logger.info("Restart remote deivce")
            extra_agrs = f"--thread --discriminator 3840 --subscription-capacity {subscription_capacity}"
            restartRemoteThread = restartRemoteDevice(remote_ip, ssh_port, "root", "admin", remote_server_app, extra_agrs)
            restartRemoteThread.start()

            # Wait for some time so that the device will be resolving the address of the first controller after restarting
            time.sleep(8)
            restartRemoteThread.join(10.0)

            self.logger.info("Send a new subscription request from the second controller")
            # Close previous session so that the second controller will res-establish the session with the remote device
            self.devCtrl.MarkSessionDefunct(nodeid)
            await self.devCtrl.ReadAttribute(nodeid, [(endpoint, Clusters.BasicInformation.Attributes.NodeLabel)], None,
                                             False, reportInterval=(1, 50),
                                             keepSubscriptions=True, autoResubscribe=False)

            if restartRemoteThread.is_alive():
                # Thread join timed out
                self.logger.error("Failed to join change thread")
                return False

            return True

        except Exception as ex:
            self.logger.exception(f"Failed to finish API test: {ex}")
            return False

        return True
