#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Shared helpers for the SC-4.x DNS-SD discovery tests (TC-SC-4.1/4.3/4.6/4.7).

Record validators live in mdns_discovery/utils/asserts.py and raw record I/O in
mdns_discovery/mdns_discovery.py. This module holds the orchestration helpers (a
read plus its assertions) shared across those tests.
"""

import asyncio
import logging
import time

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import (assert_is_commissioner_type, assert_valid_commissionable_instance_name,
                                          assert_valid_dn_key, assert_valid_dt_key, assert_valid_hostname,
                                          assert_valid_ipv6_addresses, assert_valid_vp_key)
from mobly import asserts

import matter.clusters as Clusters

log = logging.getLogger(__name__)

# Answered browses end early via MdnsDiscovery's discovery-silence monitor, so the
# full timeout is only paid when the subtype is not advertised.
SUBTYPE_BROWSE_TIMEOUT_SEC = 5
# Overall deadline for a commissioner service to appear; the browse is retried
# until then so a freshly started advertiser has time to answer queries.
COMMISSIONER_BROWSE_DEADLINE_SEC = 30
TCP_PICS_STR = "MCORE.SC.S.TCP"


async def get_verify_srv_record(instance_name: str, service_type: str) -> str:
    """Query the SRV record for instance_name/service_type, verify it, and return its hostname."""
    # TH performs an SRV record query against the instance name
    srv_record = await MdnsDiscovery().get_srv_record(
        service_name=f"{instance_name}.{service_type}",
        service_type=service_type,
        log_output=True
    )

    # Verify SRV record is returned
    asserts.assert_true(srv_record is not None, "SRV record was not returned")

    # Verify that the SRV record's instance name is equal to the browsed instance name
    asserts.assert_equal(srv_record.instance_name, instance_name,
                         "SRV record's instance name must be equal to the browsed instance name.")

    # Verify that the target hostname is expressed as a twelve or sixteen
    # capital letter hex string
    assert_valid_hostname(srv_record.hostname)

    return srv_record.hostname


async def verify_aaaa_records(hostname: str) -> list[str]:
    """Query AAAA records for hostname and return the resolved IPv6 addresses.

    Verifies at least one record is returned and every address is a valid IPv6 address.
    """
    quada_records = await MdnsDiscovery().get_quada_records(hostname=hostname, log_output=True)
    asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{hostname}'")
    ipv6_addresses = [f"{r.address}%{r.interface}" for r in quada_records]
    assert_valid_ipv6_addresses(ipv6_addresses)
    return ipv6_addresses


async def get_single_ptr_instance_name(subtype: str, must_be_present: bool = True,
                                       timeout_sec: float = SUBTYPE_BROWSE_TIMEOUT_SEC) -> str | None:
    """Query PTR records for subtype and return the single advertised instance name.

    Asserts exactly one record exists. Returns None when must_be_present is False and none exist.
    """
    ptr_records = await MdnsDiscovery().get_ptr_records(
        service_types=[subtype],
        discovery_timeout_sec=timeout_sec,
        log_output=True,
    )
    if must_be_present or ptr_records:
        asserts.assert_equal(len(ptr_records), 1,
                             f"There must only be one ({subtype}) PTR record, found {len(ptr_records)}.")
        return ptr_records[0].instance_name
    return None


async def verify_devtype_subtype(instance_name: str, dt_key: str, service_type: str) -> None:
    """Verify the 'Devtype Subtype' (_T<dt>) PTR record for service_type points at instance_name.

    The 'Devtype Subtype' _T value is the advertiser's device type. The advertiser is
    not commissioned in these tests, so the Descriptor cluster cannot be read; the
    advertised DT TXT key is the only available device type claim to verify the
    subtype against.
    """
    # Construct the 'Devtype Subtype' _T from the advertised device type
    devtype_subtype = f"_T{int(dt_key)}._sub.{service_type}"

    # TH performs a PTR record query against the 'Devtype Subtype'
    ptr_records = await MdnsDiscovery().get_ptr_records(
        service_types=[devtype_subtype],
        discovery_timeout_sec=SUBTYPE_BROWSE_TIMEOUT_SEC,
        log_output=True
    )

    # Verify that there is one, and only one, 'Devtype Subtype' PTR record
    asserts.assert_equal(len(ptr_records), 1,
                         f"There must only be one 'Devtype Subtype' ({devtype_subtype}) PTR record, found {len(ptr_records)}.")

    # Verify that the 'Devtype Subtype' PTR record's instance name is
    # equal to the browsed service instance name
    asserts.assert_equal(ptr_records[0].instance_name, instance_name,
                         "'Devtype Subtype' PTR record's instance name must be equal to the browsed service instance name.")


async def get_verify_commissioner_service():
    """Browse for the 'Commissioner Service' (_matterd._udp), verify it, and return the advertised service."""

    # TH browses for the 'Commissioner Service' (_matterd._udp) through DNS-SD, retrying to absorb the advertiser's startup lag
    deadline = time.monotonic() + COMMISSIONER_BROWSE_DEADLINE_SEC
    while True:
        services = await MdnsDiscovery().get_commissioner_services(log_output=True)
        if services or time.monotonic() >= deadline:
            break
        log.info("No commissioner service discovered yet, retrying browse...")
        await asyncio.sleep(2)

    # Verify that there is one, and only one, commissioner service advertised
    # (the advertiser is expected to be the only commissioner on the test network)
    asserts.assert_equal(len(services), 1,
                         f"There must only be one commissioner service advertised, found {len(services)}.")
    service = services[0]

    # Verify that the DNS-SD instance name is a 64-bit ID expressed as a
    # sixteen-char hex string with capital letters (the rule is shared with
    # the commissionable instance name)
    assert_valid_commissionable_instance_name(service.instance_name)

    # Verify that the service type is '_matterd._udp' and service domain '.local'
    assert_is_commissioner_type(service.service_type)

    return service


async def verify_commissioner_txt_record_keys(instance_name: str) -> str | None:
    """Query the Commissioner Service TXT record for instance_name, verify its keys, and return the DT key, if any."""
    # TH performs a 'Commissioner Service' TXT record query against the instance name
    txt_record = await MdnsDiscovery().get_txt_record(
        service_name=f"{instance_name}.{MdnsServiceType.COMMISSIONER.value}",
        service_type=MdnsServiceType.COMMISSIONER.value,
        log_output=True
    )
    # All commissioner TXT keys are optional, so the record itself may be absent;
    # treat that as an empty key set.
    txt = txt_record.txt if txt_record and txt_record.txt else {}

    # *** VP KEY ***
    # If the 'VP' key is present
    if 'VP' in txt:
        # Verify that it is non-empty
        vp_key = txt['VP']
        asserts.assert_true(vp_key, "'VP' key is present but has no value.")

        # Verify that it contains at least Vendor ID, and if Product ID
        # is present, both values must be separated by a + sign
        assert_valid_vp_key(vp_key)

    # *** DT KEY ***
    # If the 'DT' key is present
    if 'DT' in txt:
        # Verify that it is non-empty
        dt_key = txt['DT']
        asserts.assert_true(dt_key, "'DT' key is present but has no value.")

        # Verify that it contains the device type identifier encoded as a
        # variable length decimal number in ASCII text without leading zeros
        assert_valid_dt_key(dt_key)

    # *** DN KEY ***
    # If the 'DN' key is present
    if 'DN' in txt:
        # Verify that it is non-empty
        dn_key = txt['DN']
        asserts.assert_true(dn_key, "'DN' key is present but has no value.")

        # Verify that it is a valid UTF-8 encoded string of maximum length of 32 bytes
        assert_valid_dn_key(dn_key)

    return txt.get('DT')


class DiscoverySupport:
    """Mixin of DUT reads shared across the SC-4.x discovery tests.

    Intended to be mixed into a MatterBaseTest subclass.
    """

    async def get_descriptor_server_list(self, endpoint: int = 0):
        """Return the Descriptor cluster ServerList for the endpoint."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            dev_ctrl=self.default_controller,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList,
        )

    async def get_icd_feature_map(self, endpoint: int = 0):
        """Return the IcdManagement cluster FeatureMap for the endpoint."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.FeatureMap,
        )

    async def get_active_mode_threshold_ms(self, endpoint: int = 0):
        """Return the IcdManagement cluster ActiveModeThreshold for the endpoint."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.ActiveModeThreshold,
        )

    def get_dut_instance_name(self, log_result: bool = False) -> str:
        """Return the DUT operational instance name (<compressedFabricId>-<nodeId>)."""
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{self.dut_node_id:016X}'
        if log_result:
            log.info("DUT Instance Name: %s", instance_name)
        return instance_name
