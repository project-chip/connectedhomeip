# Setup ESP-IDF and CHIP Environment

Setting up IDF and CHIP environment is one time setup.

## Setup ESP-IDF (Espressif IoT Development Framework)

Building the example application requires the use of the ESP-IDF and the
xtensa-esp32-elf toolchain for ESP32 modules or the riscv-esp32-elf toolchain
for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step.

### Install Prerequisites

-   [Linux](https://docs.espressif.com/projects/esp-idf/en/v4.4.2/esp32/get-started/linux-setup.html)
-   [macOS](https://docs.espressif.com/projects/esp-idf/en/v4.4.2/esp32/get-started/macos-setup.html)

### Get IDF v4.4.2

-   Clone ESP-IDF
    [v4.4.2 release](https://github.com/espressif/esp-idf/releases/tag/v4.4.2)

    ```
    $ git clone -b v4.4.2 --recursive https://github.com/espressif/esp-idf.git
    $ cd esp-idf
    $ ./install.sh
    ```

-   To update an existing esp-idf toolchain to v4.4.2:

    ```
    $ cd path/to/esp-idf
    $ git fetch origin
    $ git checkout v4.4.2
    $ git reset --hard origin/v4.4.2
    $ git submodule update --recursive --init
    $ git clean -fdx
    $ ./install.sh
    ```

-   For ESP32H2, please checkout tag
    [v5.0-beta1](https://github.com/espressif/esp-idf/tree/v5.0-beta1),
    currently only lighting-app is supported on H2

    ```
    $ cd esp-idf
    $ git checkout v5.0-beta1
    $ git submodule update --init
    $ ./install.sh
    ```

## Setup CHIP environment

### Install Prerequisites

-   [Linux](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#installing-prerequisites-on-linux)
-   [macOS](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#installing-prerequisites-on-macos)

### Prepare for building

Before running any other build command, the `scripts/activate.sh` environment
setup script should be sourced at the top level. This script takes care of
downloading GN, ninja, and setting up a Python environment with libraries used
to build and test.

```
source scripts/activate.sh
```

If this script says the environment is out of date, it can be updated by
running:

```
source scripts/bootstrap.sh
```

---

Once IDF and CHIP environment is set up, head over to
[application building and commissioning guide](build_app_and_commission.md).
