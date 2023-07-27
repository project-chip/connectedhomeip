# Setup ESP-IDF and Matter Environment

Setting up IDF and Matter environment is one time setup.

## Setup ESP-IDF (Espressif IoT Development Framework)

Building the example application requires the use of the ESP-IDF and the
xtensa-esp32-elf toolchain for ESP32 modules or the riscv-esp32-elf toolchain
for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step.

### Install Prerequisites

-   [Linux](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/get-started/linux-macos-setup.html#for-linux-users)
-   [macOS](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/get-started/linux-macos-setup.html#for-macos-users)

### Get IDF v5.1

-   Clone ESP-IDF
    [v5.1 release](https://github.com/espressif/esp-idf/releases/tag/v5.1)

    ```
    $ git clone -b v5.1 --recursive --depth 1 --shallow-submodule https://github.com/espressif/esp-idf.git
    $ cd esp-idf
    $ ./install.sh
    ```

-   To update an existing esp-idf toolchain to v5.1:

    ```
    $ cd path/to/esp-idf
    $ git fetch origin
    $ git checkout v5.1
    $ git reset --hard origin/v5.1
    $ git submodule update --recursive --init
    $ git clean -fdx
    $ ./install.sh
    ```

## Setup Matter environment

### Install Prerequisites

-   [Linux](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#installing-prerequisites-on-linux)
-   [macOS](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#installing-prerequisites-on-macos)

### Bootstrap Matter environment

Execute the below command to bootstrap Matter environment. This script takes
care of downloading GN, ninja, and setting up a Python environment with
libraries used to build and test.

```
source scripts/bootstrap.sh
```

Whenever Matter environment is out of date, it can be updated by running above
command.

In IDF v4.4.x, `esptool` is part of the esp-idf repository, but in IDF v5.x, it
is moved out as a Python package which can be installed using pip.

If you are using IDF v5.x or later, please install `esptool` using the command
below:

```
# Please make sure to run this command in the Matter Python environment
python3 -m pip install esptool
```

For MacOS, `gdbgui` python package will not be installed using `bootstrap.sh`
script as it is restricted only for x64 Linux platforms. It is restricted
because, building wheels for `gevent` (dependency of `gdbgui`) fails on MacOS.

For ARM-based Mac, no further installation steps are necessary if Python3
version is greater than or equal to 3.11.

If Python3 version is less than 3.11 or you are using x86(Intel-based) Mac then
please run the below commands after every bootstrapping to install gdbgui wheels
as binary

```
python3 -m pip install -c scripts/setup/constraints.txt --no-cache --prefer-binary gdbgui==0.13.2.0
deactivate
```

---

Once IDF and Matter environment is set up, head over to
[application building and commissioning guide](build_app_and_commission.md).
