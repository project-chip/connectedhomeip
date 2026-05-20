# CHIP Linux Commissioning Proxy Example

An example showing the use of the Matter Commissioning Proxy cluster on Linux.
The Commissioning Proxy (CP) acts as a tunnel between a Commissioner (chip-tool)
and a Commissionee that is reachable via a transport not available on the
Commissioner — for example, Wi-Fi Aware (PAF).

This document describes how to build and run the Commissioning Proxy app and how
to commission a device through it using chip-tool.

<hr>

-   [CHIP Linux Commissioning Proxy Example](#chip-linux-commissioning-proxy-example)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Commissioning Proxy](#running-the-commissioning-proxy)
    -   [Commissioning a Device via the Proxy](#commissioning-a-device-via-the-proxy)
    -   [Foreground Scanning](#foreground-scanning)
    -   [Background Scanning](#background-scanning)
    -   [Fabric Isolation](#fabric-isolation)
    -   [Device Tracing](#device-tracing)

<hr>

## Building

**For Raspberry Pi (hardware testing):** see `../CP_getting_started.md` for the
full cross-compilation and deployment workflow, including Docker image setup,
wpa_supplicant patching, and deploying all three binaries to the RPi.

**For local x86 development:**

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application (from the repo root):

          $ cd examples/commissioning-proxy-app/linux
          $ source ../../../scripts/bootstrap.sh
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

    To omit RPC support, replace the `--args` value with `--args=''` or omit it
    entirely.

-   To delete generated executable, libraries and object files use:

          $ cd examples/commissioning-proxy-app/linux
          $ rm -rf out/

## Commandline arguments

-   `--wifi`

    Enables WiFi management feature. Required for Wi-Fi PAF commissioning.

-   `--wifipaf freq_list=<freq_1>,<freq_2>,...`

    Enables Wi-Fi PAF via wpa_supplicant and sets the NAN operating frequencies
    in MHz. At least one frequency must be supplied. The proxy derives the
    `WiFiBand` attribute value from this list at startup (2412–2484 MHz → 2.4
    GHz, 5035–5980 MHz → 5 GHz).

    Example — 2.4 GHz channel 6 only:

          --wifipaf freq_list=2437

    Example — 2.4 GHz channel 6 and 5 GHz channel 149:

          --wifipaf freq_list=2437,5745

-   `--discriminator <value>`

    Set the discriminator used to identify this proxy node during commissioning.

-   `--passcode <value>`

    Set the PASE passcode for commissioning the proxy node itself.

-   `--app-pipe <path>`

    Path of a named pipe used to send commands to the running app. Optional.

## Running the Commissioning Proxy

The proxy app must first be commissioned onto the fabric before it can proxy
commissioning for other devices. Start the proxy app on a node that has Wi-Fi
Aware (NAN) capability:

```
$ ./out/debug/chip-commissioning-proxy-app --wifi --wifipaf freq_list=2437
```

Then commission the proxy node itself using chip-tool (one time):

```
$ ./chip-tool pairing onnetwork <proxy-node-id> <passcode>
```

For example:

```
$ ./chip-tool pairing onnetwork 0x7ce 20202021
```

Once commissioned the proxy app cancels its own NAN publisher advertisement and
disconnects the associated signal handler, so that subsequent PAF subscribe
calls it makes on behalf of commissioners register exactly one handler.

## Commissioning a Device via the Proxy

Once the proxy node is commissioned, use `chip-tool pairing proxy` to commission
a Commissionee through the proxy. The proxy establishes a Wi-Fi PAF (NAN)
connection to the Commissionee and tunnels PASE and commissioning packets
between chip-tool and the device.

### Command syntax

```
$ ./chip-tool pairing proxy <node-id> <ssid> <password> <setup-pin-code> <discriminator> <proxy-node-id> [<proxy-connect-timeout>]
```

| Argument                | Description                                                  |
| ----------------------- | ------------------------------------------------------------ |
| `node-id`               | Node ID to assign to the new device                          |
| `ssid`                  | Wi-Fi network SSID to configure on the device                |
| `password`              | Wi-Fi network password                                       |
| `setup-pin-code`        | Commissionee's PASE setup PIN code                           |
| `discriminator`         | Commissionee's discriminator                                 |
| `proxy-node-id`         | Node ID of the already-commissioned commissioning proxy      |
| `proxy-connect-timeout` | Timeout in seconds for `ProxyConnectRequest` (default: 30 s) |

### Example

```
$ ./chip-tool pairing proxy 1999 "MyNetwork" "MyPassword" 20202021 3840 0x7ce
```

This commissions device node 1999 via the proxy at node 0x7ce onto the Wi-Fi
network "MyNetwork".

For a step-by-step walkthrough including what happens at each stage, see
`../CP_getting_started.md §9`. For the full packet-flow diagram and
implementation detail, see `IMPLEMENTATION_README.md §Architecture Overview`.

## Foreground Scanning

For annotated and copy-paste command forms see `../CP_getting_started.md §8.1`.

The proxy can perform a one-shot foreground NAN scan for commissionable devices
and return results immediately via `ProxyScanResponse`.

```
$ ./chip-tool commissioningproxy proxy-scan-request \
    <Transport> <proxy-node-id> <endpoint-id> \
    --allow-large-payload true \
    --WiFiBands <bands> \
    --timeout <seconds>
```

| Argument                     | Description                                                     |
| ---------------------------- | --------------------------------------------------------------- |
| `Transport`                  | Transport bitmap: `2` = BLE, `8` = WiFi-PAF                      |
| `proxy-node-id`              | Node ID of the proxy                                            |
| `endpoint-id`                | Endpoint hosting the CommissioningProxy cluster (typically `1`) |
| `--allow-large-payload true` | Required — forces TCP for this command (spec `L` quality)       |
| `--WiFiBands`                | Wi-Fi band bitmap: `1` = 2.4 GHz, `4` = 5 GHz, `5` = both       |
| `--timeout`                  | How long (seconds) chip-tool waits for the response             |

The scan runs for the duration configured by the proxy's `ScanMaxTime`
attribute. Results are returned inline in the `ProxyScanResponse`.

### Example — foreground WiFi-PAF scan on both bands

```
$ ./chip-tool commissioningproxy proxy-scan-request 8 0x7ce 1 --allow-large-payload true --WiFiBands 5 --timeout 20
```

## Background Scanning

For annotated and copy-paste command forms see
`../CP_getting_started.md §8.2–8.4`.

The proxy supports continuous background NAN scanning so that commissioners can
discover nearby devices before initiating a `ProxyConnectRequest`. The feature
is exposed as cluster commands on the `CommissioningProxy` cluster (endpoint 1).

### Starting a background scan

```
$ ./chip-tool commissioningproxy proxy-back-ground-scan-start-request \
    <Transport> <Timeout> <WiFiBands> \
    <proxy-node-id> <endpoint-id>
```

| Argument        | Description                                                     |
| --------------- | --------------------------------------------------------------- |
| `Transport`     | Transport bitmap: `2` = BLE, `8` = WiFi-PAF                      |
| `Timeout`       | Scan lifetime in seconds; `0` for no expiry                     |
| `WiFiBands`     | Wi-Fi band bitmap: `1` = 2.4 GHz, `4` = 5 GHz, `5` = both       |
| `proxy-node-id` | Node ID of the proxy                                            |
| `endpoint-id`   | Endpoint hosting the CommissioningProxy cluster (typically `1`) |

The proxy starts a NAN subscribe immediately and caches discovered devices in
the `CachedResults` attribute. Results are pushed to subscribers via attribute
reporting. Multiple fabrics may each call start independently; the hardware scan
runs as long as at least one fabric has an active request.

### Example — background WiFi-PAF scan on both bands, no expiry

```
$ ./chip-tool commissioningproxy proxy-back-ground-scan-start-request 8 0 5 0x7ce 1
```

### Stopping a background scan

```
$ ./chip-tool commissioningproxy proxy-back-ground-scan-stop-request \
    <Transport> <WiFiBands> \
    <proxy-node-id> <endpoint-id>
```

The hardware scan stops once no fabrics remain active.

### Reading cached results

```
$ ./chip-tool commissioningproxy read cached-results <proxy-node-id> <endpoint-id>
```

Returns the list of discovered commissionee peer descriptors (address, band,
discriminator) currently held in the proxy's cache.

## Fabric Isolation

Each proxy session is bound to the fabric that created it via
`ProxyConnectRequest`. Attempts by a different fabric to send
`ProxyMessageRequest`, `ProxyDisconnectRequest`, or a `CancelPendingConnect`
(SessionId `0xFFFF`) for a session owned by another fabric are rejected with
`Status::NotFound`.

## Device Tracing

Device tracing is available to analyze performance. Build with RPC enabled and
obtain a tracing JSON file:

```
$ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py \
    -s localhost:33000 \
    -o {OUTPUT_FILE} \
    -t {ELF_FILE} \
    {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```
