# Building Your Wi-Fi Matter End Device and the Chip-Tool

# Build Environment for Linux

This section will go through the steps required to build the chip-tool for Linux.

> **Do not execute any commands on this page as ROOT (no _su_ required), unless
> specified**

<br>

## Prepare Linux Packages

Update the latest packages by typing following commands in terminal:

```shell
$ sudo apt update
$ sudo apt install
```
<br>

## Prerequisites for Matter (CHIP) project on Linux

### 1. Installing packages on Ubuntu Laptop/PC

-   Open the Linux terminal from Start menu
-   Install required packages on Ubuntu Laptop/PC using the following commands:

    ```shell
    $ sudo apt install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
    libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
    python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
    ```
    <br>
### 2. Building Enviorment

-    To build enviorment follow the `Software setup` and `Compiling chip-tool` steps given in [SW_SETUP](./SW_SETUP.md),

<br>

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
