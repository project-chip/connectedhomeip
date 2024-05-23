# CHIP Example IM Application Tutorial

## Introduction

The CHIP IM example application shows you how to implement a CHIP application
program using IM protocols.

CHIP Protocols are, essentially, implementations of specific protocols over the
CHIP transport. Furthermore, when two CHIP nodes are exchanging messages of a
particular CHIP protocol, they do so over a construct called a CHIP Exchange
which is a description of a CHIP-based conversation over a CHIP protocol. A CHIP
Exchange is characterised by the ExchangeContext object, and every CHIP node
must create an ExchangeContext object before initiating a CHIP conversation.

After constructing a CHIP ExchangeContext, CHIP messages are sent and received
using the ChipMessageLayer class which sends the CHIP message over a chosen
transport (TCP, UDP, or MRP).

## Example Applications Walk Through

As part of this example, we have a ChipImInitiator program that acts as the
client and sends echo requests to a ChipImResponder program that receives
InvokeCommandRequests and sends back InvokeCommandResponse messages.

### Test a device over IP

To start the Server in echo mode, run the built executable.

    $ ./chip-im-responder

To start the Client in echo mode, run the built executable and pass it the IP
address of the server to talk to.

    $ ./chip-im-initiator <Server's IPv4 address>

If valid values are supplied, it will begin to periodically send messages to the
server address provided for three times.
