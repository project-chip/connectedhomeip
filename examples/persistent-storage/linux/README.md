# CHIP Linux Persistent Storage Example

An example testing and demonstrating the key value storage API.

<hr>

-   [CHIP Linux Persistent Storage Example](#chip-linux-persistent-storage-example)
    -   [Introduction](#introduction)
    -   [Linux](#linux)
        -   [Building](#building)
        -   [Running](#running)

<hr>

## Introduction

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

## Linux

The Linux platform KVS is fully implemented, the KVS is enabled and configured
by providing a file during the init call.

### Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/persistent-storage/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

### Running

-   Run Linux Example

            $ cd ~/connectedhomeip/examples/persistent-storage/linux
            $ sudo out/debug/persistent_storage
