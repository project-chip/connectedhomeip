from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import DNSRecordType, MdnsDiscovery, MdnsServiceType

class TC_SC_000(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_000(self):
        node_id = self.dut_node_id
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'
        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"
        mdns = MdnsDiscovery()
        
        await mdns.get_service_by_record_type(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            record_type=DNSRecordType.TXT,
            log_output=True
        )


if __name__ == "__main__":
    default_matter_test_main()
