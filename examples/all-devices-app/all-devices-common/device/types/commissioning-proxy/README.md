# Getting Started: Wi-Fi PAF for the Commissioning Proxy

This guide is a self-contained walkthrough for getting the **Commissioning Proxy
(CP)** device in `all-devices-app` working over **Wi-Fi PAF** (Wi-Fi Aware / NAN
— Neighbor Awareness Networking).

The Commissioning Proxy tunnels commissioning traffic between a Commissioner
(chip-tool) and a Commissionee that the Commissioner cannot reach directly:

```
chip-tool  ──Matter (TCP/IP)──►  all-devices-app (CP)  ──Wi-Fi PAF (NAN)──►  Commissionee
```

Wi-Fi PAF is one of the transports the CP device can be built with (BLE is
another). Both are compiled in by default on Linux; the transport used to reach
a commissionee is chosen **per command** by the `Transport` field of the
request, not by a command-line flag. This guide covers Wi-Fi PAF end to end —
everything you need to do to make PAF work, from hardware to a completed
commission-through-proxy.

<hr>

-   [1. Hardware prerequisites](#1-hardware-prerequisites)
-   [2. wpa_supplicant with the Matter NAN patch (proxy device)](#2-wpa_supplicant-with-the-matter-nan-patch-proxy-device)
    -   [2.1 Why the stock package is not enough](#21-why-the-stock-package-is-not-enough)
    -   [2.2 The patch](#22-the-patch)
    -   [2.3 Build and install](#23-build-and-install)
-   [3. Build all-devices-app with Wi-Fi PAF](#3-build-all-devices-app-with-wi-fi-paf)
-   [4. Run the Commissioning Proxy](#4-run-the-commissioning-proxy)
-   [5. Commission the proxy onto the fabric](#5-commission-the-proxy-onto-the-fabric)
-   [6. Run the End Device (commissionee)](#6-run-the-end-device-commissionee)
-   [7. Scanning (optional)](#7-scanning-optional)
-   [8. Commission the End Device through the proxy](#8-commission-the-end-device-through-the-proxy)
-   [9. How Wi-Fi PAF is wired into the CP device](#9-how-wi-fi-paf-is-wired-into-the-cp-device)
-   [10. Troubleshooting](#10-troubleshooting)

<hr>

## 1. Hardware prerequisites

Wi-Fi PAF needs a radio and driver that support **NAN USD** (Wi-Fi Aware
Unsynchronized Service Discovery). Both the proxy and the commissionee need it.

| Role                      | Requirement                                                                                                                                        |
| ------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| Commissioning Proxy (DUT) | Linux host (e.g. Raspberry Pi 4/5) with a **USB Wi-Fi dongle that supports NAN USD.** See https://groups.csa-iot.org/wg/members-all/document/44361 |
| End Device (commissionee) | Same NAN-USD-capable USB Wi-Fi dongle requirement                                                                                                  |

> The **on-board Raspberry Pi Wi-Fi does not support NAN USD** and cannot be
> used for Wi-Fi PAF. The USB dongle must be the interface wpa_supplicant
> manages.

On each device with a PAF dongle, install the wireless utilities. Without them
the proxy fails at startup with
`[PAF] Failed to start Wi-Fi PAF publish: ... CHIP Error 0x00000003: Incorrect state`:

```bash
sudo apt install net-tools wireless-tools
```

<hr>

## 2. wpa_supplicant with the Matter NAN patch (proxy device)

The Commissioning Proxy requires a custom build of wpa_supplicant. The stock
Ubuntu package does not compile in NAN USD support, and its default NAN scan
behavior interferes with the proxy's background scan.

### 2.1 Why the stock package is not enough

-   The stock `wpa_supplicant` is built with `CONFIG_NAN_USD` disabled, so it
    has no Wi-Fi Aware support at all.
-   When the proxy performs a NAN scan it subscribes to the Matter NAN service
    as a passive listener. By default wpa_supplicant automatically sends a NAN
    Follow-up frame back to every matching publisher, which triggers spurious
    session setup. The patch adds a `discovery_only` flag that suppresses those
    automatic replies for discovery-only scans.

### 2.2 The patch

The patch touches four files:

| File                                      | Change                                                                                                           |
| ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `wpa_supplicant/defconfig`                | Enables `CONFIG_NAN_USD=y` so NAN USD support is compiled in                                                     |
| `src/common/nan_de.h`                     | Adds a `discovery_only` boolean to `nan_subscribe_params`                                                        |
| `src/common/nan_de.c`                     | Skips the automatic Follow-up reply to the publisher when `discovery_only` is set                                |
| `wpa_supplicant/dbus/dbus_new_handlers.c` | Exposes `discovery_only` as a D-Bus parameter so the Matter stack can pass it from the background-scan call path |

Save the following as `wpa-supplicant-matter.patch`:

```diff
diff --git a/src/common/nan_de.c b/src/common/nan_de.c
index 2af1afd73..1196ec617 100644
--- a/src/common/nan_de.c
+++ b/src/common/nan_de.c
@@ -910,7 +910,8 @@ static void nan_de_rx_publish(struct nan_de *de, struct nan_de_service *srv,
 		nan_de_tx_multicast(de, srv, instance_id);
 	}

-	if (!de->offload && !srv->subscribe.active && req_instance_id == 0) {
+	if (!de->offload && !srv->subscribe.active && req_instance_id == 0 &&
+	    !srv->subscribe.discovery_only) {
 		/* Passive subscriber replies with a Follow-up message without
 		 * Service Specific Info field if it received a matching
 		 * unsolicited Publish message. */
diff --git a/src/common/nan_de.h b/src/common/nan_de.h
index b2688a83c..6b9e2ad1f 100644
--- a/src/common/nan_de.h
+++ b/src/common/nan_de.h
@@ -154,6 +154,11 @@ struct nan_subscribe_params {

 	/* Proximity ranging flag */
 	bool proximity_ranging;
+
+	/* If true, suppress the automatic Follow-up sent to the publisher when
+	 * a passive subscriber matches an unsolicited Publish frame.  Use for
+	 * discovery-only scans where no session is to be established. */
+	bool discovery_only;
 };

 /* Returns -1 on failure or >0 subscribe_id */
diff --git a/wpa_supplicant/dbus/dbus_new_handlers.c b/wpa_supplicant/dbus/dbus_new_handlers.c
index 76a9297eb..8a9cec6a3 100644
--- a/wpa_supplicant/dbus/dbus_new_handlers.c
+++ b/wpa_supplicant/dbus/dbus_new_handlers.c
@@ -6855,6 +6855,10 @@ DBusMessage * wpas_dbus_handler_nan_subscribe(DBusMessage *message,
 			for (i = 0; i < entry.array_len; i++)
 				int_array_add_unique(
 					&freq_list, entry.uint16array_value[i]);
+		} else if (os_strcmp(entry.key, "discovery_only") == 0 &&
+			   entry.type == DBUS_TYPE_BOOLEAN) {
+			params.discovery_only = entry.bool_value;
+			wpa_dbus_dict_entry_clear(&entry);
 		} else {
 			wpa_printf(MSG_DEBUG,
 				   "dbus: NANSubscribe - unsupported dict entry '%s'",
diff --git a/wpa_supplicant/defconfig b/wpa_supplicant/defconfig
index 84ac8ba12..2a8f3e9f3 100644
--- a/wpa_supplicant/defconfig
+++ b/wpa_supplicant/defconfig
@@ -681,4 +681,4 @@ CONFIG_DPP2=y
 #CONFIG_NO_WMM_AC=y

 # Wi-Fi Aware unsynchronized service discovery (NAN USD)
-#CONFIG_NAN_USD=y
+CONFIG_NAN_USD=y
```

### 2.3 Build and install

Building natively on the proxy device is simplest:

```bash
# 1. Clone upstream hostap and check out the pinned commit
git clone https://w1.fi/hostap.git
cd hostap
git checkout e17107912

# 2. Apply the Matter NAN patch (adjust the path to where you saved it)
git apply /path/to/wpa-supplicant-matter.patch

# 3. Use the patched defconfig as the active build config
cd wpa_supplicant
cp defconfig .config

# 4. Install build dependencies
sudo apt update
sudo apt install libnl-3-dev libnl-genl-3-dev libnl-route-3-dev

# 5. Build
make all
```

Verify NAN USD was compiled in, then install and restart the service:

```bash
strings wpa_supplicant | grep -c nan_usd   # should be > 0

sudo systemctl stop wpa_supplicant
sudo cp wpa_supplicant /usr/sbin/wpa_supplicant
sudo systemctl start wpa_supplicant
systemctl status wpa_supplicant            # expect active (running)
```

<hr>

## 3. Build all-devices-app with Wi-Fi PAF

Wi-Fi PAF is gated by the GN arg `chip_device_config_enable_wifipaf`, which
defaults to **true** on Linux builds with Wi-Fi enabled (see
`src/platform/device.gni`). The Commissioning Proxy cluster itself is enabled by
`CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY`, which is forced on for this app
in `posix/include/CHIPProjectAppConfig.h`. So a normal Linux build already
includes Wi-Fi PAF — no extra args are required.

The PAF transport driver (`CommissioningProxyPafTransport.cpp`) is compiled only
when `chip_device_config_enable_wifipaf` is set; a build with it off drops the
`WiFiNetworkInterface` cluster feature and the `WiFiBand` attribute.

**Upstream build (Linux x86-64 / ARM), both transports:**

```bash
# x86-64
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl build"

# ARM (arm64)
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py --target linux-arm64-all-devices-boringssl build"
```

The binary is at `out/linux-x64-all-devices-boringssl/all-devices-app` (or the
`arm64` equivalent).

**Select the transports at build time:**

The targets above compile every transport the platform supports. To build a
single-transport binary, configure the application's own gn root and pass the
flag that disables the other transport:

```bash
./scripts/run_in_build_env.sh "cd examples/all-devices-app/posix && \
  gn gen out/paf-only --args='chip_config_network_layer_ble=false' && \
  ninja -C out/paf-only all-devices-app"
```

| gn argument                               | Transports compiled in |
| ----------------------------------------- | ---------------------- |
| _(none of the below)_                     | BLE and Wi-Fi PAF      |
| `chip_config_network_layer_ble=false`     | Wi-Fi PAF only         |
| `chip_device_config_enable_wifipaf=false` | BLE only               |

The binary lands at `out/<name>/all-devices-app` under
`examples/all-devices-app/posix`. Cross-compiling for a Raspberry Pi adds the
usual aarch64 arguments to the same `gn gen` line — `target_os="linux"`,
`target_cpu="arm64"`, and a `sysroot=` and `system_libdir=` pointing at your
aarch64 sysroot.

<hr>

## 4. Run the Commissioning Proxy

Start the proxy on the device with the NAN-capable dongle. Select the CP device
and its endpoint with `--device commissioning-proxy:5`, and enable Wi-Fi
management. `freq_list=2437` selects channel 6 (2.4 GHz) — the default Matter
PAF channel, permitted in all regulatory regions:

```bash
./all-devices-app \
    --device commissioning-proxy:5 \
    --wifi \
    --wifipaf "freq_list=2437" \
    --discriminator 3947
```

| Argument                         | Description                                                                               |
| -------------------------------- | ----------------------------------------------------------------------------------------- |
| `--device commissioning-proxy:5` | Instantiate the CP device on endpoint 5                                                   |
| `--wifi`                         | Enable Wi-Fi management via wpa_supplicant (required for PAF)                             |
| `--wifipaf "freq_list=<MHz>"`    | NAN operating frequencies in MHz. `2437` = channel 6 (2.4 GHz); add e.g. `5745` for 5 GHz |
| `--discriminator <value>`        | 12-bit value identifying the proxy during its own commissioning                           |

The `WiFiBand` attribute advertised by the cluster is derived from `freq_list`
at startup: 2412–2484 MHz → 2.4 GHz, 5035–5980 MHz → 5 GHz. If no valid
frequency is parsed the proxy defaults to advertising 2.4 GHz.

Once the proxy is commissioned (next step), it cancels its own NAN publisher and
disconnects the associated receive handler, so the PAF subscribe calls it makes
on behalf of commissioners register exactly one handler.

<hr>

## 5. Commission the proxy onto the fabric

The proxy must itself join the Matter fabric before it can tunnel for others.
From the Commissioner run a one-time on-network pairing (the proxy is reachable
over IP):

```bash
./chip-tool pairing onnetwork 1998 20202021
```

`1998` is the node ID assigned to the proxy (any non-zero value); `20202021` is
the default PASE passcode. Success prints:

```
CHIP:TOO: Device commissioning completed with success
```

> **Tip:** the proxy re-reads its stored fabric credential on restart. You do
> not need to re-pair unless you clear its key-value store (`/tmp/chip_*`).

<hr>

## 6. Run the End Device (commissionee)

Start the commissionee (for example a lighting-app) with a `freq_list` that
matches — or overlaps — the proxy's. The End Device must **not** already be on a
network, as it is about to be commissioned:

```bash
./chip-lighting-app \
    --wifi \
    --wifipaf "freq_list=2437" \
    --discriminator 3840
```

The End Device advertises itself as a NAN publisher. Confirm in its log:

```
WiFi-PAF: Starting NAN publish
```

> The End Device discriminator (`3840` here) must match the value you pass to
> `chip-tool pairing proxy` in step 8.

<hr>

## 7. Scanning (optional)

Before connecting, a Commissioner can ask the proxy to scan for nearby
commissionable devices. In these commands `8` is the Wi-Fi PAF transport bit,
`1998` is the proxy node ID (use whatever you paired in step 5), and `5` is the
endpoint where the cluster lives. The `--WiFiBands` bitmap is `1` = 2.4 GHz, `4`
= 5 GHz, `5` = both.

**Foreground (one-shot) scan** — results are returned inline. This command
carries a large payload, so TCP must be forced:

```bash
./chip-tool commissioningproxy proxy-scan-request 8 1998 5 \
    --allow-large-payload true --WiFiBands 5 --timeout 20
```

**Background scan** — the proxy scans continuously and caches results, pushed to
subscribers via attribute reporting. `Timeout 0` means no expiry:

```bash
# start
./chip-tool commissioningproxy proxy-back-ground-scan-start-request 8 0 1998 5 \
    --WiFiBands 5

# read the cache
./chip-tool commissioningproxy read cached-results 1998 5

# stop
./chip-tool commissioningproxy proxy-back-ground-scan-stop-request 8 1998 5 \
    --WiFiBands 5
```

<hr>

## 8. Commission the End Device through the proxy

Use `chip-tool pairing proxy` to commission the End Device through the proxy.
The proxy opens a Wi-Fi PAF (NAN) session to the End Device and tunnels the PASE
and commissioning packets between chip-tool and the device:

```bash
./chip-tool pairing proxy <node-id> <ssid> <password> <setup-pin-code> \
    <ed-discriminator> <proxy-node-id> <proxy-connect-timeout> <proxy-transport> \
    [--proxy-endpoint <endpoint>] [--proxy-wifi-band 2g4|5g]
```

| Argument                | Description                                                                 |
| ----------------------- | --------------------------------------------------------------------------- |
| `node-id`               | Node ID to assign to the new device                                         |
| `ssid`                  | Wi-Fi network SSID to configure on the device                               |
| `password`              | Wi-Fi network password                                                      |
| `setup-pin-code`        | Commissionee PASE setup PIN code                                            |
| `ed-discriminator`      | Commissionee discriminator (matches step 6)                                 |
| `proxy-node-id`         | Node ID of the already-commissioned proxy                                   |
| `proxy-connect-timeout` | Seconds the proxy may spend connecting to the commissionee; `0` = no limit  |
| `proxy-transport`       | `wifipaf` or `ble` — the transport the proxy uses to reach the commissionee |
| `--proxy-endpoint`      | Endpoint hosting the cluster. Defaults to `1`; this app uses `5`            |
| `--proxy-wifi-band`     | Optional band hint, `2g4` or `5g`; accepted only with `wifipaf`             |

Example — commission device node 1999 onto network "MyNetwork" via proxy 1998:

```bash
./chip-tool pairing proxy 1999 "MyNetwork" "MyPassword" 20202021 3840 1998 0 wifipaf \
    --proxy-endpoint 5 --proxy-wifi-band 2g4
```

<hr>

## 9. How Wi-Fi PAF is wired into the CP device

The Wi-Fi PAF driver, `CommissioningProxyPafTransport`, ships with the cluster
as the separate `paf-transport` target, so a build that proxies over BLE only —
or a platform with no Wi-Fi PAF stack — can leave it out. Its design, the
transport methods it implements and the shared-scanner rules are documented in
[`src/app/clusters/commissioning-proxy-server/README.md`](../../../../../../src/app/clusters/commissioning-proxy-server/README.md).

### What this app has to supply

Session setup and message tunneling are portable. Discovery is not: scanning for
commissionable devices, and recovering the NAN subscribe id the platform
assigned, both need platform code. That is the `CommissioningProxyPafAdapter`
interface, which this app implements in
[`posix/linux/CommissioningProxyPafAdapter.cpp`](../../../../posix/linux/CommissioningProxyPafAdapter.cpp):

| Adapter method                      | Linux implementation                                             |
| ----------------------------------- | ---------------------------------------------------------------- |
| `StartForegroundScan()`             | `ConnectivityMgrImpl().WiFiPAFScan()`                            |
| `StartBackgroundScan()`             | `ConnectivityMgrImpl().WiFiPAFStartBackgroundScan()`             |
| `StopBackgroundScan()`              | `ConnectivityMgrImpl().WiFiPAFStopBackgroundScan()`              |
| `PendingConnectSubscribeId()`       | `ConnectivityMgrImpl().GetPendingConnectSubscribeId()`           |
| `DisconnectPublishReceiveHandler()` | `ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler()` |

The adapter is also where the platform's peer descriptor is unpacked.
`NanPeerInfo` is Linux-only and owns heap storage for its extended data, so the
adapter reports each peer as plain scalars and that type never reaches the
cluster.

Adapter and driver are both constructed in
`posix/linux/DeviceFactoryPlatformOverride.cpp`, which composes the driver onto
the single `CommissioningProxyDevice` with `AddTransport()` — a build with BLE
adds that driver the same way — and derives the advertised `WiFiBand` from
`--wifipaf freq_list=`, since the device itself reads no command line.
Transports are registered before `Server::Init()`, so the fabric table is empty
at that point and the driver's `DisconnectPublishReceiveHandler()` call lands on
the commissioning-complete event instead.

<hr>

## 10. Troubleshooting

### Proxy fails to start with a NAN error

-   Confirm the USB Wi-Fi dongle is present (`lsusb`) and its interface exists
    (`ip link`).
-   Confirm the patched wpa_supplicant is installed and running:
    `systemctl status wpa_supplicant` shows `active (running)`, and
    `strings /usr/sbin/wpa_supplicant | grep -c nan_usd` returns > 0.
-   Do not use the on-board RPi Wi-Fi — it does not support NAN USD.
-   Ensure `net-tools` and `wireless-tools` are installed (see section 1); a
    missing `iwconfig` surfaces as `CHIP Error 0x00000003: Incorrect state`.

### Proxy connect times out (Commissioner gets `Status::Timeout`)

The proxy waited for the connect timeout on the NAN session and gave up. Common
causes:

-   The End Device is not running or is on a different NAN channel — confirm
    both sides use the same `freq_list`.
-   The End Device host also needs a NAN-capable USB Wi-Fi dongle.
-   The NAN frequency is not allowed by the regulatory domain. `freq_list=2437`
    (channel 6, 2.4 GHz) is permitted everywhere.

### PAF session closes mid-commissioning

Look for `WiFiPAFCloseSession` in the proxy log — the PAFTP ack-receive timer
fired. The most common cause is the End Device's radio going off-channel while
it scans for and joins the Wi-Fi network. Retrying usually succeeds.

### `ConnectNetwork` is slow

`ConnectNetwork` can take 10–15 s while the End Device scans for and joins the
Wi-Fi AP. This is normal; the proxy extends the Commissioner's exchange timeout
for this step.

### `chip-tool pairing proxy` command not found

You are running a chip-tool build without the Commissioning Proxy support. Build
chip-tool from this repository.
