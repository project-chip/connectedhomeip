# CHIP Tests on Device

An appplication that runs CHIP's unit tests on ESP32 device or QEMU.

---

-   [CHIP Tests on Device](#chip-tests-on-device)
    -   [Supported Devices](#supported-devices)
    -   [Building the Application](#building-the-application)
        -   [To build the application, follow these steps:](#to-build-the-application-follow-these-steps)
            -   [Using QEMU](#using-qemu)
            -   [Using DevKitC or M5Stack](#using-devkitc-or-m5stack)

---

## Supported Devices

The CHIP application is intended to work on
[ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview),
[QEMU](https://github.com/espressif/qemu), and the
[M5Stack](http://m5stack.com). Support for the [M5Stack](http://m5stack.com) is
still a Work in Progress.

## Building the Application

Building the application requires the use of the Espressif ESP32 IoT Development
Framework and the xtensa-esp32-elf toolchain.

The `chip-build` Docker container and VSCode devcontainer has these components
pre-installed, so you can skip this step. To install these components manually,
follow these steps:

-   Clone the Expressif ESP-IDF and checkout version 4.0

          $ mkdir -p ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout release/v4.0
          $ git submodule update --init
          $ export IDF_PATH=${HOME}/tools/esp-idf
          $ ./install.sh

-   Clone and build ESP32 QEMU

          $ mkdir -p ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone --progress https://github.com/espressif/qemu.git qemu_esp32
          $ cd qemu_esp32
          $ ./configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone    $ --disable-vnc --disable-sdl --disable-gtk
          $ make -j8
          $ export QEMU_ESP32=${HOME}/tools/qemu_esp32/xtensa-softmmu/qemu-system-xtensa

### To build the application, follow these steps:

#### Using QEMU

-   In the root of the example directory, source `idf.sh` and use the
    `defconfig` make target to configure the application with defaults.

          $ source idf.sh
          $ SDKCONFIG_DEFAULTS=sdkconfig_qemu.defaults idf make defconfig

-   Run make to build the application

          $ idf make

-   Build the flash image for QEMU and run the application.

          $ ../../../scripts/tools/build_esp32_flash_image.sh ./build/chip-crypto-tests.bin test.bin
          $ ../../../scripts/tools/esp32_qemu_run.sh ./test.bin

#### Using DevKitC or M5Stack

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   In the root of the example directory, source `idf.sh` and use the
    `defconfig` make target to configure the application with defaults.

          $ source idf.sh
          $ idf make defconfig

-   Run make to build the application

          $ idf make

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing.

          $  make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.
