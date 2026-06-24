#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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

import sys
from pathlib import Path

import matter.clusters as Clusters
from matter.clusters import Attribute
from matter.testing.pics import parse_pics_xml
from matter.testing.runner import MockTestRunner

# Reachable attribute is off in the pics file
# MaxPathsPerInvoke is not include in the pics file
# Vendor ID is included on ON in the PICS file


def create_read(include_reachable: bool = False, include_max_paths: bool = False, include_vendor_id: bool = True,
                device_type: int = 0x01, include_aggregator_ep1: bool = False, include_ota_requestor: bool = False,
                groups_on_two_endpoints: bool = False) -> Attribute.AsyncReadTransaction.ReadResponse:
    # Attribute read here is set to match the example_pics_xml_basic_info.xml in this directory
    bi = Clusters.BasicInformation.Attributes
    lvl = Clusters.LevelControl.Attributes
    desc = Clusters.Descriptor.Attributes
    attrs_bi = {bi.DataModelRevision: 1,
                bi.VendorName: 'testVendor',
                bi.ProductName: 'testProduct',
                bi.ProductID: 0x8000,
                bi.NodeLabel: 'label',
                bi.Location: 'XX',
                bi.HardwareVersion: 1,
                bi.HardwareVersionString: 'one',
                bi.SoftwareVersion: 2,
                bi.SoftwareVersionString: 'two',
                bi.ManufacturingDate: 'today',
                bi.PartNumber: 'three',
                bi.ProductURL: 'example.com',
                bi.ProductLabel: 'myProduct',
                bi.SerialNumber: 'ABCD1234',
                bi.LocalConfigDisabled: False,
                bi.SpecificationVersion: 0x01040000,
                bi.UniqueID: 'Hashy-McHashface'}
    if include_reachable:
        attrs_bi[bi.Reachable] = True
    if include_max_paths:
        attrs_bi[bi.MaxPathsPerInvoke] = 2
    if include_vendor_id:
        attrs_bi[bi.VendorID] = 0xFFF1

    attrs_bi[bi.AttributeList] = [a.attribute_id for a in attrs_bi]
    attrs_bi[bi.AcceptedCommandList] = []
    attrs_bi[bi.GeneratedCommandList] = []
    attrs_bi[bi.FeatureMap] = 0

    attrs_lvl = {}
    attrs_lvl[lvl.AttributeList] = []
    attrs_lvl[lvl.AcceptedCommandList] = []
    attrs_lvl[lvl.GeneratedCommandList] = []
    attrs_lvl[lvl.FeatureMap] = 0

    attrs_desc = {}
    attrs_desc[desc.DeviceTypeList] = [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=device_type, revision=1)]
    attrs_desc[desc.AttributeList] = [a.attribute_id for a in attrs_desc]
    attrs_desc[desc.AcceptedCommandList] = []
    attrs_desc[desc.GeneratedCommandList] = []
    attrs_desc[desc.FeatureMap] = 0

    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {0: {Clusters.Descriptor: attrs_desc, Clusters.BasicInformation: attrs_bi, Clusters.LevelControl: attrs_lvl}}

    tlv_attrs_bi = {a.attribute_id: value for a, value in attrs_bi.items()}
    tlv_attrs_lvl = {a.attribute_id: value for a, value in attrs_lvl.items()}
    tlv_attrs_desc = {a.attribute_id: value for a, value in attrs_desc.items()}
    tlv_attrs_desc[desc.DeviceTypeList.attribute_id] = [d.ToTLV() for d in attrs_desc[desc.DeviceTypeList]]
    resp.tlvAttributes = {0: {Clusters.Descriptor.id: tlv_attrs_desc,
                              Clusters.BasicInformation.id: tlv_attrs_bi, Clusters.LevelControl.id: tlv_attrs_lvl}}

    # Optional extras driving the new Base/MCORE consistency step.
    if include_ota_requestor:
        # OtaSoftwareUpdateRequestor server cluster on EP0 -> MCORE.OTA.Requestor.
        ota = Clusters.OtaSoftwareUpdateRequestor
        empty_globals = {
            ota.Attributes.AttributeList.attribute_id: [],
            ota.Attributes.AcceptedCommandList.attribute_id: [],
            ota.Attributes.GeneratedCommandList.attribute_id: [],
            ota.Attributes.FeatureMap.attribute_id: 0,
            ota.Attributes.ClusterRevision.attribute_id: 1,
        }
        resp.tlvAttributes[0][ota.id] = empty_globals
        resp.attributes[0][ota] = {}

    if include_aggregator_ep1:
        # Aggregator device type on EP1 -> MCORE.BRIDGE.
        ep1_desc = {desc.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=0x000E, revision=1)],
                    desc.AttributeList: [], desc.AcceptedCommandList: [],
                    desc.GeneratedCommandList: [], desc.FeatureMap: 0}
        resp.attributes[1] = {Clusters.Descriptor: ep1_desc}
        ep1_tlv_desc = {desc.AttributeList.attribute_id: [],
                        desc.AcceptedCommandList.attribute_id: [],
                        desc.GeneratedCommandList.attribute_id: [],
                        desc.FeatureMap.attribute_id: 0,
                        desc.DeviceTypeList.attribute_id: [d.ToTLV() for d in ep1_desc[desc.DeviceTypeList]]}
        resp.tlvAttributes[1] = {Clusters.Descriptor.id: ep1_tlv_desc}

    if groups_on_two_endpoints:
        # Groups cluster on EP1 and EP2 -> MCORE.G.MULTIENDPOINT (needs >= 2).
        groups = Clusters.Groups
        groups_globals = {
            groups.Attributes.AttributeList.attribute_id: [],
            groups.Attributes.AcceptedCommandList.attribute_id: [],
            groups.Attributes.GeneratedCommandList.attribute_id: [],
            groups.Attributes.FeatureMap.attribute_id: 0,
            groups.Attributes.ClusterRevision.attribute_id: 1,
        }
        bare_desc_attrs = {desc.DeviceTypeList: [], desc.AttributeList: [],
                           desc.AcceptedCommandList: [], desc.GeneratedCommandList: [],
                           desc.FeatureMap: 0}
        bare_desc_tlv = {desc.AttributeList.attribute_id: [],
                         desc.AcceptedCommandList.attribute_id: [],
                         desc.GeneratedCommandList.attribute_id: [],
                         desc.FeatureMap.attribute_id: 0,
                         desc.DeviceTypeList.attribute_id: []}
        for ep in (1, 2):
            resp.attributes.setdefault(ep, {})[Clusters.Descriptor] = bare_desc_attrs
            resp.attributes[ep][groups] = {}
            resp.tlvAttributes.setdefault(ep, {})[Clusters.Descriptor.id] = bare_desc_tlv
            resp.tlvAttributes[ep][groups.id] = groups_globals

    print(resp)
    return resp


