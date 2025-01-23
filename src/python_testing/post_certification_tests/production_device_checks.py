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
# - DCL OTA entries have proper sizes and checksums
# - TestEventTriggers have been turned off
#
# This test is performed over PASE on a factory reset device.
#
# To run this test, first build and install the python chip wheel
# files, then add the extra dependencies. From the root:
#
# . scripts/activate.sh
# ./scripts/build_python.sh -i out/python_env
# source out/python_env/bin/activate
# pip install opencv-python requests click_option_group
# python src/python_testing/post_certification_tests/production_device_checks.py

import asyncio
import base64
import hashlib
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
from mobly import asserts

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

try:
    from chip.testing.basic_composition import BasicCompositionTests
    from chip.testing.matter_testing import (MatterBaseTest, MatterStackState, MatterTestConfig, TestStep, async_test_body,
                                             run_tests_no_exit)
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from chip.testing.basic_composition import BasicCompositionTests
    from chip.testing.matter_testing import (MatterBaseTest, MatterStackState, MatterTestConfig, TestStep, async_test_body,
                                             run_tests_no_exit)

try:
    import fetch_paa_certs_from_dcl
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(DEFAULT_CHIP_ROOT, 'credentials')))
    import fetch_paa_certs_from_dcl


@dataclass
class Failure:
    step: str
    exception: typing.Optional[Exception]


class Hooks():
    def __init__(self):
        self.failures = {}
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
        # Exception is the test assertion that caused the failure
        if exception:
            self.failures[self.current_test].exception = exception

    def step_skipped(self, name: str, expression: str):
        pass

    def step_start(self, name: str):
        self.current_step = name

    def step_success(self, logger, logs, duration: int, request):
        pass

    def step_failure(self, logger, logs, duration: int, request, received):
        self.failures[self.current_test] = Failure(self.current_step, None)

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


