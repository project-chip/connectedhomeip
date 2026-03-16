# Commissioning Proxy — Implementation Notes

This document describes the code-driven mechanism used by the Commissioning
Proxy cluster, the changes made to the WiFi-PAF subsystem, and how to
cross-compile the app for Raspberry Pi.

The first commit introducing this work is `70089581bd`.

<hr>

-   [Architecture Overview](#architecture-overview)
    -   [Code-Driven Cluster Mechanism](#code-driven-cluster-mechanism)
    -   [ProxyTransport — Virtual Transport Layer](#proxytransport--virtual-transport-layer)
    -   [WiFi-PAF Integration](#wifi-paf-integration)
    -   [Packet Flow](#packet-flow)
-   [Changed Files — Summary](#changed-files--summary)
-   [Changed Files — Detailed Notes](#changed-files--detailed-notes)
    -   [Cluster Server](#cluster-server)
    -   [Linux Platform Delegate](#linux-platform-delegate)
    -   [ProxyTransport (new)](#proxytransport-new)
    -   [WiFi-PAF Subsystem](#wifi-paf-subsystem)
    -   [Controller / chip-tool](#controller--chip-tool)
    -   [AutoCommissioner](#autocommissioner)
    -   [Generated / ZAP Artefacts](#generated--zap-artefacts)
-   [Cross-Compiling for Raspberry Pi](#cross-compiling-for-raspberry-pi)

<hr>

## Architecture Overview

### Code-Driven Cluster Mechanism

The Commissioning Proxy cluster is implemented using the **code-driven
data-model provider** (`CodeDrivenDataModelProvider`) rather than the
ZAP-generated attribute-storage approach.

In this model:

1.  The application instantiates a
    `CommissioningProxyCluster::Config` (endpoint, feature flags, delegate)
    and wraps it in a `RegisteredServerCluster<CommissioningProxyCluster>`.
2.  During `ApplicationInit()` the cluster is registered with
    `CodegenDataModelProvider::Instance().Registry().Register(...)`.
3.  Attribute reads/writes and command invocations are dispatched directly to
    `CommissioningProxyCluster::ReadAttribute()`,
    `CommissioningProxyCluster::WriteAttribute()`, and
    `CommissioningProxyCluster::InvokeCommand()` — no static ZAP attribute
    storage is used.
4.  `InvokeCommand()` dispatches to per-command handlers that decode the TLV
    payload and call the delegate.  Commands whose responses are produced
    asynchronously return `std::nullopt` to prevent the framework from
    generating a duplicate response.

### ProxyTransport — Virtual Transport Layer

`ProxyTransport` is a new `TransportBase` subclass that represents a virtual
transport in the Matter stack.  Instead of carrying packets over a physical
medium it:

-   **On send**: calls `ProxyTransportDelegate::SendProxyMessage()` which chip-tool
    implements by sending a `ProxyMessageRequest` IM command to the proxy app.
-   **On receive**: `OnProxyMessageReceived()` is called (by chip-tool's
    `CommandSender::OnResponse` handler) with the raw bytes from
    `ProxyMessageResponse`, and injects them into the Matter stack as if they
    arrived directly from the device.

`ProxyTransport` is added to `DeviceTransportMgr` (chip-tool's transport
manager) at compile time as the last transport variant. chip-tool activates it
with the `sessionId` returned by `ProxyConnectResponse` and then calls
`DeviceCommissioner::PairDevice()` with `PeerAddress::Proxy(sessionId)`.

### WiFi-PAF Integration

On the commissioning-proxy-app side, a `ProxyWiFiPAFDelegate` interposes
itself between `WiFiPAFLayer` and the original server-side transport delegate:

-   On `WiFiPAFMessageReceived`: if the `peer_id` matches an active proxy
    session the message is routed to `OnProxyWiFiPAFMessageReceived` which
    sends it back as a `ProxyMessageResponse` IM command.
-   For all other sessions the original delegate is called unchanged, so normal
    (non-proxy) PAF sessions continue to work.

The delegate is installed lazily on the first `ProxyConnectRequest`.

### Packet Flow

```
chip-tool                   commissioning-proxy-app          commissionee
   |                                 |                            |
   |-- CASE session ---------------► |                            |
   |-- ProxyConnectRequest --------► |                            |
   |                                 |-- WiFiPAFSubscribe ------► |
   |                                 |◄- PAF handshake ---------- |
   |◄- ProxyConnectResponse (sid) -- |                            |
   |                                 |                            |
   |-- ProxyMessageRequest(sid,pkt)► |                            |
   |                                 |-- WiFiPAFLayer::Send ----► |
   |                                 |◄- WiFiPAFMessageReceived - |
   |◄- ProxyMessageResponse(sid,pkt) |                            |
   |  (repeat for each PASE/commissioning round-trip)             |
   |                                 |                            |
   |-- ProxyDisconnectRequest -----► |                            |
   |                                 |-- RmPafSession ----------► |
   |◄- Status::Success ------------- |                            |
```

<hr>

## Changed Files — Summary

### Cluster Server (`src/app/clusters/commissioning-proxy-server/`)

| File | Change |
| ---- | ------ |
| `CommissioningProxyCluster.cpp` | New: full code-driven cluster implementation (attribute R/W, command dispatch, async nullopt pattern) |
| `CommissioningProxyCluster.h` | New: `CommissioningProxyCluster` class and `Config` struct |
| `CommissioningProxyDelegate.h` | New: abstract `Delegate` interface that all platform implementations must satisfy |
| `CodegenIntegration.h/.cpp` | New: thin compatibility shim for ZAP-based apps |
| `CommissioningProxyTestEventTriggerHandler.h` | New: test-event trigger handler stub |
| `BUILD.gn` / `*.cmake` / `*.gni` | Updated: build rules to include the new cluster source files |
| `tests/` | New: unit tests for the cluster (mock delegate, backwards-compatibility tests) |
| `README.md` | New: cluster server documentation |

### Linux Platform Delegate (`examples/commissioning-proxy-app/linux/`)

| File | Change |
| ---- | ------ |
| `CommissioningProxyCommandDelegate.cpp` | Replaced: full PAF delegate implementation — session tracking, `ProxyWiFiPAFDelegate`, async connect/message/disconnect flow |
| `CommissioningProxyCommandDelegate.h` | Removed: replaced by `commissioning-proxy-delegate-impl.h` |
| `CPAppCommandDelegate.cpp/.h` | New: named-pipe command handler for the proxy app |
| `main.cpp` | Updated: registers `CommissioningProxyCluster` via `CodegenDataModelProvider` |
| `args.gni` | Updated: enables TCP endpoint (`chip_inet_config_enable_tcp_endpoint`) |
| `BUILD.gn` | Updated: adds `ProxyTransport` and cluster server as build dependencies |
| `include/CHIPProjectAppConfig.h` | Updated: project-level CHIP config overrides |

### Commissioning Proxy Common (`examples/commissioning-proxy-app/commissioning-proxy-common/`)

| File | Change |
| ---- | ------ |
| `include/commissioning-proxy-delegate-impl.h` | New: `MyCPDelegate` class declaration |
| `src/commissioning-proxy-delegate-impl.cpp` | New: `MyCPDelegate` attribute getter/setter implementations |
| `commissioning-proxy.matter` / `.zap` | Updated: cluster descriptor (ZAP source of truth for code generation) |
| `BUILD.gn` | Updated: adds delegate implementation sources |

### ProxyTransport — New Files (`src/transport/raw/`)

| File | Change |
| ---- | ------ |
| `ProxyTransport.h` | New: `ProxyTransportBase`, `ProxyTransportDelegate`, `Proxy<>` template, `ProxyListenParameters` |
| `ProxyTransport.cpp` | New: `Init`, `Activate`, `Deactivate`, `SendMessage`, `OnProxyMessageReceived` |
| `PeerAddress.h` | Updated: adds `Type::kProxy`, `PeerAddress::Proxy(sessionId)`, `GetProxySessionId()` |
| `PeerAddress.cpp` | Updated: `operator==` handles `Type::kProxy`; `ToString` prints `PROXY:<id>` |
| `BUILD.gn` | Updated: adds `ProxyTransport.cpp` to the raw transport build target |

### WiFi-PAF Subsystem (`src/wifipaf/`)

| File | Change |
| ---- | ------ |
| `WiFiPAFConfig.h` | `PAFTP_ACK_TIMEOUT_MS` increased from 15 000 ms to 30 000 ms to prevent premature session closure during WiFi join |
| `WiFiPAFLayer.cpp` | `RmPafSession` now handles `kAccNodeInfo` lookup in addition to `kAccSessionId` |

### Controller / chip-tool (`src/controller/`, `examples/chip-tool/`)

| File | Change |
| ---- | ------ |
| `CHIPDeviceControllerSystemState.h` | Adds `Transport::Proxy<>` to `DeviceTransportMgr`; adds `kDeviceProxyTransportIndex` and `GetDeviceProxyTransport()` helper |
| `CHIPDeviceControllerFactory.cpp` | Passes `ProxyListenParameters` when initialising the transport manager |
| `CHIPDeviceController.h` | Adds `GetTransportMgr()` accessor |
| `examples/chip-tool/commands/pairing/Commands.h` | Adds `PairViaProxy` command using `PairingMode::Proxy` and `PairingNetworkType::WiFi` |
| `examples/chip-tool/commands/pairing/PairingCommand.h` | Adds `PairViaProxy` state (callbacks, session ID, command senders, exchange manager) |
| `examples/chip-tool/commands/pairing/PairingCommand.cpp` | Implements the full proxy commissioning flow: CASE → ProxyConnect → Pair → ProxyMessage ↔ response loop → ProxyDisconnect |

### AutoCommissioner (`src/controller/`)

| File | Change |
| ---- | ------ |
| `AutoCommissioner.cpp` | Detects `Transport::Type::kProxy` and enables `mNeedsNetworkSetup` when WiFi credentials are present, so credential commissioning stages run over the tunnel |

### Linux Platform (`src/platform/Linux/`)

| File | Change |
| ---- | ------ |
| `ConnectivityManagerImpl.cpp` | Adds `ScanDiscoveryResult()` and PAF scanning helpers; adds debug logging to `_WiFiPAFShutdown` |
| `ConnectivityManagerImpl.h` | Declares the new scan helpers |

<hr>

## Changed Files — Detailed Notes

### Cluster Server

#### `CommissioningProxyCluster.cpp` / `.h`

The cluster is implemented as a `DefaultServerCluster` subclass.

-   **`Startup()`** — validates that the delegate's endpoint ID matches the
    registered endpoint and calls the base class.
-   **`ReadAttribute()`** — serves `FeatureMap`, `Transport`, `MaxSessions`,
    and `ScanMaxTime` directly from delegate accessors and feature flags.  All
    other attribute IDs return `Status::UnsupportedAttribute`.
-   **`WriteAttribute()`** — accepts `ScanMaxTime`; all other writes return
    `Status::UnsupportedWrite`.
-   **`InvokeCommand()`** — dispatches to per-command handlers.  `ProxyConnectRequest`,
    `ProxyScanRequest`, and `ProxyMessageRequest` return `std::nullopt` on
    success so the framework does not emit a duplicate IM status after the
    delegate has already queued an asynchronous response.

**`HandleProxyConnectRequest`** validates that exactly one transport bit is
set (`HasExactlyOneBitSet`), checks that `kWiFiPAF` is only selected when the
`kWiFiNetworkInterface` feature is enabled, and returns
`Status::InvalidTransportType` if not.  It then calls the delegate and returns
`std::nullopt`.

**`HandleProxyDisconnectRequest`** decodes the session ID, calls the delegate,
and resets cluster state to `kState_CPDisconnected`.

**`HandleProxyMessageRequest`** decodes the message payload and session ID,
calls the delegate, and returns `std::nullopt`.

#### `CommissioningProxyDelegate.h`

Defines the pure-virtual `Delegate` interface.  Key methods:

| Method | Description |
| ------ | ----------- |
| `ProxyConnectRequest()` | Open a PAF session; must call `commandObj->AddResponse()` asynchronously |
| `ProxyDisconnectRequest()` | Tear down the PAF session identified by `sessionId` |
| `ProxyMessageRequest()` | Forward `message` over PAF; must call `commandObj->AddResponse()` asynchronously |
| `ProxyScanRequest()` | Initiate a NAN scan; must call `commandObj->AddResponse()` asynchronously |
| `GetScanMaxTime()` / `SetScanMaxTime()` | Attribute accessors |

---

### Linux Platform Delegate

#### `CommissioningProxyCommandDelegate.cpp`

This file is the heart of the proxy implementation. It was fully replaced from
an earlier lighting-app stub.

**Session tracking** (`sProxySessions`): a `std::map<uint16_t, ProxySessionInfo>`
maps each proxy session ID (allocated by `AllocSessionId()`) to the
`WiFiPAFSession` used to identify the commissionee on the PAF layer.

**`AllocSessionId()`**: allocates the next non-zero session ID, skipping any
already in use.  Session ID `0` is explicitly skipped as it is reserved to mean
"no session".

**`ProxyConnectCtx` / `sPendingConnectCtx`**: a guard pointer that prevents
use-after-free if the PAF layer fires both `OnPafConnectSuccess` and
`OnPafConnectError` for the same subscribe.  Whichever callback fires first
sets `sPendingConnectCtx = nullptr`; the second callback exits immediately.

**`ProxyWiFiPAFDelegate`**: a `WiFiPAFLayerDelegate` subclass installed on the
first `ProxyConnectRequest`.  It sits in front of the original server transport
delegate:
- `WiFiPAFMessageReceived`: matches `peer_id` against `sProxySessions` and
  routes matching sessions to `OnProxyWiFiPAFMessageReceived`.
- All other methods (`WiFiPAFMessageSend`, `WiFiPAFCloseSession`,
  `WiFiPAFResourceAvailable`) delegate to the original transport unchanged.

**`OnProxyWiFiPAFMessageReceived`**: looks up `sPendingProxyMsgCtx[sessionId]`,
checks the `CommandHandler::Handle` is still live, calls `cmd->AddResponse()`
with a `ProxyMessageResponse`, and deletes the context.

**`ProxyMessageRequest` stale-handle cleanup**: on each new `ProxyMessageRequest`,
if an existing handle for that session has already expired (`Get() == nullptr`),
the stale `ProxyMsgCtx` is deleted and the slot is freed instead of returning
`Status::Busy` forever.

**`ProxyDisconnectRequest`**: looks up and erases the session from
`sProxySessions`, cancels any pending `ProxyMsgCtx` for that session (without
calling `AddStatus` on the expired exchange to avoid a crash), calls
`RmPafSession(kAccSessionId, ...)`, and resets cluster state to
`kState_CPDisconnected`.

**Response timeout extension**: in `HandleProxyMessageRequest` the exchange
timeout is extended to `responseTimeout + 10` seconds so the proxy's IM
exchange stays alive while the device processes a slow command (e.g.
`ConnectNetwork` takes ~15 s on Linux).

---

### ProxyTransport (new)

#### `src/transport/raw/ProxyTransport.h`

Declares:

-   `ProxyTransportDelegate` — interface with a single method
    `SendProxyMessage(sessionId, payload)` that chip-tool implements.
-   `ProxyTransportBase` — `TransportBase` subclass with `Init`, `Activate`,
    `Deactivate`, `Close`, `CanSendToPeer`, `SendMessage`, and
    `OnProxyMessageReceived`.
-   `Proxy<>` — thin template wrapper so `TransportMgr` can hold it in its
    variant list.
-   `ProxyListenParameters` — required by the `TransportBase` `Init` contract;
    carries only a `System::Layer` pointer.

#### `src/transport/raw/PeerAddress.h` / `.cpp`

-   `Type::kProxy` added as a new transport type (before `kLast`).
-   `PeerAddress::Proxy(uint16_t sessionId)` static factory; stores `sessionId`
    in the `mId.mRemoteId` field.
-   `GetProxySessionId()` accessor.
-   `operator==` returns `mId.mRemoteId == other.mId.mRemoteId` for `kProxy`
    (same branch as `kWiFiPAF`).
-   `ToString()` prints `PROXY:<id>`.

---

### WiFi-PAF Subsystem

#### `src/wifipaf/WiFiPAFConfig.h`

`PAFTP_ACK_TIMEOUT_MS` raised from **15 000 ms to 30 000 ms**.

**Reason**: During WiFi commissioning (`ConnectNetwork`), the device's
wpa\_supplicant scan monopolises the radio for ~14.5 s, preventing it from
transmitting the PAFTP ack for the `ConnectNetwork` PAFTP frame.  With the
original 15 s timer, the proxy's PAFTP endpoint fires just after the ack is
sent (T+14.5 s) but before it arrives (T+14.5 s + NAN transit), closing the
session before the `ConnectNetwork` response (T+15.5 s) can be delivered.
30 s gives comfortable margin for the full WiFi join sequence.

This change must be applied to **both** the commissioning-proxy-app (sender)
and the commissionee (chip-lighting-app / receiver) for reliable bidirectional
ack flow.

#### `src/wifipaf/WiFiPAFLayer.cpp`

`RmPafSession()` now handles `PafInfoAccess::kAccNodeInfo` in addition to
`kAccSessionId`, allowing sessions to be looked up and removed by `nodeId`
(needed during PAF connect error paths where only the node info is available).

---

### Controller / chip-tool

#### `src/controller/CHIPDeviceControllerSystemState.h`

`DeviceTransportMgr` (the compile-time transport list in chip-tool) has
`Transport::Proxy<>` appended as the last transport.

`kDeviceProxyTransportIndex` is computed at compile time as the zero-based
index of `Proxy<>` within the `TransportMgr` variant, using a chain of
`constexpr` additions gated on the same `#if` guards used in the type alias.
This must stay in sync with the `DeviceTransportMgr` type alias.

`GetDeviceProxyTransport(DeviceTransportMgr *)` returns the embedded
`ProxyTransportBase` by index.

#### `src/controller/CHIPDeviceControllerFactory.cpp`

`ProxyListenParameters` is passed to `TransportMgr::Init()` alongside the
existing UDP/TCP/BLE/WiFiPAF parameters to initialise the proxy transport slot.

#### `examples/chip-tool/commands/pairing/PairingCommand.cpp`

The `PairViaProxy()` method implements a five-step async flow:

1.  `GetConnectedDevice(mProxyNodeId, ...)` — establishes a CASE session to
    the proxy, using large-payload capability so the TCP transport is preferred.
2.  `OnProxyDeviceConnected` — sends `ProxyConnectRequest` with the
    commissionee discriminator, WiFiPAF transport, and a 30 s timeout.
3.  `OnResponse` (for `ProxyConnectResponse`) — extracts `sessionId`, activates
    the `ProxyTransport`, and calls `Pair(mNodeId, PeerAddress::Proxy(sessionId))`.
4.  `OnResponse` (for `ProxyMessageResponse`) — extracts the raw bytes and
    calls `ProxyTransport::OnProxyMessageReceived()` to inject them into the
    Matter stack.
5.  `SendProxyDisconnect()` — called at `OnCommissioningComplete` (success or
    failure) and at `OnPairingComplete` (on PASE failure); sends
    `ProxyDisconnectRequest` as a fire-and-forget best-effort cleanup.

The `cmdTimeout` for each `ProxyMessageRequest` exchange is set to
`responseTimeout + 10` seconds so the chip-tool exchange stays alive while
the proxy waits for a slow commissionee response.

---

### AutoCommissioner

#### `src/controller/AutoCommissioner.cpp`

When `transportType == Transport::Type::kProxy`, `mNeedsNetworkSetup` is set
based on whether WiFi credentials were supplied.  Without this,
the commissioning framework would skip `AddOrUpdateWiFiNetwork` and
`ConnectNetwork` for the proxy transport (because `kProxy` is not `kWiFiPAF`), and the commissionee would never receive its network credentials.

---

### Generated / ZAP Artefacts

The following files are auto-generated and should not be edited directly.
They are regenerated from `commissioning-proxy.matter` / `.zap` by running
the ZAP code generator.

-   `zzz_generated/app-common/clusters/CommissioningProxy/Commands.h` / `.ipp`
-   `zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.h` / `.cpp`
-   `zzz_generated/app-common/app-common/zap-generated/callback.h`
-   `zzz_generated/chip-tool/zap-generated/cluster/Commands.h`
-   `src/controller/data_model/controller-clusters.matter`
-   `src/controller/java/…`, `src/controller/python/…`, `src/darwin/…`
-   `src/app/zap_cluster_list.json`, `src/app/zap-templates/zcl/…`

<hr>

## Cross-Compiling for Raspberry Pi

Raspberry Pi 4/5 runs a 64-bit Linux kernel (`aarch64`). Builds are done
inside a Docker container that carries the CHIP cross-compilation toolchain
and an Ubuntu 24.04 arm64 sysroot.  A named Docker volume caches the Pigweed
bootstrap environment so subsequent builds skip re-bootstrapping.

### 1. Create the Docker image (one time)

Create a file named `Dockerfile.chip-cross-aarch64` with the following
content and build the image:

```dockerfile
FROM ghcr.io/project-chip/chip-build-crosscompile:latest

# Enable arm64 packages
RUN dpkg --add-architecture arm64 \
 && apt-get update

# Native build tools
RUN apt-get install -y --no-install-recommends \
    git cmake ninja-build pkg-config curl unzip \
    python3 python3-venv python3-dev python3-pip \
    default-jre

# Cross toolchain
RUN apt-get install -y --no-install-recommends \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    libc6-dev-arm64-cross \
    linux-libc-dev-arm64-cross

# arm64 system libraries
RUN apt-get install -y --no-install-recommends \
    libssl-dev:arm64 \
    libdbus-1-dev:arm64 \
    libglib2.0-dev:arm64 \
    libavahi-client-dev:arm64 \
    libgirepository1.0-dev:arm64 \
    libcairo2-dev:arm64 \
    libreadline-dev:arm64 \
 && rm -rf /var/lib/apt/lists/*
```

```
$ docker build -t chip-cross-aarch64 -f Dockerfile.chip-cross-aarch64 .
```

The base image `chip-build-crosscompile` already contains Clang, GN, ninja,
and an Ubuntu 24.04 arm64 sysroot at `/opt/ubuntu-24.04-aarch64-sysroot`.

### 2. Create the Pigweed environment volume (one time)

A Docker volume persists the Pigweed bootstrap across builds so you do not
re-download it every time:

```
$ ENV_VOL="chip-pw-env-$(id -u)"
$ docker volume inspect "$ENV_VOL" >/dev/null 2>&1 || docker volume create "$ENV_VOL"

# Fix ownership so the build can run as your UID/GID
$ docker run --rm \
    -v "$ENV_VOL:/pw_env" \
    chip-cross-aarch64 \
    bash -lc "mkdir -p /pw_env && chown -R $(id -u):$(id -g) /pw_env"
```

### 3. Build the commissioning proxy app

```
$ REPO="$HOME/Work/morse/connectedhomeip"   # adjust to your repo path
$ ENV_VOL="chip-pw-env-$(id -u)"

$ docker run --rm -t \
    --user "$(id -u):$(id -g)" \
    -v "$REPO:/workdir/connectedhomeip" \
    -v "$ENV_VOL:/pw_env" \
    -w /workdir/connectedhomeip \
    chip-cross-aarch64 \
    bash -lc '
      set -eo pipefail
      git config --global --add safe.directory /workdir/connectedhomeip
      git config --global --add safe.directory /workdir/connectedhomeip/third_party/pigweed/repo
      export PW_PROJECT_ROOT=/workdir/connectedhomeip
      export PW_ENVIRONMENT_ROOT=/pw_env
      source scripts/bootstrap.sh
      cd examples/commissioning-proxy-app/linux
      gn gen out/rpi-arm64 --args='"'"'import("//with_pw_rpc.gni") target_os="linux" target_cpu="arm64" is_debug=true is_clang=true sysroot="/opt/ubuntu-24.04-aarch64-sysroot" system_libdir="lib/aarch64-linux-gnu"'"'"'
      ninja -C out/rpi-arm64
    '
```

Output binary:

```
examples/commissioning-proxy-app/linux/out/rpi-arm64/chip-commissioning-proxy-app
```

### 4. Build chip-tool (arm64)

```
$ REPO="$HOME/Work/morse/connectedhomeip"
$ ENV_VOL="chip-pw-env-$(id -u)"

$ docker run --rm -t \
    --user "$(id -u):$(id -g)" \
    -v "$REPO:/workdir/connectedhomeip" \
    -v "$ENV_VOL:/pw_env" \
    -w /workdir/connectedhomeip \
    chip-cross-aarch64 \
    bash -lc '
      set -eo pipefail
      git config --global --add safe.directory /workdir/connectedhomeip
      git config --global --add safe.directory /workdir/connectedhomeip/third_party/pigweed/repo
      export PW_PROJECT_ROOT=/workdir/connectedhomeip
      export PW_ENVIRONMENT_ROOT=/pw_env
      source scripts/bootstrap.sh
      cd examples/chip-tool
      gn gen out/rpi-arm64 --args='"'"'target_os="linux" target_cpu="arm64" is_debug=true is_clang=true sysroot="/opt/ubuntu-24.04-aarch64-sysroot" system_libdir="lib/aarch64-linux-gnu"'"'"'
      ninja -C out/rpi-arm64
    '
```

Output binary:

```
examples/chip-tool/out/rpi-arm64/chip-tool
```

### 5. Transfer and run on the Raspberry Pi

```
$ scp examples/commissioning-proxy-app/linux/out/rpi-arm64/chip-commissioning-proxy-app \
      pi@raspberrypi.local:~/
$ scp examples/chip-tool/out/rpi-arm64/chip-tool \
      pi@raspberrypi.local:~/

# On the Raspberry Pi:
$ sudo ./chip-commissioning-proxy-app --wifi --allow-large-payload
```

### Notes

-   The Raspberry Pi must have Wi-Fi Aware (NAN) hardware and a kernel with NAN
    support (`iw dev` should list NAN support under `Supported interface modes`).
    Most Raspberry Pi 4/5 firmware builds do not enable NAN by default;
    confirm with your BSP vendor.
-   `--allow-large-payload` is required so that chip-tool can connect to the
    proxy app over TCP (needed for oversized commissioning payloads).
-   The `chip-pw-env-<uid>` volume survives container restarts. Delete it
    (`docker volume rm "chip-pw-env-$(id -u)"`) only if you need a clean
    bootstrap (e.g. after a Pigweed toolchain update).
-   To disable RPC support, replace `import("//with_pw_rpc.gni") ` with an
    empty string in the `gn gen --args` for the proxy app.
