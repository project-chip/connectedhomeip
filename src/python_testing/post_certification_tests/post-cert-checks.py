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
# This test is used to evaluate that all the proper post-certification
# work has been done to make a Matter device production ready.
# This test ensure that:
# - DAC chain is valid and spec compliant, and chains up to a PAA that
#   is registered in the main net DCL
# - CD is valid and, signed by one of the known CSA signing certs and
#   is marked as a production CD
# - DCL entries for this device and vendor have all been registered
# - TestEventTriggers have been turned off
#
# This test is performed over PASE on a factory reset device.
#
# To run this test, first build and install the python chip wheel
# files, then add the extra dependencies. From the root:
#
# ./scripts/build_python.sh -i py
# source py/bin/activate
# pip install opencv-python requests click_option_group


import importlib
import logging
import os
import shutil
import sys
import time
import typing
import uuid
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path

import chip.clusters as Clusters
import cv2
import requests
from mobly import asserts, signals

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

try:
    from basic_composition_support import BasicCompositionTests
    from matter_testing_support import (MatterBaseTest, MatterStackState, MatterTestConfig, TestStep, async_test_body,
                                        run_tests_no_exit)
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from basic_composition_support import BasicCompositionTests
    from matter_testing_support import (MatterBaseTest, MatterStackState, MatterTestConfig, TestStep, async_test_body,
                                        run_tests_no_exit)

try:
    import fetch_paa_certs_from_dcl
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(DEFAULT_CHIP_ROOT, 'credentials')))
    import fetch_paa_certs_from_dcl


@dataclass
class Failure:
    test: str
    step: str


class Hooks():
    def __init__(self):
        self.failures = []
        self.current_step = 'unknown'
        self.current_test = 'unknown'

    def start(self, count: int):
        pass

    def stop(self, duration: int):
        pass

    def test_start(self, filename: str, name: str, count: int):
        self.current_test = name
        pass

    def test_stop(self, exception: Exception, duration: int):
        pass

    def step_skipped(self, name: str, expression: str):
        pass

    def step_start(self, name: str):
        self.current_step = name

    def step_success(self, logger, logs, duration: int, request):
        pass

    def step_failure(self, logger, logs, duration: int, request, received):
        self.failures.append(Failure(self.current_test, self.current_step))

    def step_unknown(self):
        pass

    def get_failures(self) -> list[str]:
        return self.failures


