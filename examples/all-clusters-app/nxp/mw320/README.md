# CHIP MW320 Lighting Example Application

The Matter MW320 Lighting Example demonstrates how to remotely control a light
bulb. The light bulb is simulated using one of the MW320 platform LED. It uses
buttons to test turn on/turn off of the light bulb. You can use this example as
a reference for creating your own application.

The example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and the NXP MW320 SDK,
and supports remote access and control of a light bulb over a low-power, WiFi
802.11n network.

The example behaves as a Project CHIP accessory, that is a device that can be
used bypass paired into an existing Matter network and can be controlled by this
network.

<hr>

-   [CHIP MW320 Lighting Example Application](#chip-mw320-lighting-example-application) -
-   [Introduction](#introduction)
-   [Building](#building)
-   [Flashing and debugging](#flashdebug)
-   [Testing the example](#testing-the-example)

<hr>

<a name="intro"></a>

## Introduction

![MW320](../../platform/mw320/doc/images/mw320.jpg)

The MW320 lighting example application provides a working demonstration of a
light led device, built using the Matter code base and the NXP MW320 SDK. The
example supports remote access (e.g.: using Matter chip Tool from a Ubuntu
laptop) and control of a led light over a low-power, WiFi 802.11n network. It is
capable of being paired into an existing Matter network along with other
Matter-enabled devices.

The example targets the
[NXP MW320 WiFi Micro controller Soc](https://www.nxp.com/products/wireless/wi-fi-plus-bluetooth/88mw32x-802-11n-wi-fi-microcontroller-soc:88MW32X)
development kit.

The Matter device that runs the lighting application is controlled by the CHIP
controller device over the WiFi 802.11 protocol. By default, the CHIP device has
Soft-AP enable with SSID:matter_mw320 WPA2:12345678, and it should be connect
over WiFi network and bypass pairing from other chip tool base on Project CHIP
test_event_6 code base. The actions required before establishing full
communication are described below.

The example also comes with a test mode, which allows to start WiFi Soft-AP with
the default settings. However, this mode does not guarantee that the device will
be able to communicate with the Matter controller and other devices.

<a name="building"></a>

## Building

<a name="flashdebug"></a>

## Flashing and debugging

Connected to Universal Asynchronous Receiver/Transmitter port on MW320 platform
to Ubuntu 20 USB port and open Linux text-based serial port communications
program at second USB interface. ex. /dev/ttyUSB1.

## Testing the example

![MW320 LED ON](../../platform/mw320/doc/images/mw320_on.jpg)
