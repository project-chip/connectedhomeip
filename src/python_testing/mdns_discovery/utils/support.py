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

import logging

from mobly import asserts

import matter.clusters as Clusters
from mdns_discovery.mdns_discovery import MdnsDiscovery
from mdns_discovery.utils.asserts import assert_valid_hostname, assert_valid_ipv6_addresses

log = logging.getLogger(__name__)

# Answered browses end early via MdnsDiscovery's discovery-silence monitor, so the
# full timeout is only paid when the subtype is not advertised.
SUBTYPE_BROWSE_TIMEOUT_SEC = 5
TCP_PICS_STR = "MCORE.SC.S.TCP"


async def verify_srv_record(instance_name: str, service_type: str) -> str:
    """Query the SRV record for instance_name/service_type and return its hostname.

    Verifies the record is returned, its instance name matches, and its hostname is valid.
    """
    srv_record = await MdnsDiscovery().get_srv_record(
        service_name=f"{instance_name}.{service_type}",
        service_type=service_type,
        log_output=True,
    )
    asserts.assert_true(srv_record is not None, "SRV record was not returned")
    asserts.assert_equal(srv_record.instance_name, instance_name,
                         "SRV record's instance name must equal the queried instance name.")
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


async def verify_devtype_subtype(instance_name: str, dt_key: str, service_type: str,
                                 timeout_sec: float = SUBTYPE_BROWSE_TIMEOUT_SEC) -> None:
    """Verify the Devtype Subtype (_T<dt>) PTR record's instance name equals instance_name."""
    devtype_subtype = f"_T{int(dt_key)}._sub.{service_type}"
    ptr_instance_name = await get_single_ptr_instance_name(devtype_subtype, timeout_sec=timeout_sec)
    asserts.assert_equal(ptr_instance_name, instance_name,
                         "'Devtype Subtype' PTR record's instance name must equal the service instance name.")


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
