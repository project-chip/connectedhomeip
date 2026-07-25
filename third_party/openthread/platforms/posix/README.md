# OpenThread POSIX platform for CHIP Linux examples

Builds OpenThread's POSIX platform so a CHIP Linux example (for example
`chip-lighting-app`) can act as a real Thread device by driving an 802.15.4 radio
co-processor (RCP) over spinel (for example `spinel+hdlc+uart:///dev/ttyACM0`),
with no `otbr` / `ot-daemon`. Matter service discovery uses OpenThread's SRP
client (`src/platform/OpenThread/OpenThreadDnssdImpl.cpp`), so the device is
reachable over Thread by any commissioner on the mesh.

By default the CHIP Linux embedded-OpenThread build links the **simulation**
platform (a UDP radio). Selecting this platform swaps that for the POSIX RCP
driver. The C++ entry point is shared: `AppMain` calls `otSysInit`, passing either
the simulation node id or, on POSIX, an `otPlatformConfig` carrying the radio URL.

## Selecting the platform

Set the gn arg `chip_openthread_linux_use_posix_rcp=true` (see
`src/platform/Linux/openthread_posix_rcp.gni`). It is only meaningful with the
embedded-OpenThread build (`chip_system_config_use_openthread_inet_endpoints=true`)
and requires a POSIX OpenThread core config
(`openthread_project_core_config_file` pointing at
`.../src/posix/platform/openthread-core-posix-config.h`).

## Running

Pass the RCP radio URL, and optionally an isolated settings directory:

    chip-lighting-app --thread-radio-url spinel+hdlc+uart:///dev/ttyACM0 \
        --thread-data-path /tmp/ot-dev1 --KVS /tmp/dev1.kvs

`--thread-data-path` isolates the OpenThread settings store so multiple RCP
device instances can run on one host.
