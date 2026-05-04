#
#    Copyright (c) 2025 Project CHIP Authors
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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pics import generate_device_element_pics_from_device_wildcard
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters


class TestPicsHelpers(MatterBaseTest):
    def test_pics_generation(self):
        xml_cluster, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        # 0: opcreds with an attribute, an accepted command, a generated command
        # 0: cadmin with a feature, an attribute, an accepted command
        # 1: Door lock cluster with two features, two attributes, two accepted commands, two generated commands
        wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})
        desc = Clusters.Descriptor
        opcreds = Clusters.OperationalCredentials
        cadmin = Clusters.AdministratorCommissioning
        lock = Clusters.DoorLock

        wildcard.tlvAttributes[0] = {desc.id: {}, opcreds.id: {}, cadmin.id: {}}
        wildcard.tlvAttributes[1] = {lock.id: {}}

        wildcard.tlvAttributes[0][desc.id][desc.Attributes.AttributeList.attribute_id] = [
            desc.Attributes.DeviceTypeList.attribute_id]
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.AcceptedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.GeneratedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.FeatureMap.attribute_id] = 0
        desc_expected = ['DESC.S', 'DESC.S.A0000']
        # The rest of the processing is done through tlvAttributes, but this one needs to be parsed, so it comes from here.
        wildcard.attributes = {0: {desc: {desc.Attributes.DeviceTypeList: [
            desc.Structs.DeviceTypeStruct(deviceType=0x16, revision=1)]}}}

        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.AttributeList.attribute_id] = [
            opcreds.Attributes.NOCs.attribute_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.AcceptedCommandList.attribute_id] = [
            opcreds.Commands.AttestationRequest.command_id, opcreds.Commands.AddTrustedRootCertificate.command_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.GeneratedCommandList.attribute_id] = [
            opcreds.Commands.AttestationResponse.command_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.FeatureMap.attribute_id] = 0
        opcreds_expected_suffix = ['A0000', 'C00.Rsp', 'C0b.Rsp', 'C01.Tx']
        opcreds_expected = [f'OPCREDS.S.{s}' for s in opcreds_expected_suffix]
        opcreds_expected.append('OPCREDS.S')

        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.AttributeList.attribute_id] = [
            cadmin.Attributes.AdminFabricIndex.attribute_id]
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.AcceptedCommandList.attribute_id] = [
            cadmin.Commands.OpenCommissioningWindow.command_id]
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.GeneratedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.FeatureMap.attribute_id] = 1
        cadmin_expected_suffix = ['A0001', 'C00.Rsp', 'F00']
        cadmin_expected = [f'CADMIN.S.{s}' for s in cadmin_expected_suffix]
        cadmin_expected.append('CADMIN.S')

        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id] = [
            lock.Attributes.DoorState.attribute_id, lock.Attributes.LockType.attribute_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AcceptedCommandList.attribute_id] = [
            lock.Commands.GetUser.command_id, lock.Commands.GetCredentialStatus.command_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.GeneratedCommandList.attribute_id] = [
            lock.Commands.GetUserResponse.command_id, lock.Commands.GetCredentialStatusResponse.command_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.FeatureMap.attribute_id] = 3
        lock_expected_suffix = ['A0003', 'A0001', 'C1b.Rsp', 'C24.Rsp', 'C1c.Tx', 'C25.Tx', 'F00', 'F01']
        lock_expected = [f'DRLK.S.{s}' for s in lock_expected_suffix]
        lock_expected.append('DRLK.S')

        def check_expected_pics(pics_list: dict[int, list[str]]):
            asserts.assert_equal(set(pics_list.keys()), {0, 1}, "Unexpected endpoints in PICS list")
            asserts.assert_equal(set(pics_list[0]), set(desc_expected + opcreds_expected + cadmin_expected +
                                 ['IDM.S', 'MCORE.ROLE.COMMISSIONEE']), "Incorrect PICS list on EP0")
            asserts.assert_equal(set(pics_list[1]), set(lock_expected + ['IDM.S']), "Incorrect PICS list on EP1")

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add globals, should be no errors, should not appear
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id].extend(
            [lock.Attributes.AttributeList.attribute_id, lock.Attributes.AcceptedCommandList.attribute_id, lock.Attributes.GeneratedCommandList.attribute_id, lock.Attributes.FeatureMap.attribute_id, lock.Attributes.ClusterRevision.attribute_id])

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add MEI cluster, MEI attribute, MEI accepted command, MEI generated command - should be no errors, should not appear
        unit_testing = Clusters.UnitTesting
        wildcard.tlvAttributes[1][unit_testing.id] = {}
        # MEI cluster
        wildcard.tlvAttributes[1][unit_testing.id][unit_testing.Attributes.AttributeList.attribute_id] = [
            unit_testing.Attributes.Bitmap16.attribute_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id].append(0x60060000)
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AcceptedCommandList.attribute_id].append(0x60060000)
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.GeneratedCommandList.attribute_id].append(0x60060001)

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add a standard cluster that's not part of the standard cluster set - this should cause an error because we don't know the PICS
        unknown_standard_cluster = max(xml_cluster.keys()) + 1
        wildcard.tlvAttributes[1][unknown_standard_cluster] = {}
        wildcard.tlvAttributes[1][unknown_standard_cluster][unit_testing.Attributes.AttributeList.attribute_id] = [0]

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 1, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)


if __name__ == "__main__":
    default_matter_test_main()
