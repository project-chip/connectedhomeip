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
    -   [Device Tracing](#device-tracing)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/commissioning-proxy-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/commissioning-proxy-app/linux
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

## Commissioning a Device via the Proxy

Once the proxy node is commissioned, you can use `chip-tool pairing proxy` to
commission a Commissionee through the proxy. The proxy will establish a Wi-Fi
PAF (NAN) connection to the Commissionee and tunnel PASE/commissioning packets
between chip-tool and the device.

### Command syntax

```
$ ./chip-tool pairing proxy <node-id> <ssid> <password> <setup-pin-code> <discriminator> <proxy-node-id>
```

| Argument          | Description                                              |
| ----------------- | -------------------------------------------------------- |
| `node-id`         | Node ID to assign to the new device to be commissioned       |
| `ssid`            | Wi-Fi network SSID to configure on the device            |
| `password`        | Wi-Fi network password                                   |
| `setup-pin-code`  | Commissionee's PASE setup PIN code                       |
| `discriminator`   | Commissionee's discriminator                             |
| `proxy-node-id`   | Node ID of the already-commissioned commissioning proxy  |

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
    the Commissionee.
3.  chip-tool sends PASE and commissioning messages via `ProxyMessageRequest`
    — the proxy forwards each message over PAF and returns the response.
4.  Upon commissioning completion, chip-tool sends `ProxyDisconnectRequest` to
    tear down the PAF session.

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
