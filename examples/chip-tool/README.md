# CHIP Client Example

An example application that uses CHIP to send messages to a CHIP server.

---

-   [CHIP Client Example](#chip-client-example)
-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Request an Echo](#using-the-client-to-request-an-echo)

---

## Building the Example Application

Building the example application is quite straightforward.

### gn

```
cd examples/chip-tool
git submodule update --init
source third_party/connectedhomeip/scripts/activate.sh
gn gen out/debug
ninja -C out/debug
```

-   After the application is built, it can be found in the build directory as
    `out/debug/chip-tool`

## Using the Client to Pair a device

In order to send commands to a device, it must be paired with the client.

#### Pair a device

To initiate a client pairing request to a device, run the built executable and
choose the pairing mode.

##### Pair a device configured to bypass Rendezvous

The command below pair a device with the provided IP address and port of the
server to talk to.

    $ chip-tool pairing bypass 192.168.0.30 11097

#### Pair a device over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device.

The command below uses the default values hard-coded into the debug versions of
the ESP32 all-clusters-app:

    $ chip-tool pairing ble 12345678 3840

### Unpair a device

    $ chip-tool pairing unpair

## Using the Client to Request an Echo

### Ping a device

    $ chip-tool echo ip

If valid values are supplied, it will send a message to the paired device and
verify that the incoming echo from the server matches what was sent out.

## Using the Client to Send CHIP Commands

To use the Client to send a CHIP commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    $ chip-tool onoff on 1

The client will send a single command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ chip-tool

### How to get the list of supported commands for a specific cluster

To get the list of commands for a specific cluster, run the built executable
with the target cluster name.

    $ chip-tool onoff

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    $ chip-tool onoff read

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    $ chip-tool onoff on
