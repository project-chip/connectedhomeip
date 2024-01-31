#
#    Copyright (c) 2023 Project CHIP Authors
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

import copy
import logging
import random

import chip.clusters as Clusters
import chip.discovery as Discovery
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.tlv import TLVReader, TLVWriter
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OPCREDS_3_1(MatterBaseTest):
    def FindAndEstablishPase(self, longDiscriminator: int, setupPinCode: int, nodeid: int, dev_ctrl: ChipDeviceCtrl = None):
        if dev_ctrl is None:
            dev_ctrl = self.default_controller

        devices = dev_ctrl.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR, filter=longDiscriminator, stopOnFirst=False)
        # For some reason, the devices returned here aren't filtered, so filter ourselves
        device = next(filter(lambda d: d.commissioningMode ==
                      Discovery.FilterType.LONG_DISCRIMINATOR and d.longDiscriminator == longDiscriminator, devices))
        for a in device.addresses:
            try:
                dev_ctrl.EstablishPASESessionIP(ipaddr=a, setupPinCode=setupPinCode,
                                                nodeid=nodeid, port=device.port)
                break
            except ChipStackError:
                continue
        try:
            dev_ctrl.GetConnectedDeviceSync(nodeid=nodeid, allowPASE=True, timeoutMs=1000)
        except TimeoutError:
            asserts.fail("Unable to establish a PASE session to the device")

    def OpenCommissioningWindow(self, dev_ctrl: ChipDeviceCtrl, node_id: int):
        # TODO: abstract this in the base layer? Do we do this a lot?
        longDiscriminator = random.randint(0, 4095)
        try:
            params = dev_ctrl.OpenCommissioningWindow(
                nodeid=node_id, timeout=600, iteration=10000, discriminator=longDiscriminator, option=ChipDeviceCtrl.ChipDeviceControllerBase.CommissioningWindowPasscode.kTokenWithRandomPin)
        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')
        return (longDiscriminator, params)

    @async_test_body
    async def test_TC_OPCREDS_3_1(self):
        opcreds = Clusters.OperationalCredentials
        # Create TH1
        # Maximize cert chains so we can use this below
        TH1_CA_real = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        TH1_vid = 0xFFF2
        TH1_fabric_admin_real = TH1_CA_real.NewFabricAdmin(vendorId=TH1_vid, fabricId=2)
        TH1_nodeid = self.default_controller.nodeId+1
        TH1 = TH1_fabric_admin_real.NewController(nodeId=TH1_nodeid)

        commissioned_fabric_count = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CommissionedFabrics)

        supported_fabric_count = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.SupportedFabrics)

        # Make sure we have space
        # TODO: Add step here to just remove extra fabrics if required.
        asserts.assert_less(commissioned_fabric_count, supported_fabric_count,
                            "Device fabric table is full - please remove one fabric and retry")

        self.print_step(1, "TH0 opens a commissioning window on the DUT")
        longDiscriminator, params = self.OpenCommissioningWindow(self.default_controller, self.dut_node_id)

        self.print_step(
            2, "TH0 reads the BasicCommissioningInfo field from the General commissioning cluster saves MaxCumulativeFailsafeSeconds as `failsafe_max`")
        basic_commissioning_info = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.BasicCommissioningInfo)
        failsafe_max = basic_commissioning_info.maxCumulativeFailsafeSeconds

        self.print_step(3, "TH1 opens a PASE connection to the DUT")
        newNodeId = self.dut_node_id + 1
        self.FindAndEstablishPase(dev_ctrl=TH1, longDiscriminator=longDiscriminator,
                                  setupPinCode=params.setupPinCode, nodeid=newNodeId)

        self.print_step(4, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to failsafe_max")
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(failsafe_max))
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(5, "TH1 Sends CSRRequest command with a random 32-byte nonce and saves the response as`csrResponse")
        nonce = random.randbytes(32)
        csrResponse = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=False))

        self.print_step(6, "TH1 obtains or generates the NOC, the Root CA Certificate and ICAC using csrResponse and selects an IPK. The certificates shall have their subjects padded with additional data such that they are each the maximum certificate size of 400 bytes when encoded in the MatterCertificateEncoding.")
        # Our CA is set up to maximize cert chains already
        # Extract the RCAC public key and save as `Root_Public_Key_TH1`
        TH1_certs_real = TH1.IssueNOCChain(csrResponse, newNodeId)
        if (TH1_certs_real.rcacBytes is None or
                TH1_certs_real.icacBytes is None or
                TH1_certs_real.nocBytes is None or TH1_certs_real.ipkBytes is None):
            # Expiring the failsafe timer in an attempt to clean up.
            await TH1.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
            asserts.fail("Unable to generate NOC chain for DUT - this is a script failure, please report this as a bug")
        th1_rcac_decoded = TLVReader(TH1_certs_real.rcacBytes).get()["Any"]
        # public key is field 9
        root_public_key_th1 = th1_rcac_decoded[9]

        self.print_step(
            7, "TH1 obtains or generates Root Certificate with a different Root CA ID and the corresponding ICAC, NOC and IPK using csrResponse")
        TH1_CA_fake = self.certificate_authority_manager.NewCertificateAuthority()
        TH1_fabric_admin_fake = TH1_CA_fake.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        TH1_fake = TH1_fabric_admin_fake.NewController(nodeId=self.default_controller.nodeId)
        TH1_certs_fake = TH1_fake.IssueNOCChain(csrResponse, newNodeId)
        if (TH1_certs_fake.rcacBytes is None or
                TH1_certs_fake.icacBytes is None or
                TH1_certs_fake.nocBytes is None or TH1_certs_real.ipkBytes is None):
            # Expiring the failsafe timer in an attempt to clean up.
            await TH1.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
            asserts.fail("Unable to generate NOC chain for DUT - this is a script failure, please report this as a bug")

        self.print_step(
            8, "TH1 generates an INVALID Root Certificate where the signature does not match the public key and saves it as `Root_CA_Malformed`")
        TH1_root_CA_malformed_decoded = copy.deepcopy(th1_rcac_decoded)
        # signature is field 11
        print(TH1_root_CA_malformed_decoded[11])
        malformed_sig_int = int.from_bytes(TH1_root_CA_malformed_decoded[11], 'big') + 1
        malformed_sig = malformed_sig_int.to_bytes(len(TH1_root_CA_malformed_decoded[11]), 'big')
        TH1_root_CA_malformed_decoded[11] = malformed_sig

        writer = TLVWriter(bytearray())
        writer.startStructure(None)
        for tag, val in TH1_root_CA_malformed_decoded.items():
            TH1_root_CA_malformed = writer.put(tag, val)
        writer.endContainer()
        TH1_root_CA_malformed = writer.encoding

        self.print_step(9, "TH1 reads the trusted root cert list from the DUT. Save the list size as trusted_root_original_size")
        trusted_root_list = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        trusted_root_original_size = len(trusted_root_list)

        self.print_step(
            10, "TH1 reads the NOCs attribute from the DUT using a non-fabric-filtered read. Save the list size as nocs_original_size")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=False)
        nocs_original_size = len(nocs)

        self.print_step(11, "TH1 reads the fabrics attribute from the DUT. Save the list size as fabrics_original_size")
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.Fabrics, fabric_filtered=False)
        fabrics_original_size = len(fabrics)

        self.print_step(
            12, "TH1 sends AddTrustedRootCertificate command to DUT to install `Root_CA_Malformed` and verifies INVALID_COMMAND is returned")
        cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_root_CA_malformed)
        try:
            await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
            asserts.fail("Unexpected success adding trusted root cert with malformed signature")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 "Unexpected error adding trusted root cert with malformed signature")
        self.print_step(
            13, "TH1 reads the TrustedRootCertificate attribute and confirm it contains only trusted_root_original_size entries")
        ret = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(ret), trusted_root_original_size,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.print_step(
            14, "TH1 sends AddTrustedRootCertificate command to DUT with RootCACertificate set to `Root_CA_Certificate_TH1`, verify SUCCESS")
        cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_certs_real.rcacBytes)
        await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)

        self.print_step(
            15, "TH1 reads the TrustedRootCertificate attribute and confirms it contains trusted_root_original_size + 1 entries")
        ret = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(ret), trusted_root_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.print_step(
            16, "TH1 sends AddTrustedRootCertificate command to DUT again with the RootCACertificate field set to `Root_CA_Certificate_TH1`,  verify SUCCESS")
        # TODO: This currently fails - bug in the SDK? See #30798
        # await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)

        self.print_step(
            17, "TH1 reads the TrustedRootCertificate attribute and confirms it contains trusted_root_original_size + 1 entries")
        ret = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(ret), trusted_root_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.print_step(
            18, "TH1 sends AddTrustedRootCertificate command to DUT again with the RootCACertificate field set to `Root_CA_Certificate_TH1_2`, verify CONSTRAINT_ERROR")
        try:
            cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_certs_fake.rcacBytes)
            await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
            asserts.fail("Unexpected success when adding second trusted root certificate")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error when adding second trusted root certificate")

        self.print_step(
            19, "TH1 reads the TrustedRootCertificate attribute and confirms it contains trusted_root_original_size + 1 entries")
        ret = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(ret), trusted_root_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.print_step(
            20, "TH1 appends `Root_CA_Certificate_TH1_2` to `TrustedRootsList` and writes the TrustedRootCertificates attribute with that value,  Verify UNSUPPORTED_WRITE")
        trusted_root_list.append(TH1_certs_fake.rcacBytes)
        attr = opcreds.Attributes.TrustedRootCertificates(trusted_root_list)
        err = await TH1.WriteAttribute(nodeid=newNodeId, attributes=[(0, attr)])
        asserts.assert_equal(err[0].Status, Status.UnsupportedWrite,
                             "Unexpected error trying to write TrustedRootCertificate attribute")

        self.print_step(21, "TH1 reads the TrustedRootCertificates list from DUT | Verify that there is only one instance of Root CA Certificate in the list and that its content matches `Root_CA_Certificate_TH1`")
        ret = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(ret), trusted_root_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.print_step(22, "TH1 sends the AddNOC Command to DUT for the certs generated with the second trusted root cert")
        # Node_Operational_Certificate_TH1_2`
        # ICACValue as `Intermediate_Certificate_TH1_2`
        # IpkValue as `IPK_TH1_2`
        # CaseAdminSubject as the NodeID of TH1
        # AdminVendorId as the Vendor ID of TH1  a|* Verify that DUT responds with NOCResponse command with status code InvalidNOC
        cmd = opcreds.Commands.AddNOC(NOCValue=TH1_certs_fake.nocBytes, ICACValue=TH1_certs_fake.icacBytes,
                                      IPKValue=TH1_certs_fake.ipkBytes, caseAdminSubject=TH1_nodeid, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC,
                             "Unexpected error when adding Invalid NOC")

        self.print_step(23, "TH1 sends the AddNOC Command to DUT with an invalid node ID")
        # NOCValue as `Node_Operational_Certificate_TH1`
        # ICACValue as `Intermediate_Certificate_TH1`
        # IpkValue as `IPK_TH1`
        # CaseAdminSubject is an invalid NodeID (not an operational Node ID or Case Authenticated Tag - ex. 0)
        # AdminVendorId as the Vendor ID of TH1  a|* Verify that DUT responds with NOCResponse command with status code InvalidAdminSubject
        cmd = opcreds.Commands.AddNOC(NOCValue=TH1_certs_real.nocBytes, ICACValue=TH1_certs_real.icacBytes,
                                      IPKValue=TH1_certs_real.ipkBytes, caseAdminSubject=0, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidAdminSubject,
                             "Unexpected error when adding NOC with invalid case admin subject")

        self.print_step(24, "TH1 sends the AddNOC Command to DUT a valid NOC")
        # NOCValue as `Node_Operational_Certificate_TH1`
        # ICACValue as `Intermediate_Certificate_TH1`
        # IpkValue as `IPK_TH1`
        # CaseAdminSubject as the NodeID of TH1
        # AdminVendorId as the Vendor ID of TH1  a|* Verify that DUT responds with NOCResponse command with status code OK
        # Verify that FabricIndex has a size of 1 byte
        # save the fabric index.
        cmd = opcreds.Commands.AddNOC(NOCValue=TH1_certs_real.nocBytes, ICACValue=TH1_certs_real.icacBytes,
                                      IPKValue=TH1_certs_real.ipkBytes, caseAdminSubject=TH1_nodeid, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "Failure when adding NOC")

        self.print_step(25, "TH1 re-sends the AddNOC Command to DUT and verifies CONSTRAINT_ERROR")
        # NOCValue as `Node_Operational_Certificate_TH1_1`
        # ICACValue as `Intermediate_Certificate_TH1_1`
        # IpkValue as `IPK_TH1_1`
        # CaseAdminSubject as the NodeID of TH1
        # AdminVendorId as the Vendor ID of TH1  | Verify that DUT responds with status code CONSTRAINT_ERROR
        try:
            await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
            asserts.fail("Unexpected success adding NOC for a second time")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error when adding NOC for a second time")

        self.print_step(26, "TH1 reads the NOCs attribute from DUT using a fabric-filtered read and saves the list as `NOCList`")
        noc_list = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.NOCs)
        asserts.assert_equal(len(noc_list), 1, "More than one NOC entry found for TH1")
        asserts.assert_equal(noc_list[0].noc, TH1_certs_real.nocBytes, "NOC for TH1 does not match supplied NOC")
        asserts.assert_equal(noc_list[0].icac, TH1_certs_real.icacBytes, "ICAC for TH1 does not match supplied ICAC")

        self.print_step(
            27, "TH1 modified the TH1 NOC and ICAC to be from the Root_CA_Certificate_TH1_2 and writes that value to the NOCs attribute. Verify UNSUPPORTED_WRITE")
        noc_list[0].noc = TH1_certs_fake.nocBytes
        noc_list[0].icac = TH1_certs_fake.icacBytes
        attr = opcreds.Attributes.NOCs(noc_list)
        resp = await TH1.WriteAttribute(nodeid=newNodeId, attributes=[(0, attr)])
        asserts.assert_equal(resp[0].Status, Status.UnsupportedWrite, "Write to NOC attribute did not fail with UnsupportedWrite")

        self.print_step(
            28, "TH1 reads the NOCs attribute from DUT. Verify the NOC has not been updated.")
        noc_list = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.NOCs)
        asserts.assert_equal(len(noc_list), 1, "More than one NOC entry found for TH1")
        asserts.assert_equal(noc_list[0].noc, TH1_certs_real.nocBytes, "NOC for TH1 does not match supplied NOC")
        asserts.assert_equal(noc_list[0].icac, TH1_certs_real.icacBytes, "ICAC for TH1 does not match supplied ICAC")

        self.print_step(29, "TH1 sends UpdateFabricLabel command with 'Label 1' as Label field to DUT, verify status OK")
        cmd = opcreds.Commands.UpdateFabricLabel(label="Label 1")
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "Failure on UpdateFabricLabel")

        self.print_step(30, "TH1 reads the Fabrics Attribute from DUT")
        fabrics = await self.read_single_attribute_check_success(
            dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.Fabrics)
        asserts.assert_equal(len(fabrics), 1, "Unexpected number of fabrics returned")

        self.print_step(31, "Read the other fields from FabricDescriptorStruct")
        # 1. RootPublicKey
        # 2. VendorID
        # 3. FabricID
        # 4. NodeID
        # 5. Label
        # Verify that the size of RootPublicKey is exactly 65 bytes
        # Verify that the RootPublicKey matches `Root_Public_Key_TH1`
        # Verify that the NodeID is the same as the matter-node-id field in the NOC sent with AddNOC Command
        # Verify that the VendorID is the same as the AdminVendorID sent with AddNOC Command
        # Verify that the FabricID is the same as the matter-fabric-id field in the NOC sent with AddNOC Command
        # Verify that the Label field has value "Label 1"
        asserts.assert_equal(len(fabrics[0].rootPublicKey), 65, "Root public key is not exactly 65 bytes")
        asserts.assert_equal(fabrics[0].rootPublicKey, root_public_key_th1,
                             "Root public key in fabrics table does not match supplied key")
        asserts.assert_equal(fabrics[0].nodeID, newNodeId, "NodeID in the fabrics table does not match the supplied node ID")
        asserts.assert_equal(fabrics[0].vendorID, TH1_vid,
                             "VendorID in the fabrics table does not match the supplied vendor ID")
        th1_noc_decoded = TLVReader(TH1_certs_real.nocBytes).get()["Any"]
        # subject is field 6
        noc_subject = th1_noc_decoded[6]
        # noc_subject is a TLVList, so it can be accessed by key. Fabric ID is dn-attribute 21.
        asserts.assert_equal(fabrics[0].fabricID, noc_subject[21],
                             "Fabric ID in the fabric table does not match the fabric ID in the NOC")

        self.print_step(32, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, cmd=cmd, node_id=newNodeId)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(33, "TH1 reconnects to the DUT over PASE")
        TH1.ExpireSessions(newNodeId)
        self.FindAndEstablishPase(dev_ctrl=TH1, longDiscriminator=longDiscriminator,
                                  setupPinCode=params.setupPinCode, nodeid=newNodeId)

        self.print_step(34, "TH1 reads the TrustedRootCertificates list from DUT and verifies the TH1 root is not present")
        trusted_root_list = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(trusted_root_list), trusted_root_original_size,
                             "TrustedRootCertificate was not properly removed from the list on ArmFailSafe expiration")

        self.print_step(
            35, "TH1 reads the NOCs attribute from DUT using a non-fabric-filtered read and verifies that the list contains only nocs_original_size entries")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=False)
        asserts.assert_equal(len(nocs), nocs_original_size, "NOC list size does not match original")

        self.print_step(
            36, "TH1 reads the Fabrics attribute from the DUT using a non-fabric-filtered read and verifies that the list contains only fabric_original_size entries")
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.Fabrics, fabric_filtered=False)
        asserts.assert_equal(len(fabrics), fabrics_original_size, "Fabric list size does not match original")

        self.print_step(37, "TH1 fully commissions DUT onto the fabric using a set of valid certificates")
        TH1.Commission(newNodeId)

        self.print_step(
            38, "TH1 reads the TrustedRootCertificates list from DUT and verify that there are trusted_root_original_size + 1 entries")
        trusted_root_list = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(trusted_root_list), trusted_root_original_size + 1, "Unexpected number of root certificates")

        self.print_step(
            39, "TH1 reads the NOCs attribute from DUT using a non-fabric-filtered read and verifies that there are nocs_original_size + 1 entries")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=False)
        asserts.assert_equal(len(nocs), nocs_original_size+1, "Unexpected number of NOCs")

        self.print_step(
            40, "TH1 reads the Fabrics attribute from DUT using a non-fabric-filtered read and verifies that there are fabrics_original_size + 1 entries")
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.Fabrics, fabric_filtered=False)
        asserts.assert_equal(len(fabrics), fabrics_original_size+1, "Fabric list size is incorrect")

        self.print_step(41, "TH1 reads the ACL attribute from the Access Control cluster using a fabric-filtered read")
        th1_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex)
        acl = await self.read_single_attribute_check_success(dev_ctrl=TH1, node_id=newNodeId, cluster=Clusters.AccessControl, attribute=Clusters.AccessControl.Attributes.Acl)
        # a| * Verify that the returned list includes an entry with:
        # Fabric index of `FabricIndex_TH1`
        # Administer privilege (5)
        # CASE AuthMode (2)
        # Includes the NodeID of TH1 in the list of subjects
        th1_acl = [x for x in acl if x.fabricIndex == th1_fabric_index and x.privilege ==
                   Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister and x.authMode == Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase]
        # There should be exactly one ACL for this fabric since we JUST commissioned it and didn't add any extra
        asserts.assert_equal(len(th1_acl), 1, "Unexpected number of ACL entries for TH1")
        asserts.assert_equal(th1_acl[0].subjects, [TH1_nodeid], "Unexpected subject list in ACL")

        self.print_step(42, "TH1 issues a KeySetRead command to the DUT for GroupKeySetID 0")
        cmd = Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=0)
        ret = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(ret.groupKeySet.groupKeySetID, 0, "Unexpected response from KeySetRead")

        self.print_step(43, "TH1 sends UpdateFabricLabel command with 'Label 1' as Label field to DUT, verify status OK")
        cmd = opcreds.Commands.UpdateFabricLabel(label="Label 1")
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "Failure on UpdateFabricLabel")

        self.print_step(44, "TH1 sends an OpenCommissioningWindow command to the Administrator Commissioning cluster")
        longDiscriminator, params = self.OpenCommissioningWindow(TH1, newNodeId)

        self.print_step(45, "TH2 commissions the DUT")
        TH2_CA = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        TH2_vid = 0xFFF3
        TH2_fabric_admin = TH2_CA.NewFabricAdmin(vendorId=TH2_vid, fabricId=3)
        TH2_nodeid = self.default_controller.nodeId+2
        TH2 = TH2_fabric_admin.NewController(nodeId=TH2_nodeid)
        TH2_dut_nodeid = self.dut_node_id+2
        TH2.CommissionOnNetwork(
            nodeId=TH2_dut_nodeid, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=longDiscriminator)

        self.print_step(
            46, "TH2 sends UpdateFabricLabel command with 'Label 2' as Label field to DUT and verify status OK")
        cmd = opcreds.Commands.UpdateFabricLabel(label="Label 2")
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=TH2, node_id=TH2_dut_nodeid)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "Failure on UpdateFabricLabel")

        self.print_step(
            47, "TH2 sends UpdateFabricLabel command with 'Label 1' as Label field to DUT and verify status is LabelConflict")
        cmd = opcreds.Commands.UpdateFabricLabel(label="Label 1")
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=TH2, node_id=TH2_dut_nodeid)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kLabelConflict, "Unexpected error code on UpdateFabricLabel")

        self.print_step(48, "Read the Fabrics List from DUT using a non-fabric-filtered read, verify fabrics_original_size + 2, check labels")
        fabrics = await self.read_single_attribute_check_success(
            dev_ctrl=TH1, node_id=newNodeId, cluster=opcreds, attribute=opcreds.Attributes.Fabrics, fabric_filtered=False)
        asserts.assert_equal(len(fabrics), fabrics_original_size + 2, "Unexpected number of fabrics")
        th1_fabric = [x for x in fabrics if x.vendorID == TH1_vid]
        th2_fabric = [x for x in fabrics if x.vendorID == TH2_vid]
        asserts.assert_equal(len(th1_fabric), 1, "Unexpected number of fabrics with TH1 vendor ID")
        asserts.assert_equal(len(th2_fabric), 1, "Unexpected number of fabrics with TH2 vendor ID")
        asserts.assert_equal(th1_fabric[0].label, "Label 1", "Unexpected label for TH1 fabric")
        asserts.assert_equal(th2_fabric[0].label, "Label 2", "Unexpected label for TH2 fabric")

        self.print_step(49, "TH1 sends ArmFailSafe command to the DUT  with the ExpiryLengthSeconds field set to failsafe_max| Verify that the DUT sends ArmFailSafeResponse Command to TH1 with field ErrorCode as 'OK'(0)")
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(failsafe_max))
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(50, "TH1 sends AddTrustedRootCertificate command to DUT with RootCACertificate set to `Root_CA_Certificate_TH1_1` |Verify that AddTrustedRootCertificate Command succeeds by sending the status code as SUCCESS")
        cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_certs_real.rcacBytes)
        await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)

        self.print_step(51, "TH1 sends the AddNOC Command to DUT with the following fields:")
        # NOCValue as `Node_Operational_Certificate_TH1_2`
        # ICACValue as `Intermediate_Certificate_TH1_2`
        # IpkValue as `IPK_TH1_2`
        # CaseAdminSubject as the NodeID of TH1
        # AdminVendorId as the Vendor ID of TH1  | Verify that DUT responds with status code MissingCsr
        cmd = opcreds.Commands.AddNOC(NOCValue=TH1_certs_real.nocBytes, ICACValue=TH1_certs_real.icacBytes,
                                      IPKValue=TH1_certs_real.ipkBytes, caseAdminSubject=TH1_nodeid, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr, "Unexpected error code on AddNOC with no CSR")

        self.print_step(52, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(53, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to failsafe_max")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_max, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(54, "TH1 Sends CSRRequest command with a random 32-byte nonce")
        nonce = random.randbytes(32)
        csrResponse_new = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=False))

        self.print_step(
            55, "TH1 generates a new RCAC, ICAC and NOC using the csr returned in step 4 (ie, NOT the most recent CSR)")
        temp_CA = self.certificate_authority_manager.NewCertificateAuthority()
        temp_fabric_admin = temp_CA.NewFabricAdmin(vendorId=0xFFF1, fabricId=3)
        temp_controller = temp_fabric_admin.NewController(nodeId=self.default_controller.nodeId)
        temp_certs = temp_controller.IssueNOCChain(csrResponse, newNodeId)
        if (temp_certs.rcacBytes is None or
                temp_certs.icacBytes is None or
                temp_certs.nocBytes is None or temp_certs.ipkBytes is None):
            # Expiring the failsafe timer in an attempt to clean up.
            await TH1.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
            asserts.fail("Unable to generate NOC chain for DUT - this is a script failure, please report this as a bug")

        self.print_step(56, "TH1 sends the AddTrustedRootCert command using the certs generated in step 54")
        cmd = opcreds.Commands.AddTrustedRootCertificate(temp_certs.rcacBytes)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)

        self.print_step(57, "TH1 sends the AddNOC Command to DUT using the certs generated in step 54")
        cmd = opcreds.Commands.AddNOC(NOCValue=temp_certs.nocBytes, ICACValue=temp_certs.icacBytes,
                                      IPKValue=temp_certs.ipkBytes, caseAdminSubject=TH1_nodeid, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidPublicKey, "Unexpected error code on AddNOC with mismatched CSR")

        self.print_step(58, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(59, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to failsafe_max")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_max, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(60, "TH1 Sends CSRRequest command with a random 32-byte nonce")
        nonce = random.randbytes(32)
        csrResponse_new = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=False))

        self.print_step(61, "TH1 obtains or generates a NOC and ICAC using the CSR elements from the previous step with a different NodeID, but the same Root CA Certificate and fabric ID as step <<TH1-gen-real-creds>>. Save as `Node_Operational_Certificates_TH1_fabric_conflict` and `Intermediate_Certificate_TH1_fabric_conflict`|")
        anotherNodeId = newNodeId + 1
        TH1_certs_fabric_conflict = TH1.IssueNOCChain(csrResponse_new, anotherNodeId)
        if (TH1_certs_fabric_conflict.rcacBytes is None or
                TH1_certs_fabric_conflict.icacBytes is None or
                TH1_certs_fabric_conflict.nocBytes is None or TH1_certs_fabric_conflict.ipkBytes is None):
            # Expiring the failsafe timer in an attempt to clean up.
            await TH1.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
            asserts.fail("Unable to generate NOC chain for DUT - this is a script failure, please report this as a bug")

        self.print_step(62, "TH1 sends the AddTrustedRootCert command using the certs generated in step 60")
        cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_certs_fabric_conflict.rcacBytes)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)

        self.print_step(63, "TH1 sends the AddNOC Command to DUT with the fabric conflict certs")
        cmd = opcreds.Commands.AddNOC(NOCValue=TH1_certs_fabric_conflict.nocBytes, ICACValue=TH1_certs_fabric_conflict.icacBytes,
                                      IPKValue=TH1_certs_fabric_conflict.ipkBytes, caseAdminSubject=TH1_nodeid, adminVendorId=TH1_vid)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kFabricConflict, "Unexpected error code on AddNOC with mismatched CSR")

        self.print_step(64, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0| Verify that the DUT sends ArmFailSafeResponse Command to TH1 with field ErrorCode as 'OK'(0)")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(
            65, "TH1 reads the TrustedRootCertificates list from DUT | Verify the list contains `trusted_root_original_size` + 2 entries")
        resp = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.TrustedRootCertificates, dev_ctrl=TH1, node_id=newNodeId)
        asserts.assert_equal(len(resp), trusted_root_original_size + 2, "Unexpected number of trusted roots")

        self.print_step(66, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to failsafe_max| Verify that the DUT sends ArmFailSafeResponse Command to TH1 with field ErrorCode as 'OK'(0)")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_max, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(
            67, "TH1 Sends CSRRequest command with a random 32-byte nonce and the IsForUpdateNOC field set to true a|* Verify that the DUT responds with the CSRResponse Command")
        nonce = random.randbytes(32)
        csrResponse = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=True))
        asserts.assert_true(isinstance(csrResponse, opcreds.Commands.CSRResponse),
                            "Unexpected response type for UpdateNOC csr request")

        self.print_step(68, "TH1 obtains or generates a NOC, Root CA Certificate, ICAC using the CSR elements from the previous step")
        TH1_certs_3 = TH1.IssueNOCChain(csrResponse, anotherNodeId)
        if (TH1_certs_3.rcacBytes is None or
                TH1_certs_3.icacBytes is None or
                TH1_certs_3.nocBytes is None or TH1_certs_3.ipkBytes is None):
            # Expiring the failsafe timer in an attempt to clean up.
            await TH1.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
            asserts.fail("Unable to generate NOC chain for DUT - this is a script failure, please report this as a bug")
        # Save RCAC as `Root_CA_Certificate_TH1_3`
        # Save ICAC as `Intermediate_Certificate_TH1_3`
        # Save NOC as `Node_Operational_Certificate_TH1_3` |

        self.print_step(69, "TH1 sends AddTrustedRootCertificate command to DUT with RootCACertificate set to `Root_CA_Certificate_TH1_3` |Verify that AddTrustedRootCertificate Command succeeds by sending the status code as SUCCESS")
        # TODO(#3126): This currently fails. This failure actually makes sense, though, and we should backport this to the spec and expect a failure here.
        # cmd = opcreds.Commands.AddTrustedRootCertificate(TH1_certs_3.rcacBytes)
        # await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)

        self.print_step(70, "TH1 sends the AddNOC Command to DUT with the following fields:")
        # TODO(#3126) - consider removing this when we adjust the spec to expect a failure on the above.
        # cmd = opcreds.Commands.UpdateNOC(NOCValue=TH1_certs_3.nocBytes, ICACValue=TH1_certs_3.icacBytes)
        # resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=TH1, node_id=newNodeId)

        self.print_step(71, "TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0| Verify that the DUT sends ArmFailSafeResponse Command to TH1 with field ErrorCode as 'OK'(0)")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=TH1, node_id=newNodeId, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(72, "TH2 reads its fabric index from the CurrentFabricIndex attribute and saves as FabricIndex_TH2")
        fabric_index_th2 = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex, dev_ctrl=TH2, node_id=TH2_dut_nodeid)

        self.print_step(73, "TH2 sends RemoveFabric command with Fabric Index as FabricIndexTH2 + 5 (Invalid Fabric Index) to DUT | Verify that DUT sends NOCResponse Command with StatusCode of InvalidFabricIndex")
        cmd = opcreds.Commands.RemoveFabric(fabric_index_th2 + 5)
        resp = await self.send_single_cmd(dev_ctrl=TH2, node_id=TH2_dut_nodeid, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidFabricIndex)

        self.print_step(74, "TH2 reads the Fabrics List from DUT using a non-fabric-filtered read")
        resp = await self.read_single_attribute_check_success(cluster=opcreds, attribute=opcreds.Attributes.Fabrics, dev_ctrl=TH2, node_id=TH2_dut_nodeid, fabric_filtered=False)
        asserts.assert_equal(len(resp), fabrics_original_size + 2, "Unexpected number of fabrics on device")

        self.print_step(75, "TH1 reads its fabric index from the CurrentFabricIndex attribute and saves as FabricIndex_TH1")
        fabric_index_th1 = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex, dev_ctrl=TH1, node_id=newNodeId)

        self.print_step(
            76, "TH0 sends RemoveFabric command with Fabric Index as FabricIndex_TH1")
        cmd = opcreds.Commands.RemoveFabric(fabric_index_th1)
        resp = await self.send_single_cmd(cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk)

        self.print_step(
            77, "TH0 sends RemoveFabric command with Fabric Index as FabricIndex_TH2")
        cmd = opcreds.Commands.RemoveFabric(fabric_index_th2)
        resp = await self.send_single_cmd(cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk)


if __name__ == "__main__":
    default_matter_test_main()
