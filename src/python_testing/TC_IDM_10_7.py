#
#    Copyright (c) 2025 Project CHIP Authors
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


import logging

from mobly import asserts

import matter.clusters as Clusters
import matter.testing.nfc
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.device_conformance_tests import DeviceConformanceTests
from matter.testing.matter_testing import TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_IDM_10_7(DeviceConformanceTests):
    disable_wildcard_subscription = True

    def desc_TC_IDM_10_7(self) -> str:
        return "[TC-IDM-10.7] Test NFC Limited Data Model [DUT as Commissionee]"

    def steps_TC_IDM_10_7(self) -> list[TestStep]:
        return [
            TestStep(1, "Perform a 'PASE only' session, over NTL", "PASE successful", is_commissioning=False),
            TestStep(2, "Perform a wildcard read on Endpoint 0 over NTL and save the result as limited_data_model.",
                     "Wildcard read successful."),
            TestStep(3, "Limited Data Model : Check clusters presence",
                     "All the mandatory clusters should be present."),
            TestStep(4, "Limited Data Model : Check clusters revisions.",
                     "All the clusters revisions should be up-to-date."),
            TestStep(5, "Limited Data Model : Check clusters conformance.",
                     "All the clusters should be compliant."),
            TestStep(6, "Complete the commissioning",
                     "Commissioning successful."),
            TestStep(7, "Discovery of full Data Model (over the operational channel) with a wildcard read.",
                     "Wildcard read successful."),
            TestStep(8, "Lmited and Full Data Model comparison: For each endpoint, compare the Descriptor Cluster present in the Limited Data Model and full Data Model.",
                     "The Descriptor Clusters should be exactly the same (DeviceTypeList, ServerList, ClientList, PartsList, TagList, EndpointUniqueID)."),
        ]

    def setup_test(self):
        super().setup_test()

    @async_test_body
    async def test_TC_IDM_10_7(self):

        self.wait_for_user_input(prompt_msg="Put the DUT in commissionable mode, bring its NFC interface close to the NFC reader"
                                 " and keep the DUT powered")

        node_id = self.matter_test_config.dut_node_ids[0]

        self.step(1)    # Perform a 'PASE only' session, over NTL

        nfc_reader_index = self.user_params.get("NFC_Reader_index", 0)
        reader = matter.testing.nfc.NFCReader(nfc_reader_index)

        nfc_onboarding_data = reader.read_nfc_tag_data()
        log.info(f"NFC Tag data : '{nfc_onboarding_data}'")
        asserts.assert_true(
            reader.is_onboarding_data(nfc_onboarding_data),
            f"'{nfc_onboarding_data}' is not a valid Matter URI"
        )
        self.matter_test_config.qr_code_content.append(nfc_onboarding_data)

        # Read the NFC onboarding data
        payload = SetupPayload().ParseQrCode(nfc_onboarding_data)
        asserts.assert_true(payload.supports_nfc_commissioning, "Device does not Support NFC Commissioning")

        commissioning_method = self.matter_test_config.in_test_commissioning_method
        asserts.assert_is_not_none(commissioning_method, "in_test_commissioning_method must not be None")
        asserts.assert_true(
            str(commissioning_method).startswith("nfc-"),
            f"Expected in_test_commissioning_method to start with 'nfc-', got: {commissioning_method}"
        )

        log.info("dut_node_ids 0x%X", node_id)
        log.info("nfc_onboarding_data: %s", nfc_onboarding_data)

        self.matter_test_config.commissioning_method = commissioning_method
        nfc_setup_payload = SetupPayload().ParseQrCode(nfc_onboarding_data)

        # Create a new onboarding_data where only the NTL bit (0b10000) is kept in the discovery capabilities bitmask
        ntl_onboarding_data = SetupPayload().GenerateQrCode(
            passcode=nfc_setup_payload.setup_passcode,
            vendorId=nfc_setup_payload.vendor_id,
            productId=nfc_setup_payload.product_id,
            discriminator=nfc_setup_payload.long_discriminator,
            customFlow=nfc_setup_payload.commissioning_flow,
            capabilities=0b10000,
            version=nfc_setup_payload.version
        )
        log.info("ntl_onboarding_data: %s", ntl_onboarding_data)

        # Setup a PASE only session over NTL
        commissionee = await self.default_controller.FindOrEstablishPASESession(
            setupCode=ntl_onboarding_data,
            nodeId=node_id
        )
        if commissionee is None:
            raise RuntimeError("Failed to find or establish PASE session")

        self.step(2)    # Read of Limited Data Model

        wildcard_success = await self._wildcard_read(node_id)
        if not wildcard_success:
            self.fail_current_test("Wildcard read of Limited Data Model failed!")
            return

        limited_data_model = self.endpoints
        log.info("limited_data_model: %s", limited_data_model)
        self.build_spec_xmls()

        self.step(3)    # Limited Data Model : Check clusters presence
        mandatory_clusters_success = self._check_mandatory_clusters_presence()
        if not mandatory_clusters_success:
            self.fail_current_test("Missing mandatory clusters in Limited Data Model")
            return

        self.step(4)    # Limited Data Model : Check clusters revisions
        ignore_in_progress_test_event_only_disallowed_for_certification = self.user_params.get(
            "ignore_in_progress_test_event_only_disallowed_for_certification", False)
        success, problems = self.check_revisions(ignore_in_progress_test_event_only_disallowed_for_certification)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with cluster revision on at least one cluster")

        self.step(5)    # Limited Data Model : Check clusters conformance
        conformance_success = self._check_conformance()
        if not conformance_success:
            self.fail_current_test("Problems with Limited Data Model conformance")
            return

        self.step(6)    # Complete the commissioning (over NTL)
        commissioning_success = await self.commission_ntl_device(payload)
        if not commissioning_success:
            self.fail_current_test("Commissioning over NTL failed")
            return

        self.step(7)    # Discovery of full Data Model
        wildcard_success = await self._wildcard_read(node_id)
        if not wildcard_success:
            self.fail_current_test("Wildcard read of full Data Model failed!")
            return
        full_data_model = self.endpoints
        log.info("full_data_model: %s", full_data_model)

        self.step(8)    # Limited and Full Data Model comparison
        descriptor_compare_success = self._compare_descriptor_clusters(
            limited_data_model=limited_data_model,
            full_data_model=full_data_model
        )
        if not descriptor_compare_success:
            self.fail_current_test("Limited and Full Data Model Descriptor clusters differ")
            return

    def _check_mandatory_clusters_presence(self) -> bool:
        """
        Check that the limited data model contains the mandatory clusters:
        - On Endpoint 0:
            Descriptor
            AccessControl
            BasicInformation
            GeneralCommissioning
            OperationalCredentials
            NetworkCommissioning (only if MCORE.COM.WIRELESS is set)
        - On every endpoint listed in Endpoint 0 Descriptor.PartsList:
            Descriptor

        Returns:
            True if all required clusters are present, False otherwise.
        """
        try:
            if not hasattr(self, "endpoints") or self.endpoints is None:
                log.error("self.endpoints is not available")
                return False

            # Check clusters presence on Endpoint 0
            if 0 not in self.endpoints:
                log.error("Endpoint 0 is missing from self.endpoints")
                return False

            ep0 = self.endpoints[0]

            mandatory_ep0_clusters = [
                Clusters.Descriptor,
                Clusters.AccessControl,
                Clusters.BasicInformation,
                Clusters.GeneralCommissioning,
                Clusters.OperationalCredentials,
            ]

            if self.check_pics("MCORE.COM.WIRELESS"):
                mandatory_ep0_clusters.append(Clusters.NetworkCommissioning)

            for cluster in mandatory_ep0_clusters:
                if cluster not in ep0:
                    log.error("Mandatory cluster missing on Endpoint 0: %s", cluster)
                    return False

            descriptor_cluster = ep0.get(Clusters.Descriptor)
            if descriptor_cluster is None:
                log.error("Descriptor cluster missing on Endpoint 0")
                return False

            parts_list = descriptor_cluster.get(Clusters.Descriptor.Attributes.PartsList, [])
            log.info("Endpoint 0 PartsList: %s", parts_list)

            # Check that every other Endpoints (listed in PartsList of EP0 descriptor) have a Descriptor cluster:
            for endpoint_id in parts_list:
                if endpoint_id == 0:
                    continue

                if endpoint_id not in self.endpoints:
                    log.error("Endpoint %s listed in PartsList is missing from self.endpoints", endpoint_id)
                    return False

                endpoint_clusters = self.endpoints[endpoint_id]
                if Clusters.Descriptor not in endpoint_clusters:
                    log.error("Descriptor cluster missing on Endpoint %s", endpoint_id)
                    return False

            log.info("Mandatory clusters presence check passed")
            return True

        except Exception as e:
            log.exception("Mandatory clusters presence check failed: %s", e)
            return False

    def _check_conformance(self) -> bool:
        """
        Run conformance checks, store problems, and return True on success or False on failure.
        """
        try:
            ignore_in_progress_test_event_only_disallowed_for_certification = self.user_params.get(
                "ignore_in_progress_test_event_only_disallowed_for_certification", True
            )
            allow_provisional_test_event_only_disallowed_for_certification = self.user_params.get(
                "allow_provisional_test_event_only_disallowed_for_certification", False
            )

            success, problems = self.check_conformance(
                ignore_in_progress_test_event_only_disallowed_for_certification,
                self.is_pics_sdk_ci_only,
                allow_provisional_test_event_only_disallowed_for_certification
            )

            self.problems.extend(problems)
            return success

        except Exception as e:
            log.exception("Conformance check failed with exception: %s", e)
            return False

    async def _wildcard_read(self, node_id: int) -> bool:
        """
        Perform a wildcard read on the node and populate:
        - self.endpoints
        - self.endpoints_tlv

        Returns:
            True if the wildcard read succeeded, False otherwise.
        """
        log.info("wildcard_read")

        try:
            wildcard_read = await self.default_controller.Read(node_id, [()])

            # All endpoints in "full object" indexing format
            self.endpoints = wildcard_read.attributes
            log.info("self.endpoints: %s", self.endpoints)

            # All endpoints in raw TLV format
            self.endpoints_tlv = wildcard_read.tlvAttributes
            log.info("self.endpoints_tlv: %s", self.endpoints_tlv)

            return True

        except Exception as e:
            log.exception("Wildcard read failed for node 0x%X: %s", node_id, e)
            return False

    def _compare_descriptor_clusters(self, limited_data_model: dict, full_data_model: dict) -> bool:
        """
        Compare limited_data_model and full_data_model:
        - same number of endpoints
        - same endpoint IDs
        - for each endpoint, Descriptor cluster must be identical

        Returns:
            True if identical, False otherwise.
        """
        try:
            limited_endpoint_ids = set(limited_data_model.keys())
            full_endpoint_ids = set(full_data_model.keys())

            log.info("Limited Data Model endpoints: %s", sorted(limited_endpoint_ids))
            log.info("Full Data Model endpoints: %s", sorted(full_endpoint_ids))

            if len(limited_endpoint_ids) != len(full_endpoint_ids):
                log.error(
                    "Endpoint count mismatch: limited=%d full=%d",
                    len(limited_endpoint_ids), len(full_endpoint_ids)
                )
                return False

            if limited_endpoint_ids != full_endpoint_ids:
                missing_in_full = limited_endpoint_ids - full_endpoint_ids
                missing_in_limited = full_endpoint_ids - limited_endpoint_ids

                if missing_in_full:
                    log.error("Endpoints missing in full_data_model: %s", sorted(missing_in_full))
                if missing_in_limited:
                    log.error("Endpoints missing in limited_data_model: %s", sorted(missing_in_limited))
                return False

            for endpoint_id in sorted(limited_endpoint_ids):

                limited_clusters = limited_data_model.get(endpoint_id, {})
                full_clusters = full_data_model.get(endpoint_id, {})

                if Clusters.Descriptor not in limited_clusters:
                    log.error("Descriptor cluster missing in limited_data_model on endpoint %s", endpoint_id)
                    return False

                if Clusters.Descriptor not in full_clusters:
                    log.error("Descriptor cluster missing in full_data_model on endpoint %s", endpoint_id)
                    return False

                limited_descriptor = limited_clusters[Clusters.Descriptor]
                full_descriptor = full_clusters[Clusters.Descriptor]

                if limited_descriptor != full_descriptor:
                    log.error("Descriptor cluster mismatch on endpoint %s", endpoint_id)
                    log.error("Limited Data Model Descriptor: %s", limited_descriptor)
                    log.error("Full Data Model Descriptor: %s", full_descriptor)
                    return False

            log.info("Limited and Full Data Model Descriptor clusters are identical on all endpoints")
            return True

        except Exception as e:
            log.exception("Descriptor cluster comparison failed: %s", e)
            return False


if __name__ == "__main__":
    default_matter_test_main()
