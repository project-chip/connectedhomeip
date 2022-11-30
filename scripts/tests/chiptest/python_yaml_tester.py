#
#    Copyright (c) 2022 Project CHIP Authors
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

import click
import tempfile

from chip import ChipDeviceCtrl
from chip.ChipStack import *
import chip.FabricAdmin
import chip.CertificateAuthority
import chip.native
import chip.yaml


@click.command()
@click.option(
    '--setup-code',
    default=None,
    help='setup-code')
@click.option(
    '--yaml-path',
    default=None,
    help='yaml-path')
@click.option(
    '--node-id',
    default=0x12344321,
    help='Node ID to use when commissioning device')
def main(setup_code, yaml_path, node_id):
    # Setting up python enviroment for running YAML CI tests using python parser.
    chip_stack_storage = tempfile.NamedTemporaryFile()
    chip.native.Init()
    chip_stack = ChipStack(chip_stack_storage.name)
    certificate_authority_manager = chip.CertificateAuthority.CertificateAuthorityManager(chip_stack, chipStack.GetStorageManager())
    certificate_authority_manager.LoadAuthoritiesFromStorage()

    if len(certificate_authority_manager.activeCaList) == 0:
        ca = certificate_authority_manager.NewCertificateAuthority()
        ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
    elif len(certificate_authority_manager.activeCaList[0].adminList) == 0:
        certificate_authority_manager.activeCaList[0].NewFabricAdmin(vendorId=0xFFF1, fabricId=1)

    ca_list = certificate_authority_manager.activeCaList

    # Creating and commissioning to a single controller to match what is currently done when
    # running.
    devCtrl = ca_list[0].adminList[0].NewController()
    devCtrl.CommissionWithCode(setup_code, node_id)

    # Parsing and executing test.
    parsed_test = chip.yaml.parser.YamlTestParser(yaml_path)
    parsed_test.execute_tests(devCtrl)

    # Tearing down chip stack. If not done in the correct order test will fail.
    certificate_authority_manager.Shutdown()
    chip_stack.Shutdown()


if __name__ == '__main__':
    main()