class DclCheck(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        self.connect_over_pase(self.default_controller)
        bi = Clusters.BasicInformation
        self.vid = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.VendorID)
        self.pid = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.ProductID)
        self.software_version = await self.read_single_attribute_check_success(cluster=bi, attribute=bi.Attributes.SoftwareVersion)
        self.url = fetch_paa_certs_from_dcl.PRODUCTION_NODE_URL_REST

        self.vid_str = f'vid = 0x{self.vid:04X}'
        self.vid_pid_str = f'{self.vid_str} pid = 0x{self.pid:04X}'
        self.vid_pid_sv_str = f'{self.vid_pid_str} software version = {self.software_version}'

    def steps_Vendor(self):
        return [TestStep(1, "Check if device VID is listed in the DCL vendor schema", "Listing found")]

    def test_Vendor(self):
        self.step(1)
        entry = requests.get(f"{self.url}/dcl/vendorinfo/vendors/{self.vid}").json()
        key = 'vendorInfo'
        asserts.assert_true(key in entry.keys(), f"Unable to find vendor entry for {self.vid_str}")
        logging.info(f'Found vendor key for {self.vid_str} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_Model(self):
        return [TestStep(1, "Check if device VID/PID are listed in the DCL model schema", "Listing found")]

    def test_Model(self):
        self.step(1)
        key = 'model'
        entry = requests.get(f"{self.url}/dcl/model/models/{self.vid}/{self.pid}").json()
        asserts.assert_true(key in entry.keys(), f"Unable to find model entry for {self.vid_pid_str}")
        logging.info(f'Found model entry for {self.vid_pid_str} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_Compliance(self):
        return [TestStep(1, "Check if device VID/PID/SoftwareVersion are listed in the DCL compliance info schema", "Listing found")]

    def test_Compliance(self):
        self.step(1)
        key = 'complianceInfo'
        entry = requests.get(
            f"{self.url}/dcl/compliance/compliance-info/{self.vid}/{self.pid}/{self.software_version}/matter").json()
        asserts.assert_true(key in entry.keys(),
                            f"Unable to find compliance entry for {self.vid_pid_sv_str}")
        logging.info(
            f'Found compliance info for {self.vid_pid_sv_str} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_CertifiedModel(self):
        return [TestStep(1, "Check if device VID/PID/SoftwareVersion are listed in the DCL certified model schema", "Listing found")]

    def test_CertifiedModel(self):
        self.step(1)
        key = 'certifiedModel'
        entry = requests.get(
            f"{self.url}/dcl/compliance/certified-models/{self.vid}/{self.pid}/{self.software_version}/matter").json()
        asserts.assert_true(key in entry.keys(),
                            f"Unable to find certified model entry for {self.vid_pid_sv_str}")
        logging.info(
            f'Found certified model for {self.vid_pid_sv_str} in the DCL:')
        logging.info(f'{entry[key]}')

    def steps_AllSoftwareVersions(self):
        return [TestStep(1, "Query the version information for this software version", "DCL entry exists"),
                TestStep(2, "For each valid software version with an OtaUrl, verify the OtaChecksumType is in the valid range and the OtaChecksum is a base64. If the softwareVersion matches the current softwareVersion on the device, ensure the entry is valid.", "OtaChecksum is base64 and OtaChecksumType is in the valid set")]

    def test_AllSoftwareVersions(self):
        self.step(1)
        versions_entry = requests.get(f"{self.url}/dcl/model/versions/{self.vid}/{self.pid}").json()
        key_model_versions = 'modelVersions'
        asserts.assert_true(key_model_versions in versions_entry.keys(),
                            f"Unable to find {key_model_versions} in software versions schema for {self.vid_pid_str}")
        logging.info(f'Found version info for vid=0x{self.vid_pid_str} in the DCL:')
        logging.info(f'{versions_entry[key_model_versions]}')
        key_software_versions = 'softwareVersions'
        asserts.assert_true(key_software_versions in versions_entry[key_model_versions].keys(
        ), f"Unable to find {key_software_versions} in software versions schema for {self.vid_pid_str}")

        problems = []
        self.step(2)
        for software_version in versions_entry[key_model_versions][key_software_versions]:
            entry_wrapper = requests.get(f"{self.url}/dcl/model/versions/{self.vid}/{self.pid}/{software_version}").json()
            key_model_version = 'modelVersion'
            if key_model_version not in entry_wrapper:
                problems.append(
                    f'Missing key {key_model_version} in entry for {self.vid_pid_str} software version={software_version}')
                continue
            logging.info(f'Found entry version entry for {self.vid_pid_str} software version={software_version}')
            logging.info(entry_wrapper)
            entry = entry_wrapper[key_model_version]
            key_ota_url = 'otaUrl'
            key_software_version_valid = 'softwareVersionValid'
            key_ota_checksum = 'otaChecksum'
            key_ota_checksum_type = 'otaChecksumType'
            key_ota_file_size = 'otaFileSize'

            def check_key(key):
                if key not in entry.keys():
                    problems.append(
                        f'Missing key {key} in DCL versions entry for {self.vid_pid_str} software version={software_version}')
            check_key(key_ota_url)
            check_key(key_software_version_valid)
            if entry[key_software_version_valid] and entry[key_ota_url]:
                check_key(key_ota_checksum)
                check_key(key_ota_checksum_type)
                checksum_types = {1: hashlib.sha256, 7: hashlib.sha384, 8: hashlib.sha256,
                                  10: hashlib.sha3_256, 11: hashlib.sha3_384, 12: hashlib.sha3_512}
                if entry[key_ota_checksum_type] not in checksum_types.keys():
                    problems.append(
                        f'OtaChecksumType for entry {self.vid_pid_str} software version={software_version} is invalid. Found {entry[key_ota_checksum_type]} valid values: {checksum_types.keys()}')
                    continue
                checksum = entry[key_ota_checksum]
                try:
                    is_base64 = base64.b64encode(base64.b64decode(checksum)).decode('utf-8') == checksum
                except (ValueError, TypeError):
                    is_base64 = False
                if not is_base64:
                    problems.append(
                        f"Checksum {checksum} is not base64 encoded for for entry {self.vid_pid_str} software version={software_version}")
                    continue

                response = requests.get(entry[key_ota_url])
                if not response.ok:
                    problems.append(
                        f"Unable to get OTA object from {entry[key_ota_url]} for {self.vid_pid_str} software version = {software_version}")
                    continue

                ota_len = str(len(response.content))
                dcl_len = entry[key_ota_file_size]
                if ota_len != dcl_len:
                    problems.append(
                        f'Incorrect OTA size for {self.vid_pid_str} software_version = {software_version}, received size: {len(response.content)} DCL states {entry[key_ota_file_size]}')
                    continue

                checksum = checksum_types[entry[key_ota_checksum_type]](response.content).digest()
                dcl_checksum = base64.b64decode(entry[key_ota_checksum])
                if checksum != dcl_checksum:
                    problems.append(
                        f'Incorrect checksum for {self.vid_pid_str} software version = {software_version}, calculated: {checksum}, DCL: {dcl_checksum}')

        msg = 'Problems found in software version DCL checks:\n'
        for problem in problems:
            msg += f'{problem}\n'
        asserts.assert_false(problems, msg)


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
            self.config.qr_code_content = [code]
        else:
            self.config.manual_code = [code]
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
    with asyncio.Runner() as runner:
        if not run_tests_no_exit(test_class, matter_config, runner.get_loop(), hooks, controller, stack):
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

        # [] means all tests.
        failures_dcl = run_test(DclCheck, [], test_config)

    report = []
    for test, failure in failures_DA_1_2.items():
        # Check for known failures first
        # step 6.9 - non-production CD
        # 9 - not signed by CSA CA
        # other steps - should have been caught in cert, but we should report none the less
        if failure.step.startswith('6.9'):
            report.append('Device is using a non-production certification declaration')
        elif failure.step.startswith('9'):
            report.append('Device is using a certification declaration that was not signed by the CSA CA')
        else:
            report.append(f'Device attestation failure: TC-DA-1.2: {failure.step}')
        report.append(f'\t{str(failure.exception)}\n')

    for test, failure in failures_DA_1_7.items():
        # Notable failures in DA-1.7:
        # 1.3 - PAI signature does not chain to a PAA in the main net DCL
        if failure.step.startswith('1.3'):
            report.append('Device DAC chain does not chain to a PAA in the main net DCL')
        else:
            report.append(f'Device attestation failure: TC-DA-1.7: {failure.step}')
        report.append(f'\t{str(failure.exception)}\n')

    for test, failure in failures_test_event_trigger.items():
        # only one possible failure here
        report.append('Device has test event triggers enabled in production')
        report.append(f'\t{str(failure.exception)}\n')

    for test, failure in failures_dcl.items():
        if test == 'test_Vendor':
            report.append('Device vendor ID is not present in the DCL')
        elif test == 'test_Model':
            report.append('Device model is not present in the DCL')
        elif test == 'test_Compliance':
            report.append('Device compliance information is not present in the DCL')
        elif test == 'test_CertifiedModel':
            report.append('Device certified model is not present in the DCL')
        elif test == 'test_AllSoftwareVersions':
            report.append('Problems with device software version in the DCL')
        else:
            report.append(f'unknown DCL failure in test {test}: {failure.step}')
        report.append(f'\t{str(failure.exception)}\n')

    print('\n\n\n')
    if report:
        print('TEST FAILED:')
        for s in report:
            print(f'\t{s}')
        return 1
    else:
        print('TEST PASSED!')
        return 0


if __name__ == "__main__":
    sys.exit(main())
