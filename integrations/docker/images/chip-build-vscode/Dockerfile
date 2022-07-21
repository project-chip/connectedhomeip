ARG VERSION=latest
FROM connectedhomeip/chip-build-nrf-platform:${VERSION} AS nrf
FROM connectedhomeip/chip-build-efr32:${VERSION} AS efr32
FROM connectedhomeip/chip-build-android:${VERSION} AS android
FROM connectedhomeip/chip-build-esp32-qemu:${VERSION} as esp32
FROM connectedhomeip/chip-build-mbed-os:${VERSION} AS mbedos
FROM connectedhomeip/chip-build-telink:${VERSION} AS telink
FROM connectedhomeip/chip-build-infineon:${VERSION} AS p6
FROM connectedhomeip/chip-build-tizen:${VERSION} AS tizen
FROM connectedhomeip/chip-build-crosscompile:${VERSION} AS crosscompile
FROM connectedhomeip/chip-build-ameba:${VERSION} AS ameba
FROM connectedhomeip/chip-build-k32w:${VERSION} AS k32w
FROM connectedhomeip/chip-build-imx:${VERSION} AS imx
FROM connectedhomeip/chip-build-ti:${VERSION} AS ti
FROM connectedhomeip/chip-build-zap:${VERSION} AS zap
FROM connectedhomeip/chip-build:${VERSION}

# qemu-src copied over because qemu directory contains symlinks to the src
# folder (notably pc-bios are required to run qemu)
COPY --from=esp32 /opt/espressif/esp-idf /opt/espressif/esp-idf
COPY --from=esp32 /opt/espressif/tools /opt/espressif/tools
COPY --from=esp32 /opt/espressif/qemu /opt/espressif/qemu
COPY --from=esp32 /opt/espressif/qemu-src /opt/espressif/qemu-src

COPY --from=nrf /opt/NordicSemiconductor/nRF5_tools /opt/NordicSemiconductor/nRF5_tools
COPY --from=nrf /opt/NordicSemiconductor/nrfconnect /opt/NordicSemiconductor/nrfconnect

COPY --from=android /opt/android/sdk /opt/android/sdk
COPY --from=android /opt/android/android-ndk-r21b /opt/android/android-ndk-r21b

COPY --from=mbedos /opt/openocd/ /opt/openocd/

COPY --from=p6 /opt/ModusToolbox /opt/ModusToolbox

COPY --from=telink /opt/telink/zephyrproject /opt/telink/zephyrproject
COPY --from=telink /opt/telink/zephyr-sdk-0.13.2 /opt/telink/zephyr-sdk-0.13.2

COPY --from=tizen /opt/tizen-sdk /opt/tizen-sdk

COPY --from=crosscompile /opt/ubuntu-21.04-aarch64-sysroot /opt/ubuntu-21.04-aarch64-sysroot

COPY --from=ameba /opt/ameba /opt/ameba

COPY --from=k32w /opt/sdk /opt/k32w_sdk

COPY --from=imx /opt/fsl-imx-xwayland /opt/fsl-imx-xwayland

COPY --from=ti /opt/ti/sysconfig_1.11.0 /opt/ti/sysconfig_1.11.0

COPY --from=zap /opt/zap /opt/zap

# Android license file "acceping" is done by writing license hashes
# into the 'licenses' subfolder. This allows any user (in particular
# 'vscode' to accept licenses)
RUN set -x \
    && mkdir -p /opt/android/sdk/licenses \
    && chmod -R a+w /opt/android/sdk/licenses \
    && : # last line

# Required for the Tizen SDK
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    zip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && : # last line

ENV AMEBA_PATH=/opt/ameba/ambd_sdk_with_chip_non_NDA
ENV ANDROID_HOME=/opt/android/sdk
ENV ANDROID_NDK_HOME=/opt/android/android-ndk-r21b
ENV CY_TOOLS_PATHS="/opt/ModusToolbox/tools_2.4"
ENV EFR32_BOARD=BRD4161A
ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
ENV IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.10-hardknott/
ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV NXP_K32W0_SDK_ROOT=/opt/k32w_sdk
ENV OPENOCD_PATH=/opt/openocd/
ENV PW_ENVIRONMENT_ROOT=/home/vscode/pigweed/env
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV SYSROOT_AARCH64=/opt/ubuntu-21.04-aarch64-sysroot
ENV TELINK_ZEPHYR_BASE=/opt/telink/zephyrproject/zephyr
ENV TELINK_ZEPHYR_SDK_DIR=/opt/telink/zephyr-sdk-0.13.2
ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.11.0
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb

ENV TIZEN_VERSION 6.0
ENV TIZEN_SDK_ROOT /opt/tizen-sdk
ENV TIZEN_SDK_SYSROOT $TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core
