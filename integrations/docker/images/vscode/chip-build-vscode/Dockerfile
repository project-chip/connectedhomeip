ARG VERSION=1

FROM ghcr.io/project-chip/chip-build-nrf-platform:${VERSION} AS nrf
FROM ghcr.io/project-chip/chip-build-efr32:${VERSION} AS efr32
FROM ghcr.io/project-chip/chip-build-android:${VERSION} AS android
FROM ghcr.io/project-chip/chip-build-esp32-qemu:${VERSION} as esp32
FROM ghcr.io/project-chip/chip-build-mbed-os:${VERSION} AS mbedos
FROM ghcr.io/project-chip/chip-build-telink:${VERSION} AS telink
FROM ghcr.io/project-chip/chip-build-infineon:${VERSION} AS p6
FROM ghcr.io/project-chip/chip-build-tizen:${VERSION} AS tizen
FROM ghcr.io/project-chip/chip-build-crosscompile:${VERSION} AS crosscompile
FROM ghcr.io/project-chip/chip-build-ameba:${VERSION} AS ameba
FROM ghcr.io/project-chip/chip-build-k32w:${VERSION} AS k32w
FROM ghcr.io/project-chip/chip-build-rw61x:${VERSION} AS rw61x
FROM ghcr.io/project-chip/chip-build-imx:${VERSION} AS imx
FROM ghcr.io/project-chip/chip-build-ti:${VERSION} AS ti
FROM ghcr.io/project-chip/chip-build-openiotsdk:${VERSION} AS openiotsdk
FROM ghcr.io/project-chip/chip-build-bouffalolab:${VERSION} AS bouffalolab
FROM ghcr.io/project-chip/chip-build-asr:${VERSION} AS asr
FROM ghcr.io/project-chip/chip-build-nuttx:${VERSION} AS nuttx
FROM ghcr.io/project-chip/chip-build:${VERSION}

LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# qemu-src copied over because qemu directory contains symlinks to the src
# folder (notably pc-bios are required to run qemu)
COPY --from=esp32 /opt/espressif/esp-idf /opt/espressif/esp-idf
COPY --from=esp32 /opt/espressif/tools /opt/espressif/tools
COPY --from=esp32 /opt/espressif/qemu /opt/espressif/qemu
COPY --from=esp32 /opt/espressif/qemu-src /opt/espressif/qemu-src

COPY --from=nrf /opt/NordicSemiconductor/nRF5_tools /opt/NordicSemiconductor/nRF5_tools
COPY --from=nrf /opt/NordicSemiconductor/nrfconnect /opt/NordicSemiconductor/nrfconnect

COPY --from=android /opt/android/sdk /opt/android/sdk
COPY --from=android /opt/android/android-ndk-r23c /opt/android/android-ndk-r23c
COPY --from=android /usr/lib/kotlinc /usr/lib/kotlinc

# FIXME: OpenOCD was removed during the move to infineon's repo
# COPY --from=mbedos /opt/openocd/ /opt/openocd/

COPY --from=p6 /opt/ModusToolbox /opt/ModusToolbox

COPY --from=telink /opt/telink/zephyrproject /opt/telink/zephyrproject
COPY --from=telink /opt/telink/zephyr-sdk-0.16.1 /opt/telink/zephyr-sdk-0.16.1

COPY --from=tizen /opt/tizen-sdk /opt/tizen-sdk

COPY --from=crosscompile /opt/ubuntu-22.04.1-aarch64-sysroot /opt/ubuntu-22.04.1-aarch64-sysroot

COPY --from=ameba /opt/ameba /opt/ameba

COPY --from=k32w /opt/sdk /opt/k32w

COPY --from=rw61x /opt/sdk /opt/nxp-sdk

COPY --from=imx /opt/fsl-imx-xwayland /opt/fsl-imx-xwayland

COPY --from=ti /opt/ti/sysconfig_1.16.2 /opt/ti/sysconfig_1.16.2

COPY --from=openiotsdk /opt/FVP_Corstone_SSE-300/ /opt/FVP_Corstone_SSE-300/

COPY --from=bouffalolab /opt/bouffalolab_sdk /opt/bouffalolab_sdk

COPY --from=asr /opt/asr /opt/asr

COPY --from=efr32 /opt/silabs/gecko_sdk /opt/silabs/gecko_sdk
COPY --from=efr32 /opt/silabs/wiseconnect-wifi-bt-sdk /opt/silabs/wiseconnect-wifi-bt-sdk
COPY --from=efr32 /opt/silabs/wifi_sdk /opt/silabs/wifi_sdk

COPY --from=nuttx /opt/nuttx /opt/nuttx

# Android license file "acceping" is done by writing license hashes
# into the 'licenses' subfolder. This allows any user (in particular
# 'vscode' to accept licenses)
RUN set -x \
    && mkdir -p /opt/android/sdk/licenses \
    && chmod -R a+w /opt/android/sdk/licenses \
    && : # last line

# Required for the Tizen SDK:
#   - zip
# Required for the Open IoT SDK platform
#   - expect
#   - telnet
#   - srecord
# For java builds:
#   - openjdk-8-jdk
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    zip \
    expect \
    telnet \
    srecord \
    openjdk-8-jdk \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Required for the Bouffalolab platform
RUN set -x \
    && pip3 install bflb-iot-tool==1.8.6 \
    && : # last line

ENV PATH $PATH:/usr/lib/kotlinc/bin
ENV AMEBA_PATH=/opt/ameba/ambd_sdk_with_chip_non_NDA
ENV ANDROID_HOME=/opt/android/sdk
ENV ANDROID_NDK_HOME=/opt/android/android-ndk-r23c
ENV CY_TOOLS_PATHS="/opt/ModusToolbox/tools_2.4"
ENV SILABS_BOARD=BRD4161A
ENV GSDK_ROOT=/opt/silabs/gecko_sdk/
ENV WISECONNECT_SDK_ROOT=/opt/silabs/wiseconnect-wifi-bt-sdk/
ENV WIFI_SDK_ROOT=/opt/silabs/wifi_sdk
ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
ENV IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.1-langdale
ENV JAVA_PATH=/usr/lib/jvm/java-8-openjdk-amd64
ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV NXP_K32W0_SDK_ROOT=/opt/k32w/core
ENV NXP_K32W1_SDK_ROOT=/opt/k32w/k32w1
ENV NXP_SDK_ROOT=/opt/nxp-sdk/rw61x
ENV OPENOCD_PATH=/opt/openocd/
ENV PW_ENVIRONMENT_ROOT=/home/vscode/pigweed/env
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV SYSROOT_AARCH64=/opt/ubuntu-22.04.1-aarch64-sysroot
ENV TELINK_ZEPHYR_BASE=/opt/telink/zephyrproject/zephyr
ENV TELINK_ZEPHYR_SDK_DIR=/opt/telink/zephyr-sdk-0.16.1
ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.16.2
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/NordicSemiconductor/nRF5_tools/zephyr-sdk-0.16.5
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb

ENV TIZEN_VERSION 7.0
ENV TIZEN_SDK_ROOT /opt/tizen-sdk
ENV TIZEN_SDK_TOOLCHAIN $TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-9.2
ENV TIZEN_SDK_SYSROOT $TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core

ENV FVP_CORSTONE_300_PATH=/opt/FVP_Corstone_SSE-300
ENV BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk
ENV ASR_TOOLCHAIN_PATH=/opt/asr/Tools/toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin/
