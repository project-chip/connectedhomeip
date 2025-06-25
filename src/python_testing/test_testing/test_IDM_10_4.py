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

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.pics import parse_pics_xml
from chip.testing.runner import MockTestRunner

# Reachable attribute is off in the pics file
# MaxPathsPerInvoke is not include in the pics file
# Vendor ID is included on ON in the PICS file


def create_read(include_reachable: bool = False, include_max_paths: bool = False, include_vendor_id: bool = True) -> Attribute.AsyncReadTransaction.ReadResponse:
    # Attribute read here is set to match the example_pics_xml_basic_info.xml in this directory
    bi = Clusters.BasicInformation.Attributes
    lvl = Clusters.LevelControl.Attributes
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

    attrs_bi[bi.AttributeList] = [a.attribute_id for a in attrs_bi.keys()]
    attrs_bi[bi.AcceptedCommandList] = []
    attrs_bi[bi.GeneratedCommandList] = []
    attrs_bi[bi.FeatureMap] = 0

    attrs_lvl = {}
    attrs_lvl[lvl.AttributeList] = []
    attrs_lvl[lvl.AcceptedCommandList] = []
    attrs_lvl[lvl.GeneratedCommandList] = []
    attrs_lvl[lvl.FeatureMap] = 0

    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {0: {Clusters.BasicInformation: attrs_bi, Clusters.LevelControl: attrs_lvl}}

    tlv_attrs_bi = {a.attribute_id: value for a, value in attrs_bi.items()}
    tlv_attrs_lvl = {a.attribute_id: value for a, value in attrs_lvl.items()}
    resp.tlvAttributes = {0: {Clusters.BasicInformation.id: tlv_attrs_bi, Clusters.LevelControl.id: tlv_attrs_lvl}}

    return resp


def main():
    # TODO: add the same test for commands and features

    script_dir = Path(__file__).resolve().parent
    with open(script_dir / 'example_pics_xml_basic_info.xml') as f:
        pics = parse_pics_xml(f.read())
    test_runner = MockTestRunner(script_dir / '../TC_pics_checker.py',
                                 'TC_PICS_Checker', 'test_TC_IDM_10_4', 0, pics)
    failures = []

    # Success, include vendor ID, which IS in the pics file, and neither of the incorrect ones
    resp = create_read()
    print(resp)
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

    pics['PICS_SDK_CI_ONLY'] = True
    test_runner.config.pics = pics
    # This is a success case for the attributes (as seen above), but the test should fail because the CI PICS is added
    resp = create_read()
    if test_runner.run_test_with_mock_read(resp):
        failures.append("Test case failure: SDK CI PICS is included - expected failure")

    test_runner.Shutdown()

    print(
        f"Test of tests: PICS - test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
