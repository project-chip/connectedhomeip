# MdnsDiscovery — Matter mDNS Service Discovery Tool

The `MdnsDiscovery` class is a utility designed to discover Matter devices on the local network using mDNS over IPv6. It facilitates service discovery for the four core Matter roles: commissioner, commissionable node, operational node, and Thread border router. Internally, it leverages the [**python-zeroconf**](https://github.com/python-zeroconf/python-zeroconf)` library to perform mDNS lookups.

In addition to high-level service queries, the class also provides direct access to low-level DNS records such as PTR, SRV, TXT, A, and AAAA.

## 📁 Folder Structure

```text
mdns_discovery/
├── data_clases/               # Containers for service info and query results
├── enums/                     # Enums for service types and other definitions
├── service_listeners/         # Service listeners used during discovery sessions
├── utils/                     # Utility functions: IPv6 filtering and other utils
├── mdns_async_service_info.py # Supports querying specific mDNS record types
├── mdns_discovery.py          # Main entry point for mDNS discovery operations
```

## 📦 Features

### Service Discovery Methods
> These methods perform targeted discovery for specific Matter service types. Each one browses for mDNS advertisements and then queries the discovered services to retrieve detailed metadata. They return a list of fully resolved service instances, including addresses, ports, TXT records, and more.

- `get_operational_services()`: Discover operational Matter nodes.
- `get_commissioner_services()`: Discover Matter commissioners.
- `get_commissionable_services()`: Discover commissionable Matter devices.
- `get_border_router_services()`: Discover Thread border routers.
- `get_all_services()`: Discover services of any kind on the network.
- `get_all_service_types()`: Discover service types of any kind on the network.
- `get_commissionable_subtypes()`: Discovers Matter commissionable subtypes advertised during an open commissioning window.

### Record Query Methods
> These methods perform direct queries for specific DNS record types (PTR, SRV, TXT, A, AAAA) and return their detailed information.

- `get_ptr_records(service_type)`
- `get_srv_record(service_name)`
- `get_txt_record(service_name)`
- `get_quada_records(hostname)`

## 🧪 Example Usage

Below is a minimal async example that performs operational node discovery and prints each service’s instance name and IP addresses.

```python
import asyncio
from mdns_discovery.mdns_discovery import MdnsDiscovery

async def main():
    discovery = MdnsDiscovery()

    # Discover operational nodes
    services = await discovery.get_operational_services()

    # Print basic info
    for service in services:
        print(f"Instance: {service.instance_name}")
        print(f"Addresses: {service.addresses}")
        print(f"Port: {service.port}")
        print("---")

asyncio.run(main())
```