class TestEventTriggersCheck(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def test_TestEventTriggersCheck(self):
        self.connect_over_pase(self.default_controller)
        gd = Clusters.GeneralDiagnostics
        ret = await self.read_single_attribute_check_success(cluster=gd, attribute=gd.Attributes.TestEventTriggersEnabled)
        asserts.assert_equal(ret, 0, "TestEventTriggers are still on")


def get_dcl_vendor(vid):
    return requests.get(f"{fetch_paa_certs_from_dcl.PRODUCTION_NODE_URL_REST}/dcl/vendorinfo/vendors/{vid}").json()


def get_dcl_model(vid, pid):
    return requests.get(f"{fetch_paa_certs_from_dcl.PRODUCTION_NODE_URL_REST}/dcl/model/models/{vid}/{pid}").json()


def get_dcl_compliance_info(vid, pid, software_version):
    return requests.get(f"{fetch_paa_certs_from_dcl.PRODUCTION_NODE_URL_REST}/dcl/compliance/compliance-info/{vid}/{pid}/{software_version}/matter").json()


def get_dcl_certified_model(vid, pid, software_version):
    return requests.get(f"{fetch_paa_certs_from_dcl.PRODUCTION_NODE_URL_REST}/dcl/compliance/certified-models/{vid}/{pid}/{software_version}/matter").json()


class DclCheck(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        self.connect_over_pase(self.default_controller)
        bi = Clusters.BasicInformation
        self.vid = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.VendorID)
        self.pid = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.ProductID)
        self.software_version = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.SoftwareVersion)

    def steps_Vendor(self):
        return [TestStep(1, "Check if device VID is listed in the DCL vendor schema", "Listing found")]

    def test_Vendor(self):
        self.step(1)
        entry = get_dcl_vendor(self.vid)
        key = 'vendorInfo'
        asserts.assert_true(key in entry.keys(), f"Unable to find vendor entry for {self.vid:04x}")
        logging.info(f'Found vendor key 0x{self.vid:04X} in DCL:')
        logging.info(f'{entry[key]}')

    def steps_Model(self):
        return [TestStep(1, "Check if device VID/PID are listed in the DCL model schema", "Listing found")]

    def test_Model(self):
        self.step(1)
        key = 'model'
        entry = get_dcl_model(self.vid, self.pid)
        asserts.assert_true(key in entry.keys(), f"Unable to find model entry for {self.vid:04x} {self.pid:04x}")
        logging.info(f'Found model entry for vid=0x{self.vid:04X} pid=0x{self.pid:04X} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_Compliance(self):
        return [TestStep(1, "Check if device VID/PID/SoftwareVersion are listed in the DCL compliance info schema", "Listing found")]

    def test_Compliance(self):
        self.step(1)
        key = 'complianceInfo'
        entry = get_dcl_compliance_info(self.vid, self.pid, self.software_version)
        asserts.assert_true(key in entry.keys(),
                            f"Unable to find compliance entry for {self.vid:04x} {self.pid:04x} {self.software_version}")
        logging.info(
            f'Found compliance info for vid=0x{self.vid:04X} pid=0x{self.pid:04X} software version={self.software_version} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_CertifiedModel(self):
        return [TestStep(1, "Check if device VID/PID/SoftwareVersion are listed in the DCL certified model schema", "Listing found")]

    def test_CertifiedModel(self):
        self.step(1)
        key = 'certifiedModel'
        entry = get_dcl_certified_model(self.vid, self.pid, self.software_version)
        asserts.assert_true(key in entry.keys(),
                            f"Unable to find certified model entry for {self.vid:04x} {self.pid:04x} {self.software_version}")
        logging.info(
            f'Found certified model for vid=0x{self.vid:04X} pid=0x{self.pid:04X} software version={self.software_version} in the DCL:')
        logging.info(f'{entry[key]}')


def get_qr() -> str:
    qr_code_detector = cv2.QRCodeDetector()
    camera_id = 0
    video_capture = cv2.VideoCapture(camera_id)
    window_name = 'Post-certification check QR code reader'
    qr = ''
    while not qr:
        ret, frame = video_capture.read()
        if ret:
            ret_qr, decoded_info, points, _ = qr_code_detector.detectAndDecodeMulti(
                frame)
            if ret_qr:
                for s, p in zip(decoded_info, points):
                    if s and s.startswith("MT:"):
                        qr = s
                        color = (0, 255, 0)
                    else:
                        color = (0, 0, 255)
                    frame = cv2.polylines(
                        frame, [p.astype(int)], True, color, 8)
            cv2.imshow(window_name, frame)

        if (cv2.waitKey(1) & 0xFF == ord('q')):
            break
        if qr:
            time.sleep(1)
            break

    cv2.destroyWindow(window_name)
    return qr


class SetupCodeType(Enum):
    UNKNOWN = auto(),
    QR = auto(),
    MANUAL = auto(),


def get_setup_code() -> (str, bool):
    ''' Returns the setup code and an enum indicating the code type.'''
    while True:
        print('Press q for qr code or m for manual code')
        pref = input()
        if pref in ['q', 'Q']:
            return (get_qr(), SetupCodeType.QR)
        elif pref in ['m', 'M']:
            print('please enter manual code')
            m = input()
            m = ''.join([i for i in m if m.isnumeric()])
            if len(m) == 11 or len(m) == 21:
                return (m, SetupCodeType.MANUAL)
            else:
                print("Invalid manual code - please try again")


class TestConfig(object):
    def __init__(self, code: str, code_type: SetupCodeType):
        tmp_uuid = str(uuid.uuid4())
        tmpdir_paa = f'paas_{tmp_uuid}'
        tmpdir_cd = f'cd_{tmp_uuid}'
        self.paa_path = os.path.join('.', tmpdir_paa)
        self.cd_path = os.path.join('.', tmpdir_cd)
        os.mkdir(self.paa_path)
        os.mkdir(self.cd_path)
        fetch_paa_certs_from_dcl.fetch_paa_certs(use_main_net_dcld='', use_test_net_dcld='',
                                                 use_main_net_http=True, use_test_net_http=False, paa_trust_store_path=tmpdir_paa)
        fetch_paa_certs_from_dcl.fetch_cd_signing_certs(tmpdir_cd)
        self.admin_storage = f'admin_storage_{tmp_uuid}.json'
        global_test_params = {'use_pase_only': True, 'post_cert_test': True}
        self.config = MatterTestConfig(endpoint=0, dut_node_ids=[
                                       1], global_test_params=global_test_params, storage_path=self.admin_storage)
        if code_type == SetupCodeType.QR:
            self.config.qr_code_content = code
        else:
            self.config.manual_code = code
        self.config.paa_trust_store_path = Path(self.paa_path)
        # Set for DA-1.2, which uses the CD signing certs for verification. This test is now set to use the production CD signing certs from the DCL.
        self.config.global_test_params['cd_cert_dir'] = tmpdir_cd
        self.stack = MatterStackState(self.config)
        self.default_controller = self.stack.certificate_authorities[0].adminList[0].NewController(
            nodeId=112233,
            paaTrustStorePath=str(self.config.paa_trust_store_path)
        )

    def get_stack(self):
        return self.stack

    def get_controller(self):
        return self.default_controller

    def get_config(self, tests: list[str]):
        self.config.tests = tests
        return self.config

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.default_controller.Shutdown()
        self.stack.Shutdown()
        os.remove(self.admin_storage)
        shutil.rmtree(self.paa_path)
        shutil.rmtree(self.cd_path)


def run_test(test_class: MatterBaseTest, tests: typing.List[str], test_config: TestConfig) -> list[str]:
    hooks = Hooks()
    stack = test_config.get_stack()
    controller = test_config.get_controller()
    matter_config = test_config.get_config(tests)
    ok = run_tests_no_exit(test_class, matter_config, hooks, controller, stack)
    if not ok:
        print(f"Test failure. Failed on step: {hooks.get_failures()}")
    return hooks.get_failures()


def run_cert_test(test: str, test_config: TestConfig) -> list[str]:
    ''' Runs the specified test, returns a list of failures'''
    # for simplicity and because I know the tests we're running follow this pattern,
    # just assume the naming convention based off the base name - ie, file and class
    # share a name, test is test_classname
    module = importlib.import_module(test)
    test_class = getattr(module, test)
    return run_test(test_class, [f'test_{test}'], test_config)


def main():
    code, code_type = get_setup_code()
    with TestConfig(code, code_type) as test_config:
        # DA-1.2 is a test of the certification declaration
        failures_DA_1_2 = run_cert_test('TC_DA_1_2', test_config)
        # DA-1.7 is a test of the DAC chain (up to a PAA in the given directory)
        failures_DA_1_7 = run_cert_test('TC_DA_1_7', test_config)

        failures_test_event_trigger = run_test(TestEventTriggersCheck, ['test_TestEventTriggersCheck'], test_config)

        failures_dcl = run_test(DclCheck, ['test_Vendor', 'test_Model', 'test_Compliance', 'test_CertifiedModel'], test_config)

        failures = failures_DA_1_2 + failures_DA_1_7 + failures_test_event_trigger + failures_dcl

    print(failures)
    return 0


if __name__ == "__main__":
    sys.exit(main())
