# MdnsDiscovery — Matter mDNS Service Discovery Tool

The `MdnsDiscovery` class is a utility designed to discover Matter devices on
the local network using mDNS over IPv6. It facilitates service discovery for the
four core Matter roles: commissioner, commissionable node, operational node, and
Thread border router. Internally, it leverages the
[**python-zeroconf**](https://github.com/python-zeroconf/python-zeroconf)
library to perform mDNS lookups.

In addition to high-level service queries, the class also provides direct access
to low-level DNS records such as PTR, SRV, TXT, A, and AAAA.

## 📁 Folder Structure

```text
📁 mdns_discovery/
├── 📁data_clases/                # Containers for service info and query results
├── 📁enums/                      # Enums for service types and other definitions
├── 📁service_listeners/          # Service listeners used during discovery sessions
├── 📁utils/                      # Utility functions: IPv6 filtering and other utils
├── 📄mdns_async_service_info.py  # Supports querying specific mDNS record types
└── 📄mdns_discovery.py           # Main entry point for mDNS discovery operations
```

## 📦 Features

### Service Discovery Methods

These methods perform targeted discovery for specific Matter service types. Each
one browses for mDNS advertisements and then queries the discovered services to
retrieve detailed metadata. They return a list of fully resolved service
instances, including addresses, ports, TXT records, and more.

-   `get_operational_services()`: Discover operational Matter nodes.
-   `get_commissioner_services()`: Discover Matter commissioners.
-   `get_commissionable_services()`: Discover commissionable Matter devices.
-   `get_border_router_services()`: Discover Thread border routers.
-   `get_all_services()`: Discover services of any kind on the network.
-   `discover()`: The mDNS discovery engine powering the discovery methods.

### Record Query Methods

These methods perform direct queries for specific DNS record types (PTR, SRV,
TXT, A, AAAA) for a given service or host and return their detailed information.

-   `get_ptr_records(service_type)`
-   `get_srv_record(service_name)`
-   `get_txt_record(service_name)`
-   `get_quada_records(hostname)`

### Service Type Discovery Methods

These methods are used to identify the types of services being advertised on the
local network, either generically or specific to Matter commissionable devices.

-   `get_all_service_types()`: Discover service types of any kind on the
    network.
-   `get_commissionable_subtypes()`: Discovers Matter commissionable subtypes
    advertised during an open commissioning window.

## 🧪 Example Usage

Below are minimal examples that perform operational node discovery and get a SRV
record respectively.

```python
import asyncio
from mdns_discovery.mdns_discovery import MdnsDiscovery

async def main():
    mdns = MdnsDiscovery()

    # Discover operational nodes
    services = await mdns.get_operational_services()

    # Print basic info
    for service in services:
        print(f"Instance: {service.instance_name}")
        print(f"Addresses: {service.addresses}")
        print(f"Port: {service.port}")
        print("---")

asyncio.run(main())
```

```python
import asyncio
from mdns_discovery.mdns_discovery import MdnsDiscovery

async def main():
    mdns = MdnsDiscovery()
    service_name = 'B7322C948581262F-0000000012344321._matter._tcp.local.'

    # Get SRV record
    srv_record = await mdns.get_srv_record(
        service_name=service_name,
        service_type=MdnsServiceType.OPERATIONAL.value,
        log_output=True
    )

    # Print Hostname
    print(f"Hostname: {srv_record.server}")

asyncio.run(main())
```

## 🧩 Discovery Logic (Method Flow)

### 🔄 Service Discovery Flow Mechanics

The following flow illustrates the internal discovery and resolution process
shared by all the **service discovery methods** including `get_all_services()`.
The method `get_commissionable_services()` is used here as a representative
example.

```text
get_commissionable_services()
        │
        ▼
   discover(query_service=True)
        │
        ├── starts AsyncServiceBrowser(...)
        │       └── invokes _on_service_state_change(...) for each discovered service
        │              └── gathers PTR record info
        │
        └── calls _query_service_info(...) to resolve service info from PTR records
                 ├── adds service listener via add_service_listener(...)
                 ├── sends query using async_request(...)
                 └── stores results in self._discovered_services
        ▼
returns list[MdnsServiceInfo]
```

### 🔄 Record Query Flow Mechanics

The following flows illustrate the internal logic behind the **record query
methods**. These methods query specific DNS record types directly.

#### 🎯 `get_srv_record` and `get_txt_record`

The method `get_srv_record()` is used here as a representative example.

```text
get_srv_record()
        │
        ▼
   _query_service_info(...)
        ├── adds service listener via add_service_listener(...)
        ├── sends query using async_request(...)
        └── returns an MdnsServiceInfo object
        ▼
returns MdnsServiceInfo
```

#### 🎯 `get_quada_records`

```text
get_quada_records()
        │
        ▼
   addr_resolver = AddressResolverIPv6(server=hostname)
        │
        ├── addr_resolver.async_request(...)
        │        └── performs mDNS query for AAAA records
        │
        └── addr_resolver.ip_addresses_by_version(...)
                 └── extracts the resulting IPv6 addresses
        ▼
returns list[QuadaRecord]
```

#### 🎯 `get_ptr_records`

```text
get_ptr_records()
        │
        ▼
   discover(query_service=False)
        │
        └── starts AsyncServiceBrowser(...)
                └── invokes _on_service_state_change(...) for each discovered service
                       ├── gathers PTR record info
                       └── stores results in self._discovered_services
        ▼
returns list[PtrRecord]
```

### 🔄 Service Types Discovery Flow Mechanics

The following flows illustrate the internal logic behind **service type
discovery**. These flows identify available mDNS service types being advertised
on the network, without resolving full service instance details.

#### 🎯 `get_all_service_types`

```text
get_all_service_types()
        │
        ▼
   AsyncZeroconfServiceTypes.async_find(...)
        │
        └── performs a scan for all advertised mDNS service types
        ▼
returns List[str]
```

#### 🎯 `get_commissionable_subtypes`

```text
get_commissionable_subtypes()
        │
        ▼
   get_all_service_types()
        │
        └── discovers all advertised mDNS service types
        ▼
filters for commissionable service subtypes
        ▼
returns List[str]
```

## 📌 General Information

> For a complete overview of available methods, their functionality, and
> supported parameters, refer to the inline docstrings or inspect the source
> code directly.
