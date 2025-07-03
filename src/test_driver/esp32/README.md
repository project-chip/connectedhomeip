# Matter Tests on Device

An application that runs Matter's unit tests on ESP32 device or QEMU.

---

-   [Matter Tests on Device](#chip-tests-on-device)
    -   [Supported Devices](#supported-devices)
    -   [Requirements](#requirements)
    -   [Building Unit Tests](#building-unit-tests)
    -   [Running Unit Tests](#running-unit-tests)

---

## Supported Devices

The Matter application is intended to work on
[ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview),
[QEMU](https://github.com/espressif/qemu), and the
[M5Stack](http://m5stack.com). Support for the [M5Stack](http://m5stack.com) is
still a Work in Progress.

## Requirements

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

## Building Unit Tests

To build all unit tests:

    $ source scripts/activate.sh
    $ scripts/build/build_examples.py --target esp32-qemu-tests build

This generates a set of QEMU images in `out/esp32-qemu-tests/`

There is one image for each test directory (i.e. each chip_test_suite). So for
example `src/inet/tests` builds to `out/esp32-qemu-tests/testInetLayer.img`

The file `out/esp32-qemu-tests/test_images.txt` contains the names of all the
images that were built.

## Running Unit Tests

To run all unit test images using QEMU:

    $ src/test_driver/esp32/run_qemu_image.py --verbose --file-image-list out/esp32-qemu-tests/test_images.txt

To run a single unit test image, such as `testInetLayer.img`:

    $ src/test_driver/esp32/run_qemu_image.py --verbose --image out/esp32-qemu-tests/testInetLayer.img
