# CHIP Tests on Device

An application that runs CHIP's unit tests on ESP32 device or QEMU.

---

-   [CHIP Tests on Device](#chip-tests-on-device)
    -   [Supported Devices](#supported-devices)
    -   [Building the Application](#building-the-application)
        -   [To build the application, follow these steps:](#to-build-the-application-follow-these-steps)
            -   [Using QEMU](#using-qemu)

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

-   Clone the Espressif ESP-IDF and checkout
    [v4.3 tag](https://github.com/espressif/esp-idf/releases/v4.3)

          $ mkdir -p ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.3
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

-   Setup ESP32 QEMU. This will build QEMU and install necessary artifacts to
    run unit tests.

          ```
          source idf.sh
          ./qemu_setup.sh
          ```

-   Run specific unit tests

          ```
          idf make -C build/chip/src/crypto/tests check
          ```
