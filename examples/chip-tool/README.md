# CHIP Client Example

An example application that uses CHIP to send messages to a CHIP server.

---

-   [CHIP Client Example](#chip-client-example)
-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Request an Echo](#using-the-client-to-request-an-echo)

---

## Building the Example Application

Building the example application is quite straightforward.

-   In the root of the example directory, run `make`.

          $ make

-   After the application is built, it can be found in the build directory as
    `chip-standalone-demo.out`

## Using the Client to Request an Echo

To start the Client in echo mode, run the built executable and pass it the IP
address and port of the server to talk to, as well as the command "echo".

          $ ./build/chip-standalone-demo.out 192.168.0.30 8000 echo

If valid values are supplied, it will begin to periodically send messages to the
server address provided.

It also verifies that the incoming echo from the server matches what was sent
out.

Stop the Client at any time with `Ctrl + C`.

## Using the Client to Send CHIP Commands

To use the Client to send a CHIP comands, run the built executable and pass it
the IP address and port of the server to talk to, the name of the command to
send, as well as an enpoint id. Right now the "off", "on", and "toggle" commands
are supported, from the On/Off cluster. The endpoint id must be between 1
and 240.

          $ ./build/chip-standalone-demo.out 192.168.0.30 8000 on 1

The client will send a single command packet and then exit.
