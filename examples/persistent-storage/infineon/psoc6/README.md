# CHIP PSoC6 Persistent Storage Example

An example testing and demonstrating the key value storage API.

<hr>

-   [CHIP PSoC6 Persistent Storage Example](#chip-PSoC6-persistent-storage-example)
    -   [Introduction](#introduction)
    -   [PSoC6](#PSoC6)
        -   [Building](#building)
        -   [Flashing the Application](#flashing-the-application)

<hr>

<a name="intro"></a>

## Introduction

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

<a name="PSoC6"></a>

## PSoC6

The Infineon PSoC6 platform KVS is fully implemented, the KVS is enabled and
configured by providing a file during the init call.

<a name="building"></a>

### Building

-   Build the example application:

          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ ./scripts/examples/gn_psoc6_example.sh examples/persistent-storage/infineon/psoc6 out/persistent_storage_app_psoc6

<a name="flashing"></a>

### Flashing the Application

-   Put CY8CKIT-062S2-43012 board on KitProg3 CMSIS-DAP Mode by pressing the
    `MODE SELECT` button. `KITPROG3 STATUS` LED is ON confirms board is in
    proper mode.

-   On the command line:

          $ cd ~/connectedhomeip
          $ python3 out/persistent_storage_app_psoc6/chip-psoc6-persistent-storage-example.flash.py
