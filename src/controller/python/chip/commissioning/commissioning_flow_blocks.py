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

import base64
import logging

import chip.credentials.cert
import chip.crypto.fabric
from chip import ChipDeviceCtrl
from chip import clusters as Clusters
from chip import commissioning
from cryptography import x509
from cryptography.hazmat.primitives import serialization


class CommissioningFlowBlocks:
    def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, credential_provider: commissioning.CredentialProvider, logger: logging.Logger):
        self._devCtrl = devCtrl
        self._logger = logger
        self._credential_provider = credential_provider

    async def arm_failsafe(self, node_id: int, duration_seconds: int = 180):
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=duration_seconds
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
        try:
            attestation_elements = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.AttestationRequest(
                attestationNonce=attestation_nonce
            ))
        except Exception as ex:
            raise commissioning.CommissionFailure(f"Failed to get AttestationElements: {ex}")

        self._logger.info("Getting CertificateChain - DAC")
        # Failures are exceptions
        try:
            dac = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CertificateChainRequest(
                certificateType=1
            ))
        except Exception as ex:
            raise commissioning.CommissionFailure(f"Failed to get DAC: {ex}")

        self._logger.info("Getting CertificateChain - PAI")
        try:
            pai = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CertificateChainRequest(
                certificateType=2
            ))
        except Exception as ex:
            raise commissioning.CommissionFailure(f"Failed to get PAI: {ex}")

        self._logger.info("Getting OpCSRRequest")
        try:
            csr = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.CSRRequest(
                CSRNonce=csr_nonce
            ))
        except Exception as ex:
            raise commissioning.CommissionFailure(f"Failed to get OpCSRRequest: {ex}")

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
        try:
            response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.AddTrustedRootCertificate(
                rootCACertificate=commissionee_credentials.rcac
            ))
        except Exception as ex:
            raise commissioning.CommissionFailure(f"Failed to add Root Certificate: {ex}")

        try:
            x509_rcac = x509.load_pem_x509_certificate(
                b'''-----BEGIN CERTIFICATE-----\n''' +
                base64.b64encode(chip.credentials.cert.convert_chip_cert_to_x509_cert(commissionee_credentials.rcac)) +
                b'''\n-----END CERTIFICATE-----''')
            root_public_key = x509_rcac.public_key().public_bytes(serialization.Encoding.X962,
                                                                  serialization.PublicFormat.UncompressedPoint)

            x509_noc = x509.load_pem_x509_certificate(
                b'''-----BEGIN CERTIFICATE-----\n''' +
                base64.b64encode(chip.credentials.cert.convert_chip_cert_to_x509_cert(commissionee_credentials.noc)) +
                b'''\n-----END CERTIFICATE-----''')

            for subject in x509_noc.subject:
                if subject.oid.dotted_string == '1.3.6.1.4.1.37244.1.1':
                    cert_fabric_id = int(subject.value, 16)
                elif subject.oid.dotted_string == '1.3.6.1.4.1.37244.1.5':
                    cert_node_id = int(subject.value, 16)

            if cert_fabric_id != commissionee_credentials.fabric_id:
                self._logger.warning("Fabric ID in certificate does not match the fabric id in commissionee credentials struct.")
            if cert_node_id != commissionee_credentials.node_id:
                self._logger.warning("Node ID in certificate does not match the node id in commissionee credentials struct.")

            compressed_fabric_id = chip.crypto.fabric.generate_compressed_fabric_id(root_public_key, cert_fabric_id)

        except Exception:
            self._logger.exception("The certificate should be a valid CHIP Certificate, but failed to parse it")
            raise

        self._logger.info(
            f"Commissioning FabricID: {cert_fabric_id:016X} "
            f"Compressed FabricID: {compressed_fabric_id:016X} "
            f"Node ID: {cert_node_id:016X}")

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

        self._logger.info("Update controller IPK")
        self._devCtrl.SetIpk(commissionee_credentials.ipk)

        self._logger.info("Setting fabric label")
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.OperationalCredentials.Commands.UpdateFabricLabel(
            label=parameter.fabric_label
        ))
        if response.statusCode != 0:
            raise commissioning.CommissionFailure(repr(response))

        return commissionee_credentials.node_id

    async def network_commissioning_thread(self, parameter: commissioning.Parameters, node_id: int):
        if not parameter.thread_credentials:
            raise TypeError("The device requires a Thread network dataset")

        self._logger.info("Adding Thread network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=parameter.thread_credentials))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for adding network: {response.networkingStatus}")

        network_list = (await self._devCtrl.ReadAttribute(nodeid=node_id, attributes=[(commissioning.ROOT_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True))[commissioning.ROOT_ENDPOINT_ID][Clusters.NetworkCommissioning].networks
        network_id = network_list[response.networkIndex].networkID

        self._logger.info("Enabling Thread network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=network_id), interactionTimeoutMs=self._devCtrl.ComputeRoundTripTimeout(node_id, upperLayerProcessingTimeoutMs=30000))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for enabling network: {response.networkingStatus}")

        self._logger.info("Thread network commissioning finished")

    async def network_commissioning_wifi(self, parameter: commissioning.Parameters, node_id: int):
        if not parameter.wifi_credentials:
            raise TypeError("The device requires WiFi credentials")

        self._logger.info("Adding WiFi network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=parameter.wifi_credentials.ssid, credentials=parameter.wifi_credentials.passphrase))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for adding network: {response.networkingStatus}")

        network_list = (await self._devCtrl.ReadAttribute(nodeid=node_id, attributes=[(commissioning.ROOT_ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks)], returnClusterObject=True))[commissioning.ROOT_ENDPOINT_ID][Clusters.NetworkCommissioning].networks
        network_id = network_list[response.networkIndex].networkID

        self._logger.info("Enabling WiFi network")
        response = await self._devCtrl.SendCommand(nodeid=node_id, endpoint=commissioning.ROOT_ENDPOINT_ID, payload=Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=network_id), interactionTimeoutMs=self._devCtrl.ComputeRoundTripTimeout(node_id, upperLayerProcessingTimeoutMs=30000))
        if response.networkingStatus != Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess:
            raise commissioning.CommissionFailure(f"Unexpected result for enabling network: {response.networkingStatus}")

        self._logger.info("WiFi network commissioning finished")

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
            newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum(
                parameter.regulatory_config.location_type),
            countryCode=parameter.regulatory_config.country_code
        ))
        if response.errorCode != 0:
            raise commissioning.CommissionFailure(repr(response))

    async def complete_commission(self, node_id: int):
        response = await self._devCtrl.SendCommand(node_id, commissioning.ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Commands.CommissioningComplete())
        if response.errorCode != 0:
            raise commissioning.CommissionFailure(repr(response))
