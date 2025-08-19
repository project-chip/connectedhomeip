# Setup OpenThread Border Router on Raspberry Pi

![OTBR](https://openthread.io/codelabs/openthread-border-router/img/699d673d05a55535.png)

A Thread Border Router connects a Thread network to other IP-based networks,
such as Wi-Fi or Ethernet. A Thread network requires a Border Router to connect
to other networks. A Thread Border Router minimally supports the following
functions:

-   Bidirectional IP connectivity between Thread and Wi-Fi/Ethernet networks.
-   Bidirectional service discovery via mDNS (on Wi-Fi/Ethernet link) and SRP
    (on Thread network).
-   Thread-over-infrastructure that merges Thread partitions over IP-based
    links.
-   External Thread Commissioning (for example, a mobile phone) to authenticate
    and join a Thread device to a Thread network.

[OpenThread Border Router](https://openthread.io/guides/border-router) (OTBR)
released by Google is an open-source implementation of the Thread Border Router.

## Tutorials

Follow the
[CodeLab tutorial on openthread.io](https://openthread.io/codelabs/openthread-border-router)
to set up your OpenThread Border Router on a Raspberry Pi 3 or Raspberry Pi 4
device. Ubuntu and debian hosts are also supported. The CodeLab uses Nordic
Semiconductor's nRF52840 DK as the Border Router RCP device.

-   See [this guide](./openthread_rcp_nrf_dongle.md) for configuring the
    nRF52840 Dongle as an RCP device.
-   See
    [this guide](https://www.silabs.com/documents/public/application-notes/an1256-using-sl-rcp-with-openthread-border-router.pdf)
    for creating Silicon Labs RCP devices.
