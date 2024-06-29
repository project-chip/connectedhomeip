# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin # noqa: F401
import chip.FabricAdmin  # Needed before chip.CertificateAuthority

# isort: on

import chip.CertificateAuthority
import chip.logging
import chip.native
from chip.ChipStack import ChipStack
from chip.yaml.runner import ReplTestRunner
from matter_yamltests.runner import TestRunner


class Runner(TestRunner):
    def __init__(self, repl_storage_path: str, node_id_to_commission: int = None):
        self._repl_runner = None
        self._chip_stack = None
        self._certificate_authority_manager = None
        self._repl_storage_path = repl_storage_path
        self._node_id_to_commission = node_id_to_commission

    async def start(self):
        chip.native.Init()
        chip.logging.RedirectToPythonLogging()
        chip_stack = ChipStack(self._repl_storage_path)
        certificate_authority_manager = chip.CertificateAuthority.CertificateAuthorityManager(
            chip_stack, chip_stack.GetStorageManager())
        certificate_authority_manager.LoadAuthoritiesFromStorage()

        if len(certificate_authority_manager.activeCaList) == 0:
            certificate_authority_manager.NewCertificateAuthority()

        if len(certificate_authority_manager.activeCaList[0].adminList) == 0:
            certificate_authority_manager.activeCaList[0].NewFabricAdmin(
                vendorId=0xFFF1, fabricId=1)

        ca_list = certificate_authority_manager.activeCaList

        dev_ctrl = ca_list[0].adminList[0].NewController()

        # Unfortunately there is no convenient way to confirm if the provided node_id has
        # already been commissioned. At this point we blindly trust that we should commission
        # device with the provided node id.
        if self._node_id_to_commission is not None:
            # Magic value is the defaults expected for YAML tests.
            await dev_ctrl.CommissionWithCode('MT:-24J0AFN00KA0648G00', self._node_id_to_commission)

        self._chip_stack = chip_stack
        self._certificate_authority_manager = certificate_authority_manager
        self._repl_runner = ReplTestRunner(None, certificate_authority_manager, dev_ctrl)

    async def stop(self):
        if self._repl_runner:
            self._repl_runner.shutdown()
        if self._certificate_authority_manager:
            self._certificate_authority_manager.Shutdown()
        if self._chip_stack:
            self._chip_stack.Shutdown()
        self._repl_runner = None
        self._chip_stack = None
        self._certificate_authority_manager = None

    async def execute(self, request):
        return await self._repl_runner.execute(request)
