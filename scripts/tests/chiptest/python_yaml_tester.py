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
def main(setup_code, yaml_path):
    chip.native.Init()
    chipStack = ChipStack("/tmp/repl-storage.json")
    certificateAuthorityManager = chip.CertificateAuthority.CertificateAuthorityManager(chipStack, chipStack.GetStorageManager())
    certificateAuthorityManager.LoadAuthoritiesFromStorage()

    if (len(certificateAuthorityManager.activeCaList) == 0):
        ca = certificateAuthorityManager.NewCertificateAuthority()
        ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
    elif (len(certificateAuthorityManager.activeCaList[0].adminList) == 0):
        certificateAuthorityManager.activeCaList[0].NewFabricAdmin(vendorId=0xFFF1, fabricId=1)

    caList = certificateAuthorityManager.activeCaList

    devCtrl = caList[0].adminList[0].NewController()
    devCtrl.CommissionWithCode(setup_code, 0x12344321)

    parsed_test = chip.yaml.parser.YamlTestParser(yaml_path)
    parsed_test.execute_tests(devCtrl)
    certificateAuthorityManager.Shutdown()
    chipStack.Shutdown()


if __name__ == '__main__':
    main()
