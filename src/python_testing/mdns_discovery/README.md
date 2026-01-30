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
├── 📁tests/                      # Unit tests for assert functions and other methods
├── 📁utils/                      # Utility functions: IPv6 filtering and other utils
├── 📄mdns_async_service_info.py  # Supports querying specific mDNS record types
└── 📄mdns_discovery.py           # Main entry point for mDNS discovery operations
```

## 📦 Features

### Service Discovery Methods

These methods perform discovery for Matter and non-Matter services. Each one
browses for mDNS advertisements and then resolves the discovered services to
retrieve complete service information.

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

These methods identify the types of services advertised on the local network,
including both non-Matter services and Matter commissionable services.

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
    print(f"Hostname: {srv_record.hostname}")

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
returns list[AaaaRecord]
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

## 📡 Zeroconf components used by the MdnsDiscovery class

These three **Zeroconf** components provide the core functionalities of the
`MdnsDiscovery` class, from discovering service types to resolving a service’s
full information.

| Component                                     |                                                Function                                                | Result Example                                                                                                                                                                                                                                                                                                                                                                                                                          |
| --------------------------------------------- | :----------------------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **AsyncZeroconfServiceTypes**                 |                      Returns all the advertised service-<br>types on the network.                      | \_matterd.\_udp.local.<br>\_matter.<br>\_tcp.local.<br>\_V65521.\_sub.\_matterd.\_udp.local.<br> \_IM7322C948581262F.\_sub.\_matter.\_tcp.local.                                                                                                                                                                                                                                                                                        |
| **AsyncServiceBrowser**                       | Browse for services of specific service-<br>types, returns PTR info, service-name<br>and service-type. | "`service_type`": "\_matterd.\_udp.local."<br>"`service_name`": "A6666A3E45CF5655.\_matterd.\_udp.local."<br>"`instance_name`": "A6666A3E45CF5655"                                                                                                                                                                                                                                                                                      |
| **async_request**<br>(from ServiceInfo class) |          Returns full-service info by service-<br>name (hostname, txt, addresses, ttl, etc.).          | "`service_name`": "354D34458F15657D.\_matterd.\_udp.local."<br>"`service_type`": "\_V65521.\_sub.\_matterd.\_udp.local."<br>"`instance_name`": "354D34458F15657D"<br>"`hostname`": "00155DD54A04.local."<br>"`port`": 5550<br>"`addresses`": ["172.30.139.182", "fe80::215:5dev:fed5:4a04"]<br>"`txt`": {"VP": "65521+32769}"<br>"`priority`": 0<br>"`interface_index`": 2<br>"`weight`": 0<br>"`host_ttl`": 120<br>"`other_ttl`": 4500 |

## 🌐 Asserts for mDNS Values

This module contains validation helpers used by mDNS discovery–related tests.
Each function raises a `TestFailure` on failure and is designed to make the
exact failing constraint obvious.

You can import them from the following file: `mdns_discovery/utils/asserts.py`

```
from mdns_discovery.utils.asserts import assert_valid_dn_key

assert_valid_dn_key("Kitchen")
```

### ✅ Available assert functions

|                        |                                             |
| ---------------------- | ------------------------------------------- |
| `assert_valid_d_key`   | `assert_valid_commissionable_instance_name` |
| `assert_valid_vp_key`  | `assert_valid_operational_instance_name`    |
| `assert_valid_cm_key`  | `assert_valid_short_discriminator_subtype`  |
| `assert_valid_dt_key`  | `assert_valid_long_discriminator_subtype`   |
| `assert_valid_dn_key`  | `assert_is_commissionable_type`             |
| `assert_valid_ri_key`  | `assert_is_border_router_type`              |
| `assert_valid_ph_key`  | `assert_valid_devtype_subtype`              |
| `assert_valid_pi_key`  | `assert_valid_vendor_subtype`               |
| `assert_valid_jf_key`  | `assert_is_commissioner_type`               |
| `assert_valid_sii_key` | `assert_valid_ipv6_addresses`               |
| `assert_valid_sai_key` | `assert_is_operational_type`                |
| `assert_valid_sat_key` | `assert_valid_product_id`                   |
| `assert_valid_t_key`   | `assert_valid_vendor_id`                    |
| `assert_valid_icd_key` | `assert_valid_hostname`                     |

## 📌 General Information

> For a complete overview of available methods, their functionality, and
> supported parameters, refer to the inline docstrings or inspect the source
> code directly.
