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

import logging
import os
import random

from chip import ChipDeviceCtrl
from chip import clusters as Clusters
from chip import commissioning
from chip.commissioning import commissioning_flow_blocks, pase


class ExampleCustomMatterCommissioningFlow(commissioning_flow_blocks.CommissioningFlowBlocks):
    def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, credential_provider: commissioning.CredentialProvider, logger: logging.Logger):
        super().__init__(devCtrl=devCtrl, credential_provider=credential_provider, logger=logger)
        self._logger = logger

    async def commission(self, parameter: commissioning.Parameters):
        # The example uses PASE, however, the blocks uses a node_id, which supports both PASE and CASE.
        with pase.establish_session(devCtrl=self._devCtrl, parameter=parameter.pase_param) as device:
            node_id = device.node_id

            self._logger.info("Sending ArmFailSafe to device")
            await self.arm_failsafe(node_id=node_id, duration_seconds=parameter.failsafe_expiry_length_seconds)

            self._logger.info("Setting Regulatory Configuration")
            await self.send_regulatory_config(parameter=parameter, node_id=node_id)

            self._logger.info("OperationalCredentials Commissioning")
            case_nodeid = await self.operational_credentials_commissioning(parameter=parameter, node_id=node_id)

            if not parameter.commissionee_info.is_ethernet_device:
                self._logger.info("Network Commissioning")
                await self.network_commissioning(parameter=parameter, node_id=node_id)
            else:
                self._logger.info("Device is an ethernet device, network commissioning not required.")

        self._logger.info("Completing Commissioning")
        await self.complete_commission(case_nodeid)

        self._logger.info("Commissioning Completed")


class ExampleCredentialProvider:
    def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceController):
        self._devCtrl = devCtrl

    async def get_attestation_nonce(self) -> bytes:
        return os.urandom(32)

    async def get_csr_nonce(self) -> bytes:
        return os.urandom(32)

    async def get_commissionee_credentials(self, request: commissioning.GetCommissioneeCredentialsRequest) -> commissioning.GetCommissioneeCredentialsResponse:
        node_id = random.randint(100000, 999999)
        nocChain = self._devCtrl.IssueNOCChain(Clusters.OperationalCredentials.Commands.CSRResponse(
            NOCSRElements=request.csr_elements, attestationSignature=request.attestation_signature), nodeId=node_id)
        return commissioning.GetCommissioneeCredentialsResponse(
            rcac=nocChain.rcacBytes,
            noc=nocChain.nocBytes,
            icac=nocChain.icacBytes,
            ipk=nocChain.ipkBytes,
            case_admin_node=self._devCtrl.nodeId,
            admin_vendor_id=self._devCtrl.fabricAdmin.vendorId,
            node_id=node_id,
            fabric_id=self._devCtrl.fabricId)
