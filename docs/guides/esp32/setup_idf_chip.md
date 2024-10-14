# Setup ESP-IDF and Matter Environment

Setting up IDF and Matter environment is one time setup.

## Setup ESP-IDF (Espressif IoT Development Framework)

Building the example application requires the use of the ESP-IDF and the
xtensa-esp32-elf toolchain for ESP32 modules or the riscv-esp32-elf toolchain
for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step.

### Install Prerequisites

-   [Linux](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32/get-started/linux-macos-setup.html#for-linux-users)
-   [macOS](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32/get-started/linux-macos-setup.html#for-macos-users)

### Get ESP-IDF v5.3

-   Clone ESP-IDF [v5.3
    release](https://github.com/espressif/esp-idf/releases/tag/v5.3

    ```
    git clone -b v5.3 --recursive --depth 1 --shallow-submodule https://github.com/espressif/esp-idf.git
    cd esp-idf
    ./install.sh
    ```

-   To update an existing esp-idf toolchain to v5.3:

    ```
    cd path/to/esp-idf
    git fetch --depth 1 origin v5.3
    git reset --hard FETCH_HEAD
    git submodule update --depth 1 --recursive --init

    # -ff is for cleaning untracked files as well as submodules
    git clean -ffdx
    ./install.sh
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

---

Once IDF and Matter environment is set up, head over to
[application building and commissioning guide](build_app_and_commission.md).
