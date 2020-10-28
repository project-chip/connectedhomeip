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

## Using the Client to Request an Echo

### Ping a device over BLE

To initiate a client echo request to a BLE device, run the built executable and
pass it the discriminator and pairing code of the remote device. The command
below uses the default values hard-coded into the debug versions of the ESP32
wifi-echo app:

    $ chip-tool echo ble 12345678 3840

### Ping a device over IP

To start the Client in echo mode, run the built executable and pass it the IP
address and port of the server to talk to, as well as the command "echo".

    $ chip-tool echo ip 192.168.0.30 11095

If valid values are supplied, it will begin to periodically send messages to the
server address provided.

It also verifies that the incoming echo from the server matches what was sent
out.

Stop the Client at any time with `Ctrl + C`.

## Using the Client to Send CHIP Commands

To use the Client to send a CHIP commands, run the built executable and pass it
the target cluster name, the target command name, the ip address and port of the
server to talk to as well as an endpoint id. The endpoint id must be between 1
and 240.

    $ chip-tool onoff on 192.168.0.30 11095 1

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
