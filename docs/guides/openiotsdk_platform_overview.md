# Open IoT SDK platform port

This platform is based on
[Open IoT SDK](https://gitlab.arm.com/iot/open-iot-sdk). Open IoT SDK is a
reference implementation of [Open-CMSIS-CDI](https://www.open-cmsis-cdi.org/)
which defines a common device interface for microcontroller-based devices used
in the Internet of Things. It is delivered as a framework of software components
with a set of feature-rich example applications.

## Building

Open IoT SDK uses CMake as its build system. To integrate with Matter's GN build
system our top level CMakeLists.txt generates GN configuration files that pass
on the required configs required by the GN build.

## Targets

Supported targets are the ones supported by the Open IoT SDK. Currently it ships
with support for
[Corstone-300](https://developer.arm.com/Processors/Corstone-300) and
[Corstone-310](https://developer.arm.com/Processors/Corstone-310). This platform
makes no assumption on the target and will support any targets added to Open IoT
SDK.

## Fast model network

The fast models of supported platforms have two network modes:

-   user mode networking - emulates a built-in IP router and DHCP server, and
    routes TCP and UDP traffic between the guest and host. It uses the user mode
    socket layer of the host to communicate with other hosts. See more details:
    [User mode networking](https://developer.arm.com/documentation/100964/1116/Introduction-to-the-Fast-Models-Reference-Manual/User-mode-networking)

-   TAP/TUN networking mode - set fast model to host bridge component which acts
    as a networking gateway to exchange Ethernet packets with the TAP device on
    the host, and to forward packets to model. See more details
    [TAP/TUN networking mode](https://developer.arm.com/documentation/100964/1116/Introduction-to-the-Fast-Models-Reference-Manual/TAP-TUN-networking)

Due the user mode limitations, the **TAP/TUN networking mode** is preferred for
implementing IP communication for a Matter project.

## RTOS

Open IoT SDK uses
[CMSIS-RTOS2 API](https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS.html)
as its RTOS API. It offers the choice of implementation between FreeRTOS or
CMSIS RTX but this is hidden below the API so your choice has no bearing on this
port and indeed your application may provide your own implementation entirely.

## Connectivity

The platform currently only offers connectivity through the Ethernet interface.
This is limited by current support for network interfaces in Open IoT SDK.

This means that commissioning is simplified since no provisioning is required to
provide the device with network credentials.

LWIP is used in the implementation of endpoints as the IP stack. LWIP library is
provided through the Open IoT SDK.

## Mbed TLS

Mbed TLS is provided through the Open IoT SDK, the Matter version is not used.
Configuration of Mbed TLS is in
[config/openiotsdk/mbedtls](../../config/openiotsdk/mbedtls).

## Storage

Storage in Open IoT SDK is provided by
[TDBStore](https://gitlab.arm.com/iot/open-iot-sdk/storage) which is a simple
Key-Value Storage over a block device.

---

**NOTE**

On the Corstone targets this currently is implemented as a RAM memory region
emulating a Flash device. This does not offer persistence between launches.

---

## Clocks

Open IoT SDK does not currently offer an RTC. Matter configuration has been set
accordingly and real time cannot be read from the system.

Monotonic clocks are available and are based on system tick count. They are
limited by the target configuration. The current targets set the tick to 1 ms.
This becomes the lower bound for timers.

## Drivers

Drivers are provided by
[Reference MCU-Driver-HAL driver implementation for Arm platforms](https://gitlab.arm.com/iot/open-iot-sdk/mcu-driver-hal/mcu-driver-reference-platforms-for-arm)
which is provided by Open IoT SDK.