def _load_baseline_pics() -> dict:
    """Re-read the example PICS file each time so test cases that mutate the
    tree don't leak state into one another. Returns an endpoint-keyed PICS tree
    ({endpoint: {code: bool}}); device-wide codes are under endpoint 0."""
    script_dir = Path(__file__).resolve().parent
    with open(script_dir / 'example_pics_xml_basic_info.xml') as f:
        return parse_pics_xml(f.read())


def main():
    # TODO: add the same test for commands and features

    ROOT_NODE_DEVICE_TYPE = 0x16

    script_dir = Path(__file__).resolve().parent
    pics = _load_baseline_pics()
    test_runner = MockTestRunner(script_dir / '../TC_pics_checker.py',
                                 'TC_PICS_Checker', 'test_TC_IDM_10_4', 0, pics)
    failures = []

    # Success, include vendor ID, which IS in the pics file, and neither of the incorrect ones
    resp = create_read()
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Vendor ID included - expected pass")

    # Failure because Vendor ID is not included in the read, but included in the PICS
    resp = create_read(include_vendor_id=False)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Vendor ID not included - expected failure")

    # Failure because Reachable is included in the read, but not in the PICS
    resp = create_read(include_reachable=True)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Reachable included but not in PICS- expected failure")

    # Failure because MaxPathsPerInvoke is included in the read, but not in the PICS
    resp = create_read(include_max_paths=True)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: MaxPathsPerInvoke included but not in PICS- expected failure")

    # If we make the device type a root node, this test should fail because MCORE.ROLE.COMMISSIONEE needs to be set
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: MCORE.ROLE.COMMISSIONEE is not included for a root node device - expected failure")

    # If we add the MCORE.ROLE.COMMISSIONEE PICS, this should pass again.
    pics[0]['MCORE.ROLE.COMMISSIONEE'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: MCORE.ROLE.COMMISSIONEE is included for a root node device - expected success")

    # If we add a QR code PICS with no manual, it should fail
    pics[0]['MCORE.DD.QR'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: QR code with no manual code - expected failure")

    # If we add the manual code, it should pass again
    pics[0]['MCORE.DD.MANUAL_PC'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: QR code with manual code - expected success")

    # NFC only with no manual should fail
    pics[0]['MCORE.DD.QR'] = False
    pics[0]['MCORE.DD.MANUAL_PC'] = False
    pics[0]['MCORE.DD.NFC'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: NFC code with no manual code - expected failure")

    # If we add the manual code again, it should pass
    pics[0]['MCORE.DD.MANUAL_PC'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: NFC code with manual code - expected success")

    # All three should also be fine.
    pics[0]['MCORE.DD.QR'] = True
    test_runner.config.pics = pics
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: NFC and QR code with manual code - expected success")

    pics[0]['PICS_SDK_CI_ONLY'] = True
    test_runner.config.pics = pics
    # This is a success case for the attributes (as seen above), but the test should fail because the CI PICS is added
    resp = create_read(device_type=ROOT_NODE_DEVICE_TYPE)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: SDK CI PICS is included - expected failure")

    # ===== New Base/MCORE consistency cases (test step 10). =====
    # Reset PICS to the example baseline (which already includes MCORE.IDM.S).
    base_pics = _load_baseline_pics()

    def _baseline_with(**device_codes: bool) -> dict:
        """Fresh copy of the baseline endpoint-keyed tree with the given
        device-wide codes added under endpoint 0."""
        tree = {endpoint: dict(codes) for endpoint, codes in base_pics.items()}
        tree.setdefault(0, {}).update(device_codes)
        return tree

    # Bridge: aggregator device type on EP1 must require MCORE.BRIDGE=1.
    test_runner.config.pics = _baseline_with()
    resp = create_read(include_aggregator_ep1=True)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Aggregator EP without MCORE.BRIDGE - expected failure")

    test_runner.config.pics = _baseline_with(**{"MCORE.BRIDGE": True})
    resp = create_read(include_aggregator_ep1=True)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Aggregator EP with MCORE.BRIDGE - expected success")

    # PICS file claims MCORE.BRIDGE=1 but device does not expose Aggregator
    # device type. Step 10 must catch the lie.
    test_runner.config.pics = _baseline_with(**{"MCORE.BRIDGE": True})
    resp = create_read()
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: MCORE.BRIDGE claimed but device has no Aggregator - expected failure")

    # OTA Requestor on EP0 must require MCORE.OTA.Requestor=1.
    test_runner.config.pics = _baseline_with()
    resp = create_read(include_ota_requestor=True)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: OTA Requestor cluster without MCORE.OTA.Requestor - expected failure")

    test_runner.config.pics = _baseline_with(**{"MCORE.OTA.Requestor": True})
    resp = create_read(include_ota_requestor=True)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: OTA Requestor cluster with MCORE.OTA.Requestor - expected success")

    # Groups on two endpoints must require MCORE.G.MULTIENDPOINT=1.
    test_runner.config.pics = _baseline_with()
    resp = create_read(groups_on_two_endpoints=True)
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Groups on 2 endpoints without MCORE.G.MULTIENDPOINT - expected failure")

    test_runner.config.pics = _baseline_with(**{"MCORE.G.MULTIENDPOINT": True})
    resp = create_read(groups_on_two_endpoints=True)
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: Groups on 2 endpoints with MCORE.G.MULTIENDPOINT - expected success")

    # Regression: PICS codes that belong to a different endpoint's slice must
    # not be evaluated against the endpoint under test (here, EP0). The Switch
    # cluster is marked supported only on endpoint 1's slice and is absent from
    # the EP0 device read; the EP0 run must still pass. Before check_pics was
    # made endpoint-aware, SWTCH.S would leak in as "found in PICS but not on
    # device" and fail the EP0 check.
    leak_pics = _baseline_with()
    leak_pics[1] = {"SWTCH.S": True, "SWTCH.S.A0000": True, "SWTCH.S.F00": True}
    test_runner.config.pics = leak_pics
    resp = create_read()
    if not test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: foreign-endpoint (EP1) PICS leaked into EP0 check - expected success")

    test_runner.Shutdown()

    print(
        f"Test of tests: PICS - test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
