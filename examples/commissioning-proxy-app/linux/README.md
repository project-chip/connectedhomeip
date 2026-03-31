# CHIP Linux Commissioning Proxy Example

An example showing the use of the Matter Commissioning Proxy cluster on Linux.
The Commissioning Proxy (CP) acts as a tunnel between a Commissioner (chip-tool)
and a Commissionee that is reachable via a transport not available on the
Commissioner — for example, Wi-Fi Aware (PAF).

This document describes how to build and run the Commissioning Proxy app and
how to commission a device through it using chip-tool.

<hr>

-   [CHIP Linux Commissioning Proxy Example](#chip-linux-commissioning-proxy-example)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Commissioning Proxy](#running-the-commissioning-proxy)
    -   [Commissioning a Device via the Proxy](#commissioning-a-device-via-the-proxy)
    -   [Background Scanning](#background-scanning)
    -   [Device Tracing](#device-tracing)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application (from the repo root):

          $ cd examples/commissioning-proxy-app/linux
          $ source ../../../scripts/bootstrap.sh
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

    To omit RPC support, replace the `--args` value with `--args=''` or omit it entirely.

-   To delete generated executable, libraries and object files use:

          $ cd examples/commissioning-proxy-app/linux
          $ rm -rf out/

## Commandline arguments

-   `--wifi`

    Enables WiFi management feature. Required for Wi-Fi PAF commissioning.

-   `--discriminator <value>`

    Set the discriminator used to identify this proxy node during commissioning.

-   `--passcode <value>`

    Set the PASE passcode for commissioning the proxy node itself.

-   `--app-pipe <path>`

    Path of a named pipe used to send commands to the running app. Optional.

-   `--allow-large-payload`

    Enable TCP transport for large payload support (recommended when chip-tool
    connects over TCP).

## Running the Commissioning Proxy

The proxy app must first be commissioned onto the fabric before it can proxy
commissioning for other devices. Start the proxy app on a node that has Wi-Fi
Aware (NAN) capability:

```
$ ./out/debug/chip-commissioning-proxy-app --wifi --allow-large-payload
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
$ ./chip-tool pairing proxy <node-id> <ssid> <password> <setup-pin-code> <discriminator> <proxy-node-id> [--proxy-connect-timeout <seconds>]
```

| Argument                  | Description                                                        |
| ------------------------- | ------------------------------------------------------------------ |
| `node-id`                 | Node ID to assign to the new device                                |
| `ssid`                    | Wi-Fi network SSID to configure on the device                      |
| `password`                | Wi-Fi network password                                             |
| `setup-pin-code`          | Commissionee's PASE setup PIN code                                 |
| `discriminator`           | Commissionee's discriminator                                       |
| `proxy-node-id`           | Node ID of the already-commissioned commissioning proxy            |
| `--proxy-connect-timeout` | Timeout in seconds for `ProxyConnectRequest` (default: 30 s)       |

### Example

```
$ ./chip-tool pairing proxy 1999 "MyNetwork" "MyPassword" 20202021 3840 0x7ce
```

This commissions device node 1999 via the proxy at node 0x7ce onto the Wi-Fi
network "MyNetwork".

### Network topology

```
chip-tool  <-- TCP/IP -->  Commissioning Proxy App  <-- Wi-Fi PAF (NAN) -->  Commissionee
```

1.  chip-tool establishes a CASE session to the commissioning proxy.
2.  chip-tool sends `ProxyConnectRequest` — the proxy opens a PAF session to
    the Commissionee.  If the PAF session is not established within the connect
    timeout, the proxy cancels the NAN subscribe and returns `Status::Timeout`.
3.  chip-tool sends PASE and commissioning messages via `ProxyMessageRequest`
    — the proxy forwards each message over PAF and returns the response.
4.  When the Commissionee connects to Wi-Fi, the proxy defers NAN publisher
    teardown until the PAFTP send queue is drained and all outstanding fragment
    acks have been received, then cancels the NAN publisher sessions.  This
    releases the radio for IPv6 DAD and mDNS, reducing post-WiFi-join mDNS
    latency from ~35 s to ~3 s.
5.  Upon commissioning completion, chip-tool sends `ProxyDisconnectRequest` to
    tear down the PAF session.

## Background Scanning

The proxy supports continuous background NAN scanning so that commissioners can
discover nearby devices before initiating a `ProxyConnectRequest`.  The feature
is exposed as cluster commands on the `CommissioningProxy` cluster (endpoint 1).

### Starting a background scan

```
$ ./chip-tool commissioning-proxy proxy-back-ground-scan-start-request \
    <Transport> <Timeout> <WiFiBands> \
    <proxy-node-id> <endpoint-id>
```

| Argument      | Description                                                                   |
| ------------- | ----------------------------------------------------------------------------- |
| `Transport`   | Transport bitmap (e.g. `1` for WiFiPAF)                                       |
| `Timeout`     | Scan lifetime in seconds; `0` for no expiry                                   |
| `WiFiBands`   | Wi-Fi band bitmap (e.g. `1` for 2.4 GHz, `2` for 5 GHz, `3` for both)        |
| `proxy-node-id` | Node ID of the proxy                                                        |
| `endpoint-id` | Endpoint hosting the CommissioningProxy cluster (typically `1`)               |

The proxy starts a NAN subscribe immediately and caches discovered devices in
the `CachedResults` attribute.  Results are pushed to subscribers via attribute
reporting.  Multiple fabrics may each call start independently; the hardware
scan runs as long as at least one fabric has an active request.

### Stopping a background scan

```
$ ./chip-tool commissioning-proxy proxy-back-ground-scan-stop-request \
    <Transport> <WiFiBands> \
    <proxy-node-id> <endpoint-id>
```

The hardware scan stops once no fabrics remain active.  If the requesting
`fabric-index` / `node-id` pair does not match the original requester the
command is silently ignored (per spec §10.5.7.7).

### Reading cached results

```
$ ./chip-tool commissioning-proxy read cached-results <proxy-node-id> <endpoint-id>
```

Returns the list of discovered commissionee peer descriptors (address, band,
discriminator) currently held in the proxy's cache.

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
