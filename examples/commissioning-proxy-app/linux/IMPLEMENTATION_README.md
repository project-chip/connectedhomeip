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
    -   [ProxyTransport](#proxytransport)
    -   [WiFi-PAF Subsystem](#wifi-paf-subsystem)
    -   [Server Commissioning Path (shared with End Device)](#server-commissioning-path-shared-with-end-device)
    -   [Network Commissioning (shared with End Device)](#network-commissioning-shared-with-end-device)
    -   [Linux Platform](#linux-platform)
    -   [Controller / chip-tool](#controller--chip-tool)
    -   [AutoCommissioner](#autocommissioner)
    -   [Generated / ZAP artifacts](#generated--zap-artifacts)
-   [Python Certification Tests](#python-certification-tests)
-   [Cross-Compiling for Raspberry Pi](#cross-compiling-for-raspberry-pi)

<hr>

## Architecture Overview

### Code-Driven Cluster Mechanism

The Commissioning Proxy cluster is implemented using the **code-driven
data-model provider** (`CodeDrivenDataModelProvider`) rather than the
ZAP-generated attribute-storage approach.

In this model:

1.  The application instantiates a `CommissioningProxyCluster::Config`
    (endpoint, feature flags, delegate) and wraps it in a
    `RegisteredServerCluster<CommissioningProxyCluster>`.
2.  During `ApplicationInit()` the cluster is registered with
    `CodegenDataModelProvider::Instance().Registry().Register(...)`.
3.  Attribute reads/writes and command invocations are dispatched directly to
    `CommissioningProxyCluster::ReadAttribute()`,
    `CommissioningProxyCluster::WriteAttribute()`, and
    `CommissioningProxyCluster::InvokeCommand()` — no static ZAP attribute
    storage is used.
4.  `InvokeCommand()` dispatches to per-command handlers that decode the TLV
    payload and call the delegate. Commands whose responses are produced
    asynchronously return null to prevent the framework from generating a
    duplicate response.

### ProxyTransport — Virtual Transport Layer

`ProxyTransport` is a new `TransportBase` subclass that represents a virtual
transport in the Matter stack. Instead of carrying packets over a physical
medium it:

-   **On send**: calls `ProxyTransportDelegate::SendProxyMessage()` which
    chip-tool implements by sending a `ProxyMessageRequest` IM command to the
    proxy app.
-   **On receive**: `OnProxyMessageReceived()` is called (by chip-tool's
    `CommandSender::OnResponse` handler) with the raw bytes from
    `ProxyMessageResponse`, and injects them into the Matter stack as if they
    arrived directly from the device.

`ProxyTransport` is added to `DeviceTransportMgr` (chip-tool's transport
manager) at compile time as the last transport variant. chip-tool activates it
with the `sessionId` returned by `ProxyConnectResponse` and then calls
`DeviceCommissioner::PairDevice()` with `PeerAddress::Proxy(sessionId)`.

### WiFi-PAF Integration

On the commissioning-proxy-app side, a `ProxyWiFiPAFDelegate` interposes itself
between `WiFiPAFLayer` and the original server-side transport delegate:

-   On `WiFiPAFMessageReceived`: if the `peer_id` matches an active proxy
    session the message is routed to `OnProxyWiFiPAFMessageReceived` which sends
    it back as a `ProxyMessageResponse` IM command.
-   For all other sessions the original delegate is called unchanged, so normal
    (non-proxy) PAF sessions continue to work.

The delegate is installed lazily on the first `ProxyConnectRequest`.

### Packet Flow, using PAF as an example

```
chip-tool                 commissioning-proxy-app (CP)       commissionee
   |                                 |                            |
   |◄- CP commissioned to Fabric ---►|                            |
   |                                 |                            |
   |-- CASE session ---------------► |                            |
   |-- ProxyScanRequest (Optional)-► |                            |
   |                                 |◄- PAF Publish ------------ |
   |◄- ProxyScanResponse------------ |                            |
   |                                 |                            |
   |-- ProxyConnectRequest --------► |                            |
   |                                 |-- WiFiPAFSubscribe ------► |
   |                                 |◄- PAF handshake ---------- |
   |◄- ProxyConnectResponse (sid) -- |                            |
   |                                 |                            |
   |-- PASE session --------------------------------------------► |
   |-- ProxyMessageRequest(sid,pkt)-►|                            |
   |                                 |-- WiFiPAFLayer::Send ----► |
   |                                 |◄- WiFiPAFMessageReceived - |
   |◄- ProxyMessageResponse(sid,pkt) |                            |
   |  (repeat for each PASE/commissioning round-trip)             |
   |                                 |                            |
   |-- ProxyDisconnectRequest -----► |                            |
   |                                 |-- RmPAFSession ----------► |
   |◄- Status::Success ------------- |                            |
```

<hr>

## Changed Files — Summary

### Cluster Server (`src/app/clusters/commissioning-proxy-server/`)

| File                                          | Change                                                                                                                                                                                                                                                                                                                                                                                                                    |
| --------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `CommissioningProxyCluster.cpp`               | New: full code-driven cluster implementation — attribute R/W, command dispatch, async null pattern, `HandleProxyBackGroundScanStartRequest` / `HandleProxyBackGroundScanStopRequest` handlers, `MaxCachedResults` / `NumCachedResults` / `CacheTimeout` / `CachedResults` / `WiFiBand` / `MaxSessions` attribute reads, `MarkCachedResultsDirty()`; WiFiBand and MaxSessions delegate validation in scan/connect handlers |
| `CommissioningProxyCluster.h`                 | New: `CommissioningProxyCluster` class and `Config` struct                                                                                                                                                                                                                                                                                                                                                                |
| `CommissioningProxyDelegate.h`                | New: abstract `Delegate` interface with `ProxyConnectRequest`, `ProxyDisconnectRequest`, `ProxyMessageRequest`, `ProxyScanRequest`, `ProxyBackgroundScanStartRequest`, `ProxyBackgroundScanStopRequest`, `CancelPendingConnect`, cache attribute accessors, `GetMaxSessions()`, `GetSupportedWiFiBands()`, and `EncodeCachedResults()`                                                                                    |
| `CodegenIntegration.h/.cpp`                   | New: thin compatibility shim for ZAP-based apps                                                                                                                                                                                                                                                                                                                                                                           |
| `CommissioningProxyTestEventTriggerHandler.h` | New: test-event trigger handler stub. Carries a single `kTemplateNotUsed` placeholder keyed off the CommissioningProxy cluster ID 0x0455; no real triggers are defined yet.                                                                                                                                                                                                                                               |
| `BUILD.gn` / `*.cmake` / `*.gni`              | Updated: build rules to include the new cluster source files                                                                                                                                                                                                                                                                                                                                                              |
| `tests/`                                      | New: `TestCommissioningProxyCluster.cpp` (session management, fabric isolation, MaxSessions, scan, timeout), `TestCommissioningProxyClusterBackwardsCompatibility.cpp`, `CommissioningProxyMockDelegate.cpp/.h`                                                                                                                                                                                                           |
| `README.md`                                   | New: cluster server documentation                                                                                                                                                                                                                                                                                                                                                                                         |

### Linux Platform Delegate (`examples/commissioning-proxy-app/linux/`)

| File                                    | Change                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| --------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `CommissioningProxyCommandDelegate.cpp` | Replaced: full PAF delegate implementation — session tracking, `ProxyWiFiPAFDelegate`, async connect/message/disconnect flow, `OnProxyConnectTimeout` (cancels NAN subscribe, returns `Status::Timeout`), `ProxyMessageResponseTimeoutCallback` (hard `responseTimeout` deadline returning `Status::Failure`), `ProxyBackgroundScanStartRequest` / `ProxyBackgroundScanStopRequest` (multi-fabric `sBgScanFabrics` map, per-fabric lifetime timer, NAN discovery cache, band encoding), `ProxyScanRequest` (async `WiFiPAFScan`, Busy guard), MaxSessions enforcement, fabric isolation (NOT_FOUND on cross-fabric access) |
| `CommissioningProxyCommandDelegate.h`   | Removed: replaced by `commissioning-proxy-delegate-impl.h`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `CPAppCommandDelegate.cpp/.h`           | New: named-pipe command handler for the proxy app                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `main.cpp`                              | Updated: registers `CommissioningProxyCluster` via `CodegenDataModelProvider`; cancels the proxy's own NAN publish on commissioning complete via `CancelWiFiPAFPublish()` / `OnChipDeviceEvent()`; derives `WiFiBand` attribute value from `--wifipaf freq_list=` and passes to `MyCPDelegate::SetSupportedWiFiBands()`                                                                                                                                                                                                                                                                                                    |
| `args.gni`                              | Updated: enables TCP endpoint (`chip_inet_config_enable_tcp_endpoint`)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `BUILD.gn`                              | Updated: adds `ProxyTransport` and cluster server as build dependencies                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| `include/CHIPProjectAppConfig.h`        | Updated: project-level CHIP config overrides. Sets `CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY=1` and `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE=1` for shared-radio hardware.                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `entrypoint.sh`                         | Updated: fix `comissioning` typo so the container launches `chip-commissioning-proxy-app` under `--thread`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |

### Commissioning Proxy Common (`examples/commissioning-proxy-app/commissioning-proxy-common/`)

| File                                          | Change                                                                                                                                                             |
| --------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `include/commissioning-proxy-delegate-impl.h` | New: `MyCPDelegate` class declaration — includes `SetSupportedWiFiBands()`, `GetSupportedWiFiBands()`, `GetMaxSessions()`, and private `mSupportedWiFiBands` field |
| `src/commissioning-proxy-delegate-impl.cpp`   | New: `MyCPDelegate` attribute getter/setter implementations                                                                                                        |
| `commissioning-proxy.matter` / `.zap`         | Updated: cluster descriptor (ZAP source of truth for code generation)                                                                                              |
| `BUILD.gn`                                    | Updated: adds delegate implementation sources                                                                                                                      |

### ProxyTransport — New Files (`src/transport/raw/`)

| File                 | Change                                                                                           |
| -------------------- | ------------------------------------------------------------------------------------------------ |
| `ProxyTransport.h`   | New: `ProxyTransportBase`, `ProxyTransportDelegate`, `Proxy<>` template, `ProxyListenParameters` |
| `ProxyTransport.cpp` | New: `Init`, `Activate`, `Deactivate`, `SendMessage`, `OnProxyMessageReceived`                   |
| `PeerAddress.h`      | Updated: adds `Type::kProxy`, `PeerAddress::Proxy(sessionId)`, `GetProxySessionId()`             |
| `PeerAddress.cpp`    | Updated: `operator==` handles `Type::kProxy`; `ToString` prints `PROXY:<id>`                     |
| `BUILD.gn`           | Updated: adds `ProxyTransport.cpp` to the raw transport build target                             |

### WiFi-PAF Subsystem (`src/wifipaf/`)

| File                                    | Change                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `WiFiPAFConfig.h`                       | `PAFTP_ACK_TIMEOUT_MS` increased from 15 000 ms to 30 000 ms to prevent premature session closure during WiFi join                                                                                                                                                                                                                                                                                                                                                                   |
| `WiFiPAFLayer.cpp`                      | `RmPafSession` handles `kAccNodeInfo` lookup in addition to `kAccSessionId`; `CloseEndPoint()` forcibly closes a PAFTP endpoint by session; `ScheduleCancelPublishersOnTxIdle()` / `CancelAllPublisherSessions()` implement deferred NAN publisher teardown after WiFi connect; tx-idle hook now also fires an optional `OnTxIdleActionFunct` after publisher cancel so callers can defer follow-up work (e.g. posting `kOperationalNetworkEnabled`) until PAFTP traffic has drained |
| `WiFiPAFLayer.h`                        | Declares `CloseEndPoint()`, `CancelAllPublisherSessions()`, `ScheduleCancelPublishersOnTxIdle(cancelCb, afterCb, afterCtx)` with the new `OnTxIdleActionFunct` and optional `afterCb`/`afterCtx`; private fields `mCancelPublishersOnTxIdle`, `mCancelPublishersCallback`, `mOnTxIdleAfterCb`, `mOnTxIdleAfterCtx`                                                                                                                                                          |
| `WiFiPAFTP.cpp` / `WiFiPAFEndPoint.cpp` | Sequence-number progress logging for PAFTP fragment debugging                                                                                                                                                                                                                                                                                                                                                 |

### Server Commissioning Path (`src/app/server/`, `src/platform/`)

| File                                    | Change                                                                                                                                                                                                                                                                                                                                                                                                           |
| --------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `CommissioningWindowManager.cpp` / `.h` | `StopAdvertisement()` gains an `aKeepPAFPublish` parameter; `OnSessionEstablished()` now passes `aKeepPAFPublish = true` so the WiFi-PAF publisher carrying the post-PASE data path is not torn down at PASE completion. Publisher is still canceled on shutdown, RevokeCommissioning, and fail-safe expiry (regression fix vs. earlier early-cancel behavior).                                                  |
| `DeviceControlServer.cpp`               | `PostConnectedToOperationalNetworkEvent()` no longer arms a blind 5 s timer to defer mDNS; instead, when a PAF session is active, it hooks `WiFiPAFLayer::ScheduleCancelPublishersOnTxIdle()` with a callback (`OnPafTxIdle`) that posts `kOperationalNetworkEnabled` once the PAFTP send queue has drained and acks have been received. A 5 s `OnPostOpEventWatchdog` timer guards against a silent PAFTP peer. |

### Network Commissioning (`src/app/clusters/network-commissioning/`)

| File                                                         | Change                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| ------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `NetworkCommissioningCluster.cpp`                            | Adds an early-response path for ConnectNetwork over WiFi-PAF, gated on `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE`. `HandleConnectNetwork` detects the PAF transport, sends `ConnectNetworkResponse(Success)` while NAN is still primary, then defers the blocking `ConnectWiFiNetworkAsync` D-Bus call via `PlatformMgr().ScheduleWork(StartWiFiConnectAfterPafAck, ...)` so the IM-queued response flushes through PAFTP first. `OnResult()` finalizes the post-success path; failure recovery is via fail-safe expiry. |
| `NetworkCommissioningCluster.h`                              | Adds `SendEarlyConnectNetworkResponseForWiFiPAF()` and static `StartWiFiConnectAfterPafAck()`. Repurposes `mConnectNetworkResponseSentEarly` to cover both the existing non-concurrent path and the new PAF-concurrent early-response path.                                                                                                                                                                                                                                                                                               |
| `src/include/platform/CHIPDeviceConfig.h`                    | Adds `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE` (default `0`). Set to `1` for shared-radio devices where post-association NAN TX is unreliable. Only active when both `CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF` and `CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION` are `1`.                                                                                                                                                                                                                                               |
| `examples/lighting-app/linux/include/CHIPProjectAppConfig.h` | Sets `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE 1` (lighting-app End Device is the shared-radio target).                                                                                                                                                                                                                                                                                                                                                                                                                  |

### Controller / chip-tool (`src/controller/`, `examples/chip-tool/`)

| File                                                     | Change                                                                                                                      |
| -------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| `CHIPDeviceControllerSystemState.h`                      | Adds `Transport::Proxy<>` to `DeviceTransportMgr`; adds `kDeviceProxyTransportIndex` and `GetDeviceProxyTransport()` helper |
| `CHIPDeviceControllerFactory.cpp`                        | Passes `ProxyListenParameters` when initializing the transport manager                                                      |
| `CHIPDeviceController.h`                                 | Adds `GetTransportMgr()` accessor                                                                                           |
| `examples/chip-tool/commands/pairing/Commands.h`         | Adds `PairViaProxy` command using `PairingMode::Proxy` and `PairingNetworkType::WiFi`                                       |
| `examples/chip-tool/commands/pairing/PairingCommand.h`   | Adds `PairViaProxy` state (callbacks, session ID, command senders, exchange manager)                                        |
| `examples/chip-tool/commands/pairing/PairingCommand.cpp` | Implements the full proxy commissioning flow: CASE → ProxyConnect → Pair → ProxyMessage ↔ response loop → ProxyDisconnect  |

### AutoCommissioner (`src/controller/`)

| File                   | Change                                                                                                                                                       |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `AutoCommissioner.cpp` | Detects `Transport::Type::kProxy` and enables `mNeedsNetworkSetup` when WiFi credentials are present, so credential commissioning stages run over the tunnel |

### Linux Platform (`src/platform/Linux/`)

| File                          | Change                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ConnectivityManagerImpl.cpp` | `ScanDiscoveryResult()` and PAF scanning helpers; debug logging in `_WiFiPAFShutdown`; `WiFiPAFDisconnectPublishReceiveHandler()` tears down the GLib `nanreceive` handler; `WiFiPAFStartBackgroundScan()` / `WiFiPAFStopBackgroundScan()` for background NAN scanning; `WiFiPAFScan()` for foreground one-shot scanning (TTL = scanMaxTime); `GetPendingConnectSubscribeId()` accessor; `PostNetworkConnect()` calls `ScheduleCancelPublishersOnTxIdle()` to defer NAN publisher teardown until PAFTP tx-idle |
| `ConnectivityManagerImpl.h`   | Declares `WiFiPAFDisconnectPublishReceiveHandler()`, `BgScanDiscoveryCallback` , `WiFiPAFStartBackgroundScan()`, `WiFiPAFStopBackgroundScan()`, `WiFiPAFScan()`, `PafScanResultsCallback` , `GetPendingConnectSubscribeId()`; `mScanFreq` field; `NanPeerInfo` struct extended with `band`, `vid`, `pid` fields                                                                                                                                                                                                |

<hr>

## Changed Files — Detailed Notes

### Cluster Server

#### `CommissioningProxyCluster.cpp` / `.h`

The cluster is implemented as a `DefaultServerCluster` subclass.

-   **`Startup()`** — validates that the delegate's endpoint ID matches the
    registered endpoint and calls the base class.
-   **`ReadAttribute()`** — serves `FeatureMap`, `Transport`, `MaxSessions`,
    `ScanMaxTime`, `MaxCachedResults`, `NumCachedResults`, `CacheTimeout`, and
    `CachedResults` directly from delegate accessors and feature flags. All
    other attribute IDs return `Status::UnsupportedAttribute`.
-   **`WriteAttribute()`** — accepts `ScanMaxTime`; all other writes return
    `Status::UnsupportedWrite`.
-   **`InvokeCommand()`** — dispatches to per-command handlers.
    `ProxyConnectRequest`, `ProxyScanRequest`, and `ProxyMessageRequest` return
    null on success so the framework does not emit a duplicate IM status after
    the delegate has already queued an asynchronous response.

**`HandleProxyConnectRequest`** validates that exactly one transport bit is set
(`HasExactlyOneBitSet`), checks that `kWiFiPAF` is only selected when the
`kWiFiNetworkInterface` feature is enabled, and returns
`Status::InvalidTransportType` if not. It then calls the delegate and returns
null.

**`HandleProxyDisconnectRequest`** decodes the session ID, calls the delegate,
and resets cluster state to `kState_CPDisconnected`.

**`HandleProxyMessageRequest`** decodes the message payload and session ID,
calls the delegate, and returns null.

**`HandleProxyBackGroundScanStartRequest`** decodes `transport`, `timeout`, and
`wiFiBands` fields, attaches the caller's `fabricIndex` and `nodeId` from the
`InvokeRequest`, and calls the delegate synchronously. Returns `Status::Success`
immediately (background scan results are delivered via attribute subscription to
`CachedResults`).

**`HandleProxyBackGroundScanStopRequest`** decodes the same fields and
delegates. Per spec §10.5.7.7 the delegate silently ignores calls from a
different fabric/node than the original requester.

**`MarkCachedResultsDirty()`** marks the `CachedResults` and `NumCachedResults`
attributes dirty so that any active subscriptions receive an updated report when
new NAN discovery results arrive.

#### `CommissioningProxyDelegate.h`

Defines the pure-virtual `Delegate` interface. Key methods:

| Method                                            | Description                                                                                                                                                                    |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `ProxyConnectRequest()`                           | Open a PAF session; must call `commandObj->AddResponse()` asynchronously                                                                                                       |
| `ProxyDisconnectRequest()`                        | Tear down the PAF session identified by `sessionId`                                                                                                                            |
| `ProxyMessageRequest()`                           | Forward `message` over PAF; must call `commandObj->AddResponse()` asynchronously                                                                                               |
| `ProxyScanRequest()`                              | Initiate a NAN scan; must call `commandObj->AddResponse()` asynchronously                                                                                                      |
| `ProxyBackgroundScanStartRequest()`               | Start a continuous background NAN scan; results cached and served as a cluster attribute                                                                                       |
| `ProxyBackgroundScanStopRequest()`                | Stop the background scan for the requesting fabric/node; silently ignores calls from a non-matching fabric/node                                                                |
| `CancelPendingConnect()`                          | Cancel any in-progress `ProxyConnectRequest` (used for SessionId `0xFFFF`); enforces fabric isolation                                                                          |
| `GetScanMaxTime()` / `SetScanMaxTime()`           | Attribute accessors                                                                                                                                                            |
| `GetMaxCachedResults()` / `GetNumCachedResults()` | Read-only cache size/count attribute accessors                                                                                                                                 |
| `GetCacheTimeout()` / `SetCacheTimeout()`         | Cache expiry timeout attribute accessors                                                                                                                                       |
| `GetMaxSessions()`                                | Returns the maximum number of simultaneous proxy sessions supported; used by the cluster server for both the `MaxSessions` attribute read and `RESOURCE_EXHAUSTED` enforcement |
| `GetSupportedWiFiBands()`                         | Returns the `WiFiBandBitmap` of bands supported by the hardware; used for the `WiFiBand` attribute read and to validate WiFiBands fields in scan and connect requests          |
| `EncodeCachedResults()`                           | Encodes the `CachedResults` list attribute into a TLV `AttributeValueEncoder`                                                                                                  |

---

### Linux Platform Delegate

#### `CommissioningProxyCommandDelegate.cpp`

This file is the heart of the proxy implementation. It was fully replaced from
an earlier lighting-app stub.

**Session tracking** (`sProxySessions`): a
`std::map<uint16_t, ProxySessionInfo>` maps each proxy session ID (allocated by
`AllocSessionId()`) to the `WiFiPAFSession` used to identify the commissionee on
the PAF layer.

**`AllocSessionId()`**: allocates the next non-zero session ID, skipping any
already in use. Session ID `0` is explicitly skipped as it is reserved to mean
"no session".

**`ProxyConnectCtx` / `sPendingConnectCtx`**: a guard pointer that prevents
use-after-free if the PAF layer fires both `OnPafConnectSuccess` and
`OnPafConnectError` for the same subscribe. Whichever callback fires first sets
`sPendingConnectCtx = nullptr`; the second callback exits immediately.

**`ProxyWiFiPAFDelegate`**: a `WiFiPAFLayerDelegate` subclass installed on the
first `ProxyConnectRequest`. It sits in front of the original server transport
delegate:

-   `WiFiPAFMessageReceived`: matches `peer_id` against `sProxySessions` and
    routes matching sessions to `OnProxyWiFiPAFMessageReceived`.
-   All other methods (`WiFiPAFMessageSend`, `WiFiPAFCloseSession`,
    `WiFiPAFResourceAvailable`) delegate to the original transport unchanged.

**`OnProxyWiFiPAFMessageReceived`**: looks up `sPendingProxyMsgCtx[sessionId]`,
checks the `CommandHandler::Handle` is still live, calls `cmd->AddResponse()`
with a `ProxyMessageResponse`, and deletes the context.

**`ProxyMessageRequest` stale-handle cleanup**: on each new
`ProxyMessageRequest`, if an existing handle for that session has already
expired (`Get() == nullptr`), the stale `ProxyMsgCtx` is deleted and the slot is
freed instead of returning `Status::Busy` forever.

**`ProxyDisconnectRequest`**: looks up and erases the session from
`sProxySessions`, cancels any pending `ProxyMsgCtx` for that session (without
calling `AddStatus` on the expired exchange to avoid a crash), calls
`RmPafSession(kAccSessionId, ...)`, and resets cluster state to
`kState_CPDisconnected`.

**Response timeout extension**: in `HandleProxyMessageRequest` the exchange
timeout is extended to `responseTimeout + 10` seconds so the proxy's IM exchange
stays alive while the device processes a slow command (e.g. `ConnectNetwork`
takes ~15 s on Linux).

**`ProxyMessageRequest` hard response timer**
(`ProxyMessageResponseTimeoutCallback`): in addition to the exchange-timeout
extension above, `HandleProxyMessageRequest` starts a per-session
`System::Timer` for `responseTimeout` seconds keyed by `sessionId`. If the timer
fires before the End Device replies, the proxy:

1.  Removes the pending `ProxyMsgCtx` from `sPendingProxyMsgCtx`.
2.  Calls `cmd->AddStatus(path, Status::Failure)` so the commissioner gets a
    deterministic failure rather than waiting for `PAFTP_ACK_TIMEOUT_MS`.

The timer is canceled (via `CancelProxyMessageResponseTimer(sessionId)`) on the
success path (`OnProxyWiFiPAFMessageReceived`), on stale-ctx cleanup at the
start of the next `ProxyMessageRequest`, on a PAF close
(`ProxyWiFiPAFDelegate::WiFiPAFCloseSession`), and on `ProxyDisconnectRequest`.

**Reason**: the End Device's periodic PAFTP standalone-acks reset the
ack-receive timer (`PAFTP_ACK_TIMEOUT_MS = 30 s`) and could keep the PAFTP
endpoint alive longer than the commissioner's interaction timeout, causing
TC_COMPRO_2_6 step 11 to hang. The hard timer guarantees the commissioner always
gets a response within the spec-defined `responseTimeout` window.

**`ProxyConnectRequest` timeout**: a per-connect `OnProxyConnectTimeout` timer
is started at the beginning of `HandleProxyConnectRequest` using the `timeout`
field from the request (defaulting to 30 s if zero). On expiry the timer:

1.  Calls `WiFiPAFCancelIncompleteSubscribe()` to abort any in-progress NAN
    subscribe handshake.
2.  Calls `WiFiPAFCancelSubscribe(subscribeId)` if a subscribe ID was already
    allocated.
3.  Calls `RmPafSession(kAccNodeInfo, ...)` to clean up the session entry.
4.  Returns `Status::Timeout` to the commissioner via `cmd->AddStatus()`.

The subscribe ID is captured into `ProxyConnectCtx::subscribeId` after
`WiFiPAFSubscribe` returns so the timeout handler can cancel the right session.

**`ProxyBackgroundScanStartRequest`**: maintains a multi-fabric map
`sBgScanFabrics` keyed by `FabricKey{fabricIndex, nodeId}`. Each entry holds a
`FabricScanRecord{transport, wiFiBands, lifetimeCtx}`. Behavior:

-   If the hardware scan is not yet running it starts
    `WiFiPAFStartBackgroundScan(OnBgScanDiscovery)`. If a scan is already
    running for a different fabric the new fabric is added to the map and shares
    the running scan.
-   If `timeout > 0` a per-fabric lifetime timer (`OnBgScanLifetimeExpired`) is
    started; on expiry the fabric is removed from the map and the hardware scan
    is stopped if no other fabrics remain.
-   When `OnBgScanDiscovery` fires, the `NanPeerInfo` result is appended to
    `sCachedResults` (by `peer_id`). The WiFi band is derived from the scan
    frequency (`FreqToBand()`). `MarkCachedResultsDirty()` is called to push an
    attribute-report update to subscribed commissioners.

**`ProxyBackgroundScanStopRequest`**: erases the requesting fabric from
`sBgScanFabrics`; cancels its lifetime timer; stops the hardware scan if the map
is now empty. Silently succeeds if the fabric key is not found (per spec).

**`ProxyScanRequest`** (foreground one-shot scan): calls
`WiFiPAFScan(scanMaxTime, OnPafScanDone, ctx)` where `scanMaxTime` is the value
of the proxy's `ScanMaxTime` attribute. The subscribe TTL passed to
wpa_supplicant equals `scanMaxTime` so the NAN subscribe expires naturally at
the right time. A boolean guard `sScanInProgress` returns `Status::Busy` if a
foreground scan is already running. The `CommandHandler::Handle` is kept live
until `OnPafScanDone` fires (called from the platform layer on completion or
timeout), at which point `ProxyScanResponse` is built from the accumulated
`NanPeerInfo` results and returned.

**Fabric isolation**: each `ProxyConnectRequest` stores
`request.subjectDescriptor.fabricIndex` into both `ProxyConnectCtx::fabricIndex`
and (on success) `ProxySessionInfo::fabricIndex`. Subsequent commands enforce
this ownership:

-   `ProxyMessageRequest` — returns `NOT_FOUND` if
    `request.subjectDescriptor.fabricIndex != session.fabricIndex`.
-   `ProxyDisconnectRequest` — returns `NOT_FOUND` on fabric mismatch.
-   `CancelPendingConnect` (SessionId `0xFFFF`) — returns `NOT_FOUND` if the
    requesting fabric does not match `sPendingConnectCtx->fabricIndex`.

**MaxSessions enforcement**: at the start of `HandleProxyConnectRequest`, if
`sProxySessions.size() >= GetMaxSessions()` (currently `kMaxProxySessions = 1`)
the command returns `Status::ResourceExhausted` without touching the transport
layer.

#### `main.cpp`

**Proxy NAN publish self-cancel**: after `ApplicationInit()`, if the proxy
already holds a fabric (i.e. it was previously commissioned and restarted)
`CancelWiFiPAFPublish()` is called immediately to cancel the proxy's own NAN
publisher and disconnect the `nanreceive` GLib signal handler. If the proxy is
not yet on a fabric, an `OnChipDeviceEvent` handler is registered to call
`CancelWiFiPAFPublish()` on `kCommissioningComplete`.

**Reason**: once the proxy is reachable by chip-tool over TCP/IP, advertising
itself as a NAN publisher serves no purpose and can confuse a subscribing
`_WiFiPAFSubscribe` call (which would register a second `nanreceive` handler on
top of the first, delivering PAF packets twice).

**WiFiBand attribute derivation**: after `ApplicationInit()`, `main.cpp` parses
the `freq_list=` value from `LinuxDeviceOptions::GetInstance().mWiFiPAFExtCmds`
(supplied via `--wifipaf freq_list=...`), maps each frequency to a band
(2412–2484 MHz → `k2g4`, 5035–5980 MHz → `k5g`), and calls
`gMyCPDelegate.SetSupportedWiFiBands(bands)`. The resulting bit mask is served
as the `WiFiBand` attribute and used to validate `WiFiBands` fields in
`ProxyScanRequest` and `ProxyConnectRequest`.

---

### ProxyTransport

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
wpa_supplicant scan monopolizes the radio for ~14.5 s, preventing it from
transmitting the PAFTP ack for the `ConnectNetwork` PAFTP frame. With the
original 15 s timer, the proxy's PAFTP endpoint fires just after the ack is sent
(T+14.5 s) but before it arrives (T+14.5 s + NAN transit), closing the session
before the `ConnectNetwork` response (T+15.5 s) can be delivered. 30 s gives
comfortable margin for the full WiFi join sequence.

This change must be applied to **both** the commissioning-proxy-app (sender) and
the commissionee (chip-lighting-app / receiver) for reliable bidirectional ack
flow.

#### `src/wifipaf/WiFiPAFLayer.cpp`

`RmPafSession()` now handles `PafInfoAccess::kAccNodeInfo` in addition to
`kAccSessionId`, allowing sessions to be looked up and removed by `nodeId`
(needed during PAF connect error paths where only the node info is available).

**`CloseEndPoint(WiFiPAFSession &)`**
(`CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY`): finds the `WiFiPAFEndPoint`
matching the given session by looking it up in `sWiFiPAFEndPointPool` and calls
`DoClose(kWiFiPAFCloseFlag_AbortTransmission, WIFIPAF_ERROR_APP_CLOSED_CONNECTION)`.
Used during `ProxyDisconnectRequest` to cleanly tear down the PAFTP stack before
removing the session entry.

**`ScheduleCancelPublishersOnTxIdle(cancelCb, afterCb = nullptr, afterCtx = nullptr)`**:
stores the callbacks and sets `mCancelPublishersOnTxIdle = true`. The actual
cancellation fires inside `OnWiFiPAFMessageReceived()` the next time an inbound
PAFTP message is processed and the send queue is empty and no acknowledgements
are outstanding (`mSendQueue.IsNull() && !mPafTP.ExpectingAck()`). After
`CancelAllPublisherSessions(cancelCb)` runs, the optional `afterCb(afterCtx)`
fires so the caller can sequence follow-up work (used by
`DeviceControlServer::PostConnectedToOperationalNetworkEvent` to post
`kOperationalNetworkEnabled` only once PAFTP has drained).

The `OnTxIdleActionFunct` is `void (*)(void * ctx)`.

**`CancelAllPublisherSessions(OnCancelDeviceHandle cb)`**: iterates
`mPafInfoVect` and calls `cb(id, role)` for every entry whose role is
`kWiFiPafRole_Publisher` and whose `id` is not `kUndefinedWiFiPafSessionId`.
Called from the tx-idle check in `OnWiFiPAFMessageReceived()` and from
`PostNetworkConnect()` (directly, when WIFIPAF is disabled).

**Rationale for deferred cancel**: active NAN prevents IPv6 Duplicate Address
Detection (DAD) from completing after WiFi association, delaying mDNS
advertising by ~35 s. Canceling NAN immediately after `ConnectNetwork` would
risk tearing down the link before the `ConnectNetworkResponse` PAFTP frame is
acknowledged by the proxy. The tx-idle trigger ensures all in-flight PAFTP data
has been confirmed by the peer before NAN is torn down. With this fix mDNS
resolves in ~3 s and CASE completes without MRP retransmit. Tearing down
publish at the same point also closes the PAFTP session per Matter spec
§4.20.3.10 [4.780] and discharges any pending standalone-ACK obligation per
§4.20.3.8 [4.771].

---

### Server Commissioning Path (shared with End Device)

These changes live in shared core SDK files but are written for the WiFi-PAF /
shared-radio scenario the Commissioning Proxy and lighting-app End Device both
rely on.

#### `src/app/server/CommissioningWindowManager.cpp` / `.h`

`StopAdvertisement()` now takes an additional `aKeepPAFPublish` parameter
(default `false`). When `true`, the WiFi-PAF publisher (`mPublishId`) is left
running even though the commissioning window is closing.
`OnSessionEstablished()` calls
`StopAdvertisement(/* aShuttingDown = */ false, /* aKeepPAFPublish = */ true)`
so that the PAFTP session carrying the post-PASE data path (ArmFailSafe /
OpCreds / ConnectNetwork) is not torn down at PASE completion.

#### `src/platform/DeviceControlServer.cpp`

`PostConnectedToOperationalNetworkEvent()`

The implementation hooks the explicit PAFTP tx-idle signal:

1.  If `WiFiPAFLayer::GetWiFiPAFState() == State::kConnected`, the function
    calls
    `paf.ScheduleCancelPublishersOnTxIdle(CancelPafPublisher, OnPafTxIdle, null)`.
2.  `CancelPafPublisher(id, role)` calls
    `ConnectivityMgr().WiFiPAFShutdown(id, role)` to tear down the publisher.
3.  `OnPafTxIdle()` posts `kOperationalNetworkEnabled` so mDNS / operational
    advertisement starts.
4.  A 5 s `OnPostOpEventWatchdog` timer is armed in parallel as a fallback in
    case the PAF peer never sends another packet to drive the tx-idle check
    (e.g. the proxy has already disconnected). Whichever fires first posts the
    event; a `sPostOpEventFired` one-shot guard prevents a double post.

If WiFi-PAF is not active, the function falls through to the original behavior
and posts the event immediately.

---

### Network Commissioning (shared with End Device)

#### `src/app/clusters/network-commissioning/NetworkCommissioningCluster.cpp` / `.h`

The standard concurrent-connection path replies to `ConnectNetwork` from the
wireless driver's `OnResult()` callback — i.e. after WiFi association either
succeeded or failed. That ordering is unsafe on hardware that shares the radio
between NAN and the WiFi station: once `wpa_supplicant` starts the association
the radio camps on the AP channel, and PAFTP frames that need to leave on the
NAN channel either fail to transmit or transmit on the wrong channel. The
`ConnectNetworkResponse` then never reaches the commissioner.

Behind the new build option
`CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE` (default `0`),
`HandleConnectNetwork` takes a different path when the incoming command is
carried over a WiFi-PAF secure session:

1.  `IsConnectNetworkRequestOverWiFiPAF(handler)` — peeks at the exchange's
    `Transport::Type` to detect the PAF transport.
2.  `SendEarlyConnectNetworkResponseForWiFiPAF()` — sends
    `ConnectNetworkResponse{ networkingStatus = Success }` immediately while NAN
    is still primary on the shared radio.
3.  `PlatformMgr().ScheduleWork(StartWiFiConnectAfterPafAck, this)` — defers the
    blocking `mpWirelessDriver->ConnectNetwork()` call to the next event loop
    iteration, so the IM-queued response has time to flush through the PAFTP TX
    queue first.
4.  `HandleConnectNetwork` returns `std::nullopt` to suppress the framework's
    default IM response (the early response is the only response).

`mConnectNetworkResponseSentEarly` is set to mark that the response has already
been emitted. `OnResult()` checks this flag and, instead of sending a second
response, either calls `PostConnectedToOperationalNetworkEvent()` on success or
logs the failure (recovery is then driven by fail-safe expiry).

The option only activates when both `CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF` and
`CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION` are `1`. It is enabled in
`examples/lighting-app/linux/include/CHIPProjectAppConfig.h` and
`examples/commissioning-proxy-app/linux/include/CHIPProjectAppConfig.h`.

#### `src/include/platform/CHIPDeviceConfig.h`

Adds the `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE` build
option, default `0`. Set to `1` on shared-radio devices where post-association
NAN TX is unreliable.

---

### Linux Platform

#### `src/platform/Linux/ConnectivityManagerImpl.cpp` / `.h`

**`WiFiPAFDisconnectPublishReceiveHandler()`**: disconnects all GLib signal
handlers connected to the `nanreceive` signal on `mWpaSupplicant.iface` with
`G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA`. This is called alongside
`WiFiPAFCancelPublish()` in `CancelWiFiPAFPublish()` (proxy app `main.cpp`) to
ensure that a subsequent `_WiFiPAFSubscribe` call registers exactly one
`nanreceive` handler, preventing duplicate PAF packet delivery.

**`WiFiPAFStartBackgroundScan(BgScanDiscoveryCallback cb, void * cbCtx)`**:
issues a NAN subscribe for the commissioning discriminator range and registers
`cb` as the discovery callback. Discovery results are marshaled from the GLib
thread to the CHIP event loop via `DeviceLayer::SystemLayer().ScheduleWork()`.
Returns `CHIP_NO_ERROR` if the hardware scan starts successfully.

**`WiFiPAFStopBackgroundScan()`**: cancels the running background NAN subscribe
and clears the stored callback.

**`GetPendingConnectSubscribeId()`** (inline): returns
`mPendingConnectSubscribeId` — the NAN subscribe ID allocated by the most recent
`_WiFiPAFSubscribe` call that has not yet transitioned to an active session.
Used by `HandleProxyConnectRequest` to capture the ID for timeout cleanup.

**`WiFiPAFScan(uint8_t scanMaxTime, PafScanResultsCallback cb, void * cbCtx)`**:
performs a foreground one-shot NAN discovery scan. Issues a NAN subscribe with
TTL = `scanMaxTime` seconds, accumulates results in `mNanScanPeers` (a
`std::set<NanPeerInfo>` by MAC + discriminator), and starts a
`scanMaxTime`-second timer (`FinishWiFiPAFScan`) that cancels the subscribe and
delivers all collected results to `cb` when it fires. Setting the subscribe TTL
equal to `scanMaxTime` ensures wpa_supplicant's own expiry and the CHIP-side
timer are aligned, preventing stale subscribe IDs from returning zero results on
subsequent scans.

**`NanPeerInfo` struct** extended fields:

| Field  | Type       | Description                                                          |
| ------ | ---------- | -------------------------------------------------------------------- |
| `band` | `uint16_t` | `WiFiBandBitmap` value derived from the scan frequency (0 = unknown) |
| `vid`  | `uint16_t` | Vendor ID parsed from the NAN SSI advertisement                      |
| `pid`  | `uint16_t` | Product ID parsed from the NAN SSI advertisement                     |

These fields populate `ScanResultStruct::vendorID`, `::productID`, and
`::wiFiBand` in the `CachedResults` attribute and `ProxyScanResponse`.

**`mScanFreq`** (private field): the frequency (MHz) reported by wpa_supplicant
for the most recent NAN discovery event. Set under `mWpaSupplicantMutex` on the
GLib thread; read in `ScanDiscoveryResult()` to derive a `WiFiBandBitmap` value
(`2.4 GHz` / `5 GHz`) for the `CachedResults` attribute.

**`PostNetworkConnect()` — deferred NAN publisher cancel**: after setting
`mPafChannelAvailable = true`, calls
`WiFiPAFLayer::ScheduleCancelPublishersOnTxIdle()` with a lambda that calls
`_WiFiPAFShutdown(id, role)` for each publisher session, deferring the cancel
until the PAFTP send queue is drained and all outstanding acknowledgements have
been received.

---

### Controller / chip-tool

#### `src/controller/CHIPDeviceControllerSystemState.h`

`DeviceTransportMgr` (the compile-time transport list in chip-tool) has
`Transport::Proxy<>` appended as the last transport.

`kDeviceProxyTransportIndex` is computed at compile time as the zero-based index
of `Proxy<>` within the `TransportMgr` variant, using a chain of `constexpr`
additions gated on the same `#if` guards used in the type alias. This must stay
in sync with the `DeviceTransportMgr` type alias.

`GetDeviceProxyTransport(DeviceTransportMgr *)` returns the embedded
`ProxyTransportBase` by index.

#### `src/controller/CHIPDeviceControllerFactory.cpp`

`ProxyListenParameters` is passed to `TransportMgr::Init()` alongside the
existing UDP/TCP/BLE/WiFi-PAF parameters to initialize the proxy transport slot.

#### `examples/chip-tool/commands/pairing/PairingCommand.cpp`

The `PairViaProxy()` method implements a five-step async flow:

1.  `GetConnectedDevice(mProxyNodeId, ...)` — establishes a CASE session to the
    proxy, using large-payload capability so the TCP transport is preferred.
2.  `OnProxyDeviceConnected` — sends `ProxyConnectRequest` with the commissionee
    discriminator, WiFi-PAF transport, and a 30 s timeout.
3.  `OnResponse` (for `ProxyConnectResponse`) — extracts `sessionId`, activates
    the `ProxyTransport`, and calls
    `Pair(mNodeId, PeerAddress::Proxy(sessionId))`.
4.  `OnResponse` (for `ProxyMessageResponse`) — extracts the raw bytes and calls
    `ProxyTransport::OnProxyMessageReceived()` to inject them into the Matter
    stack.
5.  `SendProxyDisconnect()` — called at `OnCommissioningComplete` (success or
    failure) and at `OnPairingComplete` (on PASE failure); sends
    `ProxyDisconnectRequest` as a fire-and-forget best-effort cleanup.

The `cmdTimeout` for each `ProxyMessageRequest` exchange is set to
`responseTimeout + 10` seconds so the chip-tool exchange stays alive while the
proxy waits for a slow commissionee response.

---

### AutoCommissioner

#### `src/controller/AutoCommissioner.cpp`

When `transportType == Transport::Type::kProxy`, `mNeedsNetworkSetup` is set
based on whether WiFi credentials were supplied. Without this, the commissioning
framework would skip `AddOrUpdateWiFiNetwork` and `ConnectNetwork` for the proxy
transport (because `kProxy` is not `kWiFiPAF`), and the commissionee would never
receive its network credentials.

---

### Generated / ZAP artifacts

The following files are auto-generated and should not be edited directly. They
are regenerated from `commissioning-proxy.matter` / `.zap` by running the ZAP
code generator.

-   `zzz_generated/app-common/clusters/CommissioningProxy/Commands.h` / `.ipp`
-   `zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.h` /
    `.cpp`
-   `zzz_generated/app-common/app-common/zap-generated/callback.h`
-   `zzz_generated/chip-tool/zap-generated/cluster/Commands.h`
-   `src/controller/data_model/controller-clusters.matter`
-   `src/controller/java/…`, `src/controller/python/…`, `src/darwin/…`
-   `src/app/zap_cluster_list.json`, `src/app/zap-templates/zcl/…`

<hr>

## Python Certification Tests

Python Matter certification tests for the Commissioning Proxy cluster are in
`src/python_testing/`. Each test uses `MatterBaseTest` and runs against a live
DUT (commissioning-proxy-app) over the network.

| Test file          | Description                                                                                                                                                                                   |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `TC_COMPRO_2_1.py` | Attributes with DUT as Server — reads and validates all mandatory/optional cluster attributes                                                                                                 |
| `TC_COMPRO_2_2.py` | Proxy Scan feature — `ProxyScanRequest` returns at least one result for a nearby commissionable device                                                                                        |
| `TC_COMPRO_2_3.py` | Proxy Background Scan feature — start/stop/cache lifecycle and `CachedResults` attribute reporting                                                                                            |
| `TC_COMPRO_2_4.py` | Proxy Connect, Message and Disconnect — full end-to-end commissioning via proxy                                                                                                               |
| `TC_COMPRO_2_5.py` | Writable Attributes — `ScanMaxTime` and `CacheTimeout` write/read-back                                                                                                                        |
| `TC_COMPRO_2_6.py` | Error Handling — verifies `INVALID_TRANSPORT_TYPE`, `RESOURCE_EXHAUSTED`, `NOT_FOUND`, `BUSY` status codes                                                                                    |
| `TC_COMPRO_2_7.py` | Multi-Session Support — `MaxSessions` attribute reflects implementation limit; second `ProxyConnectRequest` returns `RESOURCE_EXHAUSTED` when sessions are full                               |
| `TC_COMPRO_2_8.py` | Fabric Isolation — commissions a second in-process fabric (TH2) and verifies that cross-fabric `ProxyMessageRequest`, `ProxyDisconnectRequest`, and `CancelPendingConnect` return `NOT_FOUND` |
| `TC_COMPRO_2_9.py` | Device Type Requirements — verifies the device type descriptor and mandatory cluster list on the endpoint                                                                                     |

Tests that require a physical End Device (2.2, 2.3, 2.4, 2.6, 2.7, 2.8) are run
with the `networked` mode argument, which SSH-resets the ED RPi before each
iteration.

In networked mode, the test fixture (`EDFixture` in
`src/python_testing/support_modules/compro_support.py`) also SSH-engages an
iptables rule on the ED that blocks return traffic from the commissioner over
`eth0`. Without it the commissioner could complete CASE over ethernet after
`ConnectNetwork`, silently masking a broken WiFi-PAF / WiFi-association path.
The fixture brings the block up at ED start and tears it down at ED stop.

For full instructions on installing the helper scripts on the ED and wiring up
the three-RPi cert-test rig, see
[`CERTIFICATION_TEST_CONFIG_README.md`](CERTIFICATION_TEST_CONFIG_README.md).

<hr>

## Cross-Compiling for Raspberry Pi

For Docker-based cross-compilation of all three binaries
(commissioning-proxy-app, chip-tool, and chip-lighting-app) and full RPi
deployment instructions, see `../CP_getting_started.md §4–5`. That document also
covers the wpa_supplicant NAN patch required on the proxy RPi and hardware
prerequisites.
