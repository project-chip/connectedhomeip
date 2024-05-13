# Configuring OpenThread Radio Co-processor on nRF52840 Dongle

OpenThread Radio Co-processor design allows to add Thread network capabilities
to devices that do not natively support Thread.

You can build and program
[OpenThread Radio Co-Processor](https://openthread.io/platforms/co-processor)
(RCP) firmware onto Nordic Semiconductor's
[nRF52840 Dongle](https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52840-Dongle).
Once programmed, the dongle can be used for
[configuring Thread network on a Linux machine](./openthread_border_router_pi.md).

## Requirements

You need to set up the
[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)
to build and program the OpenThread Radio Co-Processor to the nRF52840 Dongle.
Read the
[nRF Connect SDK Getting started](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html)
documentation for more information.

## Building and programming the RCP firmware onto an nRF52840 Dongle

After you set up the nRF Connect SDK, follow the steps in the
[Configuring a radio co-processor](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/ug_thread_tools.html#configuring-a-radio-co-processor)
section for the nRF52840 Dongle (USB transport).
