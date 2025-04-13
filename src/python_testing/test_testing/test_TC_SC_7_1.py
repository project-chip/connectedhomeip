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
from random import randbytes

import chip.clusters as Clusters
from chip.clusters import Attribute
from chip.testing.matter_testing import MatterTestConfig
from chip.testing.runner import MockTestRunner


def read_trusted_root(filled: bool) -> Attribute.AsyncReadTransaction.ReadResponse:
    opcreds = Clusters.OperationalCredentials
    trusted_roots = [randbytes(400)] if filled else []
    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {0: {opcreds: {opcreds.Attributes.TrustedRootCertificates: trusted_roots}}}
    return resp


def main():
    # All QR and manual codes use vendor ID 0xFFF1, product ID 0x8000.
    qr_2222_20202021 = 'MT:Y.K908OC16750648G00'
    qr_3333_20202021 = 'MT:Y.K900C415W80648G00'
    qr_2222_20202024 = 'MT:Y.K908OC16N71648G00'
    qr_3840_20202021 = 'MT:Y.K90-Q000KA0648G00'
    manual_2222_20202021 = '20054912334'
    manual_3333_20202021 = '31693312339'
    manual_2222_20202024 = '20055212333'

    test_runner = MockTestRunner(Path(__file__).parent / '../TC_SC_7_1.py',
                                 'TC_SC_7_1', 'test_TC_SC_7_1', 0)
    failures = []

    # Tests with no code specified should fail
    test_config = MatterTestConfig()
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with no codes')

    # Tests using discriminators should fail because we need QR or manual codes, no matter the number
    test_config = MatterTestConfig(discriminators=[2222], setup_passcodes=[20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 1 discriminator')

    test_config = MatterTestConfig(discriminators=[2222, 3333], setup_passcodes=[20202021, 20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 2 discriminators')

    # Single qr code or manual without post-cert should cause a failure
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 1 QR code')

    test_config = MatterTestConfig(manual_code=[manual_2222_20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 1 manual code')

    # Two QR or manual codes with post cert marked should fail
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021, qr_3333_20202021],
                                   global_test_params={'post_cert_test': True})
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on post-cert test with 2 QR codes')

    test_config = MatterTestConfig(manual_code=[manual_2222_20202021, manual_3333_20202021],
                                   global_test_params={'post_cert_test': True})
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on post-cert test with 2 manual codes')

    # Incorrectly formatted codes should fail
    test_config = MatterTestConfig(manual_code=[qr_2222_20202021, qr_2222_20202024])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with incorrectly formatted manual codes')

    test_config = MatterTestConfig(qr_code_content=[manual_2222_20202021, manual_2222_20202024])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with incorrectly formatted QR codes')

    # Two codes with the same discriminator should fail
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021, qr_2222_20202024])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 2 QR codes with the same discriminator')

    test_config = MatterTestConfig(manual_code=[manual_2222_20202021, manual_2222_20202024])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on test with 2 manual codes with the same discriminator')

    # Post cert test should fail on default discriminator
    test_config = MatterTestConfig(qr_code_content=[qr_3840_20202021], global_test_params={'post_cert_test': True})
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if ok:
        failures.append('Expected assertion on post-cert test with default code')

    # Test should fail if there is fabric info
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021, qr_3333_20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(True))
    if ok:
        failures.append('Expected assertion on test when fabrics are present')

    # Test should pass on codes with two different discriminators
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021, qr_3333_20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if not ok:
        failures.append('Expected pass on QR code test')

    test_config = MatterTestConfig(manual_code=[manual_2222_20202021, manual_3333_20202021])
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if not ok:
        failures.append('Expected pass on manual code test')

    # Test should pass on post-cert test
    test_config = MatterTestConfig(qr_code_content=[qr_2222_20202021], global_test_params={'post_cert_test': True})
    test_runner.set_test_config(test_config)
    ok = test_runner.run_test_with_mock_read(read_trusted_root(False))
    if not ok:
        failures.append('Expected pass on post-cert test')

    test_runner.Shutdown()
    print(
        f"Test of TC-SC-7.1: test response incorrect: {len(failures)}")
    for f in failures:
        print(f)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
