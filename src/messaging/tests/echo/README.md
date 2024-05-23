# CHIP Example Application Tutorial

## Introduction

The CHIP Echo example application shows you how to implement a CHIP application
program using one of the supported CHIP protocols, namely the very simple CHIP
Echo protocol. This protocol allows you to send a CHIP message to a peer and
expect a CHIP response (similar to the ICMP Echo Request/Echo Response
messages).

CHIP Protocols are, essentially, implementations of specific protocols over the
CHIP transport. Furthermore, when two CHIP nodes are exchanging messages of a
particular CHIP protocol, they do so over a construct called a CHIP Exchange
which is a description of a CHIP-based conversation over a CHIP protocol. A CHIP
Exchange is characterised by the ExchangeContext object, and every CHIP node
must create an ExchangeContext object before initiating a CHIP conversation.

After constructing a CHIP ExchangeContext, CHIP messages are sent and received
using the ChipMessageLayer class which sends the CHIP message over a chosen
transport (TCP, UDP, or MRP).

## Building

```
source scripts/activate.sh
gn gen out/debug
ninja -C out/debug
```

-   After the applications are built, it can be found in the build directory as
    `out/debug/chip-echo-requester and out/debug/chip-echo-responder`

## Example Applications Walk Through

As part of this example, we have a ChipEchoRequester program that acts as the
client and sends echo requests to a ChipEchoResponder program that receives
EchoRequests and sends back EchoResponse messages.

### Ping a device over IP

To start the Server in echo mode, run the built executable.

    $ ./chip-echo-responder [--tcp]

To start the Client in echo mode, run the built executable and pass it the IP
address of the server to talk to.

    $ ./chip-echo-requester <Server's IPv4 address> [--tcp]

If valid values are supplied, it will begin to periodically send messages to the
server address provided for three times.
