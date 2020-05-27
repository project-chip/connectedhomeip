# CHIP Echo Client Example

An example application that uses CHIP to send messages to a CHIP echo server.

---

-   [CHIP Echo Client Example](#chip-wifi-echo-client-example)
-   [Building the Example Application](#building-the-example-application)
-   [Using the Echo Client](#using-the-echo-client)

---

## Building the Example Application

Building the example application is quite straightforward.

-   In the root of the example directory, run `make`.

          $ make

-   After the application is built, it can be found in the build directory as
    `chip-standalone-demo.out`

## Using the Echo Client

To start the Echo Client run the built executable and pass it the IP address and
port of the server to talk to.

          $ ./build/chip-standalone-demo.out 192.168.0.30 8000

If valid values are supplied, it will begin to periodically send messages to the
server address provided.

It also verifies that the incoming echo from the server matches what was sent
out.

Stop the Client at anytime with `Ctrl + C`.
