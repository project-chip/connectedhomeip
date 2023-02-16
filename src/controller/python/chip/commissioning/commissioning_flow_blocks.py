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

from chip import ChipDeviceCtrl
from chip import clusters as Clusters
from chip import commissioning, tlv

from . import pase


class CommissioningFlowBlocks:
    def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, credential_provider: commissioning.CredentialProvider, logger: logging.Logger):
        self._devCtrl = devCtrl
        self._logger = logger
        self._credential_provider = credential_provider

    async def arm_failsafe(self, parameter: commissioning.PaseParameters, node_id: int):
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=parameter.failsafe_expiry_length_seconds
        ))
        if response.errorCode != 0:
            raise commissioning.CommissionFailure(repr(response))

    async def operational_credentials_commissioning(self, parameter: commissioning.Parameters, node_id: int):
        self._logger.info("Getting Remote Device Info")
        device_info = (await self._devCtrl.ReadAttribute(node_id, [
            (commissioning.ROOT_ENDPOINT_ID, Clusters.BasicInformation.Attributes.VendorID),
            (commissioning.ROOT_ENDPOINT_ID, Clusters.BasicInformation.Attributes.ProductID)], returnClusterObject=True))[commissioning.ROOT_ENDPOINT_ID][Clusters.BasicInformation]

        self._logger.info("Getting AttestationNonce")
        attestation_nonce = await self._credential_provider.get_attestation_nonce()

        self._logger.info("Getting CSR Nonce")
        csr_nonce = await self._credential_provider.get_csr_nonce()

        self._logger.info("Sending AttestationRequest")
        attestation_elements = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.AttestationRequest(
            attestationNonce=attestation_nonce
        ))

        self._logger.info("Getting CertificateChain - DAC")
        dac = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            certificateType=1
        ))

        self._logger.info("Getting CertificateChain - PAI")
        pai = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            certificateType=2
        ))

        self._logger.info("Getting OpCSRRequest")
        csr = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CSRRequest(
            CSRNonce=csr_nonce
        ))

        self._logger.info("Getting device certificate")
        commissionee_credentials = await self._credential_provider.get_commissionee_credentials(
            commissioning.GetCommissioneeCredentialsRequest(
                dac=dac, pai=pai,
                attestation_nonce=attestation_nonce,
                attestation_elements=attestation_elements.attestationElements,
                attestation_signature=attestation_elements.attestationSignature,
                csr_nonce=csr_nonce,
                csr_elements=csr.NOCSRElements,
                csr_signature=csr.attestationSignature,
                vendor_id=device_info.vendorID,
                product_id=device_info.productID))

        self._logger.info("Adding Trusted Root Certificate")
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.AddTrustedRootCertificate(
            rootCACertificate=commissionee_credentials.rcac
        ))

        self._logger.info("Adding Operational Certificate")
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.AddNOC(
            NOCValue=commissionee_credentials.noc,
            ICACValue=commissionee_credentials.icac,
            IPKValue=commissionee_credentials.ipk,
            caseAdminSubject=commissionee_credentials.case_admin_node,
            adminVendorId=commissionee_credentials.admin_vendor_id
        ))
        if response.statusCode != 0:
            raise commissioning.CommissionFailure(repr(response))

        self._logger.info("Setting fabric label")
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.UpdateFabricLabel(
            label=parameter.fabric_label
        ))
        if response.statusCode != 0:
            raise commissioning.CommissionFailure(repr(response))

        try:
            certificate_data = tlv.TLVReader(commissionee_credentials.noc).get()['Any']

            self._logger.info(f"Parsed NOC TLV: {certificate_data}")

            # TODO: chip.tlv.Reader will remove tag information in List containers. Which is used in NOC TLV
            # Should extract matter-node-id and matter-fabric-id after List TLV is well handled.
        except:
            self._logger.exception("The certificate should be a valid CHIP Certificate, but failed to parse it")

        self._logger.info(f"Device peer id: {commissionee_credentials.fabric_id:016X}:{commissionee_credentials.node_id:016X}")

        return commissionee_credentials.node_id

    async def network_commissioning_thread(self, parameter: commissioning.Parameters, node_id: int):
        if not parameter.thread_credentials:
            raise TypeError("The device requires a Thread network dataset")

        self._logger.info("Adding Thread network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=parameter.thread_credentials))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for adding network: {response.networkingStatus}")

        network_list = (await self._devCtrl.ReadAttribute(nodeid=node_id, attributes=[(commissioning.ROOT_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True))[commissioning.ROOT_ENDPOINT_ID][Clusters.NetworkCommissioning].networks
        network_id = network_list[response.networkIndex].networkID

        self._logger.info("Enabling Thread network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=network_id), interactionTimeoutMs=self._devCtrl.ComputeRoundTripTimeout(node_id, upperLayerProcessingTimeoutMs=30000))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for enabling network: {response.networkingStatus}")

        self._logger.info(f"Thread network commissioning finished")

    async def network_commissioning_wifi(self, parameter: commissioning.Parameters, node_id: int):
        if not parameter.wifi_credentials:
            raise TypeError("The device requires WiFi credentials")

        self._logger.info("Adding WiFi network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=parameter.wifi_credentials.ssid, credentials=parameter.wifi_credentials.passphrase))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for adding network: {response.networkingStatus}")

        network_list = (await self._devCtrl.ReadAttribute(nodeid=node_id, attributes=[(commissioning.ROOT_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True))[commissioning.ROOT_ENDPOINT_ID][Clusters.NetworkCommissioning].networks
        network_id = network_list[response.networkIndex].networkID

        self._logger.info("Enabling WiFi network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=network_id), interactionTimeoutMs=self._devCtrl.ComputeRoundTripTimeout(node_id, upperLayerProcessingTimeoutMs=30000))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatus.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for enabling network: {response.networkingStatus}")

        self._logger.info(f"WiFi network commissioning finished")

    async def network_commissioning(self, parameter: commissioning.Parameters, node_id: int):
        clusters = await self._devCtrl.ReadAttribute(nodeid=node_id, attributes=[(Clusters.Descriptor.Attributes.ServerList)], returnClusterObject=True)
        if Clusters.NetworkCommissioning.id not in clusters[commissioning.ROOT_ENDPOINT_ID][Clusters.Descriptor].serverList:
            self._logger.info(
                f"Network commissioning cluster {commissioning.ROOT_ENDPOINT_ID} is not enabled on this device.")
            return

        network_commissioning_cluster_state = (await self._devCtrl.ReadAttribute(
            nodeid=node_id,
            attributes=[(commissioning.ROOT_ENDPOINT_ID, Clusters.NetworkCommissioning)], returnClusterObject=True))[0][Clusters.NetworkCommissioning]

        if network_commissioning_cluster_state.networks:
            for networks in network_commissioning_cluster_state.networks:
                if networks.connected:
                    self._logger.info(
                        f"Device already connected to {networks.networkID.hex()} skip network commissioning")
                    return

        if parameter.commissionee_info.is_wifi_device:
            if network_commissioning_cluster_state.featureMap != commissioning.NetworkCommissioningFeatureMap.WIFI_NETWORK_FEATURE_MAP:
                raise AssertionError("Device is expected to be a WiFi device")
            return await self.network_commissioning_wifi(parameter=parameter, node_id=node_id)
        elif parameter.commissionee_info.is_thread_device:
            if network_commissioning_cluster_state.featureMap != commissioning.NetworkCommissioningFeatureMap.THREAD_NETWORK_FEATURE_MAP:
                raise AssertionError("Device is expected to be a Thread device")
            return await self.network_commissioning_thread(parameter=parameter, node_id=node_id)

    async def send_regulatory_config(self, parameter: commissioning.Parameters, node_id: int):
        self._logger.info("Sending Regulatory Config")
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
            newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationType(
                parameter.regulatory_config.location_type),
            countryCode=parameter.regulatory_config.country_code
        ))
        if response.errorCode != 0:
            raise commissioning.CommissionFailure(repr(response))

    async def complete_commission(self, node_id: int):
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Commands.CommissioningComplete())
        if response.errorCode != 0:
            raise commissioning.CommissionFailure(repr(response))
