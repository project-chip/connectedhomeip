import json
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from dataclasses import dataclass, asdict

@staticmethod
def get_operational_service_name(node_id, compressed_fabric_id) -> str:
    """
    Constructs the operational service name for the Device.

    This method generates a service name string based on the Device's node ID and the compressed
    fabric ID, following the operational service naming convention.

    Returns:
        str: A string representing the operational service name.
    """
    service_name = f'{compressed_fabric_id:016X}-{node_id:016X}.{MdnsServiceType.OPERATIONAL.value}'
    return service_name

@staticmethod
def get_operational_subtype(compressed_fabric_id) -> str:
    """
    Constructs the operational service name for the Device.

    This method generates a service name string based on the Device's node ID and the compressed
    fabric ID, following the operational service naming convention.

    Returns:
        str: A string representing the operational service name.
    """
    service_name = f'_I{compressed_fabric_id:016X}._sub.{MdnsServiceType.OPERATIONAL.value}'
    return service_name

class TC_EXA(MatterBaseTest):

    @async_test_body
    async def test_TC_EXA(self):
        print(f":::::\n"*10)

        service_name = get_operational_service_name(self.dut_node_id, self.default_controller.GetCompressedFabricId())
        subtype_name = get_operational_subtype(self.default_controller.GetCompressedFabricId())

        # Instantiate MdnsDiscovery class
        mdns = MdnsDiscovery()
        # discovered_services = await mdns.discover()

        op_si = await mdns.get_operational_service_info(
            service_name=service_name,
            type=subtype_name
        )
        json_string = json.dumps(asdict(op_si), indent=4)
        prefixed_json_string = '\n'.join(":::::" + line for line in json_string.split('\n'))
        print(prefixed_json_string)
        
        
        print(f":::::\n"*10)
        
        # input()


if __name__ == "__main__":
    default_matter_test_main()