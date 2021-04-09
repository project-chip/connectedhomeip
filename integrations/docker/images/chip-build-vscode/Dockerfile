ARG VERSION=latest
FROM connectedhomeip/chip-build-nrf-platform:${VERSION} AS nrf
FROM connectedhomeip/chip-build-efr32:${VERSION} AS efr32
FROM connectedhomeip/chip-build-android:${VERSION} AS android
FROM connectedhomeip/chip-build-esp32-qemu:${VERSION} as esp32
FROM connectedhomeip/chip-build-mbed-os:${VERSION} AS mbedos
FROM connectedhomeip/chip-build:${VERSION}
COPY --from=esp32 /opt/espressif/esp-idf /opt/espressif/esp-idf
COPY --from=esp32 /opt/espressif/tools /opt/espressif/tools
COPY --from=esp32 /opt/espressif/qemu /opt/espressif/qemu
COPY --from=nrf /opt/NordicSemiconductor/nRF5_tools /opt/NordicSemiconductor/nRF5_tools
COPY --from=nrf /opt/NordicSemiconductor/nrfconnect /opt/NordicSemiconductor/nrfconnect
COPY --from=nrf /opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major /opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major
COPY --from=android /opt/android/sdk /opt/android/sdk
COPY --from=android /opt/android/android-ndk-r21b /opt/android/android-ndk-r21b
COPY --from=mbedos /opt/mbed-os /opt/mbed-os
COPY --from=mbedos /opt/mbed-os-toolchain/ /opt/mbed-os-toolchain/
ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
ENV GNUARMEMB_TOOLCHAIN_PATH=/opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major
ENV ARM_GCC_INSTALL_ROOT=/opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major/bin
ENV EFR32_BOARD=BRD4161A
ENV ANDROID_HOME=/opt/android/sdk
ENV ANDROID_NDK_HOME=/opt/android/android-ndk-r21b
ENV MBED_GCC_ARM_PATH=/opt/mbed-os-toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin/
ENV MBED_OS_PATH=/opt/mbed-os/
ENV PW_ENVIRONMENT_ROOT=/home/vscode/pigweed/env
